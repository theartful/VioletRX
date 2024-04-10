#include "async_vfo.h"
#include "async_core/events.h"
#include "error_codes.h"
#include "utility/worker_thread.h"

// FIXME: We need this becausae "start_audio_recording" checks first whether the
// receiver is running or not. I think that this is generally really bad, since
// error checking should be done in the "receiver" and "vfo" classes, and not in
// the "async_receiver" and "async_vfo" classes. These should only return the
// error codes.
#include "core/receiver.h" // IWYU: pragma keep

#include <memory>

namespace violetrx
{

#define INVOKE(callback, ...)                                                  \
    do {                                                                       \
        if (callback) {                                                        \
            callback(__VA_ARGS__);                                             \
        }                                                                      \
    } while (false)

#define CALLBACK_ON_SUCCESS(...)                                               \
    INVOKE(callback, ErrorCode::OK __VA_OPT__(, ) __VA_ARGS__)

#define CALLBACK_ON_ERROR(code) invokeDefault(callback, code)

#define RETURN_IF_WORKER_BUSY()                                                \
    do {                                                                       \
        if (workerThread->isPaused()) {                                        \
            CALLBACK_ON_ERROR(ErrorCode::WORKER_BUSY);                         \
            return;                                                            \
        }                                                                      \
    } while (0)

template <typename... Args>
void invokeDefault(Callback<Args...>& callback, ErrorCode code)
{
    if (callback) {
        callback(code, std::decay_t<Args>{}...);
    }
}

template <typename Event, typename... Args>
void AsyncVfo::stateChanged(Args... args)
{
    signalStateChanged(
        createEvent<Event>(VfoEventCommon::make(getId()), std::move(args)...));
}

template <>
VfoSyncStart AsyncVfo::createEvent<VfoSyncStart>(VfoEventCommon ec) const
{
    return VfoSyncStart{ec};
}
template <>
VfoRemoved AsyncVfo::createEvent<VfoRemoved>(VfoEventCommon ec) const
{
    return VfoRemoved{ec};
}
template <>
VfoSyncEnd AsyncVfo::createEvent<VfoSyncEnd>(VfoEventCommon ec) const
{
    return VfoSyncEnd{ec};
}
template <>
DemodChanged AsyncVfo::createEvent<DemodChanged>(VfoEventCommon ec) const
{
    return DemodChanged{ec, getDemod()};
}
template <>
OffsetChanged AsyncVfo::createEvent<OffsetChanged>(VfoEventCommon ec) const
{
    return OffsetChanged{ec, getOffset()};
}
template <>
CwOffsetChanged AsyncVfo::createEvent<CwOffsetChanged>(VfoEventCommon ec) const
{
    return CwOffsetChanged{ec, getCwOffset()};
}
template <>
FilterChanged AsyncVfo::createEvent<FilterChanged>(VfoEventCommon ec) const
{
    auto filter = getFilter();
    return FilterChanged{ec, filter.shape, filter.low, filter.high};
}
template <>
NoiseBlankerOnChanged
AsyncVfo::createEvent<NoiseBlankerOnChanged>(VfoEventCommon ec, int nb_id,
                                             bool enabled) const
{
    return NoiseBlankerOnChanged{ec, nb_id, enabled};
}
template <>
NoiseBlankerThresholdChanged
AsyncVfo::createEvent<NoiseBlankerThresholdChanged>(VfoEventCommon ec,
                                                    int nb_id,
                                                    float threshold) const
{
    return NoiseBlankerThresholdChanged{ec, nb_id, threshold};
}
template <>
SqlLevelChanged AsyncVfo::createEvent<SqlLevelChanged>(VfoEventCommon ec) const
{
    return SqlLevelChanged{ec, getSqlLevel()};
}
template <>
SqlAlphaChanged AsyncVfo::createEvent<SqlAlphaChanged>(VfoEventCommon ec) const
{
    return SqlAlphaChanged{ec, getSqlAlpha()};
}
template <>
AgcOnChanged AsyncVfo::createEvent<AgcOnChanged>(VfoEventCommon ec) const
{
    return AgcOnChanged{ec, isAgcOn()};
}
template <>
AgcHangChanged AsyncVfo::createEvent<AgcHangChanged>(VfoEventCommon ec) const
{
    return AgcHangChanged{ec, isAgcHangOn()};
}
template <>
AgcThresholdChanged
AsyncVfo::createEvent<AgcThresholdChanged>(VfoEventCommon ec) const
{
    return AgcThresholdChanged{ec, getAgcThreshold()};
}
template <>
AgcSlopeChanged AsyncVfo::createEvent<AgcSlopeChanged>(VfoEventCommon ec) const
{
    return AgcSlopeChanged{ec, getAgcSlope()};
}
template <>
AgcDecayChanged AsyncVfo::createEvent<AgcDecayChanged>(VfoEventCommon ec) const
{
    return AgcDecayChanged{ec, getAgcDecay()};
}
template <>
AgcManualGainChanged
AsyncVfo::createEvent<AgcManualGainChanged>(VfoEventCommon ec) const
{
    return AgcManualGainChanged{ec, getAgcManualGain()};
}
template <>
FmMaxDevChanged AsyncVfo::createEvent<FmMaxDevChanged>(VfoEventCommon ec) const
{
    return FmMaxDevChanged{ec, getFmMaxDev()};
}
template <>
FmDeemphChanged AsyncVfo::createEvent<FmDeemphChanged>(VfoEventCommon ec) const
{
    return FmDeemphChanged{ec, getFmDeemph()};
}
template <>
AmDcrChanged AsyncVfo::createEvent<AmDcrChanged>(VfoEventCommon ec) const
{
    return AmDcrChanged{ec, getAmDcr()};
}
template <>
AmSyncDcrChanged
AsyncVfo::createEvent<AmSyncDcrChanged>(VfoEventCommon ec) const
{
    return AmSyncDcrChanged{ec, getAmSyncDcr()};
}
template <>
AmSyncPllBwChanged
AsyncVfo::createEvent<AmSyncPllBwChanged>(VfoEventCommon ec) const
{
    return AmSyncPllBwChanged{ec, getAmSyncPllBw()};
}
template <>
RecordingStarted
AsyncVfo::createEvent<RecordingStarted>(VfoEventCommon ec) const
{
    return RecordingStarted{ec, getRecordingFilename()};
}
template <>
RecordingStopped
AsyncVfo::createEvent<RecordingStopped>(VfoEventCommon ec) const
{
    return RecordingStopped{ec};
}
template <>
SnifferStarted AsyncVfo::createEvent<SnifferStarted>(VfoEventCommon ec) const
{
    auto params = getSnifferParams();
    return SnifferStarted{ec, params.sampleRate, params.buffSize};
}
template <>
SnifferStopped AsyncVfo::createEvent<SnifferStopped>(VfoEventCommon ec) const
{
    return SnifferStopped{ec};
}
template <>
UdpStreamingStarted
AsyncVfo::createEvent<UdpStreamingStarted>(VfoEventCommon ec) const
{
    auto params = getUdpStreamParams();
    return UdpStreamingStarted{ec, params.host, params.port, params.stereo};
}
template <>
UdpStreamingStopped
AsyncVfo::createEvent<UdpStreamingStopped>(VfoEventCommon ec) const
{
    return UdpStreamingStopped{ec};
}
template <>
RdsDecoderStarted
AsyncVfo::createEvent<RdsDecoderStarted>(VfoEventCommon ec) const
{
    return RdsDecoderStarted{ec};
}
template <>
RdsDecoderStopped
AsyncVfo::createEvent<RdsDecoderStopped>(VfoEventCommon ec) const
{
    return RdsDecoderStopped{ec};
}
template <>
RdsParserReset AsyncVfo::createEvent<RdsParserReset>(VfoEventCommon ec) const
{
    return RdsParserReset{ec};
}
template <>
AudioGainChanged
AsyncVfo::createEvent<AudioGainChanged>(VfoEventCommon ec) const
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

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, offset, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        bool result = sptr->vfo->set_filter_offset((double)offset);
        if (result) {
            CALLBACK_ON_SUCCESS();
            sptr->vfo->reset_rds_parser();
            sptr->stateChanged<OffsetChanged>();
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

// FIXME: maybe receiver Filter type instead?
void AsyncVfo::setFilter(int64_t low, int64_t high, FilterShape filter_shape,
                         Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, low, high, filter_shape,
              callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (!sptr->isValidFilter(low, high)) {
            CALLBACK_ON_ERROR(INVALID_FILTER);
            return;
        }

        bool result = sptr->vfo->set_filter(
            (double)low, (double)high, (vfo_channel::filter_shape)filter_shape);
        if (result) {
            CALLBACK_ON_SUCCESS();
            sptr->stateChanged<FilterChanged>();
        } else {
            CALLBACK_ON_ERROR(INVALID_FILTER);
            return;
        }
    });
}
void AsyncVfo::setCwOffset(int64_t offset, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, offset, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        bool result = sptr->vfo->set_cw_offset(offset);
        if (result) {
            CALLBACK_ON_SUCCESS();
            sptr->stateChanged<CwOffsetChanged>();
        } else {
            CALLBACK_ON_ERROR(INVALID_CW_OFFSET);
            return;
        }
    });
}
void AsyncVfo::setDemod(Demod demod, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, demod, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (sptr->m_demod == demod) {
            CALLBACK_ON_SUCCESS();
            return;
        }

        if ((int)demod < 0 || (int)demod >= (int)Demod::LAST) {
            CALLBACK_ON_ERROR(INVALID_DEMOD);
            return;
        }

        if (sptr->vfo->is_rds_decoder_active())
            sptr->stopRdsDecoder();

        if (sptr->vfo->is_recording_audio()) {
            sptr->stopAudioRecording();
        }

        bool result = false;
        switch (demod) {
        case Demod::OFF:
            if (sptr->vfo->is_recording_audio()) {
                sptr->stopAudioRecording();
            }
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_OFF);
            break;

        case Demod::RAW:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_NONE);
            break;

        case Demod::AM:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_AM);
            if (result) {
                sptr->vfo->set_am_dcr(sptr->m_amDcr);
            }
            break;

        case Demod::AM_SYNC:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_AMSYNC);
            if (result) {
                sptr->vfo->set_amsync_dcr(sptr->m_amSyncDcr);
                sptr->vfo->set_amsync_pll_bw(sptr->m_amSyncPllBw);
            }
            break;

        case Demod::NFM:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_NFM);
            if (result) {
                sptr->vfo->set_fm_deemph(sptr->m_fmDeemph);
                sptr->vfo->set_fm_maxdev(sptr->m_fmMaxDev);
            }
            break;

        case Demod::WFM_MONO:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_WFM_M);
            break;

        case Demod::WFM_STEREO:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_WFM_S);
            break;

        case Demod::WFM_STEREO_OIRT:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_WFM_S_OIRT);
            break;

        case Demod::LSB:
        case Demod::USB:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_SSB);
            break;
        case Demod::CWL:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_SSB);
            if (result) {
                sptr->vfo->set_cw_offset(sptr->vfo->get_cw_offset());
            }
            break;
        case Demod::CWU:
            result = sptr->vfo->set_demod(vfo_channel::RX_DEMOD_SSB);
            if (result) {
                sptr->vfo->set_cw_offset(sptr->vfo->get_cw_offset());
            }
            break;

        default:
            // this should never happen
            CALLBACK_ON_ERROR(INVALID_DEMOD);
            return;
        }

        // reset agc options
        sptr->vfo->set_agc_on(sptr->m_agcOn);
        sptr->vfo->set_agc_hang(sptr->m_agcHang);
        sptr->vfo->set_agc_threshold(sptr->m_agcThreshold);
        sptr->vfo->set_agc_slope(sptr->m_agcSlope);
        sptr->vfo->set_agc_decay(sptr->m_agcDecay);
        sptr->vfo->set_agc_manual_gain(sptr->m_agcManualGain);

        // reset squelch
        sptr->vfo->set_sql_level(sptr->m_sqlLevel);
        sptr->vfo->set_sql_alpha(sptr->m_sqlAlpha);

        // reset noise blanker
        sptr->vfo->set_nb_on(1, sptr->m_nb1On);
        sptr->vfo->set_nb_threshold(1, sptr->m_nb1Threshold);
        sptr->vfo->set_nb_on(2, sptr->m_nb2On);
        sptr->vfo->set_nb_threshold(2, sptr->m_nb2Threshold);

        // reset rds options
        sptr->vfo->reset_rds_parser();

        sptr->m_demod = demod;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<DemodChanged>();

        bool invalidFilter = true;
        if (sptr->isValidFilter(sptr->vfo->get_filter_low(),
                                sptr->vfo->get_filter_high())) {
            bool result = sptr->vfo->set_filter(sptr->vfo->get_filter_low(),
                                                sptr->vfo->get_filter_high(),
                                                sptr->vfo->get_filter_shape());
            if (result)
                invalidFilter = false;
        }

        if (invalidFilter) {
            sptr->setDefaultFilter();
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

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        CALLBACK_ON_SUCCESS(sptr->vfo->get_signal_pwr());
    });
}

