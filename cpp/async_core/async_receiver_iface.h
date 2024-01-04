#ifndef ASYNC_RECEIVER_IFACE_H
#define ASYNC_RECEIVER_IFACE_H

#include <cstdint>
#include <memory>
#include <string>

#include "async_core/events.h"
#include "async_core/types.h"

#include <boost/signals2.hpp>
#include <function2/function2.hpp>

namespace core
{

class AsyncReceiverIface
    : public std::enable_shared_from_this<AsyncReceiverIface>
{
public:
    using sptr = std::shared_ptr<AsyncReceiverIface>;

public:
    virtual ~AsyncReceiverIface() {}

    virtual Connection subscribe(ReceiverSubCallback) = 0;
    virtual void unsubscribe(const Connection&) = 0;

    virtual void start(Callback<> = {}) = 0;
    virtual void stop(Callback<> = {}) = 0;
    virtual void setInputDevice(std::string, Callback<> = {}) = 0;
    virtual void setAntenna(std::string, Callback<> = {}) = 0;
    virtual void setInputRate(int, Callback<int> = {}) = 0;
    virtual void setInputDecim(int, Callback<int> = {}) = 0;
    virtual void setIqSwap(bool, Callback<> = {}) = 0;
    virtual void setDcCancel(bool, Callback<> = {}) = 0;
    virtual void setIqBalance(bool, Callback<> = {}) = 0;
    virtual void setRfFreq(int64_t, Callback<int64_t> = {}) = 0;
    virtual void setAutoGain(bool, Callback<> = {}) = 0;
    virtual void setGain(std::string, double, Callback<double> = {}) = 0;
    virtual void setFreqCorr(double, Callback<double> = {}) = 0;

    /* I/Q FFT */
    virtual void setIqFftSize(int, Callback<> = {}) = 0;
    virtual void setIqFftWindow(WindowType, bool, Callback<> = {}) = 0;
    virtual void
    getIqFftData(float*, int,
                 Callback<Timestamp, int64_t, int, float*, int> = {}) = 0;

    /* I/Q Recording */
    virtual void startIqRecording(std::string, Callback<> = {}) = 0;
    virtual void stopIqRecording(Callback<> = {}) = 0;

    /* VFO channels */
    virtual void addVfoChannel(Callback<AsyncVfoIfaceSptr> = {}) = 0;
    virtual void removeVfoChannel(AsyncVfoIfaceSptr, Callback<> = {}) = 0;

    /* Sync API: getters can only be called inside a successful callback
     * function */
    virtual void synchronize(Callback<>) = 0;
    virtual bool isRunning() const = 0;
    virtual std::string getInputDevice() const = 0;
    virtual std::string getAntenna() const = 0;
    virtual int getInputRate() const = 0;
    virtual int getInputDecim() const = 0;
    virtual bool getDcCancel() const = 0;
    virtual bool getIqBalance() const = 0;
    virtual bool getIqSwap() const = 0;
    virtual int64_t getRfFreq() const = 0;
    virtual std::vector<GainStage> getGainStages() const = 0;
    virtual std::vector<std::string> getAntennas() const = 0;
    virtual bool getAutoGain() const = 0;
    virtual double getFreqCorr() const = 0;
    virtual int getIqFftSize() const = 0;
    virtual WindowType getIqFftWindow() const = 0;
    virtual bool isIqFftWindowNormalized() const = 0;
    virtual std::vector<std::shared_ptr<AsyncVfoIface>> getVfos() const = 0;
    virtual AsyncVfoIfaceSptr getVfo(uint64_t handle) const = 0;
    virtual std::string iqRecordingPath() const = 0;

protected:
    Signal<void(const ReceiverEvent&)> signalStateChanged;
};

} // namespace core

#endif // ASYNC_RECEIVER_IFACE_H
