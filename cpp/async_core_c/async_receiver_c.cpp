#include "async_receiver_c.h"
#include "async_core/async_receiver.h"
#include "async_core/async_vfo_iface.h"
#include "async_core/error_codes.h"
#include "async_core/events.h"
#include "async_core/types.h"
#include "async_core_c/events_c.h"
#include "async_core_c/events_conversion.h"

#include <boost/signals2/connection.hpp>
#include <cstring>
#include <deque>
#include <memory>

// THIS IS VERY UNSAFE!
template <typename T>
void increment_shared_ptr(const std::shared_ptr<T>& ptr)
{
    alignas(std::shared_ptr<T>) char storage[sizeof(std::shared_ptr<T>)];
    std::shared_ptr<T>* copy = reinterpret_cast<std::shared_ptr<T>*>(storage);
    std::construct_at(copy);

    *copy = ptr;
}

// THIS IS VERY UNSAFE!
template <typename T>
void decrement_shared_ptr(const std::shared_ptr<T>& ptr)
{
    const void* src_storage = reinterpret_cast<const void*>(&ptr);
    alignas(std::shared_ptr<T>) char dst_storage[sizeof(std::shared_ptr<T>)];
    std::memcpy(dst_storage, src_storage, sizeof(std::shared_ptr<T>));

    // now call the destructor of the copy to decrement the reference counter
    std::shared_ptr<T>* copy =
        reinterpret_cast<std::shared_ptr<T>*>(dst_storage);
    copy->~shared_ptr();
}

VioletReceiver* violet_rx_init()
{
    auto rx = core::AsyncReceiver::make();

    // the other side has to decrement the shared ptr by calling
    // violet_rx_destroy
    increment_shared_ptr(rx);
    return rx.get();
}

VioletReceiver* violet_rx_new_ref(VioletReceiver* rx_erased)
{
    increment_shared_ptr(
        static_cast<core::AsyncReceiverIface*>(rx_erased)->shared_from_this());

    return rx_erased;
}

void violet_rx_destroy(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    decrement_shared_ptr(rx->shared_from_this());
}

void violet_vfo_destroy(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<core::AsyncVfoIface*>(vfo_erased);
    decrement_shared_ptr(vfo->shared_from_this());
}

