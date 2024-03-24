#ifndef ASYNC_RECEIVER_H
#define ASYNC_RECEIVER_H

#include <memory>

#include <source_location>

#include "async_core/async_receiver_iface.h"
#include "async_core/events.h"
#include "core/receiver.h"

namespace violetrx
{
class WorkerThread;

class AsyncVfo;

class AsyncReceiver : public AsyncReceiverIface
{
public:
    static sptr make();
    AsyncReceiver();
    ~AsyncReceiver() override;

    void subscribe(ReceiverEventHandler, Callback<Connection>) override;
    void unsubscribe(const Connection&) override;

    void start(Callback<> = {}) override;
    void stop(Callback<> = {}) override;
    void setInputDevice(std::string, Callback<> = {}) override;
    void setAntenna(std::string, Callback<> = {}) override;
    void setInputRate(int, Callback<int> = {}) override;
    void setInputDecim(int, Callback<int> = {}) override;
    void setRfFreq(int64_t, Callback<int64_t> = {}) override;
    void setIqSwap(bool, Callback<> = {}) override;
    void setDcCancel(bool, Callback<> = {}) override;
    void setIqBalance(bool, Callback<> = {}) override;
    void setAutoGain(bool, Callback<> = {}) override;
    void setGain(std::string, double, Callback<double> = {}) override;
    void setFreqCorr(double, Callback<double> = {}) override;

    /* I/Q FFT */
    void setIqFftSize(int, Callback<> = {}) override;
    void setIqFftWindow(WindowType, bool, Callback<> = {}) override;
    void
    getIqFftData(float*, int,
                 Callback<Timestamp, int64_t, int, float*, int> = {}) override;

    /* I/Q Recording */
    void startIqRecording(std::string, Callback<> = {}) override;
    void stopIqRecording(Callback<> = {}) override;

    /* VFO channels */
    void addVfoChannel(Callback<AsyncVfoIfaceSptr> = {}) override;
    void removeVfoChannel(AsyncVfoIfaceSptr, Callback<> = {}) override;
    void removeVfoChannel(uint64_t, Callback<> = {}) override;

    /* Sync API: getters can only be called inside a successful synchronize
     * function */
    void synchronize(Callback<>) override;
    bool isRunning() const override;
    std::string getInputDevice() const override;
    std::string getAntenna() const override;
    int getInputRate() const override;
    int getInputDecim() const override;
    bool getDcCancel() const override;
    bool getIqBalance() const override;
    bool getIqSwap() const override;
    int64_t getRfFreq() const override;
    std::vector<GainStage> getGainStages() const override;
    std::vector<std::string> getAntennas() const override;
    bool getAutoGain() const override;
    double getFreqCorr() const override;
    int getIqFftSize() const override;
    WindowType getIqFftWindow() const override;
    bool isIqFftWindowNormalized() const override;
    std::vector<std::shared_ptr<AsyncVfoIface>> getVfos() const override;
    AsyncVfoIfaceSptr getVfo(uint64_t handle) const override;
    std::string iqRecordingPath() const override;

private:
    template <typename Function>
    auto schedule(Function&& func,
                  const std::source_location = std::source_location::current());

    template <typename Event, typename... Args>
    Event createEvent(EventCommon, Args...);

    template <typename Event, typename... Args>
    void stateChanged(Args... args);

    void removeVfoChannelImpl(std::shared_ptr<AsyncVfo>, Callback<>);

private:
    receiver::sptr rx;
    std::vector<std::shared_ptr<AsyncVfo>> vfos;
    std::shared_ptr<WorkerThread> workerThread;
};

} // namespace violetrx

#endif
