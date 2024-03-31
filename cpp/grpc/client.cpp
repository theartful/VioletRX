#include <memory>

#include <boost/type_traits.hpp>
#include <grpcpp/channel.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/channel_arguments.h>
#include <spdlog/spdlog.h>

#include "async_core/events.h"
#include "client.h"
#include "type_conversion.h"

namespace violetrx
{

#define WRAP(data, call, function)                                             \
    WrapCallback(                                                              \
        std::move(call),                                                       \
        [](GrpcClient* self, UniqueClientCallPtr call, grpc::Status status) {  \
            using CallType = std::decay_t<decltype(data)>;                     \
            std::invoke(function, self, std::get<CallType>(*call),             \
                        std::move(status));                                    \
        })

#define INVOKE(callback, ...)                                                  \
    if (callback) {                                                            \
        if (callback_thread_) {                                                \
            callback_thread_->scheduleForced(                                  \
                __func__, std::move(callback) __VA_OPT__(, ) __VA_ARGS__);     \
        } else {                                                               \
            callback(__VA_ARGS__);                                             \
        }                                                                      \
    }

static constexpr int ALLOCATOR_CAPACITY = 16;

GrpcClient::GrpcClient(const std::string& addr_url,
                       std::shared_ptr<WorkerThread> callback_thread) :
    storage_{broadcast_queue::bitmap_allocator_storage::create<ClientCall>(
        ALLOCATOR_CAPACITY)},
    allocator_{&storage_},
    callback_thread_{std::move(callback_thread)}

{
    // Unlimited message size. This is scary!
    grpc::ChannelArguments ch_args;
    ch_args.SetMaxReceiveMessageSize(-1);

    stub_ = Receiver::Rx::NewStub(grpc::CreateCustomChannel(
        addr_url, grpc::InsecureChannelCredentials(), ch_args));
}

template <typename Lambda>
auto GrpcClient::WrapCallback(UniqueClientCallPtr call, Lambda)
{
    static_assert(boost::is_stateless<Lambda>::value);

    ClientCall* call_raw = call.release();

    // The libstdc++ std::function can hold up to two pointers in its internal
    // storage, and not allocate memory.
    return [call_raw, this](grpc::Status status) {
        // Promote the call again
        UniqueClientCallPtr call{call_raw, Deleter{allocator_}};

        std::invoke(Lambda{}, this, std::move(call), std::move(status));
    };
}

void GrpcClient::Start(Callback<> callback)
{
    auto [call, data] = Allocate<StartCall>();

    data.callback = std::move(callback);

    stub_->async()->Start(&data.context, &data.request, &data.response,
                          WRAP(data, call, &GrpcClient::OnStartCallDone));
}

void GrpcClient::Stop(Callback<> callback)
{
    auto [call, data] = Allocate<StopCall>();

    data.callback = std::move(callback);

    stub_->async()->Start(&data.context, &data.request, &data.response,
                          WRAP(data, call, &GrpcClient::OnStopCallDone));
}

void GrpcClient::SetInputDevice(std::string device, Callback<> callback)
{
    auto [call, data] = Allocate<SetInputDeviceCall>();

    data.callback = std::move(callback);
    data.request.set_value(device);

    stub_->async()->SetInputDevice(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnInputDeviceCallDone));
}

void GrpcClient::SetAntenna(std::string antenna, Callback<> callback)
{
    auto [call, data] = Allocate<SetAntennaCall>();

    data.callback = std::move(callback);
    data.request.set_value(std::move(antenna));

    stub_->async()->SetAntenna(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetAntennaCallDone));
}

void GrpcClient::SetInputRate(int rate, Callback<int> callback)
{
    auto [call, data] = Allocate<SetInputRateCall>();

    data.callback = std::move(callback);
    data.request.set_value(rate);

    stub_->async()->SetInputRate(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetInputRateCallDone));
}