/* Noise blanker */
void AsyncVfo::setNoiseBlanker(int nbid, bool on, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, nbid, on, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (nbid == 1) {
            sptr->vfo->set_nb_on(nbid, on);
            sptr->m_nb1On = on;
            CALLBACK_ON_SUCCESS();
            sptr->stateChanged<NoiseBlankerOnChanged>(nbid, on);
        } else if (nbid == 2) {
            sptr->vfo->set_nb_on(nbid, on);
            sptr->m_nb2On = on;
            CALLBACK_ON_SUCCESS();
            sptr->stateChanged<NoiseBlankerOnChanged>(nbid, on);
        } else {
            CALLBACK_ON_ERROR(INVALID_NOISE_BLANKER_ID);
        }
    });
}
void AsyncVfo::setNoiseBlankerThreshold(int nbid, float threshold,
                                        Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, nbid, threshold, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (nbid == 1) {
            sptr->vfo->set_nb_threshold(nbid, threshold);
            sptr->m_nb1Threshold = threshold;
            CALLBACK_ON_SUCCESS();
            sptr->stateChanged<NoiseBlankerThresholdChanged>(nbid, threshold);
        } else if (nbid == 2) {
            sptr->vfo->set_nb_threshold(nbid, threshold);
            sptr->m_nb2Threshold = threshold;
            CALLBACK_ON_SUCCESS();
            sptr->stateChanged<NoiseBlankerThresholdChanged>(nbid, threshold);
        } else {
            CALLBACK_ON_ERROR(INVALID_NOISE_BLANKER_ID);
        }
    });
}

