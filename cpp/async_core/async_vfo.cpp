#include "async_vfo.h"
#include "async_core/events.h"
#include "async_core/worker_thread.h"
#include "error_codes.h"

// FIXME: We need this becausae "start_audio_recording" checks first whether the
// receiver is running or not. I think that this is generally really bad, since
// error checking should be done in the "receiver" and "vfo" classes, and not in
// the "async_receiver" and "async_vfo" classes. These should only return the
// error codes.
#include "core/receiver.h" // IWYU: pragma keep

#include <memory>

namespace violetrx
{

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
void AsyncVfo::stateChanged(Args... args)
{
    signalStateChanged(
        createEvent<Event>(VfoEventCommon::make(getId()), std::move(args)...));
}

template <>
VfoSyncStart AsyncVfo::createEvent<VfoSyncStart>(VfoEventCommon ec)
{
    return VfoSyncStart{ec};
}
template <>
VfoRemoved AsyncVfo::createEvent<VfoRemoved>(VfoEventCommon ec)
{
    return VfoRemoved{ec};
}
template <>
VfoSyncEnd AsyncVfo::createEvent<VfoSyncEnd>(VfoEventCommon ec)
{
    return VfoSyncEnd{ec};
}
template <>
DemodChanged AsyncVfo::createEvent<DemodChanged>(VfoEventCommon ec)
{
    return DemodChanged{ec, getDemod()};
}
template <>
OffsetChanged AsyncVfo::createEvent<OffsetChanged>(VfoEventCommon ec)
{
    return OffsetChanged{ec, getOffset()};
}
template <>
CwOffsetChanged AsyncVfo::createEvent<CwOffsetChanged>(VfoEventCommon ec)
{
    return CwOffsetChanged{ec, getCwOffset()};
}
template <>
FilterChanged AsyncVfo::createEvent<FilterChanged>(VfoEventCommon ec)
{
    auto filter = getFilter();
    return FilterChanged{ec, filter.shape, filter.low, filter.high};
}
template <>
NoiseBlankerOnChanged
AsyncVfo::createEvent<NoiseBlankerOnChanged>(VfoEventCommon ec, int nb_id,
                                             bool enabled)
{
    return NoiseBlankerOnChanged{ec, nb_id, enabled};
}
template <>
NoiseBlankerThresholdChanged
AsyncVfo::createEvent<NoiseBlankerThresholdChanged>(VfoEventCommon ec,
                                                    int nb_id, float threshold)
{
    return NoiseBlankerThresholdChanged{ec, nb_id, threshold};
}
template <>
SqlLevelChanged AsyncVfo::createEvent<SqlLevelChanged>(VfoEventCommon ec)
{
    return SqlLevelChanged{ec, getSqlLevel()};
}
template <>
SqlAlphaChanged AsyncVfo::createEvent<SqlAlphaChanged>(VfoEventCommon ec)
{
    return SqlAlphaChanged{ec, getSqlAlpha()};
}
template <>
AgcOnChanged AsyncVfo::createEvent<AgcOnChanged>(VfoEventCommon ec)
{
    return AgcOnChanged{ec, isAgcOn()};
}
template <>
AgcHangChanged AsyncVfo::createEvent<AgcHangChanged>(VfoEventCommon ec)
{
    return AgcHangChanged{ec, isAgcHangOn()};
}
template <>
AgcThresholdChanged
AsyncVfo::createEvent<AgcThresholdChanged>(VfoEventCommon ec)
{
    return AgcThresholdChanged{ec, getAgcThreshold()};
}
template <>
AgcSlopeChanged AsyncVfo::createEvent<AgcSlopeChanged>(VfoEventCommon ec)
{
    return AgcSlopeChanged{ec, getAgcSlope()};
}
template <>
AgcDecayChanged AsyncVfo::createEvent<AgcDecayChanged>(VfoEventCommon ec)
{
    return AgcDecayChanged{ec, getAgcDecay()};
}
template <>
AgcManualGainChanged
AsyncVfo::createEvent<AgcManualGainChanged>(VfoEventCommon ec)
{
    return AgcManualGainChanged{ec, getAgcManualGain()};
}
template <>
FmMaxDevChanged AsyncVfo::createEvent<FmMaxDevChanged>(VfoEventCommon ec)
{
    return FmMaxDevChanged{ec, getFmMaxDev()};
}
template <>
FmDeemphChanged AsyncVfo::createEvent<FmDeemphChanged>(VfoEventCommon ec)
{
    return FmDeemphChanged{ec, getFmDeemph()};
}
template <>
AmDcrChanged AsyncVfo::createEvent<AmDcrChanged>(VfoEventCommon ec)
{
    return AmDcrChanged{ec, getAmDcr()};
}
template <>
AmSyncDcrChanged AsyncVfo::createEvent<AmSyncDcrChanged>(VfoEventCommon ec)
{
    return AmSyncDcrChanged{ec, getAmSyncDcr()};
}
template <>
AmSyncPllBwChanged AsyncVfo::createEvent<AmSyncPllBwChanged>(VfoEventCommon ec)
{
    return AmSyncPllBwChanged{ec, getAmSyncPllBw()};
}
template <>
RecordingStarted AsyncVfo::createEvent<RecordingStarted>(VfoEventCommon ec)
{
    return RecordingStarted{ec, getRecordingFilename()};
}
template <>
RecordingStopped AsyncVfo::createEvent<RecordingStopped>(VfoEventCommon ec)
{
    return RecordingStopped{ec};
}
template <>
SnifferStarted AsyncVfo::createEvent<SnifferStarted>(VfoEventCommon ec)
{
    auto params = getSnifferParams();
    return SnifferStarted{ec, params.sampleRate, params.buffSize};
}
template <>
SnifferStopped AsyncVfo::createEvent<SnifferStopped>(VfoEventCommon ec)
{
    return SnifferStopped{ec};
}
template <>
UdpStreamingStarted
AsyncVfo::createEvent<UdpStreamingStarted>(VfoEventCommon ec)
{
    auto params = getUdpStreamParams();
    return UdpStreamingStarted{ec, params.host, params.port, params.stereo};
}
template <>
UdpStreamingStopped
AsyncVfo::createEvent<UdpStreamingStopped>(VfoEventCommon ec)
{
    return UdpStreamingStopped{ec};
}
template <>
RdsDecoderStarted AsyncVfo::createEvent<RdsDecoderStarted>(VfoEventCommon ec)
{
    return RdsDecoderStarted{ec};
}
template <>
RdsDecoderStopped AsyncVfo::createEvent<RdsDecoderStopped>(VfoEventCommon ec)
{
    return RdsDecoderStopped{ec};
}
template <>
RdsParserReset AsyncVfo::createEvent<RdsParserReset>(VfoEventCommon ec)
{
    return RdsParserReset{ec};
}
template <>
AudioGainChanged AsyncVfo::createEvent<AudioGainChanged>(VfoEventCommon ec)
{
    return AudioGainChanged{ec, getAfGain()};
}

AsyncVfo::sptr AsyncVfo::make(vfo_channel::sptr vfo,
                              WorkerThread::sptr workerThread)
{
    return std::make_shared<AsyncVfo>(vfo, workerThread);
}

template <typename Function>
auto AsyncVfo::schedule(Function&& func, const std::source_location loc)
{
    return workerThread->scheduleForced(loc.function_name(),
                                        std::forward<Function>(func));
}

AsyncVfo::AsyncVfo(vfo_channel::sptr vfo_, WorkerThread::sptr workerThread_) :
    vfo(vfo_),
    workerThread(workerThread_),
    m_demod(Demod::OFF),
    m_fmMaxDev(5000),
    m_fmDeemph(75.0e-6),
    m_amDcr(true),
    m_amSyncDcr(true),
    m_amSyncPllBw(0.001),
    m_agcOn(true),
    m_agcHang(false),
    m_agcThreshold(-100),
    m_agcSlope(0),
    m_agcDecay(500),
    m_agcManualGain(0),
    m_sqlLevel(-150.0),
    m_sqlAlpha(0.001),
    m_nb1On(false),
    m_nb1Threshold(3.3),
    m_nb2On(false),
    m_nb2Threshold(2.5),
    m_removed(false)
{
}

AsyncVfo::~AsyncVfo() { spdlog::debug("AsyncVfo::~AsyncVfo"); }

void AsyncVfo::setFilterOffset(int64_t offset, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, offset, callback = std::move(callback)]() mutable {
        bool result = vfo->set_filter_offset((double)offset);
        if (result) {
            CALLBACK_ON_SUCCESS();
            vfo->reset_rds_parser();
            stateChanged<OffsetChanged>();
        } else {
            CALLBACK_ON_ERROR(INVALID_FILTER_OFFSET);
        }
    });
}

