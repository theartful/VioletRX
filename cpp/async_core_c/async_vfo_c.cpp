#include "async_vfo_c.h"
#include "async_core/async_vfo_iface.h"
#include "async_core/error_codes.h"
#include "async_core/types.h"
#include "async_core_c/events_c.h"
#include "async_core_c/events_conversion.h"

/* filter */
void violet_vfo_set_filter_offset(VioletVfo* vfo_erased, int64_t offset,
                                  VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setFilterOffset(offset,
                         [callback, userdata](violetrx::ErrorCode code) {
                             callback(code, userdata);
                         });
}
void violet_vfo_set_filter(VioletVfo* vfo_erased, int64_t low, int64_t high,
                           VioletFilterShape filter_shape,
                           VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setFilter(low, high, (violetrx::FilterShape)filter_shape,
                   [callback, userdata](violetrx::ErrorCode code) {
                       callback(code, userdata);
                   });
}
void violet_vfo_set_cw_offset(VioletVfo* vfo_erased, int64_t offset,
                              VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setCwOffset(offset, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_set_demod(VioletVfo* vfo_erased, VioletDemod demod,
                          VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setDemod((violetrx::Demod)demod,
                  [callback, userdata](violetrx::ErrorCode code) {
                      callback(code, userdata);
                  });
}
void violet_vfo_get_signal_pwr(VioletVfo* vfo_erased,
                               VioletFloatCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->getSignalPwr(
        [callback, userdata](violetrx::ErrorCode code, float power) {
            callback(code, power, userdata);
        });
}

/* Noise blanker */
void violet_vfo_set_noise_blanker(VioletVfo* vfo_erased, int id, bool on,
                                  VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setNoiseBlanker(id, on,
                         [callback, userdata](violetrx::ErrorCode code) {
                             callback(code, userdata);
                         });
}
void violet_vfo_set_noise_threshold(VioletVfo* vfo_erased, int id,
                                    float threshold,
                                    VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setNoiseBlankerThreshold(
        id, threshold, [callback, userdata](violetrx::ErrorCode code) {
            callback(code, userdata);
        });
}

/* Sql parameter */
void violet_vfo_set_sql_level(VioletVfo* vfo_erased, double level_db,
                              VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setSqlLevel(level_db, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_set_sql_alpha(VioletVfo* vfo_erased, double alpha,
                              VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setSqlAlpha(alpha, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}

/* AGC */
void violet_vfo_set_agc_on(VioletVfo* vfo_erased, bool on,
                           VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAgcOn(on, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_set_agc_hang(VioletVfo* vfo_erased, bool on,
                             VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAgcHang(on, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_set_agc_threshold(VioletVfo* vfo_erased, int threshold,
                                  VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAgcThreshold(threshold,
                         [callback, userdata](violetrx::ErrorCode code) {
                             callback(code, userdata);
                         });
}
void violet_vfo_set_agc_slope(VioletVfo* vfo_erased, int slope,
                              VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAgcSlope(slope, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_set_agc_decay(VioletVfo* vfo_erased, int decay,
                              VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAgcDecay(decay, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_set_agc_manual_gain(VioletVfo* vfo_erased, int gain,
                                    VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAgcManualGain(gain, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}

/* FM parameters */
void violet_vfo_set_fm_maxdev(VioletVfo* vfo_erased, float maxdev,
                              VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setFmMaxDev(maxdev, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_set_fm_deemph(VioletVfo* vfo_erased, double tau,
                              VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setFmDeemph(tau, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}

/* AM parameters */
void violet_vfo_set_am_dcr(VioletVfo* vfo_erased, bool enable,
                           VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAmDcr(enable, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}

/* AM-Sync parameters */
void violet_vfo_set_am_sync_dcr(VioletVfo* vfo_erased, bool enable,
                                VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAmSyncDcr(enable, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_set_am_sync_pll_bw(VioletVfo* vfo_erased, float bw,
                                   VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAmSyncPllBw(bw, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}

/* I/Q FFT */
// TODO

/* Audio Recording */
void violet_vfo_start_audio_recording(VioletVfo* vfo_erased,
                                      const char* filename,
                                      VioletVoidCallback callback,
                                      void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->startAudioRecording(filename,
                             [callback, userdata](violetrx::ErrorCode code) {
                                 callback(code, userdata);
                             });
}
void violet_vfo_stop_audio_recording(VioletVfo* vfo_erased,
                                     VioletVoidCallback callback,
                                     void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->stopAudioRecording([callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_set_audio_gain(VioletVfo* vfo_erased, float gain,
                               VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->setAudioGain(gain, [callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}

/* sample sniffer */
void violet_vfo_start_sniffer(VioletVfo* vfo_erased, int samplerate,
                              int buffsize, VioletVoidCallback callback,
                              void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->startSniffer(samplerate, buffsize,
                      [callback, userdata](violetrx::ErrorCode code) {
                          callback(code, userdata);
                      });
}
void violet_vfo_stop_sniffer(VioletVfo* vfo_erased, VioletVoidCallback callback,
                             void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->stopSniffer([callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_get_sniffer_data(VioletVfo* vfo_erased, float* data, int size,
                                 VioletSnifferDataCallback callback,
                                 void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->getSnifferData(
        data, size,
        [callback, userdata](violetrx::ErrorCode code, float* data, int size) {
            callback(code, data, size, userdata);
        });
}

/* rds functions */
void violet_vfo_get_rds_data(VioletVfo* vfo_erased,
                             VioletRdsDataCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->getRdsData([callback, userdata](violetrx::ErrorCode code,
                                         std::string data, int type) {
        callback(code, data.c_str(), type, userdata);
    });
}
void violet_vfo_start_rds_decoder(VioletVfo* vfo_erased,
                                  VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->startRdsDecoder([callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_stop_rds_decoder(VioletVfo* vfo_erased,
                                 VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->stopRdsDecoder([callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
void violet_vfo_reset_rds_parser(VioletVfo* vfo_erased,
                                 VioletVoidCallback callback, void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->resetRdsParser([callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}

uint64_t violet_vfo_get_id(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    return vfo->getId();
}
VioletFilterRange violet_vfo_get_filter_range(VioletVfo* vfo_erased,
                                              VioletDemod demod)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    auto range = vfo->getFilterRange((violetrx::Demod)demod);
    return VioletFilterRange{.lowMin = range.lowMin,
                             .lowMax = range.lowMax,
                             .highMin = range.highMin,
                             .highMax = range.highMax,
                             .symmetric = range.symmetric};
}

/* Sync API: getters can only be called inside a successful callback function */
void violet_vfo_synchronize(VioletVfo* vfo_erased, VioletSyncCallback callback,
                            void* userdata)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->synchronize([callback, userdata](violetrx::ErrorCode code) {
        callback(code, userdata);
    });
}
VioletDemod violet_vfo_get_demod(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return (VioletDemod)vfo->getDemod();
}
int64_t violet_vfo_get_offset(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getOffset();
}
VioletFilter violet_vfo_get_filter(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    auto filter = vfo->getFilter();
    return VioletFilter{
        .shape = (VioletFilterShape)filter.shape,
        .low = filter.low,
        .high = filter.high,
    };
}
int64_t violet_vfo_get_cw_offset(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getCwOffset();
}
float violet_vfo_get_fm_maxdev(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getFmMaxDev();
}
double violet_vfo_get_fm_deemph(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getFmDeemph();
}
bool violet_vfo_get_am_dcr(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getAmDcr();
}
bool violet_vfo_get_am_sync_dcr(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getAmSyncDcr();
}
float violet_vfo_get_am_sync_pll_bw(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getAmSyncPllBw();
}
bool violet_vfo_is_audio_recording(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->isAudioRecording();
}
char* violet_vfo_recording_filename(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return strdup(vfo->getRecordingFilename().c_str());
}
bool violet_vfo_is_udp_streaming(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->isUdpStreaming();
}
bool violet_vfo_is_sniffing(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->isSniffing();
}
bool violet_vfo_is_rds_decoder_active(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->isRdsDecoderActive();
}
bool violet_vfo_is_agc_on(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->isAgcOn();
}
bool violet_vfo_is_agc_hang_on(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->isAgcHangOn();
}
int violet_vfo_get_agc_threshold(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getAgcThreshold();
}
int violet_vfo_get_agc_slope(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getAgcSlope();
}
int violet_vfo_get_agc_decay(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getAgcDecay();
}
int violet_vfo_get_agc_manual_gain(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getAgcManualGain();
}
double violet_vfo_get_sql_level(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getSqlLevel();
}
double violet_vfo_get_sql_alpha(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getSqlAlpha();
}
bool violet_vfo_is_noise_blanker1_on(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->isNoiseBlanker1On();
}
bool violet_vfo_is_noise_blanker2_on(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->isNoiseBlanker2On();
}
float violet_vfo_get_noise_blanker1_threshold(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getNoiseBlanker1Threshold();
}
float violet_vfo_get_noise_blanker2_threshold(VioletVfo* vfo_erased)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);
    return vfo->getNoiseBlanker2Threshold();
}

// FIXME: DRY please!
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

void violet_vfo_subscribe(VioletVfo* vfo_erased, VioletVfoEventHandler handler,
                          void* userdata, VioletConnectionCallback callback,
                          void* userdata2)
{
    auto vfo = static_cast<violetrx::AsyncVfoIface*>(vfo_erased);

    vfo->subscribe(
        [=](const violetrx::VfoEvent& ev) {
            violetrx::CEvent c_event(ev);
            handler(c_event.inner_as_vfo_event(), userdata);
        },
        [=](violetrx::ErrorCode err, violetrx::Connection connection) {
            Connection exposed_connection = std::move(connection);
            callback(err, exposed_connection.ptr(), userdata2);
        });
}