void AsyncVfo::setSqlLevel(double level_db, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, level_db, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_sql_level(level_db);
        sptr->m_sqlLevel = level_db;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<SqlLevelChanged>();
    });
}

void AsyncVfo::setSqlAlpha(double alpha, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, alpha, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_sql_alpha(alpha);
        sptr->m_sqlAlpha = alpha;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<SqlAlphaChanged>();
    });
}

void AsyncVfo::setAgcOn(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, enable, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_agc_on(enable);
        sptr->m_agcOn = enable;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AgcOnChanged>();
    });
}
void AsyncVfo::setAgcHang(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, enable, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_agc_hang(enable);
        sptr->m_agcHang = enable;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AgcHangChanged>();
    });
}
void AsyncVfo::setAgcThreshold(int threshold, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, threshold, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_agc_threshold(threshold);
        sptr->m_agcThreshold = threshold;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AgcThresholdChanged>();
    });
}
void AsyncVfo::setAgcSlope(int slope, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, slope, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_agc_slope(slope);
        sptr->m_agcSlope = slope;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AgcSlopeChanged>();
    });
}
void AsyncVfo::setAgcDecay(int decay, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, decay, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_agc_decay(decay);
        sptr->m_agcDecay = decay;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AgcDecayChanged>();
    });
}
void AsyncVfo::setAgcManualGain(int gain, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, gain, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_agc_manual_gain(gain);
        sptr->m_agcManualGain = gain;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AgcManualGainChanged>();
    });
}

