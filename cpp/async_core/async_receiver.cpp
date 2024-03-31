#include "async_receiver.h"
#include "async_core/events.h"
#include "async_vfo.h"
#include "core/vfo_channel.h"
#include "error_codes.h"
#include "utility/worker_thread.h"

#include <algorithm>
#include <memory>
#include <spdlog/spdlog.h>
#include <utility>
#include <variant>

namespace violetrx
{

int64_t EventCommon::last_id = 0;

#define INVOKE(callback, ...)                                                  \
    if (callback) {                                                            \
        callback(__VA_ARGS__);                                                 \
    }

#define CALLBACK_ON_ERROR(code)                                                \
    if (callback) {                                                            \
        invokeDefault(callback, code);                                         \
    }

#define CALLBACK_ON_SUCCESS(...)                                               \
    if (callback) {                                                            \
        callback(ErrorCode::OK __VA_OPT__(, ) __VA_ARGS__);                    \
    }

#define RETURN_IF_WORKER_BUSY()                                                \
    do {                                                                       \
        if (workerThread->isPaused()) {                                        \
            CALLBACK_ON_ERROR(WORKER_BUSY);                                    \
            return;                                                            \
        }                                                                      \
    } while (0)

template <typename... Args>
void invokeDefault(Callback<Args...>& callback, ErrorCode code)
{
    callback(code, std::decay_t<Args>{}...);
}

template <typename Event, typename... Args>
void AsyncReceiver::stateChanged(Args... args)
{
    signalStateChanged(
        createEvent<Event>(EventCommon::make(), std::move(args)...));
}

template <>
SyncStart AsyncReceiver::createEvent<SyncStart>(EventCommon ec) const
{
    return SyncStart{ec};
}
template <>
SyncEnd AsyncReceiver::createEvent<SyncEnd>(EventCommon ec) const
{
    return SyncEnd{ec};
}
template <>
Started AsyncReceiver::createEvent<Started>(EventCommon ec) const
{
    return Started{ec};
}
template <>
Stopped AsyncReceiver::createEvent<Stopped>(EventCommon ec) const
{
    return Stopped{ec};
}
template <>
InputDeviceChanged
AsyncReceiver::createEvent<InputDeviceChanged>(EventCommon ec) const
{
    return InputDeviceChanged{ec, getInputDevice()};
}
template <>
GainStagesChanged
AsyncReceiver::createEvent<GainStagesChanged>(EventCommon ec) const
{
    return GainStagesChanged{ec, getGainStages()};
}
template <>
AntennasChanged
AsyncReceiver::createEvent<AntennasChanged>(EventCommon ec) const
{
    return AntennasChanged{ec, getAntennas()};
}
template <>
AntennaChanged AsyncReceiver::createEvent<AntennaChanged>(EventCommon ec) const
{
    return AntennaChanged{ec, getAntenna()};
}
template <>
RfFreqChanged AsyncReceiver::createEvent<RfFreqChanged>(EventCommon ec) const
{
    return RfFreqChanged{ec, getRfFreq()};
}
template <>
InputRateChanged
AsyncReceiver::createEvent<InputRateChanged>(EventCommon ec) const
{
    return InputRateChanged{ec, getInputRate()};
}
template <>
InputDecimChanged
AsyncReceiver::createEvent<InputDecimChanged>(EventCommon ec) const
{
    return InputDecimChanged{ec, getInputDecim()};
}
template <>
IqSwapChanged AsyncReceiver::createEvent<IqSwapChanged>(EventCommon ec) const
{
    return IqSwapChanged{ec, getIqSwap()};
}
template <>
DcCancelChanged
AsyncReceiver::createEvent<DcCancelChanged>(EventCommon ec) const
{
    return DcCancelChanged{ec, getDcCancel()};
}
template <>
IqBalanceChanged
AsyncReceiver::createEvent<IqBalanceChanged>(EventCommon ec) const
{
    return IqBalanceChanged{ec, getIqBalance()};
}
template <>
AutoGainChanged
AsyncReceiver::createEvent<AutoGainChanged>(EventCommon ec) const
{
    return AutoGainChanged{ec, getAutoGain()};
}
template <>
FreqCorrChanged
AsyncReceiver::createEvent<FreqCorrChanged>(EventCommon ec) const
{
    return FreqCorrChanged{ec, getFreqCorr()};
}
template <>
VfoAdded AsyncReceiver::createEvent<VfoAdded>(EventCommon ec,
                                              uint64_t handle) const
{
    return VfoAdded{ec, handle};
}
template <>
VfoRemoved AsyncReceiver::createEvent<VfoRemoved>(EventCommon ec,
                                                  uint64_t handle) const
{
    return VfoRemoved{ec, handle};
}
template <>
GainChanged AsyncReceiver::createEvent<GainChanged>(EventCommon ec,
                                                    std::string name,
                                                    double value) const
{
    return GainChanged{ec, std::move(name), value};
}
template <>
IqRecordingStarted
AsyncReceiver::createEvent<IqRecordingStarted>(EventCommon ec) const
{
    return IqRecordingStarted{ec, getIqRecordingPath()};
}
template <>
IqRecordingStopped
AsyncReceiver::createEvent<IqRecordingStopped>(EventCommon ec) const
{
    return IqRecordingStopped{ec};
}
template <>
FftWindowChanged
AsyncReceiver::createEvent<FftWindowChanged>(EventCommon ec) const
{
    return FftWindowChanged{ec, getIqFftWindow()};
}
template <>
FftSizeChanged AsyncReceiver::createEvent<FftSizeChanged>(EventCommon ec) const
{
    return FftSizeChanged{ec, getIqFftSize()};
}
AsyncReceiver::sptr AsyncReceiver::make()
{
    return std::make_shared<AsyncReceiver>();
}

AsyncReceiver::AsyncReceiver()
{
    rx = receiver::make();
    workerThread = std::make_shared<WorkerThread>();
    workerThread->start();
}

AsyncReceiver::~AsyncReceiver()
{
    spdlog::debug("AsyncReceiver::~AsyncReceiver");
}

template <typename Function>
auto AsyncReceiver::schedule(Function&& func, const std::source_location loc)
{
    return workerThread->scheduleForced(loc.function_name(),
                                        std::forward<Function>(func));
}

void AsyncReceiver::start(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        if (!rx->is_running()) {
            rx->start();
            CALLBACK_ON_SUCCESS();
            stateChanged<Started>();
        } else {
            CALLBACK_ON_SUCCESS();
        }
    });
}