bool AsyncVfo::isValidFilter(int64_t low, int64_t high)
{
    auto filterRange = getFilterRange(m_demod);

    // TODO check symmetric
    return high >= low && low >= filterRange.lowMin &&
           low <= filterRange.lowMax && high >= filterRange.highMin &&
           high <= filterRange.highMax;
}

FilterRange AsyncVfo::getFilterRange(Demod d) const
{
    switch (d) {
    case Demod::OFF:
        return FilterRange{0, 0, 0, 0, true};
    case Demod::RAW:
        return FilterRange{-40000, -200, 200, 40000, true};
    case Demod::AM:
        return FilterRange{-40000, -200, 200, 40000, true};
    case Demod::AM_SYNC:
        return FilterRange{-40000, -200, 200, 40000, true};
    case Demod::LSB:
        return FilterRange{-40000, -100, -5000, 0, false};
    case Demod::USB:
        return FilterRange{0, 5000, 100, 40000, false};
    case Demod::CWL:
        return FilterRange{-5000, -100, 100, 5000, true};
    case Demod::CWU:
        return FilterRange{-5000, -100, 100, 5000, true};
    case Demod::NFM:
        return FilterRange{-40000, -1000, 1000, 40000, true};
    case Demod::WFM_MONO:
        return FilterRange{-120000, -10000, 10000, 120000, true};
    case Demod::WFM_STEREO:
        return FilterRange{-120000, -10000, 10000, 120000, true};
    case Demod::WFM_STEREO_OIRT:
        return FilterRange{-120000, -10000, 10000, 120000, true};
    default:
        return FilterRange{0, 0, 0, 0, true};
    }
}