void GrpcClient::SetInputDecim(int decim, Callback<int> callback)
{
    auto [call, data] = Allocate<SetInputDecimCall>();

    data.callback = std::move(callback);
    data.request.set_value(decim);

    stub_->async()->SetInputDecim(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetInputDecimCallDone));
}

void GrpcClient::SetRfFreq(int64_t freq, Callback<int64_t> callback)
{
    auto [call, data] = Allocate<SetRfFreqCall>();

    data.callback = std::move(callback);
    data.request.set_value(freq);

    stub_->async()->SetRfFreq(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetRfFreqCallDone));
}

void GrpcClient::SetIqSwap(bool enable, Callback<> callback)
{
    auto [call, data] = Allocate<SetIqSwapCall>();
    data.callback = std::move(callback);
    data.request.set_value(enable);

    stub_->async()->SetIqSwap(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetIqSwapCallDone));
}

void GrpcClient::SetDcCancel(bool enable, Callback<> callback)
{
    auto [call, data] = Allocate<SetDcCancelCall>();

    data.callback = std::move(callback);
    data.request.set_value(enable);

    stub_->async()->SetDcCancel(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetDcCancelCallDone));
}

void GrpcClient::SetIqBalance(bool enable, Callback<> callback)
{
    auto [call, data] = Allocate<SetIqBalanceCall>();

    data.callback = std::move(callback);
    data.request.set_value(enable);

    stub_->async()->SetIqBalance(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetIqBalanceCallDone));
}

void GrpcClient::SetAutoGain(bool enable, Callback<> callback)
{
    auto [call, data] = Allocate<SetAutoGainCall>();

    data.callback = std::move(callback);
    data.request.set_value(enable);

    stub_->async()->SetAutoGain(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetAutoGainCallDone));
}

void GrpcClient::SetGain(std::string name, double value,
                         Callback<double> callback)
{
    auto [call, data] = Allocate<SetGainCall>();

    data.callback = std::move(callback);
    data.request.set_name(name);
    data.request.set_value(value);

    stub_->async()->SetGain(&data.context, &data.request, &data.response,
                            WRAP(data, call, &GrpcClient::OnSetGainCallDone));
}

void GrpcClient::SetFreqCorr(double ppm, Callback<double> callback)
{
    auto [call, data] = Allocate<SetFreqCorrCall>();

    data.callback = std::move(callback);
    data.request.set_value(ppm);

    stub_->async()->SetFreqCorr(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetFreqCorrCallDone));
}

void GrpcClient::SetFftSize(int size, Callback<> callback)
{
    auto [call, data] = Allocate<SetFftSizeCall>();

    data.callback = std::move(callback);
    data.request.set_value(size);

    stub_->async()->SetFftSize(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetFftSizeCallDone));
}

void GrpcClient::SetFftWindow(WindowType window, Callback<> callback)
{
    auto [call, data] = Allocate<SetFftWindowCall>();

    data.callback = std::move(callback);
    data.request.set_window_type(WindowTypeCoreToProto(window));

    stub_->async()->SetFftWindow(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetFftWindowCallDone));
}

void GrpcClient::GetFftData(
    float* data, int size,
    Callback<Timestamp, int64_t, int, float*, int> callback)
{
    auto [call, call_data] = Allocate<GetFftDataCall>();

    call_data.callback = std::move(callback);
    call_data.data = data;
    call_data.size = size;

    stub_->async()->GetFftData(
        &call_data.context, &call_data.request, &call_data.response,
        WRAP(call_data, call, &GrpcClient::OnGetFftDataCallDone));
}

void GrpcClient::AddVfoChannel(Callback<uint64_t> callback)
{
    auto [call, data] = Allocate<AddVfoChannelCall>();

    data.callback = std::move(callback);

    stub_->async()->AddVfoChannel(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnAddVfoChannelCallDone));
}

void GrpcClient::RemoveVfoChannel(uint64_t handle, Callback<> callback)
{
    auto [call, data] = Allocate<RemoveVfoChannelCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);

    stub_->async()->RemoveVfoChannel(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnRemoveVfoChannelCallDone));
}