void AsyncReceiver::stop(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        if (rx->is_running()) {
            rx->stop();
            CALLBACK_ON_SUCCESS();
            stateChanged<Stopped>();
        } else {
            CALLBACK_ON_SUCCESS();
        }
    });
}

void AsyncReceiver::setInputDevice(std::string dev, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule(
        [this, dev = std::move(dev), callback = std::move(callback)]() mutable {
            try {
                rx->set_input_device(dev);
            } catch (const std::exception& e) {
                CALLBACK_ON_ERROR(INVALID_INPUT_DEVICE);
                return;
            }
            CALLBACK_ON_SUCCESS();

            stateChanged<InputDeviceChanged>();
            stateChanged<GainStagesChanged>();
            stateChanged<AntennasChanged>();
            stateChanged<AntennaChanged>();
            stateChanged<RfFreqChanged>();
            stateChanged<InputRateChanged>();
        });
}

void AsyncReceiver::setAntenna(std::string antenna, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, antenna = std::move(antenna),
              callback = std::move(callback)]() mutable {
        std::string old_antenna = rx->get_antenna();

        if (antenna == old_antenna) {
            CALLBACK_ON_SUCCESS();
            return;
        }

        rx->set_antenna(antenna);
        CALLBACK_ON_SUCCESS();

        std::string new_antenna = rx->get_antenna();

        if (old_antenna != new_antenna) {
            stateChanged<AntennaChanged>();
        }
    });
}

void AsyncReceiver::setInputRate(int rate, Callback<int> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, rate, callback = std::move(callback)]() mutable {
        int old_rate = (int)rx->get_input_rate();
        if (old_rate == rate) {
            CALLBACK_ON_SUCCESS(rate);
            return;
        }

        int new_rate = (int)rx->set_input_rate((double)rate);
        CALLBACK_ON_SUCCESS(new_rate);

        if (new_rate != old_rate) {
            stateChanged<InputRateChanged>();
        }
    });
}
void AsyncReceiver::setInputDecim(int decim, Callback<int> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, decim, callback = std::move(callback)]() mutable {
        int old_decim = (int)rx->get_input_decim();
        if (old_decim == decim) {
            CALLBACK_ON_SUCCESS(decim);
            return;
        }

        int new_decim = (int)rx->set_input_decim(decim);
        CALLBACK_ON_SUCCESS(new_decim);

        if (new_decim != old_decim) {
            stateChanged<InputDecimChanged>();
        }
    });
}