// FIXME: maybe receiver Filter type instead?
void AsyncVfo::setFilter(int64_t low, int64_t high, FilterShape filter_shape,
                         Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, low, high, filter_shape,
              callback = std::move(callback)]() mutable {
        if (!isValidFilter(low, high)) {
            CALLBACK_ON_ERROR(INVALID_FILTER);
            return;
        }

        bool result = vfo->set_filter((double)low, (double)high,
                                      (vfo_channel::filter_shape)filter_shape);
        if (result) {
            CALLBACK_ON_SUCCESS();
            stateChanged<FilterChanged>();
        } else {
            CALLBACK_ON_ERROR(INVALID_FILTER);
            return;
        }
    });
}
void AsyncVfo::setCwOffset(int64_t offset, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, offset, callback = std::move(callback)]() mutable {
        bool result = vfo->set_cw_offset(offset);
        if (result) {
            CALLBACK_ON_SUCCESS();
            stateChanged<CwOffsetChanged>();
        } else {
            CALLBACK_ON_ERROR(INVALID_CW_OFFSET);
            return;
        }
    });
}
void AsyncVfo::setDemod(Demod demod, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, demod, callback = std::move(callback)]() mutable {
        if (m_demod == demod) {
            CALLBACK_ON_SUCCESS();
            return;
        }

        if ((int)demod < 0 || (int)demod >= (int)Demod::LAST) {
            CALLBACK_ON_ERROR(INVALID_DEMOD);
            return;
        }

        if (vfo->is_rds_decoder_active())
            stopRdsDecoder();

        if (vfo->is_recording_audio()) {
            stopAudioRecording();
        }

        bool result = false;
        switch (demod) {
        case Demod::OFF:
            if (vfo->is_recording_audio()) {
                stopAudioRecording();
            }
            result = vfo->set_demod(vfo_channel::RX_DEMOD_OFF);
            break;

        case Demod::RAW:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_NONE);
            break;

        case Demod::AM:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_AM);
            if (result) {
                vfo->set_am_dcr(m_amDcr);
            }
            break;

        case Demod::AM_SYNC:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_AMSYNC);
            if (result) {
                vfo->set_amsync_dcr(m_amSyncDcr);
                vfo->set_amsync_pll_bw(m_amSyncPllBw);
            }
            break;

        case Demod::NFM:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_NFM);
            if (result) {
                vfo->set_fm_deemph(m_fmDeemph);
                vfo->set_fm_maxdev(m_fmMaxDev);
            }
            break;

        case Demod::WFM_MONO:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_WFM_M);
            break;

        case Demod::WFM_STEREO:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_WFM_S);
            break;

        case Demod::WFM_STEREO_OIRT:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_WFM_S_OIRT);
            break;

        case Demod::LSB:
        case Demod::USB:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_SSB);
            break;
        case Demod::CWL:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_SSB);
            if (result) {
                vfo->set_cw_offset(vfo->get_cw_offset());
            }
            break;
        case Demod::CWU:
            result = vfo->set_demod(vfo_channel::RX_DEMOD_SSB);
            if (result) {
                vfo->set_cw_offset(vfo->get_cw_offset());
            }
            break;

        default:
            // this should never happen
            CALLBACK_ON_ERROR(INVALID_DEMOD);
            return;
        }

        // reset agc options
        vfo->set_agc_on(m_agcOn);
        vfo->set_agc_hang(m_agcHang);
        vfo->set_agc_threshold(m_agcThreshold);
        vfo->set_agc_slope(m_agcSlope);
        vfo->set_agc_decay(m_agcDecay);
        vfo->set_agc_manual_gain(m_agcManualGain);

        // reset squelch
        vfo->set_sql_level(m_sqlLevel);
        vfo->set_sql_alpha(m_sqlAlpha);

        // reset noise blanker
        vfo->set_nb_on(1, m_nb1On);
        vfo->set_nb_threshold(1, m_nb1Threshold);
        vfo->set_nb_on(2, m_nb2On);
        vfo->set_nb_threshold(2, m_nb2Threshold);

        // reset rds options
        vfo->reset_rds_parser();

        m_demod = demod;

        CALLBACK_ON_SUCCESS();
        stateChanged<DemodChanged>();

        bool invalidFilter = true;
        if (isValidFilter(vfo->get_filter_low(), vfo->get_filter_high())) {
            bool result =
                vfo->set_filter(vfo->get_filter_low(), vfo->get_filter_high(),
                                vfo->get_filter_shape());
            if (result)
                invalidFilter = false;
        }

        if (invalidFilter) {
            setDefaultFilter();
        }
    });
}