void GrpcClient::SetFilterOffset(uint64_t handle, int64_t offset,
                                 Callback<> callback)
{
    auto [call, data] = Allocate<SetFilterOffsetCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(offset);

    stub_->async()->SetFilterOffset(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetFilterOffsetCallDone));
}

void GrpcClient::SetFilter(uint64_t handle, int64_t low, int64_t high,
                           FilterShape shape, Callback<> callback)
{
    auto [call, data] = Allocate<SetFilterCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_low(low);
    data.request.set_high(high);
    data.request.set_shape(FilterShapeCoreToProto(shape));

    stub_->async()->SetFilter(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetFilterCallDone));
}

void GrpcClient::SetCwOffset(uint64_t handle, int64_t offset,
                             Callback<> callback)
{
    auto [call, data] = Allocate<SetCwOffsetCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(offset);

    stub_->async()->SetCwOffset(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetCwOffsetCallDone));
}

void GrpcClient::SetDemod(uint64_t handle, Demod demod, Callback<> callback)
{
    auto [call, data] = Allocate<SetDemodCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_demod(DemodCoreToProto(demod));

    stub_->async()->SetDemod(&data.context, &data.request, &data.response,
                             WRAP(data, call, &GrpcClient::OnSetDemodCallDone));
}

void GrpcClient::GetSignalPwr(uint64_t handle, Callback<float> callback)
{
    auto [call, data] = Allocate<GetSignalPwrCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);

    stub_->async()->GetSignalPwr(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnGetSignalPwrCallDone));
}

void GrpcClient::SetNoiseBlanker(uint64_t handle, int id, bool enabled,
                                 Callback<> callback)
{
    auto [call, data] = Allocate<SetNoiseBlankerCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_id(id);
    data.request.set_enabled(enabled);

    stub_->async()->SetNoiseBlanker(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetNoiseBlankerCallDone));
}

void GrpcClient::SetNoiseBlankerThreshold(uint64_t handle, int id,
                                          float threshold, Callback<> callback)
{
    auto [call, data] = Allocate<SetNoiseBlankerThresholdCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_id(id);
    data.request.set_threshold(threshold);

    stub_->async()->SetNoiseBlankerThreshold(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetNoiseBlankerThresholdCallDone));
}

void GrpcClient::SetSqlLevel(uint64_t handle, double lvl, Callback<> callback)
{
    auto [call, data] = Allocate<SetSqlLevelCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(lvl);

    stub_->async()->SetSqlLevel(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetSqlLevelCallDone));
}