void AsyncReceiver::setIqSwap(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, enable, callback = std::move(callback)]() mutable {
        if (enable == rx->get_iq_swap()) {
            CALLBACK_ON_SUCCESS();
            return;
        }

        rx->set_iq_swap(enable);
        CALLBACK_ON_SUCCESS();

        stateChanged<IqSwapChanged>();
    });
}
void AsyncReceiver::setDcCancel(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, enable, callback = std::move(callback)]() mutable {
        if (enable == rx->get_dc_cancel()) {
            CALLBACK_ON_SUCCESS();
            return;
        }

        rx->set_dc_cancel(enable);
        CALLBACK_ON_SUCCESS();

        stateChanged<DcCancelChanged>();
    });
}
void AsyncReceiver::setIqBalance(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, enable, callback = std::move(callback)]() mutable {
        if (enable == rx->get_iq_balance()) {
            CALLBACK_ON_SUCCESS();
            return;
        }

        rx->set_iq_balance(enable);
        CALLBACK_ON_SUCCESS();

        stateChanged<IqBalanceChanged>();
    });
}
void AsyncReceiver::setAutoGain(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, enable, callback = std::move(callback)]() mutable {
        bool old_auto_gain = rx->get_auto_gain();

        if (enable == old_auto_gain) {
            CALLBACK_ON_SUCCESS();
            return;
        }

        bool new_auto_gain = rx->set_auto_gain(enable);
        CALLBACK_ON_SUCCESS();

        if (old_auto_gain != new_auto_gain) {
            stateChanged<AutoGainChanged>();
        }
    });
}

void AsyncReceiver::setRfFreq(int64_t freq, Callback<int64_t> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, freq, callback = std::move(callback)]() mutable {
        int64_t old_freq = (int64_t)rx->get_rf_freq();
        if (old_freq == freq) {
            CALLBACK_ON_SUCCESS(freq);
            return;
        }

        int64_t new_freq = rx->set_rf_freq(freq);
        CALLBACK_ON_SUCCESS(new_freq);

        stateChanged<RfFreqChanged>();
    });
}

void AsyncReceiver::setGain(std::string gain, double val,
                            Callback<double> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, gain = std::move(gain), val,
              callback = std::move(callback)]() mutable {
        // should we cache this vector?
        std::vector<std::string> gains = rx->get_gain_names();

        auto it = std::find(gains.begin(), gains.end(), gain);
        if (it == gains.end()) {
            CALLBACK_ON_ERROR(GAIN_NOT_FOUND);
            return;
        }

        double new_val = rx->set_gain(gain, val);
        CALLBACK_ON_SUCCESS(new_val);

        stateChanged<GainChanged>(gain, new_val);
    });
}

void AsyncReceiver::setFreqCorr(double corr, Callback<double> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, corr, callback = std::move(callback)]() mutable {
        double new_ppm = rx->set_freq_corr(std::clamp(corr, -200.0, 200.0));
        CALLBACK_ON_SUCCESS(new_ppm);

        stateChanged<FreqCorrChanged>();
    });
}

void AsyncReceiver::setIqFftSize(int fftsize, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, fftsize, callback = std::move(callback)]() mutable {
        int oldsize = rx->iq_fft_size();

        if (oldsize == fftsize) {
            CALLBACK_ON_SUCCESS();
            return;
        }

        rx->set_iq_fft_size(fftsize);
        CALLBACK_ON_SUCCESS();

        stateChanged<FftSizeChanged>();
    });
}

void AsyncReceiver::setIqFftWindow(WindowType window, bool normalize,
                                   Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule(
        [this, window, normalize, callback = std::move(callback)]() mutable {
            rx->set_iq_fft_window((int)window, normalize);
            CALLBACK_ON_SUCCESS();

            stateChanged<FftWindowChanged>();
        });
}