void AsyncVfo::setDefaultFilter()
{
    switch (m_demod) {
    case Demod::OFF:
        break;
    case Demod::RAW:
    case Demod::AM:
    case Demod::AM_SYNC:
    case Demod::NFM:
        setFilter(-5000, 5000, FilterShape::NORMAL);
        break;
    case Demod::LSB:
        setFilter(-2800, -100, FilterShape::NORMAL);
        break;
    case Demod::USB:
        setFilter(100, 2800, FilterShape::NORMAL);
        break;
    case Demod::CWL:
    case Demod::CWU:
        setFilter(-250, 250, FilterShape::NORMAL);
        break;
    case Demod::WFM_MONO:
    case Demod::WFM_STEREO:
    case Demod::WFM_STEREO_OIRT:
        setFilter(-80000, 80000, FilterShape::NORMAL);
        break;
    default:
        break;
    }
}

void AsyncVfo::getSignalPwr(Callback<float> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        CALLBACK_ON_SUCCESS(vfo->get_signal_pwr());
    });
}

/* Noise blanker */
void AsyncVfo::setNoiseBlanker(int nbid, bool on, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, nbid, on, callback = std::move(callback)]() mutable {
        if (nbid == 1) {
            vfo->set_nb_on(nbid, on);
            m_nb1On = on;
            CALLBACK_ON_SUCCESS();
            stateChanged<NoiseBlankerOnChanged>(nbid, on);
        } else if (nbid == 2) {
            vfo->set_nb_on(nbid, on);
            m_nb2On = on;
            CALLBACK_ON_SUCCESS();
            stateChanged<NoiseBlankerOnChanged>(nbid, on);
        } else {
            CALLBACK_ON_ERROR(INVALID_NOISE_BLANKER_ID);
        }
    });
}
void AsyncVfo::setNoiseBlankerThreshold(int nbid, float threshold,
                                        Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, nbid, threshold, callback = std::move(callback)]() mutable {
        if (nbid == 1) {
            vfo->set_nb_threshold(nbid, threshold);
            m_nb1Threshold = threshold;
            CALLBACK_ON_SUCCESS();
            stateChanged<NoiseBlankerThresholdChanged>(nbid, threshold);
        } else if (nbid == 2) {
            vfo->set_nb_threshold(nbid, threshold);
            m_nb2Threshold = threshold;
            CALLBACK_ON_SUCCESS();
            stateChanged<NoiseBlankerThresholdChanged>(nbid, threshold);
        } else {
            CALLBACK_ON_ERROR(INVALID_NOISE_BLANKER_ID);
        }
    });
}

