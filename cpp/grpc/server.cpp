#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/server_builder.h>
#include <spdlog/spdlog.h>

#include "async_core/async_vfo_iface.h"
#include "receiver.pb.h"
#include "server.h"

namespace violetrx
{

#define ENTER_VFO_CONTEXT(reactor, response, handle, vfo)                      \
    async_receiver_->getVfo(                                                   \
        handle, [=](ErrorCode ___err, AsyncVfoIface::sptr vfo) {               \
            if (___err != ErrorCode::OK) {                                     \
                response->set_code(ErrorCodeCoreToProto(___err));              \
                reactor->Finish(grpc::Status::OK);                             \
                return;                                                        \
            }                                                                  \
            (void)0

#define EXIT_VFO_CONTEXT()                                                     \
    })

inline Receiver::ErrorCode ErrorCodeCoreToProto(ErrorCode code)
{
    // FIXME
    return static_cast<Receiver::ErrorCode>(code);
}

inline WindowType WindowProtoToCore(Receiver::WindowType window)
{
    // FIXME
    return static_cast<WindowType>(window);
}

inline FilterShape FilterShapeProtoToCore(Receiver::FilterShape filter_shape)
{
    // FIXME
    return static_cast<FilterShape>(filter_shape);
}

inline Demod DemodProtoToCore(Receiver::DemodType demod)
{
    // FIXME
    return static_cast<Demod>(demod);
}

inline google::protobuf::Timestamp TimestampCoreToProto(Timestamp timestamp)
{
    google::protobuf::Timestamp result;
    result.set_nanos(timestamp.nanos);
    result.set_seconds(timestamp.seconds);

    return result;
}

GrpcServer::GrpcServer(AsyncReceiverIface::sptr async_receiver,
                       const std::string& addr_url) :
    async_receiver_{std::move(async_receiver)}
{
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder;

    // FIXME: (unlimited message sizes) this is scary
    builder.SetMaxSendMessageSize(-1);
    builder.AddListeningPort(addr_url, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    server_ = builder.BuildAndStart();

    spdlog::info("Server listening on {}", addr_url);
}

grpc::ServerUnaryReactor*
GrpcServer::Start(grpc::CallbackServerContext* context,
                  [[maybe_unused]] const google::protobuf::Empty* request,
                  Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->start([=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::Stop(grpc::CallbackServerContext* context,
                 [[maybe_unused]] const google::protobuf::Empty* request,
                 Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->start([=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetInputDevice(grpc::CallbackServerContext* context,
                           const google::protobuf::StringValue* request,
                           Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setInputDevice(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAntenna(grpc::CallbackServerContext* context,
                       const google::protobuf::StringValue* request,
                       Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setAntenna(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetInputRate(grpc::CallbackServerContext* context,
                         const google::protobuf::UInt32Value* request,
                         Receiver::UInt32Response* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setInputRate(
        request->value(), [=](ErrorCode err, int rate) {
            response->set_code(ErrorCodeCoreToProto(err));
            response->set_value(rate);
            reactor->Finish(grpc::Status::OK);
        });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetInputDecim(grpc::CallbackServerContext* context,
                          const google::protobuf::UInt32Value* request,
                          Receiver::UInt32Response* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setInputDecim(
        request->value(), [=](ErrorCode err, int rate) {
            response->set_code(ErrorCodeCoreToProto(err));
            response->set_value(rate);
            reactor->Finish(grpc::Status::OK);
        });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetIqSwap(grpc::CallbackServerContext* context,
                      const google::protobuf::BoolValue* request,
                      Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setIqSwap(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetDcCancel(grpc::CallbackServerContext* context,
                        const google::protobuf::BoolValue* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setDcCancel(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetIqBalance(grpc::CallbackServerContext* context,
                         const google::protobuf::BoolValue* request,
                         Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setIqBalance(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAutoGain(grpc::CallbackServerContext* context,
                        const google::protobuf::BoolValue* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setAutoGain(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetRfFreq(grpc::CallbackServerContext* context,
                      const google::protobuf::UInt64Value* request,
                      Receiver::UInt64Response* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setRfFreq(
        request->value(), [=](ErrorCode err, int64_t freq) {
            response->set_code(ErrorCodeCoreToProto(err));
            response->set_value(freq);
            reactor->Finish(grpc::Status::OK);
        });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetGain(grpc::CallbackServerContext* context,
                    const Receiver::SetGainRequest* request,
                    Receiver::DoubleResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setGain(request->name(), request->value(),
                             [=](ErrorCode err, double gain) {
                                 response->set_code(ErrorCodeCoreToProto(err));
                                 response->set_value(gain);
                                 reactor->Finish(grpc::Status::OK);
                             });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetFreqCorr(grpc::CallbackServerContext* context,
                        const google::protobuf::DoubleValue* request,
                        Receiver::DoubleResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setFreqCorr(
        request->value(), [=](ErrorCode err, double ppm) {
            response->set_code(ErrorCodeCoreToProto(err));
            response->set_value(ppm);
            reactor->Finish(grpc::Status::OK);
        });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetFftSize(grpc::CallbackServerContext* context,
                       const google::protobuf::UInt32Value* request,
                       Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->setIqFftSize(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetFftWindow(grpc::CallbackServerContext* context,
                         const Receiver::SetFftWindowRequest* request,
                         Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    // FIXME: normalization
    async_receiver_->setIqFftWindow(
        WindowProtoToCore(request->window_type()), false, [=](ErrorCode err) {
            response->set_code(ErrorCodeCoreToProto(err));
            reactor->Finish(grpc::Status::OK);
        });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::GetFftData(grpc::CallbackServerContext* context,
                       [[maybe_unused]] const google::protobuf::Empty* request,
                       Receiver::FftFrameResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    // FIXME: This might fail if the fft frame size changed.
    int fft_size = async_receiver_->getIqFftSize();

    std::unique_ptr<Receiver::FftFrame> fft_frame =
        std::make_unique<Receiver::FftFrame>();
    fft_frame->mutable_data()->Reserve(fft_size);

    async_receiver_->getIqFftData(
        fft_frame->mutable_data()->mutable_data(), // Data
        fft_frame->mutable_data()->Capacity(),     // Size
        // Callback
        [=, fft_frame = std::move(fft_frame)](
            ErrorCode err, Timestamp timestamp, int64_t center_freq,
            int sample_rate, [[maybe_unused]] float* data,
            int fft_size) mutable {
            response->set_code(ErrorCodeCoreToProto(err));

            if (err != ErrorCode::OK) {
                reactor->Finish(grpc::Status::OK);
                return;
            }

            fft_frame->mutable_data()->AddNAlreadyReserved(fft_size);
            fft_frame->set_allocated_timestamp(new google::protobuf::Timestamp(
                TimestampCoreToProto(timestamp)));
            fft_frame->set_center_freq(center_freq);
            fft_frame->set_sample_rate(sample_rate);

            response->set_allocated_fft_frame(fft_frame.release());

            reactor->Finish(grpc::Status::OK);
        });

    return reactor;
}

grpc::ServerUnaryReactor* GrpcServer::AddVfoChannel(
    grpc::CallbackServerContext* context,
    [[maybe_unused]] const google::protobuf::Empty* request,
    Receiver::VfoResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->addVfoChannel([=](ErrorCode err, AsyncVfoIface::sptr vfo) {
        response->set_code(ErrorCodeCoreToProto(err));

        if (err != ErrorCode::OK) {
            reactor->Finish(grpc::Status::OK);
            return;
        }

        response->set_handle(vfo->getId());
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::RemoveVfoChannel(grpc::CallbackServerContext* context,
                             const Receiver::VfoHandle* request,
                             [[maybe_unused]] Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    async_receiver_->removeVfoChannel(request->handle(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetFilterOffset(grpc::CallbackServerContext* context,
                            const Receiver::VfoUInt64Request* request,
                            Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setFilterOffset(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetFilter(grpc::CallbackServerContext* context,
                      const Receiver::VfoFilterRequest* request,
                      Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setFilter(request->low(), request->high(),
                   FilterShapeProtoToCore(request->shape()),
                   [=](ErrorCode err) {
                       response->set_code(ErrorCodeCoreToProto(err));
                       reactor->Finish(grpc::Status::OK);
                   });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetCwOffset(grpc::CallbackServerContext* context,
                        const Receiver::VfoUInt64Request* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setCwOffset(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}
grpc::ServerUnaryReactor*
GrpcServer::SetDemod(grpc::CallbackServerContext* context,
                     const Receiver::VfoDemodRequest* request,
                     Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setDemod(DemodProtoToCore(request->demod()), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::GetSignalPwr(grpc::CallbackServerContext* context,
                         const Receiver::VfoHandle* request,
                         Receiver::FloatResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->getSignalPwr([=](ErrorCode err, double pwr) {
        response->set_code(ErrorCodeCoreToProto(err));
        response->set_value(pwr);
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetNoiseBlanker(grpc::CallbackServerContext* context,
                            const Receiver::VfoNoiseBlankerRequest* request,
                            Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setNoiseBlanker(request->id(), request->enabled(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor* GrpcServer::SetNoiseBlankerThreshold(
    grpc::CallbackServerContext* context,
    const Receiver::VfoNoiseBlankerThresholdRequest* request,
    Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setNoiseBlankerThreshold(
        request->id(), request->threshold(), [=](ErrorCode err) {
            response->set_code(ErrorCodeCoreToProto(err));
            reactor->Finish(grpc::Status::OK);
        });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetSqlLevel(grpc::CallbackServerContext* context,
                        const Receiver::VfoDoubleRequest* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setSqlLevel(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetSqlAlpha(grpc::CallbackServerContext* context,
                        const Receiver::VfoDoubleRequest* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setSqlAlpha(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAgcOn(grpc::CallbackServerContext* context,
                     const Receiver::VfoBoolRequest* request,
                     Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setAgcOn(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAgcHang(grpc::CallbackServerContext* context,
                       const Receiver::VfoBoolRequest* request,
                       Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setAgcHang(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAgcThreshold(grpc::CallbackServerContext* context,
                            const Receiver::VfoInt32Request* request,
                            Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setAgcThreshold(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAgcSlope(grpc::CallbackServerContext* context,
                        const Receiver::VfoInt32Request* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setAgcSlope(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAgcDecay(grpc::CallbackServerContext* context,
                        const Receiver::VfoInt32Request* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setAgcDecay(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAgcManualGain(grpc::CallbackServerContext* context,
                             const Receiver::VfoInt32Request* request,
                             Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setAgcManualGain(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetFmMaxDev(grpc::CallbackServerContext* context,
                        const Receiver::VfoFloatRequest* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setFmMaxDev(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetFmDeemph(grpc::CallbackServerContext* context,
                        const Receiver::VfoDoubleRequest* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setFmDeemph(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAmDcr(grpc::CallbackServerContext* context,
                     const Receiver::VfoBoolRequest* request,
                     Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setAmDcr(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAmSyncDcr(grpc::CallbackServerContext* context,
                         const Receiver::VfoBoolRequest* request,
                         Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setAmSyncDcr(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::SetAmSyncPllBw(grpc::CallbackServerContext* context,
                           const Receiver::VfoFloatRequest* request,
                           Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->setAmSyncPllBw(request->value(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::StartAudioRecording(grpc::CallbackServerContext* context,
                                const Receiver::VfoRecordingRequest* request,
                                Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->startAudioRecording(request->path(), [=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::StopAudioRecording(grpc::CallbackServerContext* context,
                               const Receiver::VfoHandle* request,
                               Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->stopAudioRecording([=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::StartSniffer(grpc::CallbackServerContext* context,
                         const Receiver::VfoSnifferRequest* request,
                         Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->startSniffer(request->samplerate(), request->buffsize(),
                      [=](ErrorCode err) {
                          response->set_code(ErrorCodeCoreToProto(err));
                          reactor->Finish(grpc::Status::OK);
                      });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::StopSniffer(grpc::CallbackServerContext* context,
                        const Receiver::VfoHandle* request,
                        Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->stopSniffer([=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::GetSnifferData(grpc::CallbackServerContext* context,
                           const Receiver::VfoHandle* request,
                           Receiver::SnifferDataResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    int buffsize = vfo->getSnifferParams().buffSize;
    response->mutable_data()->Reserve(buffsize);

    vfo->getSnifferData(
        response->mutable_data()->mutable_data(), buffsize,
        [=](ErrorCode err, [[maybe_unused]] float* data, int size) {
            response->set_code(ErrorCodeCoreToProto(err));

            if (err != ErrorCode::OK) {
                reactor->Finish(grpc::Status::OK);
                return;
            }

            response->mutable_data()->AddNAlreadyReserved(size);

            reactor->Finish(grpc::Status::OK);
        });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::StartRdsDecoder(grpc::CallbackServerContext* context,
                            const Receiver::VfoHandle* request,
                            Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->startRdsDecoder([=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::StopRdsDecoder(grpc::CallbackServerContext* context,
                           const Receiver::VfoHandle* request,
                           Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->stopRdsDecoder([=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::ResetRdsParser(grpc::CallbackServerContext* context,
                           const Receiver::VfoHandle* request,
                           Receiver::EmptyResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->resetRdsParser([=](ErrorCode err) {
        response->set_code(ErrorCodeCoreToProto(err));
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

grpc::ServerUnaryReactor*
GrpcServer::GetRdsData(grpc::CallbackServerContext* context,
                       const Receiver::VfoHandle* request,
                       Receiver::RdsDataResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    ENTER_VFO_CONTEXT(reactor, response, request->handle(), vfo);

    vfo->getRdsData([=](ErrorCode err, std::string data, int type) {
        response->set_code(ErrorCodeCoreToProto(err));
        response->set_data(data);
        response->set_type(type);
        reactor->Finish(grpc::Status::OK);
    });

    EXIT_VFO_CONTEXT();

    return reactor;
}

GrpcServer::~GrpcServer() { Shutdown(); }

void GrpcServer::Wait() { server_->Wait(); }
void GrpcServer::Shutdown() { server_->Shutdown(); }

} // namespace violetrx