void AsyncReceiver::getIqFftData(
    float* data, int size,
    Callback<Timestamp, int64_t, int, float*, int> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, data, size, callback = std::move(callback)]() mutable {
        using namespace std::chrono;

        int fftsize = rx->iq_fft_size();

        if (data == nullptr) {
            data = new float[rx->iq_fft_size()];
        } else if (size < fftsize) {
            CALLBACK_ON_ERROR(ErrorCode::INSUFFICIENT_BUFFER_SIZE);
        }

        rx->get_iq_fft_data(data);

        int64_t center_freq = rx->get_rf_freq();
        int sample_rate = (int)rx->get_quad_rate();

        Timestamp timestamp = Timestamp::Now();

        CALLBACK_ON_SUCCESS(timestamp, center_freq, sample_rate, data, fftsize);
    });
}

void AsyncReceiver::startIqRecording(std::string filepath, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, filepath = std::move(filepath),
              callback = std::move(callback)]() mutable {
        if (rx->is_iq_recording()) {
            CALLBACK_ON_ERROR(ALREADY_RECORDING);
            return;
        }
        if (rx->start_iq_recording(filepath)) {
            CALLBACK_ON_SUCCESS();
            stateChanged<IqRecordingStarted>();
        } else {
            CALLBACK_ON_ERROR(UNKNOWN_ERROR);
        }
    });
}

void AsyncReceiver::stopIqRecording(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        if (!rx->is_iq_recording()) {
            CALLBACK_ON_ERROR(ALREADY_NOT_RECORDING);
            return;
        }

        if (rx->stop_iq_recording()) {
            CALLBACK_ON_SUCCESS();
            stateChanged<IqRecordingStopped>();
        } else {
            CALLBACK_ON_ERROR(UNKNOWN_ERROR);
        }
    });
}

void AsyncReceiver::addVfoChannel(Callback<AsyncVfoIfaceSptr> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        vfo_channel::sptr vfo = rx->add_vfo_channel();
        auto asyncVfo = AsyncVfo::make(vfo, workerThread);
        vfos.push_back(asyncVfo);

        CALLBACK_ON_SUCCESS(asyncVfo);

        stateChanged<VfoAdded>(asyncVfo->getId());
    });
}

void AsyncReceiver::removeVfoChannelImpl(AsyncVfo::sptr vfo,
                                         Callback<> callback)
{
    auto it = std::find(vfos.begin(), vfos.end(), vfo);
    if (it == vfos.end()) {
        CALLBACK_ON_ERROR(VFO_NOT_FOUND);
        return;
    }

    rx->remove_vfo_channel(vfo->inner());

    auto vfo_removed_event =
        createEvent<VfoRemoved>(EventCommon::make(), vfo->getId());

    CALLBACK_ON_SUCCESS();

    vfos.erase(it);

    // Emit the same event with the same ID from the receiver and the vfo
    // itself.
    vfo->prepareToDie(vfo_removed_event);
    signalStateChanged(vfo_removed_event);
}

void AsyncReceiver::removeVfoChannel(AsyncVfoIfaceSptr vfoIface,
                                     Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, vfoIface, callback = std::move(callback)]() mutable {
        auto vfo = std::dynamic_pointer_cast<AsyncVfo>(vfoIface);

        if (!vfo) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        removeVfoChannelImpl(vfo, std::move(callback));
    });
}

void AsyncReceiver::removeVfoChannel(uint64_t handle, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, handle, callback = std::move(callback)]() mutable {
        auto vfo = std::dynamic_pointer_cast<AsyncVfo>(getVfo(handle));

        if (!vfo) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        removeVfoChannelImpl(vfo, std::move(callback));
    });
}

std::vector<ReceiverEvent> AsyncReceiver::getStateAsEvents() const
{
    std::vector<ReceiverEvent> result;

    forEachStateEvent(
        [&](const ReceiverEvent& event) { result.push_back(event); });

    return result;
}