void AsyncVfo::setFmMaxDev(float maxdev, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, maxdev, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_fm_maxdev(maxdev);
        sptr->m_fmMaxDev = maxdev;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<FmMaxDevChanged>();
    });
}
void AsyncVfo::setFmDeemph(double tau, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, tau, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_fm_deemph(tau);
        sptr->m_fmDeemph = tau;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<FmDeemphChanged>();
    });
}
void AsyncVfo::setAmDcr(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, enable, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_am_dcr(enable);
        sptr->m_amDcr = enable;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AmDcrChanged>();
    });
}
void AsyncVfo::setAmSyncDcr(bool enable, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, enable, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_amsync_dcr(enable);
        sptr->m_amSyncDcr = enable;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AmSyncDcrChanged>();
    });
}
void AsyncVfo::setAmSyncPllBw(float bw, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, bw, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_amsync_pll_bw(bw);
        sptr->m_amSyncPllBw = bw;

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AmSyncPllBwChanged>();
    });
}

void AsyncVfo::startAudioRecording(const std::string& filename,
                                   Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, filename, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (sptr->vfo->is_recording_audio()) {
            CALLBACK_ON_ERROR(ALREADY_RECORDING);
            return;
        } else if (sptr->m_demod == Demod::OFF) {
            CALLBACK_ON_ERROR(DEMOD_IS_OFF);
            return;
        } else if (!sptr->vfo->get_parent_receiver()->is_running()) {
            CALLBACK_ON_ERROR(NOT_RUNNING);
            return;
        }

        bool result = sptr->vfo->start_audio_recording(filename);
        if (result) {
            CALLBACK_ON_SUCCESS();
            sptr->stateChanged<RecordingStarted>();
        } else {
            // FIXME: start_audio_recording should indicate this!
            CALLBACK_ON_ERROR(COULDNT_CREATE_FILE);
        }
    });
}
void AsyncVfo::stopAudioRecording(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (!sptr->vfo->is_recording_audio()) {
            CALLBACK_ON_ERROR(ALREADY_NOT_RECORDING);
            return;
        }

        sptr->vfo->stop_audio_recording();

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<RecordingStopped>();
    });
}
void AsyncVfo::setAudioGain(float gain, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, gain, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->set_af_gain(gain);

        CALLBACK_ON_SUCCESS();
        sptr->stateChanged<AudioGainChanged>();
    });
}

