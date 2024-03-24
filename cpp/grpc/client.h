#ifndef VIOLET_GRPC_CLIENT_H
#define VIOLET_GRPC_CLIENT_H

#include <memory>
#include <string>

#include <bitmap_allocator.h>

#include "async_core/types.h"
#include "client_call.h"
#include "receiver.grpc.pb.h"

namespace violetrx
{

class GrpcClient
{
public:
    GrpcClient(const std::string& addr_url);

    void Start(Callback<> = {});
    void Stop(Callback<> = {});
    void SetInputDevice(std::string, Callback<> = {});
    void SetAntenna(std::string, Callback<> = {});
    void SetInputRate(int, Callback<int> = {});
    void SetInputDecim(int, Callback<int> = {});
    void SetRfFreq(int64_t, Callback<int64_t> = {});
    void SetIqSwap(bool, Callback<> = {});
    void SetDcCancel(bool, Callback<> = {});
    void SetIqBalance(bool, Callback<> = {});
    void SetAutoGain(bool, Callback<> = {});
    void SetGain(std::string, double, Callback<double> = {});
    void SetFreqCorr(double, Callback<double> = {});
    void SetFftSize(int, Callback<> = {});
    void SetFftWindow(WindowType, Callback<> = {});
    void GetFftData(float*, int,
                    Callback<Timestamp, int64_t, int, float*, int> = {});
    void AddVfoChannel(Callback<uint64_t> = {});
    void RemoveVfoChannel(uint64_t, Callback<> = {});

    void SetFilterOffset(uint64_t, int64_t, Callback<> = {});
    void SetFilter(uint64_t, int64_t, int64_t, FilterShape, Callback<> = {});
    void SetCwOffset(uint64_t, int64_t, Callback<> = {});
    void SetDemod(uint64_t, Demod, Callback<> = {});
    void GetSignalPwr(uint64_t, Callback<float> = {});
    void SetNoiseBlanker(uint64_t, int, bool, Callback<> = {});
    void SetNoiseBlankerThreshold(uint64_t, int, float, Callback<> = {});
    void SetSqlLevel(uint64_t, double, Callback<> = {});
    void SetSqlAlpha(uint64_t, double, Callback<> = {});
    void SetAgcOn(uint64_t, bool, Callback<> = {});
    void SetAgcHang(uint64_t, bool, Callback<> = {});
    void SetAgcThreshold(uint64_t, int, Callback<> = {});
    void SetAgcSlope(uint64_t, int, Callback<> = {});
    void SetAgcDecay(uint64_t, int, Callback<> = {});
    void SetAgcManualGain(uint64_t, int, Callback<> = {});
    void SetFmMaxDev(uint64_t, float, Callback<> = {});
    void SetFmDeemph(uint64_t, double, Callback<> = {});
    void SetAmDcr(uint64_t, bool, Callback<> = {});
    void SetAmSyncDcr(uint64_t, bool, Callback<> = {});
    void SetAmSyncPllBw(uint64_t, float, Callback<> = {});
    void StartAudioRecording(uint64_t, const std::string&, Callback<> = {});
    void StopAudioRecording(uint64_t, Callback<> = {});
    void SetAudioGain(uint64_t, float, Callback<> = {});
    void StartUdpStreaming(uint64_t, const std::string&, int, bool,
                           Callback<> = {});
    void StopUdpStreaming(uint64_t, Callback<> = {});
    void StartSniffer(uint64_t, int, int, Callback<> = {});
    void StopSniffer(uint64_t, Callback<> = {});
    void GetSnifferData(uint64_t, float*, int, Callback<float*, int> = {});
    void GetRdsData(uint64_t, Callback<std::string, int> = {});
    void StartRdsDecoder(uint64_t, Callback<> = {});
    void StopRdsDecoder(uint64_t, Callback<> = {});
    void ResetRdsParser(uint64_t, Callback<> = {});

private:
    using Allocator = broadcast_queue::bitmap_allocator<ClientCall>;
    using AllocatorStorage = broadcast_queue::bitmap_allocator_storage;

    struct Deleter {
        template <typename T>
        void operator()(T* data)
        {
            std::allocator_traits<Allocator>::destroy(allocator, data);
            std::allocator_traits<Allocator>::deallocate(allocator, data, 1);
        }

        Allocator allocator;
    };
    using UniqueClientCallPtr = std::unique_ptr<ClientCall, Deleter>;

    template <typename CallType, typename... T>
    std::pair<UniqueClientCallPtr, CallType&> Allocate(T&&... args);

    template <typename Lambda>
    auto WrapCallback(UniqueClientCallPtr call, Lambda lambda);

private:
    template <typename CallType>
    void OnEmptyResponseCallDone(CallType& call, const grpc::Status& status);

    template <typename CallType>
    void OnValueResponseCallDone(CallType& call, const grpc::Status& status);