void AsyncVfo::setSqlLevel(double level_db, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, level_db, callback = std::move(callback)]() mutable {
        vfo->set_sql_level(level_db);
        m_sqlLevel = level_db;

        CALLBACK_ON_SUCCESS();
        stateChanged<SqlLevelChanged>();
    });
}

void AsyncVfo::setSqlAlpha(double alpha, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, alpha, callback = std::move(callback)]() mutable {
        vfo->set_sql_alpha(alpha);
        m_sqlAlpha = alpha;

        CALLBACK_ON_SUCCESS();
        stateChanged<SqlAlphaChanged>();
    });
}

void AsyncVfo::setAgcOn(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, enable, callback = std::move(callback)]() mutable {
        vfo->set_agc_on(enable);
        m_agcOn = enable;

        CALLBACK_ON_SUCCESS();
        stateChanged<AgcOnChanged>();
    });
}
void AsyncVfo::setAgcHang(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, enable, callback = std::move(callback)]() mutable {
        vfo->set_agc_hang(enable);
        m_agcHang = enable;

        CALLBACK_ON_SUCCESS();
        stateChanged<AgcHangChanged>();
    });
}
void AsyncVfo::setAgcThreshold(int threshold, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, threshold, callback = std::move(callback)]() mutable {
        vfo->set_agc_threshold(threshold);
        m_agcThreshold = threshold;

        CALLBACK_ON_SUCCESS();
        stateChanged<AgcThresholdChanged>();
    });
}
void AsyncVfo::setAgcSlope(int slope, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, slope, callback = std::move(callback)]() mutable {
        vfo->set_agc_slope(slope);
        m_agcSlope = slope;

        CALLBACK_ON_SUCCESS();
        stateChanged<AgcSlopeChanged>();
    });
}
void AsyncVfo::setAgcDecay(int decay, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, decay, callback = std::move(callback)]() mutable {
        vfo->set_agc_decay(decay);
        m_agcDecay = decay;

        CALLBACK_ON_SUCCESS();
        stateChanged<AgcDecayChanged>();
    });
}
void AsyncVfo::setAgcManualGain(int gain, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, gain, callback = std::move(callback)]() mutable {
        vfo->set_agc_manual_gain(gain);
        m_agcManualGain = gain;

        CALLBACK_ON_SUCCESS();
        stateChanged<AgcManualGainChanged>();
    });
}