template <typename Lambda>
void AsyncReceiver::forEachStateEvent(Lambda&& lambda) const
{
    // FIXME: Maybe somehow keep track of when each state changed.
    EventCommon ec{.id = -1, .timestamp = {}};

    lambda(createEvent<InputDeviceChanged>(ec));
    lambda(createEvent<AntennasChanged>(ec));
    lambda(createEvent<AntennaChanged>(ec));
    lambda(createEvent<InputRateChanged>(ec));
    lambda(createEvent<InputDecimChanged>(ec));
    lambda(createEvent<DcCancelChanged>(ec));
    lambda(createEvent<IqBalanceChanged>(ec));
    lambda(createEvent<IqSwapChanged>(ec));
    lambda(createEvent<RfFreqChanged>(ec));
    lambda(createEvent<GainStagesChanged>(ec));
    lambda(createEvent<AutoGainChanged>(ec));
    lambda(createEvent<FreqCorrChanged>(ec));
    lambda(createEvent<FftSizeChanged>(ec));
    lambda(createEvent<FftWindowChanged>(ec));

    if (isRunning()) {
        lambda(createEvent<Started>(ec));
    } else {
        lambda(createEvent<Stopped>(ec));
    }

    if (isIqRecording()) {
        lambda(createEvent<IqRecordingStarted>(ec));
    }

    for (auto& vfo : vfos) {
        lambda(createEvent<VfoAdded>(ec, vfo->getId()));
    }
}

void AsyncReceiver::subscribe(ReceiverEventHandler handler,
                              Callback<Connection> callback)
{
    schedule([this, handler = std::move(handler),
              callback = std::move(callback)]() mutable {
        Connection connection =
            handler ? signalStateChanged.connect(handler) : Connection{};
        INVOKE(callback, violetrx::ErrorCode::OK, std::move(connection));

        if (handler) {
            EventCommon ec{.id = -1, .timestamp = {}};

            handler(createEvent<SyncStart>(ec));
            forEachStateEvent(handler);
            handler(createEvent<SyncEnd>(ec));
        }
    });
}

void AsyncReceiver::unsubscribe(const Connection& c) { c.disconnect(); }

void AsyncReceiver::synchronize(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule(
        [callback = std::move(callback)]() mutable { CALLBACK_ON_SUCCESS(); });
}

bool AsyncReceiver::isRunning() const { return rx->is_running(); }

bool AsyncReceiver::isIqRecording() const { return rx->is_iq_recording(); }

std::string AsyncReceiver::getInputDevice() const
{
    return rx->get_input_device();
}

std::string AsyncReceiver::getAntenna() const { return rx->get_antenna(); }

int AsyncReceiver::getInputRate() const { return rx->get_input_rate(); }

int AsyncReceiver::getInputDecim() const { return rx->get_input_decim(); }

bool AsyncReceiver::getDcCancel() const { return rx->get_dc_cancel(); }

bool AsyncReceiver::getIqBalance() const { return rx->get_iq_balance(); }
bool AsyncReceiver::getIqSwap() const { return rx->get_iq_swap(); }
int64_t AsyncReceiver::getRfFreq() const { return rx->get_rf_freq(); }

std::vector<GainStage> AsyncReceiver::getGainStages() const
{
    std::vector<GainStage> gainStages;
    for (const std::string& name : rx->get_gain_names()) {
        double value = rx->get_gain(name);
        double start, stop, step;
        rx->get_gain_range(name, &start, &stop, &step);
        gainStages.push_back(GainStage{name, start, stop, step, value});
    }

    return gainStages;
}

std::vector<std::string> AsyncReceiver::getAntennas() const
{
    return rx->get_antennas();
}
bool AsyncReceiver::getAutoGain() const { return rx->get_auto_gain(); }
double AsyncReceiver::getFreqCorr() const { return rx->get_freq_corr(); }
int AsyncReceiver::getIqFftSize() const { return rx->iq_fft_size(); }
WindowType AsyncReceiver::getIqFftWindow() const
{
    return (WindowType)rx->get_iq_fft_window();
}
std::vector<std::shared_ptr<AsyncVfoIface>> AsyncReceiver::getVfos() const
{
    std::vector<std::shared_ptr<AsyncVfoIface>> result;
    for (auto& vfo : vfos)
        result.push_back(vfo);

    return result;
}

AsyncVfoIfaceSptr AsyncReceiver::getVfo(uint64_t handle) const
{
    for (auto& vfo : vfos) {
        if (vfo->getId() == handle)
            return vfo;
    }

    return {};
}

std::string AsyncReceiver::getIqRecordingPath() const
{
    return rx->get_iq_filename();
}

} // namespace violetrx