    void OnStartCallDone(StartCall& call, const grpc::Status& status);
    void OnStopCallDone(StopCall& call, const grpc::Status& status);
    void OnInputDeviceCallDone(SetInputDeviceCall& call,
                               const grpc::Status& status);
    void OnSetAntennaCallDone(SetAntennaCall& call, const grpc::Status& status);
    void OnSetInputRateCallDone(SetInputRateCall& call,
                                const grpc::Status& status);
    void OnSetInputDecimCallDone(SetInputDecimCall& call,
                                 const grpc::Status& status);
    void OnSetRfFreqCallDone(SetRfFreqCall& call, const grpc::Status& status);
    void OnSetIqSwapCallDone(SetIqSwapCall& call, const grpc::Status& status);
    void OnSetDcCancelCallDone(SetDcCancelCall& call,
                               const grpc::Status& status);
    void OnSetIqBalanceCallDone(SetIqBalanceCall& call,
                                const grpc::Status& status);
    void OnSetAutoGainCallDone(SetAutoGainCall& call,
                               const grpc::Status& status);
    void OnSetGainCallDone(SetGainCall& call, const grpc::Status& status);
    void OnSetFreqCorrCallDone(SetFreqCorrCall& call,
                               const grpc::Status& status);
    void OnSetFftSizeCallDone(SetFftSizeCall& call, const grpc::Status& status);
    void OnSetFftWindowCallDone(SetFftWindowCall& call,
                                const grpc::Status& status);
    void OnGetFftDataCallDone(GetFftDataCall& call, const grpc::Status& status);
    void OnAddVfoChannelCallDone(AddVfoChannelCall& call,
                                 const grpc::Status& status);
    void OnRemoveVfoChannelCallDone(RemoveVfoChannelCall& call,
                                    const grpc::Status& status);
    void OnSetFilterOffsetCallDone(SetFilterOffsetCall& call,
                                   const grpc::Status& status);
    void OnSetFilterCallDone(SetFilterCall& call, const grpc::Status& status);

    void OnSetCwOffsetCallDone(SetCwOffsetCall& call,
                               const grpc::Status& status);

    void OnSetDemodCallDone(SetDemodCall& call, const grpc::Status& status);

    void OnGetSignalPwrCallDone(GetSignalPwrCall& call,
                                const grpc::Status& status);
    void OnSetNoiseBlankerCallDone(SetNoiseBlankerCall& call,
                                   const grpc::Status& status);
    void OnSetNoiseBlankerThresholdCallDone(SetNoiseBlankerThresholdCall& call,
                                            const grpc::Status& status);
    void OnSetSqlLevelCallDone(SetSqlLevelCall& call,
                               const grpc::Status& status);
    void OnSetSqlAlphaCallDone(SetSqlAlphaCall& call,
                               const grpc::Status& status);
    void OnSetAgcOnCallDone(SetAgcOnCall& call, const grpc::Status& status);
    void OnSetAgcHangCallDone(SetAgcHangCall& call, const grpc::Status& status);
    void OnSetAgcSlopeCallDone(SetAgcSlopeCall& call,
                               const grpc::Status& status);
    void OnSetAgcThresholdCallDone(SetAgcThresholdCall& call,
                                   const grpc::Status& status);
    void OnSetAgcDecayCallDone(SetAgcDecayCall& call,
                               const grpc::Status& status);
    void OnSetAgcManualGainCallDone(SetAgcManualGainCall& call,
                                    const grpc::Status& status);
    void OnSetFmMaxDevCallDone(SetFmMaxDevCall& call,
                               const grpc::Status& status);
    void OnSetFmDeemphCallDone(SetFmDeemphCall& call,
                               const grpc::Status& status);
    void OnSetAmDcrCallDone(SetAmDcrCall& call, const grpc::Status& status);
    void OnSetAmSyncDcrCallDone(SetAmSyncDcrCall& call,
                                const grpc::Status& status);
    void OnSetAmSyncPllBwCallDone(SetAmSyncPllBwCall& call,
                                  const grpc::Status& status);
    void OnStartAudioRecordingCallDone(StartAudioRecordingCall& call,
                                       const grpc::Status& status);
    void OnStopAudioRecordingCallDone(StopAudioRecordingCall& call,
                                      const grpc::Status& status);
    void OnStartSnifferCallDone(StartSnifferCall& call,
                                const grpc::Status& status);
    void OnStopSnifferCallDone(StopSnifferCall& call,
                               const grpc::Status& status);
    void OnGetSnifferDataCallDone(GetSnifferDataCall& call,
                                  const grpc::Status& status);
    void OnStartRdsDecoderCallDone(StartRdsDecoderCall& call,
                                   const grpc::Status& status);
    void OnStopRdsDecoderCallDone(StopRdsDecoderCall& call,
                                  const grpc::Status& status);
    void OnResetRdsParserCallDone(ResetRdsParserCall& call,
                                  const grpc::Status& status);
    void OnGetRdsDataCallDone(GetRdsDataCall& call, const grpc::Status& status);

private:
    std::shared_ptr<Receiver::Rx::Stub> stub_;

    struct ClientCallDeleter {
    };

    AllocatorStorage storage_;
    Allocator allocator_;
};

} // namespace violetrx

#endif // VIOLET_GRPC_CLIENT_H