void AsyncVfo::setFmMaxDev(float maxdev, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, maxdev, callback = std::move(callback)]() mutable {
        vfo->set_fm_maxdev(maxdev);
        m_fmMaxDev = maxdev;

        CALLBACK_ON_SUCCESS();
        stateChanged<FmMaxDevChanged>();
    });
}
void AsyncVfo::setFmDeemph(double tau, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, tau, callback = std::move(callback)]() mutable {
        vfo->set_fm_deemph(tau);
        m_fmDeemph = tau;

        CALLBACK_ON_SUCCESS();
        stateChanged<FmDeemphChanged>();
    });
}
void AsyncVfo::setAmDcr(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, enable, callback = std::move(callback)]() mutable {
        vfo->set_am_dcr(enable);
        m_amDcr = enable;

        CALLBACK_ON_SUCCESS();
        stateChanged<AmDcrChanged>();
    });
}
void AsyncVfo::setAmSyncDcr(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, enable, callback = std::move(callback)]() mutable {
        vfo->set_amsync_dcr(enable);
        m_amSyncDcr = enable;

        CALLBACK_ON_SUCCESS();
        stateChanged<AmSyncDcrChanged>();
    });
}
void AsyncVfo::setAmSyncPllBw(float bw, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, bw, callback = std::move(callback)]() mutable {
        vfo->set_amsync_pll_bw(bw);
        m_amSyncPllBw = bw;

        CALLBACK_ON_SUCCESS();
        stateChanged<AmSyncPllBwChanged>();
    });
}

void AsyncVfo::startAudioRecording(const std::string& filename,
                                   Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, filename, callback = std::move(callback)]() mutable {
        if (vfo->is_recording_audio()) {
            CALLBACK_ON_ERROR(ALREADY_RECORDING);
            return;
        } else if (m_demod == Demod::OFF) {
            CALLBACK_ON_ERROR(DEMOD_IS_OFF);
            return;
        } else if (!vfo->get_parent_receiver()->is_running()) {
            CALLBACK_ON_ERROR(NOT_RUNNING);
            return;
        }

        bool result = vfo->start_audio_recording(filename);
        if (result) {
            CALLBACK_ON_SUCCESS();
            stateChanged<RecordingStarted>();
        } else {
            // FIXME: start_audio_recording should indicate this!
            CALLBACK_ON_ERROR(COULDNT_CREATE_FILE);
        }
    });
}
void AsyncVfo::stopAudioRecording(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        if (!vfo->is_recording_audio()) {
            CALLBACK_ON_ERROR(ALREADY_NOT_RECORDING);
            return;
        }

        vfo->stop_audio_recording();

        CALLBACK_ON_SUCCESS();
        stateChanged<RecordingStopped>();
    });
}
void AsyncVfo::setAudioGain(float gain, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, gain, callback = std::move(callback)]() mutable {
        vfo->set_af_gain(gain);

        CALLBACK_ON_SUCCESS();
        stateChanged<AudioGainChanged>();
    });
}

void AsyncVfo::startUdpStreaming(const std::string& host, int port, bool stereo,
                                 Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule(
        [this, host, port, stereo, callback = std::move(callback)]() mutable {
            // FIXME: we should validate host/port
            vfo->start_udp_streaming(host, port, stereo);
            CALLBACK_ON_SUCCESS();

            stateChanged<UdpStreamingStarted>();
        });
}

void AsyncVfo::stopUdpStreaming(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        // FIXME: we should validate host/port
        vfo->stop_udp_streaming();
        CALLBACK_ON_SUCCESS();

        stateChanged<UdpStreamingStopped>();
    });
}

void AsyncVfo::startSniffer(int samplerate, int buffsize, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule(
        [this, samplerate, buffsize, callback = std::move(callback)]() mutable {
            if (vfo->is_snifffer_active()) {
                CALLBACK_ON_ERROR(SNIFFER_ALREADY_ACTIVE);
                return;
            }

            vfo->start_sniffer(samplerate, buffsize);

            CALLBACK_ON_SUCCESS();

            stateChanged<SnifferStarted>();
        });
}
void AsyncVfo::stopSniffer(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        if (!vfo->is_snifffer_active()) {
            CALLBACK_ON_ERROR(SNIFFER_ALREADY_INACTIVE);
            return;
        }

        vfo->stop_sniffer();

        CALLBACK_ON_SUCCESS();

        stateChanged<SnifferStopped>();
    });
}
void AsyncVfo::getSnifferData(float* data, int size,
                              Callback<float*, int> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, data, size, callback = std::move(callback)]() mutable {
        if (!vfo->is_snifffer_active()) {
            CALLBACK_ON_ERROR(SNIFFER_ALREADY_INACTIVE);
            return;
        }

        if (data == nullptr) {
            data = new float[vfo->get_sniffer_buffsize()];
        } else if (vfo->get_sniffer_buffsize() > size) {
            CALLBACK_ON_ERROR(INSUFFICIENT_BUFFER_SIZE);
            return;
        }

        int num;
        vfo->get_sniffer_data(data, num);

        CALLBACK_ON_SUCCESS(data, num);
    });
}

