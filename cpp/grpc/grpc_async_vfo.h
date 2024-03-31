#ifndef VIOLETRX_GRPC_ASYNC_VFO_H
#define VIOLETRX_GRPC_ASYNC_VFO_H

#include <memory>
#include <source_location>
#include <string>

#include "async_core/async_vfo_iface.h"

namespace violetrx
{

class GrpcClient;
class GrpcAsyncReceiver;
class WorkerThread;

class GrpcAsyncVfo : public AsyncVfoIface
{
    friend GrpcAsyncReceiver;

public:
    using sptr = std::shared_ptr<GrpcAsyncVfo>;

public:
    static sptr make(uint64_t handle, std::shared_ptr<GrpcClient>,
                     std::shared_ptr<WorkerThread>);
    GrpcAsyncVfo(uint64_t handle, std::shared_ptr<GrpcClient>,
                 std::shared_ptr<WorkerThread>);
    ~GrpcAsyncVfo() override;

    void subscribe(VfoEventHandler, Callback<Connection>) override;
    void unsubscribe(const Connection&) override;

    /* filter */
    void setFilterOffset(int64_t, Callback<> = {}) override;
    void setFilter(int64_t, int64_t, FilterShape, Callback<> = {}) override;
    void setCwOffset(int64_t, Callback<> = {}) override;
    void setDemod(Demod, Callback<> = {}) override;
    void getSignalPwr(Callback<float> = {}) override;

    /* Noise blanker */
    void setNoiseBlanker(int, bool, Callback<> = {}) override;
    void setNoiseBlankerThreshold(int, float, Callback<> = {}) override;

    /* Sql parameter */
    void setSqlLevel(double, Callback<> = {}) override;
    void setSqlAlpha(double, Callback<> = {}) override;

    /* AGC */
    void setAgcOn(bool, Callback<> = {}) override;
    void setAgcHang(bool, Callback<> = {}) override;
    void setAgcThreshold(int, Callback<> = {}) override;
    void setAgcSlope(int, Callback<> = {}) override;
    void setAgcDecay(int, Callback<> = {}) override;
    void setAgcManualGain(int, Callback<> = {}) override;

    /* FM parameters */
    void setFmMaxDev(float, Callback<> = {}) override;
    void setFmDeemph(double, Callback<> = {}) override;

    /* AM parameters */
    void setAmDcr(bool, Callback<> = {}) override;

    /* AM-Sync parameters */
    void setAmSyncDcr(bool, Callback<> = {}) override;
    void setAmSyncPllBw(float, Callback<> = {}) override;

    /* Audio Recording */
    void startAudioRecording(const std::string&, Callback<> = {}) override;
    void stopAudioRecording(Callback<> = {}) override;
    void setAudioGain(float, Callback<> = {}) override;

    /* UDP streaming */
    void startUdpStreaming(const std::string&, int, bool,
                           Callback<> = {}) override;
    void stopUdpStreaming(Callback<> = {}) override;

    /* sample sniffer */
    void startSniffer(int, int, Callback<> = {}) override;
    void stopSniffer(Callback<> = {}) override;
    void getSnifferData(float*, int, Callback<float*, int> = {}) override;

    /* rds functions */
    void getRdsData(Callback<std::string, int> = {}) override;
    void startRdsDecoder(Callback<> = {}) override;
    void stopRdsDecoder(Callback<> = {}) override;
    void resetRdsParser(Callback<> = {}) override;

    /* Sync API: getters can only be called inside a successful callback
     * function */
    void synchronize(Callback<>) override;
    Demod getDemod() const override;
    int32_t getOffset() const override;
    Filter getFilter() const override;
    int32_t getCwOffset() const override;
    float getFmMaxDev() const override;
    double getFmDeemph() const override;
    bool getAmDcr() const override;
    bool getAmSyncDcr() const override;
    float getAmSyncPllBw() const override;
    bool isAudioRecording() const override;
    std::string getRecordingFilename() const override;
    bool isUdpStreaming() const override;
    bool isSniffing() const override;
    bool isRdsDecoderActive() const override;
    bool isAgcOn() const override;
    bool isAgcHangOn() const override;
    int getAgcThreshold() const override;
    int getAgcSlope() const override;
    int getAgcDecay() const override;
    int getAgcManualGain() const override;
    double getSqlLevel() const override;
    double getSqlAlpha() const override;
    bool isNoiseBlanker1On() const override;
    bool isNoiseBlanker2On() const override;
    float getNoiseBlanker1Threshold() const override;
    float getNoiseBlanker2Threshold() const override;
    float getAfGain() const override;
    UdpStreamParams getUdpStreamParams() const override;
    SnifferParams getSnifferParams() const override;
    std::vector<VfoEvent> getStateAsEvents() const override;

    uint64_t getId() const override;

private:
    void prepareToDie(VfoRemoved);
    void onEvent(const VfoEvent&);

private:
    template <typename Function>
    auto schedule(Function&& func,
                  const std::source_location = std::source_location::current());

    template <typename Lambda>
    void forEachStateEvent(Lambda&&) const;

private:
    uint64_t handle_;
    std::shared_ptr<GrpcClient> client_;
    std::shared_ptr<WorkerThread> worker_thread_;

    Demod demod_;

    // Filter params
    int32_t offset_;
    int32_t cw_offset_;
    FilterShape filter_shape_;
    int32_t filter_low_;
    int32_t filter_high_;

    // Recording params
    bool is_audio_recording_;
    std::string recording_path_;

    // FM parameters
    float fm_maxdev_;
    double fm_deemph_;

    // AM parameters
    bool am_dcr_;

    // AM-Sync parameters
    bool am_sync_dcr_;
    float am_sync_pll_bw_;

    // agc
    bool agc_on_;
    bool agc_hang_;
    int agc_threshold_;
    int agc_slope_;
    int agc_decay_;
    int agc_manual_gain_;

    // squelch
    double sql_level_;
    double sql_alpha_;

    // noise blanker
    bool nb1_on_;
    float nb1_threshold_;

    bool nb2_on_;
    float nb2_threshold_;

    // sniffer
    bool is_sniffing_;
    int sniffer_sample_rate_;
    int sniffer_buff_size_;

    // udp streaming
    bool is_udp_streaming_;
    std::string udp_host_;
    int udp_port_;
    bool udp_stereo_;

    // rds
    bool is_rds_decoder_active_;

    bool removed_;
};

} // namespace violetrx

#endif // VIOLETRX_GRPC_ASYNC_VFO_H