void AsyncVfo::startUdpStreaming(const std::string& host, int port, bool stereo,
                                 Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule(
        [self, host, port, stereo, callback = std::move(callback)]() mutable {
            auto sptr = self.lock();
            if (!sptr || sptr->m_removed) {
                CALLBACK_ON_ERROR(VFO_NOT_FOUND);
                return;
            }

            // FIXME: we should validate host/port
            sptr->vfo->start_udp_streaming(host, port, stereo);
            CALLBACK_ON_SUCCESS();

            sptr->stateChanged<UdpStreamingStarted>();
        });
}

void AsyncVfo::stopUdpStreaming(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        // FIXME: we should validate host/port
        sptr->vfo->stop_udp_streaming();
        CALLBACK_ON_SUCCESS();

        sptr->stateChanged<UdpStreamingStopped>();
    });
}

void AsyncVfo::startSniffer(int samplerate, int buffsize, Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule(
        [self, samplerate, buffsize, callback = std::move(callback)]() mutable {
            auto sptr = self.lock();
            if (!sptr || sptr->m_removed) {
                CALLBACK_ON_ERROR(VFO_NOT_FOUND);
                return;
            }

            if (sptr->vfo->is_snifffer_active()) {
                CALLBACK_ON_ERROR(SNIFFER_ALREADY_ACTIVE);
                return;
            }

            sptr->vfo->start_sniffer(samplerate, buffsize);

            CALLBACK_ON_SUCCESS();

            sptr->stateChanged<SnifferStarted>();
        });
}
void AsyncVfo::stopSniffer(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (!sptr->vfo->is_snifffer_active()) {
            CALLBACK_ON_ERROR(SNIFFER_ALREADY_INACTIVE);
            return;
        }

        sptr->vfo->stop_sniffer();

        CALLBACK_ON_SUCCESS();

        sptr->stateChanged<SnifferStopped>();
    });
}
void AsyncVfo::getSnifferData(float* data, int size,
                              Callback<float*, int> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, data, size, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (!sptr->vfo->is_snifffer_active()) {
            CALLBACK_ON_ERROR(SNIFFER_ALREADY_INACTIVE);
            return;
        }

        if (data == nullptr) {
            data = new float[sptr->vfo->get_sniffer_buffsize()];
        } else if (sptr->vfo->get_sniffer_buffsize() > size) {
            CALLBACK_ON_ERROR(INSUFFICIENT_BUFFER_SIZE);
            return;
        }

        int num;
        sptr->vfo->get_sniffer_data(data, num);

        CALLBACK_ON_SUCCESS(data, num);
    });
}