void GrpcClient::SetSqlAlpha(uint64_t handle, double alpha, Callback<> callback)
{
    auto [call, data] = Allocate<SetSqlAlphaCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(alpha);

    stub_->async()->SetSqlAlpha(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetSqlAlphaCallDone));
}
void GrpcClient::SetAgcOn(uint64_t handle, bool enabled, Callback<> callback)
{
    auto [call, data] = Allocate<SetAgcOnCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(enabled);

    stub_->async()->SetAgcOn(&data.context, &data.request, &data.response,
                             WRAP(data, call, &GrpcClient::OnSetAgcOnCallDone));
}
void GrpcClient::SetAgcHang(uint64_t handle, bool enabled, Callback<> callback)
{
    auto [call, data] = Allocate<SetAgcHangCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(enabled);

    stub_->async()->SetAgcHang(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetAgcHangCallDone));
}
void GrpcClient::SetAgcThreshold(uint64_t handle, int threshold,
                                 Callback<> callback)
{
    auto [call, data] = Allocate<SetAgcThresholdCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(threshold);

    stub_->async()->SetAgcThreshold(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetAgcThresholdCallDone));
}
void GrpcClient::SetAgcSlope(uint64_t handle, int slope, Callback<> callback)
{
    auto [call, data] = Allocate<SetAgcSlopeCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(slope);

    stub_->async()->SetAgcSlope(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetAgcSlopeCallDone));
}
void GrpcClient::SetAgcDecay(uint64_t handle, int decay, Callback<> callback)
{
    auto [call, data] = Allocate<SetAgcDecayCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(decay);

    stub_->async()->SetAgcDecay(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetAgcDecayCallDone));
}
void GrpcClient::SetAgcManualGain(uint64_t handle, int gain,
                                  Callback<> callback)
{
    auto [call, data] = Allocate<SetAgcManualGainCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(gain);

    stub_->async()->SetAgcManualGain(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetAgcManualGainCallDone));
}
void GrpcClient::SetFmMaxDev(uint64_t handle, float maxdev, Callback<> callback)
{
    auto [call, data] = Allocate<SetFmMaxDevCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(maxdev);

    stub_->async()->SetFmMaxDev(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetFmMaxDevCallDone));
}
void GrpcClient::SetFmDeemph(uint64_t handle, double deemph,
                             Callback<> callback)
{
    auto [call, data] = Allocate<SetFmDeemphCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(deemph);

    stub_->async()->SetFmDeemph(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetFmDeemphCallDone));
}
void GrpcClient::SetAmDcr(uint64_t handle, bool enabled, Callback<> callback)
{
    auto [call, data] = Allocate<SetAmDcrCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(enabled);

    stub_->async()->SetAmDcr(&data.context, &data.request, &data.response,
                             WRAP(data, call, &GrpcClient::OnSetAmDcrCallDone));
}
void GrpcClient::SetAmSyncDcr(uint64_t handle, bool enabled,
                              Callback<> callback)
{
    auto [call, data] = Allocate<SetAmSyncDcrCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(enabled);

    stub_->async()->SetAmSyncDcr(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetAmSyncDcrCallDone));
}
void GrpcClient::SetAmSyncPllBw(uint64_t handle, float bw, Callback<> callback)
{
    auto [call, data] = Allocate<SetAmSyncPllBwCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_value(bw);

    stub_->async()->SetAmSyncPllBw(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnSetAmSyncPllBwCallDone));
}
void GrpcClient::StartAudioRecording(uint64_t handle, const std::string& path,
                                     Callback<> callback)
{
    auto [call, data] = Allocate<StartAudioRecordingCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_path(path);

    stub_->async()->StartAudioRecording(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnStartAudioRecordingCallDone));
}
void GrpcClient::StopAudioRecording(uint64_t handle, Callback<> callback)
{
    auto [call, data] = Allocate<StopAudioRecordingCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);

    stub_->async()->StopAudioRecording(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnStopAudioRecordingCallDone));
}
void GrpcClient::StartUdpStreaming(uint64_t, const std::string&, int, bool,
                                   Callback<> callback)
{
    // TODO
    INVOKE(callback, ErrorCode::UNIMPLEMENTED);
}
void GrpcClient::StopUdpStreaming(uint64_t, Callback<> callback)
{
    // TODO
    INVOKE(callback, ErrorCode::UNIMPLEMENTED);
}
void GrpcClient::StartSniffer(uint64_t handle, int samplerate, int buffsize,
                              Callback<> callback)
{
    auto [call, data] = Allocate<StartSnifferCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);
    data.request.set_samplerate(samplerate);
    data.request.set_buffsize(buffsize);

    stub_->async()->StartSniffer(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnStartSnifferCallDone));
}
void GrpcClient::StopSniffer(uint64_t handle, Callback<> callback)
{
    auto [call, data] = Allocate<StopSnifferCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);

    stub_->async()->StopSniffer(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnStopSnifferCallDone));
}
void GrpcClient::GetSnifferData(uint64_t handle, float* data, int size,
                                Callback<float*, int> callback)
{
    auto [call, call_data] = Allocate<GetSnifferDataCall>();

    call_data.callback = std::move(callback);
    call_data.request.set_handle(handle);
    call_data.data = data;
    call_data.size = size;

    stub_->async()->GetSnifferData(
        &call_data.context, &call_data.request, &call_data.response,
        WRAP(call_data, call, &GrpcClient::OnGetSnifferDataCallDone));
}
void GrpcClient::GetRdsData(uint64_t handle,
                            Callback<std::string, int> callback)
{
    auto [call, data] = Allocate<GetRdsDataCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);

    stub_->async()->GetRdsData(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnGetRdsDataCallDone));
}
void GrpcClient::StartRdsDecoder(uint64_t handle, Callback<> callback)
{
    auto [call, data] = Allocate<StartRdsDecoderCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);

    stub_->async()->StartRdsDecoder(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnStartRdsDecoderCallDone));
}
void GrpcClient::StopRdsDecoder(uint64_t handle, Callback<> callback)
{
    auto [call, data] = Allocate<StopRdsDecoderCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);

    stub_->async()->StopRdsDecoder(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnStopRdsDecoderCallDone));
}
void GrpcClient::ResetRdsParser(uint64_t handle, Callback<> callback)
{
    auto [call, data] = Allocate<ResetRdsParserCall>();

    data.callback = std::move(callback);
    data.request.set_handle(handle);

    stub_->async()->ResetRdsParser(
        &data.context, &data.request, &data.response,
        WRAP(data, call, &GrpcClient::OnResetRdsParserCallDone));
}

