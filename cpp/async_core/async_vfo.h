#ifndef ASYNC_VFO
#define ASYNC_VFO

#include "async_core/async_vfo_iface.h"
#include "core/vfo_channel.h"

namespace violetrx
{
class WorkerThread;
class AsyncReceiver;

class AsyncVfo : public AsyncVfoIface
{
    friend AsyncReceiver;

public:
    using sptr = std::shared_ptr<AsyncVfo>;

public:
    static sptr make(vfo_channel::sptr, std::shared_ptr<WorkerThread>);
    AsyncVfo(vfo_channel::sptr, std::shared_ptr<WorkerThread>);
    ~AsyncVfo() override;

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

    FilterRange getFilterRange(Demod d) const override;

    vfo_channel::sptr inner() { return vfo; }

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

    uint64_t getId() const override;

private:
    bool isValidFilter(int64_t low, int64_t high);
    void setDefaultFilter();
    void prepareToDie();

private:
    template <typename Function>
    auto schedule(Function&& func,
                  const std::source_location = std::source_location::current());

    template <typename Event, typename... Args>
    Event createEvent(VfoEventCommon, Args...);

    template <typename Event, typename... Args>
    void stateChanged(Args... args);

private:
    vfo_channel::sptr vfo;
    std::shared_ptr<WorkerThread> workerThread;

    Demod m_demod;

    // FM parameters
    float m_fmMaxDev;
    double m_fmDeemph;

    // AM parameters
    bool m_amDcr;

    // AM-Sync parameters
    bool m_amSyncDcr;
    float m_amSyncPllBw;

    // agc
    bool m_agcOn;
    bool m_agcHang;
    int m_agcThreshold;
    int m_agcSlope;
    int m_agcDecay;
    int m_agcManualGain;

    // squelch
    double m_sqlLevel;
    double m_sqlAlpha;

    // noise blanker
    bool m_nb1On;
    float m_nb1Threshold;

    bool m_nb2On;
    float m_nb2Threshold;

    bool m_removed;
};
} // namespace violetrx

#endif