void AsyncVfo::getRdsData(Callback<std::string, int> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        std::string message;
        int num;
        sptr->vfo->get_rds_data(message, num);

        CALLBACK_ON_SUCCESS(std::move(message), num);
    });
}
void AsyncVfo::startRdsDecoder(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (sptr->vfo->is_rds_decoder_active()) {
            CALLBACK_ON_ERROR(RDS_ALREADY_ACTIVE);
            return;
        }

        sptr->vfo->reset_rds_parser();
        sptr->vfo->start_rds_decoder();

        CALLBACK_ON_SUCCESS();

        sptr->stateChanged<RdsDecoderStarted>();
    });
}
void AsyncVfo::stopRdsDecoder(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        if (!sptr->vfo->is_rds_decoder_active()) {
            CALLBACK_ON_ERROR(RDS_ALREADY_INACTIVE);
            return;
        }

        sptr->vfo->reset_rds_parser();
        sptr->vfo->stop_rds_decoder();

        CALLBACK_ON_SUCCESS();

        sptr->stateChanged<RdsDecoderStopped>();
    });
}
void AsyncVfo::resetRdsParser(Callback<> callback)
{
    RETURN_IF_WORKER_BUSY();

    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        sptr->vfo->reset_rds_parser();

        CALLBACK_ON_SUCCESS();

        sptr->stateChanged<RdsParserReset>();
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

void AsyncVfo::prepareToDie(VfoRemoved event)
{
    m_removed = true;
    signalStateChanged(event);

    signalStateChanged.disconnect_all_slots();
}

std::vector<VfoEvent> AsyncVfo::getStateAsEvents() const
{
    std::vector<VfoEvent> result;
    forEachStateEvent([&](const VfoEvent& event) { result.push_back(event); });
    return result;
}

template <typename Lambda>
void AsyncVfo::forEachStateEvent(Lambda&& lambda) const
{
    // FIXME: Maybe somehow keep track of when each state changed.
    VfoEventCommon ec;
    ec.id = -1;
    ec.timestamp = {};
    ec.handle = getId();

    lambda(createEvent<DemodChanged>(ec));
    lambda(createEvent<OffsetChanged>(ec));
    lambda(createEvent<CwOffsetChanged>(ec));
    lambda(createEvent<FilterChanged>(ec));
    lambda(createEvent<NoiseBlankerOnChanged>(ec, 1, isNoiseBlanker1On()));
    lambda(createEvent<NoiseBlankerOnChanged>(ec, 2, isNoiseBlanker2On()));
    lambda(createEvent<NoiseBlankerThresholdChanged>(
        ec, 1, getNoiseBlanker1Threshold()));
    lambda(createEvent<NoiseBlankerThresholdChanged>(
        ec, 2, getNoiseBlanker2Threshold()));
    lambda(createEvent<SqlLevelChanged>(ec));
    lambda(createEvent<SqlAlphaChanged>(ec));
    lambda(createEvent<AgcOnChanged>(ec));
    lambda(createEvent<AgcHangChanged>(ec));
    lambda(createEvent<AgcThresholdChanged>(ec));
    lambda(createEvent<AgcSlopeChanged>(ec));
    lambda(createEvent<AgcDecayChanged>(ec));
    lambda(createEvent<AgcManualGainChanged>(ec));
    lambda(createEvent<FmMaxDevChanged>(ec));
    lambda(createEvent<FmDeemphChanged>(ec));
    lambda(createEvent<AmDcrChanged>(ec));
    lambda(createEvent<AmSyncDcrChanged>(ec));
    lambda(createEvent<AmSyncPllBwChanged>(ec));
    if (isAudioRecording()) {
        lambda(createEvent<RecordingStarted>(ec));
    }
    if (isSniffing()) {
        lambda(createEvent<SnifferStarted>(ec));
    }
    if (isUdpStreaming()) {
        lambda(createEvent<UdpStreamingStarted>(ec));
    }
    if (isRdsDecoderActive()) {
        lambda(createEvent<RdsDecoderStarted>(ec));
    }
    lambda(createEvent<AudioGainChanged>(ec));
}

void AsyncVfo::subscribe(VfoEventHandler handler, Callback<Connection> callback)
{
    std::weak_ptr<AsyncVfo> self =
        static_pointer_cast<AsyncVfo>(shared_from_this());

    schedule([self, handler = std::move(handler),
              callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->m_removed) {
            CALLBACK_ON_ERROR(VFO_NOT_FOUND);
            return;
        }

        Connection connection =
            handler ? sptr->signalStateChanged.connect(handler) : Connection{};

        CALLBACK_ON_SUCCESS(std::move(connection));

        if (handler) {
            VfoEventCommon ec;
            ec.id = -1;
            ec.timestamp = {};
            ec.handle = sptr->getId();

            handler(sptr->createEvent<VfoSyncStart>(ec));
            sptr->forEachStateEvent(handler);
            handler(sptr->createEvent<VfoSyncEnd>(ec));
        }
    });
}

void AsyncVfo::unsubscribe(const Connection& c) { c.disconnect(); }

} // namespace violetrx