class GrpcClient::EventsReactor
    : public grpc::ClientReadReactor<::Receiver::Event>
{
public:
    EventsReactor(std::shared_ptr<Receiver::Rx::Stub> stub,
                  EventHandler callback,
                  std::shared_ptr<WorkerThread> callback_thread) :
        stub_{std::move(stub)},
        callback_thread_{std::move(callback_thread)},
        callback_{std::move(callback)},
        unsubscribed_{false}
    {
        stub_->async()->Subscribe(&context_, &request_, this);
        StartRead(&response_);
        StartCall();
    }

    void DeclareUnsubscription()
    {
        if (!unsubscribed_) {
            INVOKE(callback_,
                   Unsubscribed{{.id = -1, .timestamp = Timestamp::Now()}});
            unsubscribed_ = true;
        }
    }

    void OnReadDone(bool ok) override
    {
        if (!ok) {
            DeclareUnsubscription();
            return;
        }

        std::optional<Event> maybe_event = EventProtoToCore(response_);
        if (maybe_event.has_value()) {
            const Event& event = *maybe_event;

            if (std::holds_alternative<Unsubscribed>(event)) {
                DeclareUnsubscription();
            } else {
                INVOKE(callback_, event);
            }
        } else {
            spdlog::error(
                "Receiver an event of type {} that I couldn't convert!",
                (int)response_.tx_case());
        }

        // Read the next event!
        StartRead(&response_);
    }

    void OnDone(const grpc::Status&) override
    {
        DeclareUnsubscription();

        // Very scary!
        delete this;
    }

private:
    std::shared_ptr<Receiver::Rx::Stub> stub_;
    grpc::ClientContext context_;
    google::protobuf::Empty request_;
    Receiver::Event response_;

    std::shared_ptr<WorkerThread> callback_thread_;
    EventHandler callback_;

    bool unsubscribed_;
};

void GrpcClient::Subscribe(EventHandler callback)
{
    new EventsReactor(stub_, std::move(callback), callback_thread_);
}

template <typename CallType>
void GrpcClient::OnEmptyResponseCallDone(CallType& call,
                                         const grpc::Status& status)
{
    if (status.ok()) {
        INVOKE(call.callback, ErrorCodeProtoToCore(call.response.code()));
    } else {
        spdlog::error(status.error_message());
        INVOKE(call.callback, ErrorCode::CALL_ERROR);
    }
}

