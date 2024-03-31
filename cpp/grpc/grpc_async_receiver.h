#ifndef VIOLET_RX_GRPC_ASYNC_RECEIVER_H
#define VIOLET_RX_GRPC_ASYNC_RECEIVER_H

#include <memory>
#include <source_location>
#include <string>
#include <vector>

#include "async_core/async_receiver.h"
#include "grpc/client.h"
#include "utility/worker_thread.h"

namespace violetrx
{

class GrpcAsyncVfo;

class GrpcAsyncReceiver : public AsyncReceiverIface
{
public:
    static sptr make(std::string uri);
    GrpcAsyncReceiver(std::string uri);
    ~GrpcAsyncReceiver() override;

    void subscribe(ReceiverEventHandler, Callback<Connection>) override;
    void unsubscribe(const Connection&) override;

    void start(Callback<> = {}) override;
    void stop(Callback<> = {}) override;
    void setInputDevice(std::string, Callback<> = {}) override;
    void setAntenna(std::string, Callback<> = {}) override;
    void setInputRate(int, Callback<int> = {}) override;
    void setInputDecim(int, Callback<int> = {}) override;
    void setIqSwap(bool, Callback<> = {}) override;
    void setDcCancel(bool, Callback<> = {}) override;
    void setIqBalance(bool, Callback<> = {}) override;
    void setRfFreq(int64_t, Callback<int64_t> = {}) override;
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

    /* Sync API: getters can only be called inside a successful callback
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
    std::vector<std::shared_ptr<AsyncVfoIface>> getVfos() const override;
    AsyncVfoIfaceSptr getVfo(uint64_t handle) const override;
    std::string getIqRecordingPath() const override;
    bool isIqRecording() const override;
    std::vector<ReceiverEvent> getStateAsEvents() const override;

private:
    template <typename Function>
    auto schedule(Function&& func,
                  const std::source_location = std::source_location::current());

    template <typename Lambda>
    void forEachStateEvent(Lambda&&) const;

    AsyncVfoIfaceSptr addVfoIfDoesntExist(uint64_t handle);
    std::shared_ptr<GrpcAsyncVfo> removeVfoIfExists(uint64_t handle);

    void onEvent(const Event& event);
    void onReceiverEvent(const ReceiverEvent& event);
    void onVfoEvent(const VfoEvent& event);

    std::shared_ptr<GrpcAsyncVfo> getVfoImpl(uint64_t handle) const;

private:
    // All callbacks will be called from this thread.
    std::shared_ptr<WorkerThread> worker_thread_;
    std::shared_ptr<GrpcClient> client_;

    // State
    bool is_subscribed_;
    bool is_running_;
    std::string input_device_;
    std::string current_antenna_;
    std::vector<std::string> antennas_;
    int input_rate_;
    int input_decim_;
    bool dc_cancel_;
    bool iq_balance_;
    bool iq_swap_;
    int64_t rf_freq_;
    std::vector<GainStage> gain_stages_;
    bool auto_gain_;
    double freq_corr_;
    int iq_fft_size_;
    WindowType iq_fft_window_;
    std::string iq_recording_path_;
    bool is_iq_recording_;

    std::vector<std::shared_ptr<GrpcAsyncVfo>> vfos_;
};

} // namespace violetrx

#endif // VIOLET_RX_GRPC_ASYNC_RECEIVER_H
