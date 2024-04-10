#ifndef VIOLETRX_GRPC_SERVER_H
#define VIOLETRX_GRPC_SERVER_H

#include <memory>
#include <shared_mutex>
#include <string>

#include <boost/signals2.hpp>
#include <broadcast_queue.h>
#include <unordered_map>

#include "async_core/async_receiver_iface.h"
#include "async_core/types.h"
#include "receiver.grpc.pb.h"

namespace violetrx
{

class GrpcServer : public Receiver::Rx::CallbackService
{
public:
    GrpcServer(violetrx::AsyncReceiverIface::sptr async_receiver,
               const std::string& addr_url);
    ~GrpcServer();

    void Shutdown();
    void Wait();

private:
    // Runs in the async receiver thread
    void HandleReceiverEvent(const ReceiverEvent&);
    void HandleVfoEvent(const VfoEvent&);

private:
    grpc::ServerUnaryReactor* Start(grpc::CallbackServerContext* context,
                                    const google::protobuf::Empty* request,
                                    Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor* Stop(grpc::CallbackServerContext* context,
                                   const google::protobuf::Empty* request,
                                   Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetInputDevice(grpc::CallbackServerContext* context,
                   const google::protobuf::StringValue* request,
                   Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAntenna(grpc::CallbackServerContext* context,
               const google::protobuf::StringValue* request,
               Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetInputRate(grpc::CallbackServerContext* context,
                 const google::protobuf::UInt32Value* request,
                 Receiver::UInt32Response* response) override;

    grpc::ServerUnaryReactor*
    SetInputDecim(grpc::CallbackServerContext* context,
                  const google::protobuf::UInt32Value* request,
                  Receiver::UInt32Response* response) override;

    grpc::ServerUnaryReactor*
    SetIqSwap(grpc::CallbackServerContext* context,
              const google::protobuf::BoolValue* request,
              Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetDcCancel(grpc::CallbackServerContext* context,
                const google::protobuf::BoolValue* request,
                Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetIqBalance(grpc::CallbackServerContext* context,
                 const google::protobuf::BoolValue* request,
                 Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAutoGain(grpc::CallbackServerContext* context,
                const google::protobuf::BoolValue* request,
                Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetRfFreq(grpc::CallbackServerContext* context,
              const google::protobuf::UInt64Value* request,
              Receiver::UInt64Response* response) override;

    grpc::ServerUnaryReactor*
    SetGain(grpc::CallbackServerContext* context,
            const Receiver::SetGainRequest* request,
            Receiver::DoubleResponse* response) override;

    grpc::ServerUnaryReactor*
    SetFreqCorr(grpc::CallbackServerContext* context,
                const google::protobuf::DoubleValue* request,
                Receiver::DoubleResponse* response) override;

    grpc::ServerUnaryReactor*
    SetFftSize(grpc::CallbackServerContext* context,
               const google::protobuf::UInt32Value* request,
               Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetFftWindow(grpc::CallbackServerContext* context,
                 const Receiver::SetFftWindowRequest* request,
                 Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    GetFftData(grpc::CallbackServerContext* context,
               const google::protobuf::Empty* request,
               Receiver::FftFrameResponse* response) override;

    grpc::ServerUnaryReactor*
    AddVfoChannel(grpc::CallbackServerContext* context,
                  const google::protobuf::Empty* request,
                  Receiver::VfoResponse* response) override;

    grpc::ServerUnaryReactor*
    RemoveVfoChannel(grpc::CallbackServerContext* context,
                     const Receiver::VfoHandle* request,
                     Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetFilterOffset(grpc::CallbackServerContext* context,
                    const Receiver::VfoUInt64Request* request,
                    Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetFilter(grpc::CallbackServerContext* context,
              const Receiver::VfoFilterRequest* request,
              Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetCwOffset(grpc::CallbackServerContext* context,
                const Receiver::VfoUInt64Request* request,
                Receiver::EmptyResponse* response) override;
    grpc::ServerUnaryReactor*
    SetDemod(grpc::CallbackServerContext* context,
             const Receiver::VfoDemodRequest* request,
             Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    GetSignalPwr(grpc::CallbackServerContext* context,
                 const Receiver::VfoHandle* request,
                 Receiver::FloatResponse* response) override;

    grpc::ServerUnaryReactor*
    SetNoiseBlanker(grpc::CallbackServerContext* context,
                    const Receiver::VfoNoiseBlankerRequest* request,
                    Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor* SetNoiseBlankerThreshold(
        grpc::CallbackServerContext* context,
        const Receiver::VfoNoiseBlankerThresholdRequest* request,
        Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetSqlLevel(grpc::CallbackServerContext* context,
                const Receiver::VfoDoubleRequest* request,
                Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetSqlAlpha(grpc::CallbackServerContext* context,
                const Receiver::VfoDoubleRequest* request,
                Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAgcOn(grpc::CallbackServerContext* context,
             const Receiver::VfoBoolRequest* request,
             Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAgcHang(grpc::CallbackServerContext* context,
               const Receiver::VfoBoolRequest* request,
               Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAgcThreshold(grpc::CallbackServerContext* context,
                    const Receiver::VfoInt32Request* request,
                    Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAgcSlope(grpc::CallbackServerContext* context,
                const Receiver::VfoInt32Request* request,
                Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAgcDecay(grpc::CallbackServerContext* context,
                const Receiver::VfoInt32Request* request,
                Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAgcManualGain(grpc::CallbackServerContext* context,
                     const Receiver::VfoInt32Request* request,
                     Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetFmMaxDev(grpc::CallbackServerContext* context,
                const Receiver::VfoFloatRequest* request,
                Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetFmDeemph(grpc::CallbackServerContext* context,
                const Receiver::VfoDoubleRequest* request,
                Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAmDcr(grpc::CallbackServerContext* context,
             const Receiver::VfoBoolRequest* request,
             Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAmSyncDcr(grpc::CallbackServerContext* context,
                 const Receiver::VfoBoolRequest* request,
                 Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    SetAmSyncPllBw(grpc::CallbackServerContext* context,
                   const Receiver::VfoFloatRequest* request,
                   Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    StartAudioRecording(grpc::CallbackServerContext* context,
                        const Receiver::VfoRecordingRequest* request,
                        Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    StopAudioRecording(grpc::CallbackServerContext* context,
                       const Receiver::VfoHandle* request,
                       Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    StartSniffer(grpc::CallbackServerContext* context,
                 const Receiver::VfoSnifferRequest* request,
                 Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    StopSniffer(grpc::CallbackServerContext* context,
                const Receiver::VfoHandle* request,
                Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    GetSnifferData(grpc::CallbackServerContext* context,
                   const Receiver::VfoHandle* request,
                   Receiver::SnifferDataResponse* response) override;

    grpc::ServerUnaryReactor*
    StartRdsDecoder(grpc::CallbackServerContext* context,
                    const Receiver::VfoHandle* request,
                    Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    StopRdsDecoder(grpc::CallbackServerContext* context,
                   const Receiver::VfoHandle* request,
                   Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    ResetRdsParser(grpc::CallbackServerContext* context,
                   const Receiver::VfoHandle* request,
                   Receiver::EmptyResponse* response) override;

    grpc::ServerUnaryReactor*
    GetRdsData(grpc::CallbackServerContext* context,
               const Receiver::VfoHandle* request,
               Receiver::RdsDataResponse* response) override;

    grpc::ServerWriteReactor<Receiver::Event>*
    Subscribe(grpc::CallbackServerContext* context,
              const google::protobuf::Empty* request) override;

private:
    class EventsReactor;

private:
    std::unique_ptr<grpc::Server> server_;
    violetrx::AsyncReceiverIface::sptr async_receiver_;

    boost::signals2::scoped_connection connection_;
    std::unordered_map<uint64_t, boost::signals2::scoped_connection>
        vfo_connections_;
    // FIXME: Currently broadcast_queue is single producer multiple consumer.
    // But this queue will accept events from the receiver and the vfos, so the
    // underlying assumption here is that these events will come from a single
    // thread, which is true but maybe it's time to extend the broadcast queue
    // to be multiple producer?
    broadcast_queue::sender<Event> events_queue_;

    // Fft caching
    FftFrame last_fft_frame_;
    FftFrame other_fft_frame_; // Ping-ponging between two fft frames

    std::shared_mutex fft_mutex_;
    std::atomic<bool> updating_fft_frame_;
};

} // namespace violetrx

#endif // VIOLETRX_GRPC_SERVER_H