void AsyncVfo::getRdsData(Callback<std::string, int> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        std::string message;
        int num;
        vfo->get_rds_data(message, num);

        CALLBACK_ON_SUCCESS(std::move(message), num);
    });
}
void AsyncVfo::startRdsDecoder(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        if (vfo->is_rds_decoder_active()) {
            CALLBACK_ON_ERROR(RDS_ALREADY_ACTIVE);
            return;
        }

        vfo->reset_rds_parser();
        vfo->start_rds_decoder();

        CALLBACK_ON_SUCCESS();

        stateChanged<RdsDecoderStarted>();
    });
}
void AsyncVfo::stopRdsDecoder(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        if (!vfo->is_rds_decoder_active()) {
            CALLBACK_ON_ERROR(RDS_ALREADY_INACTIVE);
            return;
        }

        vfo->reset_rds_parser();
        vfo->stop_rds_decoder();

        CALLBACK_ON_SUCCESS();

        stateChanged<RdsDecoderStopped>();
    });
}
void AsyncVfo::resetRdsParser(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule([this, callback = std::move(callback)]() mutable {
        vfo->reset_rds_parser();

        CALLBACK_ON_SUCCESS();

        stateChanged<RdsParserReset>();
    });
}

void AsyncVfo::synchronize(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    schedule(
        [callback = std::move(callback)]() mutable { CALLBACK_ON_SUCCESS(); });
}

Demod AsyncVfo::getDemod() const { return m_demod; }
int32_t AsyncVfo::getOffset() const { return vfo->get_filter_offset(); }
Filter AsyncVfo::getFilter() const
{
    return Filter{(FilterShape)vfo->get_filter_shape(),
                  (int32_t)vfo->get_filter_low(),
                  (int32_t)vfo->get_filter_high()};
}
int32_t AsyncVfo::getCwOffset() const { return vfo->get_cw_offset(); }
float AsyncVfo::getFmMaxDev() const { return m_fmMaxDev; }
double AsyncVfo::getFmDeemph() const { return m_fmDeemph; }
bool AsyncVfo::getAmDcr() const { return m_amDcr; }
bool AsyncVfo::getAmSyncDcr() const { return m_amSyncDcr; }
float AsyncVfo::getAmSyncPllBw() const { return m_amSyncPllBw; }
bool AsyncVfo::isAudioRecording() const { return vfo->is_recording_audio(); }
std::string AsyncVfo::getRecordingFilename() const
{
    return vfo->get_recording_filename();
}
bool AsyncVfo::isUdpStreaming() const { return vfo->is_udp_streaming(); }
bool AsyncVfo::isSniffing() const { return vfo->is_snifffer_active(); }
bool AsyncVfo::isRdsDecoderActive() const
{
    return vfo->is_rds_decoder_active();
}
bool AsyncVfo::isAgcOn() const { return m_agcOn; }
bool AsyncVfo::isAgcHangOn() const { return m_agcHang; }
int AsyncVfo::getAgcThreshold() const { return m_agcThreshold; }
int AsyncVfo::getAgcSlope() const { return m_agcSlope; }
int AsyncVfo::getAgcDecay() const { return m_agcDecay; }
int AsyncVfo::getAgcManualGain() const { return m_agcManualGain; }
double AsyncVfo::getSqlLevel() const { return m_sqlLevel; }
double AsyncVfo::getSqlAlpha() const { return m_sqlAlpha; }
bool AsyncVfo::isNoiseBlanker1On() const { return m_nb1On; }
bool AsyncVfo::isNoiseBlanker2On() const { return m_nb2On; }
float AsyncVfo::getNoiseBlanker1Threshold() const { return m_nb1Threshold; }
float AsyncVfo::getNoiseBlanker2Threshold() const { return m_nb2Threshold; }
float AsyncVfo::getAfGain() const { return vfo->get_af_gain(); }
SnifferParams AsyncVfo::getSnifferParams() const
{
    auto params = vfo->get_sniffer_params();
    return SnifferParams{.sampleRate = params.samplerate,
                         .buffSize = params.buffsize};
}
UdpStreamParams AsyncVfo::getUdpStreamParams() const
{
    // YUCK
    auto params = vfo->get_udp_stream_params();
    return UdpStreamParams{
        .host = params.host, .port = params.port, .stereo = params.stereo};
}

