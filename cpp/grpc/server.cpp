#include <atomic>
#include <chrono>
#include <queue>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/server_builder.h>
#include <spdlog/spdlog.h>

#include "async_core/async_vfo_iface.h"
#include "async_core/events_format.h" // IWYU pragma: keep
#include "receiver.pb.h"
#include "server.h"
#include "type_conversion.h"
#include "utility/worker_thread.h"

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

constexpr int kEventsQueueSize = 64;

GrpcServer::GrpcServer(AsyncReceiverIface::sptr async_receiver,
                       const std::string& addr_url) :
    async_receiver_{std::move(async_receiver)},
    events_queue_{kEventsQueueSize},
    last_fft_frame_{},
    other_fft_frame_{}
{
    // FIXME: Should we wait for subscription to succeed before we start the
    // server?
    async_receiver_->subscribe(
        // Fun fact: Using a lambda is better than using std::bind_front for
        // member functions. That's because a lambda holds the member function
        // information in its type, while std::bind_front holds the member
        // function information as a pointer.
        // For example, this lambda should have size 8, while std::bind_front
        // would have around 24 (8 for this, 16 for the function and the virtual
        // table)
        [this](const ReceiverEvent& event) { HandleReceiverEvent(event); },
        [this](ErrorCode err, Connection connection) {
            if (err == ErrorCode::OK) {
                connection_ = std::move(connection);
            } else {
                // FIXME
            }
        });

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

void GrpcServer::HandleReceiverEvent(const ReceiverEvent& event)
{
    spdlog::debug("{}", event);

    // TODO: Assert that we're in the async receiver thread
    events_queue_.push(ToGeneralEvent(event));

    if (std::holds_alternative<VfoAdded>(event)) {
        const auto& vfo_added_event = std::get<VfoAdded>(event);
        const uint64_t handle = vfo_added_event.handle;
        auto vfo = async_receiver_->getVfo(handle);

        vfo->subscribe( //
            [this](const VfoEvent& event) { HandleVfoEvent(event); },
            [this, handle](ErrorCode err, Connection connection) {
                if (err == ErrorCode::OK) {
                    vfo_connections_.emplace(handle,
                                             boost::signals2::scoped_connection(
                                                 std::move(connection)));
                } else {
                    // FIXME
                }
            });
    } else if (std::holds_alternative<VfoRemoved>(event)) {
        const auto& vfo_removed_event = std::get<VfoRemoved>(event);
        const uint64_t handle = vfo_removed_event.handle;

        vfo_connections_.erase(handle);
    }
}
void GrpcServer::HandleVfoEvent(const VfoEvent& event)
{
    spdlog::debug("{}", event);

    // TODO: Assert that we're in the async receiver thread

    // VfoRemoved events are always fired from the receiver, but can be fired
    // from the Vfo as well, so let's not handle them since they will be handled
    // in HandleReceiverEvent
    if (std::holds_alternative<VfoRemoved>(event)) {
        return;
    }

    events_queue_.push(ToGeneralEvent(event));
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

    async_receiver_->stop([=](ErrorCode err) {
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

bool IsFftFrameStillValid(const FftFrame& frame)
{
    static constexpr int kMaxFramerate = 100;
    static constexpr auto kMaxDuration =
        std::chrono::milliseconds(1000) / kMaxFramerate;

    return std::chrono::system_clock::now() - frame.timestamp.ToTimepoint() <=
           kMaxDuration;
}

grpc::ServerUnaryReactor*
GrpcServer::GetFftData(grpc::CallbackServerContext* context,
                       [[maybe_unused]] const google::protobuf::Empty* request,
                       Receiver::FftFrameResponse* response)
{
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();

    // Check if cached fft_frame is still valid.
    do {
        std::shared_lock<std::shared_mutex> lk{fft_mutex_};

        if (IsFftFrameStillValid(last_fft_frame_)) {
            Receiver::FftFrame* fft_frame = new Receiver::FftFrame();
            FftFrameCoreToProto(last_fft_frame_, fft_frame);

            response->set_code(ErrorCodeCoreToProto(ErrorCode::OK));
            response->set_allocated_fft_frame(fft_frame);

            reactor->Finish(grpc::Status::OK);
            return reactor;
        }
    } while (false);

    // We have to update the fft_frame since it's not valid.
    bool expected = false;
    const bool desired = true;
    if (updating_fft_frame_.compare_exchange_strong(expected, desired)) {
        // We're the thread that is going to update the fft frame.

        // FIXME: This might fail if the fft frame size changed.
        int fft_size = async_receiver_->getIqFftSize();
        other_fft_frame_.fft_points.resize(fft_size);

        async_receiver_->getIqFftData(
            other_fft_frame_.fft_points.data(),
            other_fft_frame_.fft_points.size(),
            // Callback
            [=, this](ErrorCode err, Timestamp timestamp, int64_t center_freq,
                      int sample_rate, [[maybe_unused]] float* data,
                      int fft_size) mutable {
                // Here we have to set `updating_fft_frame_` to false to allow
                // for other threads to update the frame, and we have to swap
                // last_fft_frame and other_fft_frame before we set it to false
                // if we succeeded.

                response->set_code(ErrorCodeCoreToProto(err));

                if (err != ErrorCode::OK) {
                    updating_fft_frame_.store(false, std::memory_order_relaxed);

                    reactor->Finish(grpc::Status::OK);
                    return;
                }

                other_fft_frame_.sample_rate = sample_rate;
                other_fft_frame_.center_freq = center_freq;
                other_fft_frame_.timestamp = timestamp;
                other_fft_frame_.fft_points.resize(fft_size);

                Receiver::FftFrame* fft_frame = new Receiver::FftFrame();
                FftFrameCoreToProto(other_fft_frame_, fft_frame);

                response->set_code(ErrorCodeCoreToProto(ErrorCode::OK));
                response->set_allocated_fft_frame(fft_frame);

                reactor->Finish(grpc::Status::OK);

                do {
                    std::unique_lock lk{fft_mutex_};
                    std::swap(last_fft_frame_, other_fft_frame_);
                    updating_fft_frame_.store(false, std::memory_order_release);
                } while (false);
            });
    } else {
        // Fft frame is currently being updated.
        // FIXME: We will return the old fft_frame for the sake of simplicity.
        // but ideally, we should use a condition_variable and wait until the
        // fft frame has been updated
        Receiver::FftFrame* fft_frame = new Receiver::FftFrame();
        do {
            std::shared_lock<std::shared_mutex> lk{fft_mutex_};
            FftFrameCoreToProto(last_fft_frame_, fft_frame);
        } while (false);

        response->set_code(ErrorCodeCoreToProto(ErrorCode::OK));
        response->set_allocated_fft_frame(fft_frame);

        reactor->Finish(grpc::Status::OK);
    }
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

class GrpcServer::EventsReactor
    : public grpc::ServerWriteReactor<Receiver::Event>
{
public:
    EventsReactor(grpc::CallbackServerContext* context, GrpcServer* server) :
        context_{context},
        server_{server},
        finished_{false},
        unsubscribed_{false}
    {
        spdlog::info("GrpcServer: Client ({}) has subscribed",
                     context_->peer());

        server_->async_receiver_->synchronize([this](ErrorCode err) {
            if (err != ErrorCode::OK) {
                // FIXME: Should probably pass the error somehow.
                Finish(grpc::Status::CANCELLED);
                return;
            }

            // Setting up sync events. We begin with a SyncStart event.
            sync_events_.push(
                SyncStart{{.id = -1, .timestamp = Timestamp::Now()}});

            // First we set up sync events from the receiver.
            std::vector<ReceiverEvent> events =
                server_->async_receiver_->getStateAsEvents();

            for (const ReceiverEvent& event : events) {
                sync_events_.push(ToGeneralEvent(event));
            }

            // Now we set up sync events from each vfo.
            auto vfos = server_->async_receiver_->getVfos();
            for (const auto& vfo : vfos) {
                std::vector<VfoEvent> vfo_events = vfo->getStateAsEvents();

                for (const VfoEvent& event : vfo_events) {
                    sync_events_.push(ToGeneralEvent(event));
                }
            }

            // We finish with a sync end event.
            sync_events_.push(
                SyncEnd{{.id = -1, .timestamp = Timestamp::Now()}});

            // Now subscribe so that when we finish sending the sync events,
            // we continue with the new events that happened directly after the
            // point in time sync events were taken from.
            events_reader_ = server_->events_queue_.subscribe();

            // Let's send the first sync event (SyncStart)
            WriteSyncEvent();

            // Start the worker thread, which will be used to wait on the events
            // queue.
            worker_thread_.start();
        });
    }

    void WriteSyncEvent()
    {
        bool convertible_event;
        do {
            convertible_event = WriteEvent(sync_events_.front());
            sync_events_.pop();
        } while (!convertible_event);
    }

    void WaitAndWriteEvent()
    {
        Event event;
        while (true) {
            broadcast_queue::Error err = events_reader_.wait_dequeue_timed(
                &event, std::chrono::seconds(5));

            switch (err) {
            case broadcast_queue::Error::None:
                if (std::holds_alternative<Unsubscribed>(event)) {
                    unsubscribed_ = true;
                }

                if (WriteEvent(event)) {
                    return;
                }
                break;
            case broadcast_queue::Error::Timeout:
                // Keep trying!
                break;
            case broadcast_queue::Error::Lagged:
                spdlog::info("Client ({}) lagged. Disconnecting...",
                             context_->peer());
                FinishIfNotAlreadyFinished(grpc::Status::CANCELLED);
                return;

            case broadcast_queue::Error::Closed:
                // Events queue closed. This can only mean that we're shutting
                // down. So we just return.
                FinishIfNotAlreadyFinished(grpc::Status::CANCELLED);
                return;
            }
        }
    }

    void FinishIfNotAlreadyFinished(const grpc::Status& status)
    {
        bool expected = false;

        // CAS shouldn't be necessary since the threads running this class never
        // intersect, but just to be safe.
        if (finished_.compare_exchange_strong(expected, true)) {
            Finish(status);
        }
    }

    // Returns false if the event doesn't have an equivalent proto event.
    bool WriteEvent(const Event& event)
    {
        bool ok = EventCoreToProto(event, &response_);

        if (ok) {
            StartWrite(&response_);
        }

        return ok;
    }

    void OnWriteDone(bool ok) override
    {
        if (!ok) {
            spdlog::info("Failed to send event to ({}). Disconnecting...",
                         context_->peer());
            FinishIfNotAlreadyFinished(grpc::Status::OK);
            return;
        }

        // If there are still sync events, let's write it.
        if (!sync_events_.empty()) {
            WriteSyncEvent();
        } else if (unsubscribed_) {
            Finish(grpc::Status::OK);
        } else {
            worker_thread_.schedule("WaitAndWriteEvent",
                                    [this]() { WaitAndWriteEvent(); });
        }
    }

    void OnDone() override
    {
        spdlog::info("GrpcServer: Finished dealing with ({})",
                     context_->peer());
        // Very scary!
        delete this;
    }

private:
    grpc::CallbackServerContext* context_;
    Receiver::Event response_;
    GrpcServer* server_;

    // First events to send, and after we finish them, we read from the
    // broadcast_queue
    std::queue<Event> sync_events_;
    broadcast_queue::receiver<Event> events_reader_;

    // Is using a worker thread an overkill? Maybe use a raw thread?
    WorkerThread worker_thread_;

    // There are two logical threads: The worker thread, and the grpc callback
    // thread. They never intersect, but just to be safe.
    std::atomic<bool> finished_;
    std::atomic<bool> unsubscribed_;
};

grpc::ServerWriteReactor<Receiver::Event>*
GrpcServer::Subscribe(grpc::CallbackServerContext* context,
                      [[maybe_unused]] const google::protobuf::Empty* request)
{
    return new EventsReactor(context, this);
}

GrpcServer::~GrpcServer() { Shutdown(); }

void GrpcServer::Wait() { server_->Wait(); }
void GrpcServer::Shutdown()
{
    // Manually close the queue so that subscribers finish their wait loop for
    // new events.
    events_queue_.close();

    server_->Shutdown();
}

} // namespace violetrx
