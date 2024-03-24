#ifndef ASYNC_VFO_CHANNEL_IFACE_H
#define ASYNC_VFO_CHANNEL_IFACE_H

#include <cstdint>
#include <memory>

#include "async_core/events.h"
#include "async_core/types.h"

namespace violetrx
{

class AsyncVfoIface : public std::enable_shared_from_this<AsyncVfoIface>
{
public:
    using sptr = std::shared_ptr<AsyncVfoIface>;

public:
    virtual ~AsyncVfoIface() {}

    virtual void subscribe(VfoEventHandler, Callback<Connection>) = 0;
    virtual void unsubscribe(const Connection&) = 0;

    /* filter */
    virtual void setFilterOffset(int64_t, Callback<> = {}) = 0;
    virtual void setFilter(int64_t, int64_t, FilterShape, Callback<> = {}) = 0;
    virtual void setCwOffset(int64_t, Callback<> = {}) = 0;
    virtual void setDemod(Demod, Callback<> = {}) = 0;
    virtual void getSignalPwr(Callback<float> = {}) = 0;

    /* Noise blanker */
    virtual void setNoiseBlanker(int, bool, Callback<> = {}) = 0;
    virtual void setNoiseBlankerThreshold(int, float, Callback<> = {}) = 0;

    /* Sql parameter */
    virtual void setSqlLevel(double, Callback<> = {}) = 0;
    virtual void setSqlAlpha(double, Callback<> = {}) = 0;

    /* AGC */
    virtual void setAgcOn(bool, Callback<> = {}) = 0;
    virtual void setAgcHang(bool, Callback<> = {}) = 0;
    virtual void setAgcThreshold(int, Callback<> = {}) = 0;
    virtual void setAgcSlope(int, Callback<> = {}) = 0;
    virtual void setAgcDecay(int, Callback<> = {}) = 0;
    virtual void setAgcManualGain(int, Callback<> = {}) = 0;

    /* FM parameters */
    virtual void setFmMaxDev(float, Callback<> = {}) = 0;
    virtual void setFmDeemph(double, Callback<> = {}) = 0;

    /* AM parameters */
    virtual void setAmDcr(bool, Callback<> = {}) = 0;

    /* AM-Sync parameters */
    virtual void setAmSyncDcr(bool, Callback<> = {}) = 0;
    virtual void setAmSyncPllBw(float, Callback<> = {}) = 0;

    /* Audio Recording */
    virtual void startAudioRecording(const std::string&, Callback<> = {}) = 0;
    virtual void stopAudioRecording(Callback<> = {}) = 0;
    virtual void setAudioGain(float, Callback<> = {}) = 0;

    /* UDP streaming */
    virtual void startUdpStreaming(const std::string&, int, bool,
                                   Callback<> = {}) = 0;
    virtual void stopUdpStreaming(Callback<> = {}) = 0;

    /* sample sniffer */
    virtual void startSniffer(int, int, Callback<> = {}) = 0;
    virtual void stopSniffer(Callback<> = {}) = 0;
    virtual void getSnifferData(float*, int, Callback<float*, int> = {}) = 0;

    /* rds functions */
    virtual void getRdsData(Callback<std::string, int> = {}) = 0;
    virtual void startRdsDecoder(Callback<> = {}) = 0;
    virtual void stopRdsDecoder(Callback<> = {}) = 0;
    virtual void resetRdsParser(Callback<> = {}) = 0;

    /* Sync API: getters can only be called inside a successful callback
     * function */
    virtual void synchronize(Callback<>) = 0;
    virtual Demod getDemod() const = 0;
    virtual int32_t getOffset() const = 0;
    virtual Filter getFilter() const = 0;
    virtual int32_t getCwOffset() const = 0;
    virtual float getFmMaxDev() const = 0;
    virtual double getFmDeemph() const = 0;
    virtual bool getAmDcr() const = 0;
    virtual bool getAmSyncDcr() const = 0;
    virtual float getAmSyncPllBw() const = 0;
    virtual bool isAudioRecording() const = 0;
    virtual std::string getRecordingFilename() const = 0;
    virtual bool isUdpStreaming() const = 0;
    virtual bool isSniffing() const = 0;
    virtual bool isRdsDecoderActive() const = 0;
    virtual bool isAgcOn() const = 0;
    virtual bool isAgcHangOn() const = 0;
    virtual int getAgcThreshold() const = 0;
    virtual int getAgcSlope() const = 0;
    virtual int getAgcDecay() const = 0;
    virtual int getAgcManualGain() const = 0;
    virtual double getSqlLevel() const = 0;
    virtual double getSqlAlpha() const = 0;
    virtual bool isNoiseBlanker1On() const = 0;
    virtual bool isNoiseBlanker2On() const = 0;
    virtual float getNoiseBlanker1Threshold() const = 0;
    virtual float getNoiseBlanker2Threshold() const = 0;
    virtual float getAfGain() const = 0;
    virtual UdpStreamParams getUdpStreamParams() const = 0;
    virtual SnifferParams getSnifferParams() const = 0;

    virtual uint64_t getId() const = 0;
    // This doesn't seem right
    virtual FilterRange getFilterRange(Demod d) const = 0;

protected:
    Signal<void(const VfoEvent&)> signalStateChanged;
};

} // namespace violetrx
#endif