template <typename CallType>
void GrpcClient::OnValueResponseCallDone(CallType& call,
                                         const grpc::Status& status)
{
    if (status.ok()) {
        INVOKE(call.callback, ErrorCodeProtoToCore(call.response.code()),
               call.response.value());
    } else {
        spdlog::error(status.error_message());
        INVOKE(call.callback, ErrorCode::CALL_ERROR,
               decltype(call.response.value()){});
    }
}

void GrpcClient::OnStopCallDone(StopCall& call, const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}

void GrpcClient::OnStartCallDone(StartCall& call, const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}

void GrpcClient::OnInputDeviceCallDone(SetInputDeviceCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAntennaCallDone(SetAntennaCall& call,
                                      const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetInputRateCallDone(SetInputRateCall& call,
                                        const grpc::Status& status)
{
    OnValueResponseCallDone(call, status);
}
void GrpcClient::OnSetInputDecimCallDone(SetInputDecimCall& call,
                                         const grpc::Status& status)
{
    OnValueResponseCallDone(call, status);
}
void GrpcClient::OnSetRfFreqCallDone(SetRfFreqCall& call,
                                     const grpc::Status& status)
{
    OnValueResponseCallDone(call, status);
}
void GrpcClient::OnSetIqSwapCallDone(SetIqSwapCall& call,
                                     const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetDcCancelCallDone(SetDcCancelCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetIqBalanceCallDone(SetIqBalanceCall& call,
                                        const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAutoGainCallDone(SetAutoGainCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetGainCallDone(SetGainCall& call,
                                   const grpc::Status& status)
{
    OnValueResponseCallDone(call, status);
}
void GrpcClient::OnSetFreqCorrCallDone(SetFreqCorrCall& call,
                                       const grpc::Status& status)
{
    OnValueResponseCallDone(call, status);
}
void GrpcClient::OnSetFftSizeCallDone(SetFftSizeCall& call,
                                      const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetFftWindowCallDone(SetFftWindowCall& call,
                                        const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnRemoveVfoChannelCallDone(RemoveVfoChannelCall& call,
                                            const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnGetFftDataCallDone(GetFftDataCall& call,
                                      const grpc::Status& status)
{
    if (!status.ok()) {
        INVOKE(call.callback, ErrorCode::CALL_ERROR, Timestamp{}, 0, 0, nullptr,
               0);
        return;
    }

    if (call.response.code() != Receiver::ErrorCode::OK) {
        INVOKE(call.callback, ErrorCodeProtoToCore(call.response.code()),
               Timestamp{}, 0, 0, nullptr, 0);
    }

    const auto& proto_frame = call.response.fft_frame();

    if (proto_frame.data_size() > call.size) {
        INVOKE(call.callback, ErrorCode::INSUFFICIENT_BUFFER_SIZE, Timestamp{},
               0, 0, nullptr, 0);
        return;
    }

    Timestamp timestamp = TimestampProtoToCore(proto_frame.timestamp());
    int64_t center_freq = static_cast<int64_t>(proto_frame.center_freq());
    int sample_rate = static_cast<int>(proto_frame.sample_rate());

    std::memcpy(call.data, proto_frame.data().data(),
                sizeof(float) * proto_frame.data_size());

    INVOKE(call.callback, ErrorCode::OK, timestamp, center_freq, sample_rate,
           call.data, proto_frame.data_size());
}

void GrpcClient::OnAddVfoChannelCallDone(AddVfoChannelCall& call,
                                         const grpc::Status& status)
{
    if (status.ok()) {
        INVOKE(call.callback, ErrorCodeProtoToCore(call.response.code()),
               call.response.handle());
    } else {
        spdlog::error(status.error_message());
        INVOKE(call.callback, ErrorCode::CALL_ERROR, 0);
    }
}

void GrpcClient::OnSetFilterOffsetCallDone(SetFilterOffsetCall& call,
                                           const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetFilterCallDone(SetFilterCall& call,
                                     const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetCwOffsetCallDone(SetCwOffsetCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetDemodCallDone(SetDemodCall& call,
                                    const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnGetSignalPwrCallDone(GetSignalPwrCall& call,
                                        const grpc::Status& status)
{
    OnValueResponseCallDone(call, status);
}
void GrpcClient::OnSetNoiseBlankerCallDone(SetNoiseBlankerCall& call,
                                           const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetNoiseBlankerThresholdCallDone(
    SetNoiseBlankerThresholdCall& call, const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetSqlLevelCallDone(SetSqlLevelCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetSqlAlphaCallDone(SetSqlAlphaCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAgcOnCallDone(SetAgcOnCall& call,
                                    const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAgcHangCallDone(SetAgcHangCall& call,
                                      const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAgcSlopeCallDone(SetAgcSlopeCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAgcThresholdCallDone(SetAgcThresholdCall& call,
                                           const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAgcDecayCallDone(SetAgcDecayCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAgcManualGainCallDone(SetAgcManualGainCall& call,
                                            const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetFmMaxDevCallDone(SetFmMaxDevCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetFmDeemphCallDone(SetFmDeemphCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAmDcrCallDone(SetAmDcrCall& call,
                                    const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAmSyncDcrCallDone(SetAmSyncDcrCall& call,
                                        const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnSetAmSyncPllBwCallDone(SetAmSyncPllBwCall& call,
                                          const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnStartAudioRecordingCallDone(StartAudioRecordingCall& call,
                                               const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnStopAudioRecordingCallDone(StopAudioRecordingCall& call,
                                              const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnStartSnifferCallDone(StartSnifferCall& call,
                                        const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnStopSnifferCallDone(StopSnifferCall& call,
                                       const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnGetSnifferDataCallDone(GetSnifferDataCall& call,
                                          const grpc::Status& status)
{
    if (!status.ok()) {
        INVOKE(call.callback, ErrorCode::CALL_ERROR, call.data, 0);
        return;
    }

    if (call.response.code() != Receiver::ErrorCode::OK) {
        INVOKE(call.callback, ErrorCodeProtoToCore(call.response.code()),
               call.data, 0);
    }

    const auto& proto_data = call.response.data();

    if (proto_data.size() > call.size) {
        INVOKE(call.callback, ErrorCode::INSUFFICIENT_BUFFER_SIZE, call.data,
               0);
        return;
    }

    std::memcpy(call.data, proto_data.data(),
                sizeof(float) * proto_data.size());

    INVOKE(call.callback, ErrorCode::OK, call.data, proto_data.size());
}
void GrpcClient::OnStartRdsDecoderCallDone(StartRdsDecoderCall& call,
                                           const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnStopRdsDecoderCallDone(StopRdsDecoderCall& call,
                                          const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnResetRdsParserCallDone(ResetRdsParserCall& call,
                                          const grpc::Status& status)
{
    OnEmptyResponseCallDone(call, status);
}
void GrpcClient::OnGetRdsDataCallDone(GetRdsDataCall& call,
                                      const grpc::Status& status)
{
    if (status.ok()) {
        INVOKE(call.callback, ErrorCodeProtoToCore(call.response.code()),
               call.response.data(), call.response.type());
    } else {
        spdlog::error(status.error_message());
        INVOKE(call.callback, ErrorCode::CALL_ERROR, std::string{}, 0);
    }
}

template <typename CallType, typename... T>
std::pair<GrpcClient::UniqueClientCallPtr, CallType&>
GrpcClient::Allocate(T&&... args)
{
    ClientCall* ptr = std::allocator_traits<Allocator>::allocate(allocator_, 1);

    std::allocator_traits<Allocator>::construct(
        allocator_, ptr, std::in_place_type_t<CallType>{},
        std::forward<T>(args)...);

    return {UniqueClientCallPtr{ptr, Deleter{allocator_}},
            std::get<CallType>(*ptr)};
}

GrpcClient::~GrpcClient() { spdlog::debug("~GrpcClient"); }

} // namespace violetrx
