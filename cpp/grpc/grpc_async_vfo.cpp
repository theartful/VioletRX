#include "grpc_async_vfo.h"
#include "client.h"
#include "utility/worker_thread.h"

namespace violetrx
{

#define INVOKE(callback, ...)                                                  \
    if (callback) {                                                            \
        callback(__VA_ARGS__);                                                 \
    }

GrpcAsyncVfo::sptr
GrpcAsyncVfo::make(uint64_t handle, std::shared_ptr<GrpcClient> client,
                   std::shared_ptr<WorkerThread> worker_thread)
{
    return std::make_shared<GrpcAsyncVfo>(handle, std::move(client),
                                          std::move(worker_thread));
}

GrpcAsyncVfo::GrpcAsyncVfo(uint64_t handle, std::shared_ptr<GrpcClient> client,
                           std::shared_ptr<WorkerThread> worker_thread) :
    handle_{handle},
    client_{std::move(client)},
    worker_thread_{std::move(worker_thread)},
    demod_{Demod::OFF},
    offset_{0},
    cw_offset_{0},
    filter_shape_{FilterShape::NORMAL},
    filter_low_{0},
    filter_high_{0},
    is_audio_recording_{false},
    recording_path_{},
    fm_maxdev_{0},
    fm_deemph_{0},
    am_dcr_{false},
    am_sync_dcr_{false},
    am_sync_pll_bw_{0.0},
    agc_on_{false},
    agc_hang_{false},
    agc_threshold_{false},
    agc_slope_{0},
    agc_decay_{0},
    agc_manual_gain_{0},
    sql_level_{0.0},
    sql_alpha_{0.0},
    nb1_on_{false},
    nb1_threshold_{0.0},
    nb2_on_{false},
    nb2_threshold_{0.0},
    is_sniffing_{false},
    sniffer_sample_rate_{0},
    sniffer_buff_size_{0},
    is_udp_streaming_{false},
    udp_host_{},
    udp_port_{0},
    udp_stereo_{false},
    is_rds_decoder_active_{false},
    removed_{false}
{
    //
}

template <typename Function>
auto GrpcAsyncVfo::schedule(Function&& func, const std::source_location loc)
{
    return worker_thread_->scheduleForced(loc.function_name(),
                                          std::forward<Function>(func));
}

void GrpcAsyncVfo::subscribe(VfoEventHandler handler,
                             Callback<Connection> callback)
{
    std::weak_ptr<GrpcAsyncVfo> self =
        static_pointer_cast<GrpcAsyncVfo>(shared_from_this());

    schedule([self, handler = std::move(handler),
              callback = std::move(callback)]() mutable {
        auto sptr = self.lock();
        if (!sptr || sptr->removed_) {
            callback(violetrx::ErrorCode::VFO_NOT_FOUND, {});
            return;
        }

        Connection connection = {};
        if (handler) {
            connection = sptr->signalStateChanged.connect(handler);
        }
        INVOKE(callback, violetrx::ErrorCode::OK, std::move(connection));

        VfoEventCommon ec;
        ec.id = -1;
        ec.timestamp = {};
        ec.handle = sptr->getId();

        handler(VfoSyncStart{ec});
        sptr->forEachStateEvent(handler);
        handler(VfoSyncEnd{ec});
    });
}

template <typename Lambda>
void GrpcAsyncVfo::forEachStateEvent(Lambda&& lambda) const
{
    // FIXME: Maybe somehow keep track of when each state changed.
    VfoEventCommon ec;
    ec.id = -1;
    ec.timestamp = Timestamp::Now();
    ec.handle = getId();

    lambda(DemodChanged{ec, getDemod()});
    lambda(OffsetChanged{ec, getOffset()});
    lambda(CwOffsetChanged{ec, getCwOffset()});
    lambda(FilterChanged::fromFilter(ec, getFilter()));
    lambda(NoiseBlankerOnChanged{ec, 1, isNoiseBlanker1On()});
    lambda(NoiseBlankerOnChanged{ec, 2, isNoiseBlanker2On()});
    lambda(NoiseBlankerThresholdChanged{ec, 1, getNoiseBlanker1Threshold()});
    lambda(NoiseBlankerThresholdChanged{ec, 2, getNoiseBlanker2Threshold()});
    lambda(SqlLevelChanged{ec, getSqlLevel()});
    lambda(SqlAlphaChanged{ec, getSqlAlpha()});
    lambda(AgcOnChanged{ec, isAgcOn()});
    lambda(AgcHangChanged{ec, isAgcHangOn()});
    lambda(AgcThresholdChanged{ec, getAgcThreshold()});
    lambda(AgcSlopeChanged{ec, getAgcSlope()});
    lambda(AgcDecayChanged{ec, getAgcDecay()});
    lambda(AgcManualGainChanged{ec, getAgcManualGain()});
    lambda(FmMaxDevChanged{ec, getFmMaxDev()});
    lambda(FmDeemphChanged{ec, getFmDeemph()});
    lambda(AmDcrChanged{ec, getAmDcr()});
    lambda(AmSyncDcrChanged{ec, getAmSyncDcr()});
    lambda(AmSyncPllBwChanged{ec, getAmSyncPllBw()});
    if (isAudioRecording()) {
        lambda(RecordingStarted{ec, getRecordingFilename()});
    }
    if (isSniffing()) {
        // TODO
        lambda(SnifferStarted{ec, sniffer_sample_rate_, sniffer_buff_size_});
    }
    if (isUdpStreaming()) {
        lambda(UdpStreamingStarted{ec, udp_host_, udp_port_, udp_stereo_});
    }
    if (isRdsDecoderActive()) {
        lambda(RdsDecoderStarted{ec});
    }
    // TODO: Handle audio gain client side
    // lambda(AudioGainChanged{ec});
}

void GrpcAsyncVfo::setFilterOffset(int64_t offset, Callback<> callback)
{
    client_->SetFilterOffset(handle_, offset, std::move(callback));
}

void GrpcAsyncVfo::setFilter(int64_t low, int64_t high, FilterShape shape,
                             Callback<> callback)
{
    client_->SetFilter(handle_, low, high, shape, std::move(callback));
}

void GrpcAsyncVfo::setCwOffset(int64_t offset, Callback<> callback)
{
    client_->SetCwOffset(handle_, offset, std::move(callback));
}

void GrpcAsyncVfo::setDemod(Demod demod, Callback<> callback)
{
    client_->SetDemod(handle_, demod, std::move(callback));
}

void GrpcAsyncVfo::getSignalPwr(Callback<float> callback)
{
    client_->GetSignalPwr(handle_, std::move(callback));
}

void GrpcAsyncVfo::setNoiseBlanker(int id, bool enabled, Callback<> callback)
{
    client_->SetNoiseBlanker(handle_, id, enabled, std::move(callback));
}

void GrpcAsyncVfo::setNoiseBlankerThreshold(int id, float threshold,
                                            Callback<> callback)
{
    client_->SetNoiseBlankerThreshold(handle_, id, threshold,
                                      std::move(callback));
}

void GrpcAsyncVfo::setSqlLevel(double lvl, Callback<> callback)
{
    client_->SetSqlLevel(handle_, lvl, std::move(callback));
}

void GrpcAsyncVfo::setSqlAlpha(double alpha, Callback<> callback)
{
    client_->SetSqlAlpha(handle_, alpha, std::move(callback));
}

void GrpcAsyncVfo::setAgcOn(bool enabled, Callback<> callback)
{
    client_->SetAgcOn(handle_, enabled, std::move(callback));
}

void GrpcAsyncVfo::setAgcHang(bool enabled, Callback<> callback)
{
    client_->SetAgcHang(handle_, enabled, std::move(callback));
}

void GrpcAsyncVfo::setAgcThreshold(int threshold, Callback<> callback)
{
    client_->SetAgcThreshold(handle_, threshold, std::move(callback));
}

void GrpcAsyncVfo::setAgcSlope(int slope, Callback<> callback)
{
    client_->SetAgcSlope(handle_, slope, std::move(callback));
}

void GrpcAsyncVfo::setAgcDecay(int decay, Callback<> callback)
{
    client_->SetAgcDecay(handle_, decay, std::move(callback));
}

void GrpcAsyncVfo::setAgcManualGain(int gain, Callback<> callback)
{
    client_->SetAgcManualGain(handle_, gain, std::move(callback));
}

void GrpcAsyncVfo::setFmMaxDev(float maxdev, Callback<> callback)
{
    client_->SetFmMaxDev(handle_, maxdev, std::move(callback));
}

void GrpcAsyncVfo::setFmDeemph(double tau, Callback<> callback)
{
    client_->SetFmMaxDev(handle_, tau, std::move(callback));
}

void GrpcAsyncVfo::setAmDcr(bool enabled, Callback<> callback)
{
    client_->SetAmDcr(handle_, enabled, std::move(callback));
}

void GrpcAsyncVfo::setAmSyncDcr(bool enabled, Callback<> callback)
{
    client_->SetAmSyncDcr(handle_, enabled, std::move(callback));
}

void GrpcAsyncVfo::setAmSyncPllBw(float bw, Callback<> callback)
{
    client_->SetAmSyncPllBw(handle_, bw, std::move(callback));
}

void GrpcAsyncVfo::startAudioRecording(const std::string& path,
                                       Callback<> callback)
{
    client_->StartAudioRecording(handle_, path, std::move(callback));
}

void GrpcAsyncVfo::stopAudioRecording(Callback<> callback)
{
    client_->StopAudioRecording(handle_, std::move(callback));
}

void GrpcAsyncVfo::setAudioGain(float /* gain */, Callback<> callback)
{
    INVOKE(callback, ErrorCode::UNIMPLEMENTED);
}

void GrpcAsyncVfo::startUdpStreaming(const std::string&, int, bool,
                                     Callback<> callback)
{
    INVOKE(callback, ErrorCode::UNIMPLEMENTED);
}

void GrpcAsyncVfo::stopUdpStreaming(Callback<> callback)
{
    INVOKE(callback, ErrorCode::UNIMPLEMENTED);
}

void GrpcAsyncVfo::startSniffer(int sample_rate, int buff_size,
                                Callback<> callback)
{
    client_->StartSniffer(handle_, sample_rate, buff_size, std::move(callback));
}

void GrpcAsyncVfo::stopSniffer(Callback<> callback)
{
    client_->StopSniffer(handle_, std::move(callback));
}

void GrpcAsyncVfo::getSnifferData(float* data, int buffsize,
                                  Callback<float*, int> callback)
{
    client_->GetSnifferData(handle_, data, buffsize, std::move(callback));
}

void GrpcAsyncVfo::getRdsData(Callback<std::string, int> callback)
{

    client_->GetRdsData(handle_, std::move(callback));
}

void GrpcAsyncVfo::startRdsDecoder(Callback<> callback)
{
    client_->StartRdsDecoder(handle_, std::move(callback));
}

void GrpcAsyncVfo::stopRdsDecoder(Callback<> callback)
{
    client_->StopRdsDecoder(handle_, std::move(callback));
}

void GrpcAsyncVfo::resetRdsParser(Callback<> callback)
{
    client_->ResetRdsParser(handle_, std::move(callback));
}

uint64_t GrpcAsyncVfo::getId() const { return handle_; }
Demod GrpcAsyncVfo::getDemod() const { return demod_; }
int32_t GrpcAsyncVfo::getOffset() const { return offset_; }
Filter GrpcAsyncVfo::getFilter() const
{
    return Filter{
        .shape = filter_shape_, .low = filter_low_, .high = filter_high_};
}
int32_t GrpcAsyncVfo::getCwOffset() const { return cw_offset_; }
float GrpcAsyncVfo::getFmMaxDev() const { return fm_maxdev_; }
double GrpcAsyncVfo::getFmDeemph() const { return fm_deemph_; }
bool GrpcAsyncVfo::getAmDcr() const { return am_dcr_; }
bool GrpcAsyncVfo::getAmSyncDcr() const { return am_sync_dcr_; }
float GrpcAsyncVfo::getAmSyncPllBw() const { return am_sync_pll_bw_; }
bool GrpcAsyncVfo::isAudioRecording() const { return is_audio_recording_; }
std::string GrpcAsyncVfo::getRecordingFilename() const
{
    return recording_path_;
}
bool GrpcAsyncVfo::isUdpStreaming() const { return is_udp_streaming_; }
bool GrpcAsyncVfo::isSniffing() const { return is_sniffing_; }
bool GrpcAsyncVfo::isRdsDecoderActive() const { return is_rds_decoder_active_; }
bool GrpcAsyncVfo::isAgcOn() const { return agc_on_; }
bool GrpcAsyncVfo::isAgcHangOn() const { return agc_hang_; }
int GrpcAsyncVfo::getAgcThreshold() const { return agc_threshold_; }
int GrpcAsyncVfo::getAgcSlope() const { return agc_slope_; }
int GrpcAsyncVfo::getAgcDecay() const { return agc_decay_; }
int GrpcAsyncVfo::getAgcManualGain() const { return agc_manual_gain_; }
double GrpcAsyncVfo::getSqlLevel() const { return sql_level_; }
double GrpcAsyncVfo::getSqlAlpha() const { return sql_alpha_; }
bool GrpcAsyncVfo::isNoiseBlanker1On() const { return nb1_on_; }
bool GrpcAsyncVfo::isNoiseBlanker2On() const { return nb2_on_; }
float GrpcAsyncVfo::getNoiseBlanker1Threshold() const { return nb1_threshold_; }
float GrpcAsyncVfo::getNoiseBlanker2Threshold() const { return nb2_threshold_; }
float GrpcAsyncVfo::getAfGain() const { return 1.0f; }
UdpStreamParams GrpcAsyncVfo::getUdpStreamParams() const
{
    return UdpStreamParams{
        .host = udp_host_, .port = udp_port_, .stereo = udp_stereo_};
}
SnifferParams GrpcAsyncVfo::getSnifferParams() const
{
    return SnifferParams{.sampleRate = sniffer_sample_rate_,
                         .buffSize = sniffer_buff_size_};
}
std::vector<VfoEvent> GrpcAsyncVfo::getStateAsEvents() const
{
    std::vector<VfoEvent> result;
    forEachStateEvent([&](const VfoEvent& event) { result.push_back(event); });
    return result;
}

void GrpcAsyncVfo::unsubscribe(const Connection& c) { c.disconnect(); }

void GrpcAsyncVfo::prepareToDie(VfoRemoved event)
{
    removed_ = true;
    signalStateChanged(event);

    signalStateChanged.disconnect_all_slots();
}

void GrpcAsyncVfo::onEvent(const VfoEvent& event)
{
    std::visit(
        Visitor{
            [&](const VfoSyncStart&) {
                // Do nothing.
            },
            [&](const VfoSyncEnd&) {
                // Do nothing.
            },
            [&](const VfoRemoved&) {},
            [&](const DemodChanged& ev) { demod_ = ev.demod; },
            [&](const OffsetChanged& ev) { offset_ = ev.offset; },
            [&](const CwOffsetChanged& ev) { cw_offset_ = ev.offset; },
            [&](const FilterChanged& ev) {
                filter_shape_ = ev.shape;
                filter_low_ = ev.low;
                filter_high_ = ev.high;
            },
            [&](const NoiseBlankerOnChanged& ev) {
                if (ev.id == 1) {
                    nb1_on_ = ev.enabled;
                } else if (ev.id == 2) {
                    nb2_on_ = ev.enabled;
                }
            },
            [&](const NoiseBlankerThresholdChanged& ev) {
                if (ev.id == 1) {
                    nb1_threshold_ = ev.threshold;
                } else if (ev.id == 2) {
                    nb2_threshold_ = ev.threshold;
                }
            },
            [&](const SqlLevelChanged& ev) { sql_level_ = ev.level; },
            [&](const SqlAlphaChanged& ev) { sql_alpha_ = ev.alpha; },
            [&](const AgcOnChanged& ev) { agc_on_ = ev.enabled; },
            [&](const AgcHangChanged& ev) { agc_hang_ = ev.enabled; },
            [&](const AgcThresholdChanged& ev) {
                agc_threshold_ = ev.threshold;
            },
            [&](const AgcSlopeChanged& ev) { agc_slope_ = ev.slope; },
            [&](const AgcDecayChanged& ev) { agc_decay_ = ev.decay; },
            [&](const AgcManualGainChanged& ev) { agc_manual_gain_ = ev.gain; },
            [&](const FmMaxDevChanged& ev) { fm_maxdev_ = ev.maxdev; },
            [&](const FmDeemphChanged& ev) { fm_deemph_ = ev.tau; },
            [&](const AmDcrChanged& ev) { am_dcr_ = ev.enabled; },
            [&](const AmSyncDcrChanged& ev) { am_sync_dcr_ = ev.enabled; },
            [&](const AmSyncPllBwChanged& ev) { am_sync_pll_bw_ = ev.bw; },
            [&](const RecordingStarted& ev) {
                is_audio_recording_ = true;
                recording_path_ = ev.path;
            },
            [&](const RecordingStopped&) {
                is_audio_recording_ = false;
                // recording_path_ = {};
            },
            [&](const SnifferStarted& ev) {
                sniffer_buff_size_ = ev.size;
                sniffer_sample_rate_ = ev.sample_rate;
                is_sniffing_ = true;
            },
            [&](const SnifferStopped&) { is_sniffing_ = false; },
            [&](const UdpStreamingStarted& ev) {
                udp_stereo_ = ev.stereo;
                udp_host_ = ev.host;
                udp_port_ = ev.port;
                is_udp_streaming_ = true;
            },
            [&](const UdpStreamingStopped&) { is_udp_streaming_ = false; },
            [&](const RdsDecoderStarted&) { is_rds_decoder_active_ = true; },
            [&](const RdsDecoderStopped&) { is_rds_decoder_active_ = false; },
            [&](const RdsParserReset&) {},
            [&](const AudioGainChanged&) {
                // FIXME: Audio should be client side
            },
        },
        event);

    if (std::holds_alternative<VfoRemoved>(event)) {
        prepareToDie(std::get<VfoRemoved>(event));
    } else {
        signalStateChanged(event);
    }
}

GrpcAsyncVfo::~GrpcAsyncVfo() {}

} // namespace violetrx
