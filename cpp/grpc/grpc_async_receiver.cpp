#include "grpc_async_receiver.h"
#include "async_core/events_format.h" // IWYU pragma: keep
#include "grpc_async_vfo.h"
#include "utility/assert.h"

#define INVOKE(callback, ...)                                                  \
    if (callback) {                                                            \
        callback(__VA_ARGS__);                                                 \
    }

namespace violetrx
{

GrpcAsyncReceiver::sptr GrpcAsyncReceiver::make(std::string uri)
{
    return std::make_shared<GrpcAsyncReceiver>(std::move(uri));
}

GrpcAsyncReceiver::GrpcAsyncReceiver(std::string uri) :
    worker_thread_{std::make_shared<WorkerThread>()},
    client_{std::make_shared<GrpcClient>(uri, worker_thread_)},
    is_subscribed_{false},
    is_running_{false},
    input_device_{},
    current_antenna_{},
    antennas_{},
    input_rate_{0},
    input_decim_{0},
    dc_cancel_{false},
    iq_balance_{false},
    iq_swap_{false},
    rf_freq_{false},
    gain_stages_{},
    auto_gain_{false},
    freq_corr_{0},
    iq_fft_size_{0},
    iq_fft_window_{0},
    iq_recording_path_{},
    is_iq_recording_{false},
    vfos_{}
{
    worker_thread_->start();
}

template <typename Function>
auto GrpcAsyncReceiver::schedule(Function&& func,
                                 const std::source_location loc)
{
    return worker_thread_->scheduleForced(loc.function_name(),
                                          std::forward<Function>(func));
}

void GrpcAsyncReceiver::subscribe(ReceiverEventHandler handler,
                                  Callback<Connection> callback)
{
    schedule([this, handler = std::move(handler),
              callback = std::move(callback)]() mutable {
        Connection connection =
            handler ? signalStateChanged.connect(handler) : Connection{};
        INVOKE(callback, violetrx::ErrorCode::OK, std::move(connection));

        if (is_subscribed_) {
            if (handler) {
                EventCommon ec{.id = -1, .timestamp = Timestamp::Now()};
                handler(SyncStart{ec});
                forEachStateEvent(handler);
                handler(SyncEnd{ec});
            }
        } else {
            // Assume that the subscription will succeed. If it didn't, we will
            // get an Unsubscribed event anyways from the GrpcClient, which will
            // set is_subscribed_ again to fals
            is_subscribed_ = true;
            client_->Subscribe([this](const Event& event) { onEvent(event); });
        }
    });
}

void GrpcAsyncReceiver::unsubscribe(const Connection& c) { c.disconnect(); }

void GrpcAsyncReceiver::onReceiverEvent(const ReceiverEvent& event)
{
    std::visit( //
        Visitor{
            [&](const SyncStart&) {
                // Do nothing.
            },
            [&](const SyncEnd&) {
                // Do nothing.
            },
            [&](const Unsubscribed&) {
                // Will not handlet this now!
            },
            [&](const Started&) { is_running_ = true; },
            [&](const Stopped&) { is_running_ = false; },
            [&](const InputDeviceChanged& ev) { input_device_ = ev.device; },
            [&](const AntennaChanged& ev) { current_antenna_ = ev.antenna; },
            [&](const AntennasChanged& ev) { antennas_ = ev.antennas; },
            [&](const InputRateChanged& ev) { input_rate_ = ev.input_rate; },
            [&](const RfFreqChanged& ev) { rf_freq_ = ev.freq; },
            [&](const InputDecimChanged& ev) { input_decim_ = ev.decim; },
            [&](const IqSwapChanged& ev) { iq_swap_ = ev.enabled; },
            [&](const IqBalanceChanged& ev) { iq_balance_ = ev.enabled; },
            [&](const DcCancelChanged& ev) { dc_cancel_ = ev.enabled; },
            [&](const GainStagesChanged& ev) { gain_stages_ = ev.stages; },
            [&](const AutoGainChanged& ev) { auto_gain_ = ev.enabled; },
            [&](const GainChanged& ev) {
                for (GainStage& gain_stage : gain_stages_) {
                    if (gain_stage.name == ev.name) {
                        gain_stage.value = ev.value;
                        break;
                    }
                }
            },
            [&](const FftWindowChanged& ev) { iq_fft_window_ = ev.window; },
            [&](const FftSizeChanged& ev) { iq_fft_size_ = ev.size; },
            [&](const FreqCorrChanged& ev) { freq_corr_ = ev.ppm; },
            [&](const IqRecordingStarted& ev) {
                is_iq_recording_ = true;
                iq_recording_path_ = ev.path;
            },
            [&](const IqRecordingStopped&) { is_iq_recording_ = false; },
            [&](const VfoAdded& ev) { addVfoIfDoesntExist(ev.handle); },
            [&](const VfoRemoved&) {
                // Will not handle this now!
            },
        },
        event);

    signalStateChanged(event);

    is_subscribed_ = !std::holds_alternative<Unsubscribed>(event);
    if (!is_subscribed_) {
        spdlog::debug("Unsubscribed!! Disconnecting everything!!");
        signalStateChanged.disconnect_all_slots();
        for (auto& vfo : vfos_) {
            vfo->signalStateChanged.disconnect_all_slots();
        }
        vfos_.clear();
    }

    // Remove the vfo after we've notified that it's going to die to give a
    // chance to handlers to still access it.
    if (std::holds_alternative<VfoRemoved>(event)) {
        const auto& ev = std::get<VfoRemoved>(event);
        auto vfo = removeVfoIfExists(ev.handle);

        VIOLET_ASSERT(vfo);
        vfo->prepareToDie(ev);
    }
}

void GrpcAsyncReceiver::onVfoEvent(const VfoEvent& event)
{
    uint64_t handle = std::visit(
        [](const auto& specific_event) { return specific_event.handle; },
        event);

    auto vfo = getVfoImpl(handle);
    VIOLET_ASSERT(vfo);

    vfo->onEvent(event);
}

void GrpcAsyncReceiver::onEvent(const Event& event)
{
    if (IsReceiverEvent(event)) {
        // Extra copy here.
        auto receiver_event = ToReceiverEvent(event);

        VIOLET_ASSERT(receiver_event.has_value());
        onReceiverEvent(std::move(receiver_event.value()));
    } else {
        VIOLET_ASSERT(IsVfoEvent(event));

        // Extra copy here.
        auto vfo_event = ToVfoEvent(event);
        VIOLET_ASSERT(vfo_event.has_value());
        onVfoEvent(std::move(vfo_event.value()));
    }
}

void GrpcAsyncReceiver::getDevices(Callback<std::vector<Device>> callback) const
{
    client_->GetDevices(std::move(callback));
}

void GrpcAsyncReceiver::start(Callback<> callback)
{
    // Note: We do not update the local state even the callback indicated
    // success. Instead we only update the state from the events stream.
    //
    // client_->Start(
    //     [this, callback = std::move(callback)](ErrorCode err) mutable {
    //         if (err == ErrorCode::OK) {
    //             is_running_ = true;
    //         }
    //         INVOKE(callback, err);
    //     });

    client_->Start(std::move(callback));
}
void GrpcAsyncReceiver::stop(Callback<> callback)
{
    client_->Stop(std::move(callback));
}
void GrpcAsyncReceiver::setInputDevice(std::string device, Callback<> callback)
{
    client_->SetInputDevice(device, std::move(callback));
}
void GrpcAsyncReceiver::setAntenna(std::string antenna, Callback<> callback)
{
    client_->SetAntenna(antenna, std::move(callback));
}
void GrpcAsyncReceiver::setInputRate(int input_rate, Callback<int> callback)
{
    client_->SetInputRate(input_rate, std::move(callback));
}
void GrpcAsyncReceiver::setInputDecim(int decim, Callback<int> callback)
{
    client_->SetInputDecim(decim, std::move(callback));
}
void GrpcAsyncReceiver::setIqSwap(bool enabled, Callback<> callback)
{
    client_->SetIqSwap(enabled, std::move(callback));
}
void GrpcAsyncReceiver::setDcCancel(bool enabled, Callback<> callback)
{
    client_->SetDcCancel(enabled, std::move(callback));
}
void GrpcAsyncReceiver::setIqBalance(bool enabled, Callback<> callback)
{
    client_->SetIqBalance(enabled, std::move(callback));
}
void GrpcAsyncReceiver::setRfFreq(int64_t freq, Callback<int64_t> callback)
{
    client_->SetRfFreq(freq, std::move(callback));
}
void GrpcAsyncReceiver::setAutoGain(bool enabled, Callback<> callback)
{
    client_->SetAutoGain(enabled, std::move(callback));
}
void GrpcAsyncReceiver::setGain(std::string name, double value,
                                Callback<double> callback)
{
    client_->SetGain(name, value, std::move(callback));
}
void GrpcAsyncReceiver::setFreqCorr(double ppm, Callback<double> callback)
{
    client_->SetFreqCorr(ppm, std::move(callback));
}

void GrpcAsyncReceiver::setIqFftSize(int size, Callback<> callback)
{
    client_->SetFftSize(size, std::move(callback));
}
void GrpcAsyncReceiver::setIqFftWindow(WindowType window_type, bool,
                                       Callback<> callback)
{
    client_->SetFftWindow(window_type, std::move(callback));
}
void GrpcAsyncReceiver::getIqFftData(
    float* data, int size,
    Callback<Timestamp, int64_t, int, float*, int> callback)
{
    client_->GetFftData(data, size, std::move(callback));
}
void GrpcAsyncReceiver::startIqRecording(std::string, Callback<> callback)
{
    // TODO
    callback(ErrorCode::UNIMPLEMENTED);
}
void GrpcAsyncReceiver::stopIqRecording(Callback<> callback)
{
    // TODO
    callback(ErrorCode::UNIMPLEMENTED);
}

AsyncVfoIfaceSptr GrpcAsyncReceiver::addVfoIfDoesntExist(uint64_t handle)
{
    for (auto& vfo : vfos_) {
        if (vfo->getId() == handle) {
            return vfo;
        }
    }

    auto vfo = GrpcAsyncVfo::make(handle, client_, worker_thread_);
    vfos_.push_back(vfo);

    return vfo;
}

void GrpcAsyncReceiver::addVfoChannel(Callback<AsyncVfoIfaceSptr> callback)
{
    client_->AddVfoChannel([this, callback = std::move(callback)](
                               ErrorCode err, uint64_t handle) mutable {
        if (err != ErrorCode::OK) {
            INVOKE(callback, err, nullptr);
            return;
        }

        if (is_subscribed_) {
            // FIXME: This is probably buggy. What if the server replies to us
            // here that the vfo was added successfuly, but before the the reply
            // arrives, someone else removes the vfo, and the server sends the
            // VfoAdded then the VfoRemoved? This will lead to an extra vfo that
            // doesn't exist.
            auto vfo = addVfoIfDoesntExist(handle);
            INVOKE(callback, err, vfo);
        } else {
            // While we're not subscribed, we don't allow the state to change.
            callback(err, GrpcAsyncVfo::make(handle, client_, worker_thread_));
        }
    });
}

std::shared_ptr<GrpcAsyncVfo>
GrpcAsyncReceiver::removeVfoIfExists(uint64_t handle)
{
    auto it = std::find_if(vfos_.begin(), vfos_.end(), [&](const auto& vfo) {
        return vfo->getId() == handle;
    });

    if (it == vfos_.end()) {
        return nullptr;
    }

    auto vfo = *it;
    vfos_.erase(it);

    return vfo;
}

void GrpcAsyncReceiver::removeVfoChannel(AsyncVfoIfaceSptr vfo,
                                         Callback<> callback)
{
    removeVfoChannel(vfo->getId(), std::move(callback));
}

void GrpcAsyncReceiver::removeVfoChannel(uint64_t handle, Callback<> callback)
{
    client_->RemoveVfoChannel(handle, std::move(callback));

    // client_->RemoveVfoChannel(
    //     handle, [this, handle, callback = std::move(callback)](ErrorCode err)
    //     mutable {
    //         callback(err);
    //         removeVfoIfExists(handle);
    //     });
}

bool GrpcAsyncReceiver::isRunning() const { return is_running_; }

std::string GrpcAsyncReceiver::getInputDevice() const { return input_device_; }

std::string GrpcAsyncReceiver::getAntenna() const { return current_antenna_; }

int GrpcAsyncReceiver::getInputRate() const { return input_rate_; }

int GrpcAsyncReceiver::getInputDecim() const { return input_decim_; }

bool GrpcAsyncReceiver::getDcCancel() const { return dc_cancel_; }

bool GrpcAsyncReceiver::getIqBalance() const { return iq_balance_; }

bool GrpcAsyncReceiver::getIqSwap() const { return iq_swap_; }

int64_t GrpcAsyncReceiver::getRfFreq() const { return rf_freq_; }

std::vector<GainStage> GrpcAsyncReceiver::getGainStages() const
{
    return gain_stages_;
}

std::vector<std::string> GrpcAsyncReceiver::getAntennas() const
{
    return antennas_;
}

bool GrpcAsyncReceiver::getAutoGain() const { return auto_gain_; }

double GrpcAsyncReceiver::getFreqCorr() const { return freq_corr_; }

int GrpcAsyncReceiver::getIqFftSize() const { return iq_fft_size_; }

WindowType GrpcAsyncReceiver::getIqFftWindow() const { return iq_fft_window_; }

std::vector<std::shared_ptr<AsyncVfoIface>> GrpcAsyncReceiver::getVfos() const
{
    std::vector<std::shared_ptr<AsyncVfoIface>> result;
    for (auto& vfo : vfos_)
        result.push_back(vfo);

    return result;
}

std::shared_ptr<GrpcAsyncVfo>
GrpcAsyncReceiver::getVfoImpl(uint64_t handle) const
{
    for (auto& vfo : vfos_) {
        if (vfo->getId() == handle)
            return vfo;
    }
    return {};
}

AsyncVfoIfaceSptr GrpcAsyncReceiver::getVfo(uint64_t handle) const
{
    return getVfoImpl(handle);
}

std::string GrpcAsyncReceiver::getIqRecordingPath() const
{
    return iq_recording_path_;
}

bool GrpcAsyncReceiver::isIqRecording() const { return is_iq_recording_; }

template <typename Lambda>
void GrpcAsyncReceiver::forEachStateEvent(Lambda&& lambda) const
{
    // FIXME: Maybe somehow keep track of when each state changed.
    EventCommon ec{.id = -1, .timestamp = Timestamp::Now()};

    lambda(InputDeviceChanged{ec, getInputDevice()});
    lambda(AntennasChanged{ec, getAntennas()});
    lambda(AntennaChanged{ec, getAntenna()});
    lambda(InputRateChanged{ec, getInputRate()});
    lambda(InputDecimChanged{ec, getInputDecim()});
    lambda(DcCancelChanged{ec, getDcCancel()});
    lambda(IqBalanceChanged{ec, getIqBalance()});
    lambda(IqSwapChanged{ec, getIqSwap()});
    lambda(RfFreqChanged{ec, getRfFreq()});
    lambda(GainStagesChanged{ec, getGainStages()});
    lambda(AutoGainChanged{ec, getAutoGain()});
    lambda(FreqCorrChanged{ec, getFreqCorr()});
    lambda(FftSizeChanged{ec, getIqFftSize()});
    lambda(FftWindowChanged{ec, getIqFftWindow()});

    if (isRunning()) {
        lambda(Started{ec});
    } else {
        lambda(Stopped{ec});
    }

    if (isIqRecording()) {
        lambda(IqRecordingStarted{ec, getIqRecordingPath()});
    }

    for (auto& vfo : vfos_) {
        lambda(VfoAdded{ec, vfo->getId()});
    }
}

std::vector<ReceiverEvent> GrpcAsyncReceiver::getStateAsEvents() const
{
    std::vector<ReceiverEvent> result;

    forEachStateEvent(
        [&](const ReceiverEvent& event) { result.push_back(event); });

    return result;
}

void GrpcAsyncReceiver::synchronize(Callback<> callback)
{
    schedule([callback = std::move(callback)]() mutable {
        if (callback) {
            callback(ErrorCode::OK);
        }
    });
}

GrpcAsyncReceiver::~GrpcAsyncReceiver()
{
    spdlog::debug("~GrpcAsyncReceiver");

    // If we didn't manually delete the worker thread, then after this
    // destructor is finished, the worker thread will be deleted, and while it
    // is being deleted it might be running a task having a reference of the
    // this object, which will be an error.
    if (worker_thread_->isJoinable()) {
        worker_thread_->stop();
        worker_thread_->join();
    }
}

} // namespace violetrx
