#ifndef VIOLET_RX_CLIENT_CALL_H
#define VIOLET_RX_CLIENT_CALL_H

#include <variant>

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/client_callback.h>

#include "async_core/events.h"
#include "async_core/types.h"
#include "receiver.pb.h"

namespace violetrx
{

struct ClientCallCommon {
    grpc::ClientContext context;
};

struct StartCall : public ClientCallCommon {
    google::protobuf::Empty request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct StopCall : public ClientCallCommon {
    google::protobuf::Empty request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetInputDeviceCall : public ClientCallCommon {
    google::protobuf::StringValue request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAntennaCall : public ClientCallCommon {
    google::protobuf::StringValue request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetInputRateCall : public ClientCallCommon {
    google::protobuf::UInt32Value request;
    Receiver::UInt32Response response;
    Callback<int> callback;
};

struct SetInputDecimCall : public ClientCallCommon {
    google::protobuf::UInt32Value request;
    Receiver::UInt32Response response;
    Callback<int> callback;
};

struct SetIqSwapCall : public ClientCallCommon {
    google::protobuf::BoolValue request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetDcCancelCall : public ClientCallCommon {
    google::protobuf::BoolValue request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetIqBalanceCall : public ClientCallCommon {
    google::protobuf::BoolValue request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAutoGainCall : public ClientCallCommon {
    google::protobuf::BoolValue request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetRfFreqCall : public ClientCallCommon {
    google::protobuf::UInt64Value request;
    Receiver::UInt64Response response;
    Callback<int64_t> callback;
};

struct SetGainCall : public ClientCallCommon {
    Receiver::SetGainRequest request;
    Receiver::DoubleResponse response;
    Callback<double> callback;
};

struct SetFreqCorrCall : public ClientCallCommon {
    google::protobuf::DoubleValue request;
    Receiver::DoubleResponse response;
    Callback<double> callback;
};

struct SetFftSizeCall : public ClientCallCommon {
    google::protobuf::UInt32Value request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetFftWindowCall : public ClientCallCommon {
    Receiver::SetFftWindowRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct GetFftDataCall : public ClientCallCommon {
    google::protobuf::Empty request;
    Receiver::FftFrameResponse response;

    float* data;
    int size;

    Callback<Timestamp, int64_t, int, float*, int> callback;
};

struct AddVfoChannelCall : public ClientCallCommon {
    google::protobuf::Empty request;
    Receiver::VfoResponse response;
    Callback<uint64_t> callback;
};

struct RemoveVfoChannelCall : public ClientCallCommon {
    Receiver::VfoHandle request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetFilterOffsetCall : public ClientCallCommon {
    Receiver::VfoUInt64Request request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetFilterCall : public ClientCallCommon {
    Receiver::VfoFilterRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetCwOffsetCall : public ClientCallCommon {
    Receiver::VfoUInt64Request request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetDemodCall : public ClientCallCommon {
    Receiver::VfoDemodRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct GetSignalPwrCall : public ClientCallCommon {
    Receiver::VfoHandle request;
    Receiver::FloatResponse response;
    Callback<float> callback;
};

struct SetNoiseBlankerCall : public ClientCallCommon {
    Receiver::VfoNoiseBlankerRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetNoiseBlankerThresholdCall : public ClientCallCommon {
    Receiver::VfoNoiseBlankerThresholdRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetSqlLevelCall : public ClientCallCommon {
    Receiver::VfoDoubleRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetSqlAlphaCall : public ClientCallCommon {
    Receiver::VfoDoubleRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAgcOnCall : public ClientCallCommon {
    Receiver::VfoBoolRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAgcThresholdCall : public ClientCallCommon {
    Receiver::VfoInt32Request request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAgcHangCall : public ClientCallCommon {
    Receiver::VfoBoolRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAgcSlopeCall : public ClientCallCommon {
    Receiver::VfoInt32Request request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAgcDecayCall : public ClientCallCommon {
    Receiver::VfoInt32Request request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAgcManualGainCall : public ClientCallCommon {
    Receiver::VfoInt32Request request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetFmMaxDevCall : public ClientCallCommon {
    Receiver::VfoFloatRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetFmDeemphCall : public ClientCallCommon {
    Receiver::VfoDoubleRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAmDcrCall : public ClientCallCommon {
    Receiver::VfoBoolRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAmSyncDcrCall : public ClientCallCommon {
    Receiver::VfoBoolRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct SetAmSyncPllBwCall : public ClientCallCommon {
    Receiver::VfoFloatRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct StartAudioRecordingCall : public ClientCallCommon {
    Receiver::VfoRecordingRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct StopAudioRecordingCall : public ClientCallCommon {
    Receiver::VfoHandle request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct StartSnifferCall : public ClientCallCommon {
    Receiver::VfoSnifferRequest request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct StopSnifferCall : public ClientCallCommon {
    Receiver::VfoHandle request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct GetSnifferDataCall : public ClientCallCommon {
    Receiver::VfoHandle request;
    Receiver::SnifferDataResponse response;

    float* data;
    int size;

    Callback<float*, int> callback;
};

struct StartRdsDecoderCall : public ClientCallCommon {
    Receiver::VfoHandle request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct StopRdsDecoderCall : public ClientCallCommon {
    Receiver::VfoHandle request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct ResetRdsParserCall : public ClientCallCommon {
    Receiver::VfoHandle request;
    Receiver::EmptyResponse response;
    Callback<> callback;
};

struct GetRdsDataCall : public ClientCallCommon {
    Receiver::VfoHandle request;
    Receiver::RdsDataResponse response;
    Callback<std::string, int> callback;
};

using ClientCall = std::variant<
    StartCall, StopCall, SetInputDeviceCall, SetAntennaCall, SetInputRateCall,
    SetInputDecimCall, SetIqSwapCall, SetDcCancelCall, SetIqBalanceCall,
    SetAutoGainCall, SetRfFreqCall, SetGainCall, SetFreqCorrCall,
    SetFftSizeCall, SetFftWindowCall, GetFftDataCall, AddVfoChannelCall,
    RemoveVfoChannelCall, SetFilterOffsetCall, SetFilterCall, SetCwOffsetCall,
    SetDemodCall, GetSignalPwrCall, SetNoiseBlankerCall,
    SetNoiseBlankerThresholdCall, SetSqlLevelCall, SetSqlAlphaCall,
    SetAgcOnCall, SetAgcHangCall, SetAgcThresholdCall, SetAgcSlopeCall,
    SetAgcDecayCall, SetAgcManualGainCall, SetFmMaxDevCall, SetFmDeemphCall,
    SetAmDcrCall, SetAmSyncDcrCall, SetAmSyncPllBwCall, StartAudioRecordingCall,
    StopAudioRecordingCall, StartSnifferCall, StopSnifferCall,
    GetSnifferDataCall, StartRdsDecoderCall, StopRdsDecoderCall,
    ResetRdsParserCall, GetRdsDataCall>;

} // namespace violetrx

#endif