void violet_rx_start(VioletReceiver* rx_erased, VioletVoidCallback callback,
                     void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->start([callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}

void violet_rx_stop(VioletReceiver* rx_erased, VioletVoidCallback callback,
                    void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->stop([callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}

void violet_rx_set_input_dev(VioletReceiver* rx_erased, const char* dev,
                             VioletVoidCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setInputDevice(dev, [callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}

void violet_rx_set_antenna(VioletReceiver* rx_erased, const char* antenna,
                           VioletVoidCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setAntenna(antenna, [callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}

void violet_rx_set_input_rate(VioletReceiver* rx_erased, int samplerate,
                              VioletIntCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setInputRate(samplerate,
                     [callback, userdata](core::ErrorCode code, int rate) {
                         callback(code, rate, userdata);
                     });
}

void violet_rx_set_input_decim(VioletReceiver* rx_erased, int decim,
                               VioletIntCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setInputDecim(decim,
                      [callback, userdata](core::ErrorCode code, int decim) {
                          callback(code, decim, userdata);
                      });
}

void violet_rx_set_rf_freq(VioletReceiver* rx_erased, int64_t freq,
                           VioletInt64Callback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setRfFreq(freq, [callback, userdata](core::ErrorCode code, int freq) {
        callback(code, freq, userdata);
    });
}

void violet_rx_set_iq_swap(VioletReceiver* rx_erased, bool enable,
                           VioletVoidCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setIqSwap(enable, [callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_rx_set_dc_cancel(VioletReceiver* rx_erased, bool enable,
                             VioletVoidCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setDcCancel(enable, [callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_rx_set_iq_balance(VioletReceiver* rx_erased, bool enable,
                              VioletVoidCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setIqBalance(enable, [callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_rx_set_auto_gain(VioletReceiver* rx_erased, bool enable,
                             VioletVoidCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setAutoGain(enable, [callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_rx_set_gain(VioletReceiver* rx_erased, const char* gain_name,
                        double value, VioletDoubleCallback callback,
                        void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setGain(gain_name, value,
                [callback, userdata](core::ErrorCode code, double gain) {
                    callback(code, gain, userdata);
                });
}
void violet_rx_set_freq_corr(VioletReceiver* rx_erased, double ppm,
                             VioletDoubleCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setFreqCorr(ppm,
                    [callback, userdata](core::ErrorCode code, double ppm) {
                        callback(code, ppm, userdata);
                    });
}
void violet_rx_set_fft_size(VioletReceiver* rx_erased, int fftsize,
                            VioletVoidCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setIqFftSize(fftsize, [callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_rx_set_fft_window(VioletReceiver* rx_erased,
                              VioletWindowType window, bool normalize_energy,
                              VioletVoidCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->setIqFftWindow((core::WindowType)window, normalize_energy,
                       [callback, userdata](core::ErrorCode code) {
                           callback(code, userdata);
                       });
}
void violet_rx_get_fft_data(VioletReceiver* rx_erased, float* data, int size,
                            VioletFftDataCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->getIqFftData(data, size,
                     [callback, userdata](
                         core::ErrorCode code, core::Timestamp timestamp,
                         int64_t freq, int samplerate, float* data, int size) {
                         callback(code,
                                  VioletTimestamp{
                                      timestamp.seconds,
                                      timestamp.nanos,
                                  },
                                  freq, samplerate, data, size, userdata);
                     });
}

void violet_rx_add_vfo(VioletReceiver* rx_erased, VioletVfoCallback callback,
                       void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->addVfoChannel([callback, userdata](core::ErrorCode code,
                                           core::AsyncVfoIfaceSptr vfo) {
        // the other side has to decrement the shared ptr by calling
        // violet_vfo_destroy
        increment_shared_ptr(vfo);
        callback(code, vfo.get(), userdata);
    });
}

void violet_rx_remove_vfo(VioletReceiver* rx_erased, VioletVfo* vfo_erased,
                          VioletVoidCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    auto vfo = static_cast<core::AsyncVfoIface*>(vfo_erased);

    rx->removeVfoChannel(vfo->shared_from_this(),
                         [callback, userdata](core::ErrorCode code) {
                             callback(code, userdata);
                         });
}

void voilet_rx_synchronize(VioletReceiver* rx_erased,
                           VioletSyncCallback callback, void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    rx->synchronize([callback, userdata](core::ErrorCode code) {
        callback(code, userdata);
    });
}
char* violet_rx_get_input_dev(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return strdup(rx->getInputDevice().c_str());
}
char* violet_rx_get_cur_antenna(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return strdup(rx->getAntenna().c_str());
}
int violet_rx_get_input_rate(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getInputRate();
}
int violet_rx_get_input_decim(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getInputDecim();
}
bool violet_rx_get_dc_cancel(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getDcCancel();
}
bool violet_rx_get_iq_balance(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getIqBalance();
}
bool violet_rx_get_iq_swap(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getIqSwap();
}
int64_t violet_rx_get_rf_freq(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getRfFreq();
}
int violet_rx_get_gain_stages_count(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    // FIXME: not very efficient
    return rx->getGainStages().size();
}
VioletGainStage violet_rx_get_gain_stage(VioletReceiver* rx_erased, int idx)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    auto stages = rx->getGainStages();
    if (idx < std::ssize(stages)) {
        core::GainStage stage = stages[idx];

        return VioletGainStage{.name = strdup(stage.name.c_str()),
                               .start = stage.start,
                               .stop = stage.stop,
                               .step = stage.step,
                               .value = stage.value};
    } else {
        return VioletGainStage{
            .name = nullptr, .start = 0, .stop = 0, .step = 0, .value = 0};
    }
}
int violet_rx_get_antennas_count(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getAntennas().size();
}
char* violet_rx_get_antenna(VioletReceiver* rx_erased, int idx)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    auto antennas = rx->getAntennas();
    if (idx < std::ssize(antennas))
        return strdup(antennas[idx].c_str());
    else
        return nullptr;
}
bool violet_rx_get_auto_gain(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getAutoGain();
}
double violet_rx_get_freq_corr(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getFreqCorr();
}
int violet_rx_get_fft_size(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getIqFftSize();
}
VioletWindowType violet_rx_get_fft_window(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return (VioletWindowType)rx->getIqFftWindow();
}
int violet_rx_get_vfos_count(VioletReceiver* rx_erased)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    return rx->getVfos().size();
}
VioletVfo* violet_rx_get_vfo(VioletReceiver* rx_erased, int idx)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);
    auto vfos = rx->getVfos();
    if (idx < std::ssize(vfos)) {
        auto vfo = vfos[idx];
        // the other side has to decrement the shared ptr by calling
        // violet_vfo_destroy
        increment_shared_ptr(vfo);
        return vfo.get();
    } else {
        return nullptr;
    }
}

class Connection : public boost::signals2::connection
{
public:
    Connection() noexcept {}
    Connection(const connection& other) : connection(other) {}
    Connection(connection&& other) : connection(std::move(other)) {}
    Connection(
        const boost::weak_ptr<boost::signals2::detail::connection_body_base>&
            connectionBody) noexcept :
        connection(connectionBody)
    {
    }

    boost::signals2::detail::connection_body_base* ptr()
    {
        return _weak_connection_body.lock().get();
    }
};

// FIXME: this should be the behaviour of subscribe
VioletConnection* violet_rx_subscribe(VioletReceiver* rx_erased,
                                      VioletEventCallback callback,
                                      void* userdata)
{
    auto rx = static_cast<core::AsyncReceiverIface*>(rx_erased);

    Connection connection = rx->subscribe([=](const core::ReceiverEvent& ev) {
        core::CEvent c_event{ev};
        callback(c_event.inner(), userdata);
    });

    return connection.ptr();
}
void violet_unsubscribe(VioletConnection* connection_erased)
{
    auto connection =
        (boost::signals2::detail::connection_body_base*)connection_erased;

    connection->disconnect();
}