uint64_t AsyncVfo::getId() const { return (uint64_t)vfo.get(); }

void AsyncVfo::prepareToDie()
{
    m_removed = true;
    stateChanged<VfoRemoved>();
}

void AsyncVfo::subscribe(VfoEventHandler handler, Callback<Connection> callback)
{
    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, handler = std::move(handler),
              callback = std::move(callback), handle = getId()]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            callback(violetrx::ErrorCode::VFO_NOT_FOUND, {});
            return;
        }

        callback(violetrx::ErrorCode::OK,
                 sptr->signalStateChanged.connect(handler));

        VfoEventCommon ec;
        ec.id = -1;
        ec.timestamp = {};
        ec.handle = handle;

        handler(sptr->createEvent<VfoSyncStart>(ec));

        handler(sptr->createEvent<DemodChanged>(ec));
        handler(sptr->createEvent<OffsetChanged>(ec));
        handler(sptr->createEvent<CwOffsetChanged>(ec));
        handler(sptr->createEvent<FilterChanged>(ec));
        handler(sptr->createEvent<NoiseBlankerOnChanged>(
            ec, 1, sptr->isNoiseBlanker1On()));
        handler(sptr->createEvent<NoiseBlankerOnChanged>(
            ec, 2, sptr->isNoiseBlanker2On()));
        handler(sptr->createEvent<NoiseBlankerThresholdChanged>(
            ec, 1, sptr->getNoiseBlanker1Threshold()));
        handler(sptr->createEvent<NoiseBlankerThresholdChanged>(
            ec, 2, sptr->getNoiseBlanker2Threshold()));
        handler(sptr->createEvent<SqlLevelChanged>(ec));
        handler(sptr->createEvent<SqlAlphaChanged>(ec));
        handler(sptr->createEvent<AgcOnChanged>(ec));
        handler(sptr->createEvent<AgcHangChanged>(ec));
        handler(sptr->createEvent<AgcThresholdChanged>(ec));
        handler(sptr->createEvent<AgcSlopeChanged>(ec));
        handler(sptr->createEvent<AgcDecayChanged>(ec));
        handler(sptr->createEvent<AgcManualGainChanged>(ec));
        handler(sptr->createEvent<FmMaxDevChanged>(ec));
        handler(sptr->createEvent<FmDeemphChanged>(ec));
        handler(sptr->createEvent<AmDcrChanged>(ec));
        handler(sptr->createEvent<AmSyncDcrChanged>(ec));
        handler(sptr->createEvent<AmSyncPllBwChanged>(ec));
        if (sptr->isAudioRecording()) {
            handler(sptr->createEvent<RecordingStarted>(ec));
        }
        if (sptr->isSniffing()) {
            handler(sptr->createEvent<SnifferStarted>(ec));
        }
        if (sptr->isUdpStreaming()) {
            handler(sptr->createEvent<UdpStreamingStarted>(ec));
        }
        if (sptr->isRdsDecoderActive()) {
            handler(sptr->createEvent<RdsDecoderStarted>(ec));
        }
        handler(sptr->createEvent<AudioGainChanged>(ec));

        handler(sptr->createEvent<VfoSyncEnd>(ec));
    });
}

void AsyncVfo::unsubscribe(const Connection& c) { c.disconnect(); }

} // namespace violetrx
