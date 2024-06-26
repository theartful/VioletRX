#ifndef VIOLETRX_TYPE_CONVERSION_H
#define VIOLETRX_TYPE_CONVERSION_H

#include <optional>

#include "async_core/events.h"
#include "receiver.pb.h"

namespace violetrx
{

Receiver::ErrorCode ErrorCodeCoreToProto(ErrorCode code);
ErrorCode ErrorCodeProtoToCore(Receiver::ErrorCode code);

Receiver::WindowType WindowCoreToProto(WindowType code);
WindowType WindowProtoToCore(Receiver::WindowType window);

Receiver::FilterShape FilterShapeCoreToProto(FilterShape filter_shape);
FilterShape FilterShapeProtoToCore(Receiver::FilterShape filter_shape);

Receiver::DemodType DemodCoreToProto(Demod demod);
Demod DemodProtoToCore(Receiver::DemodType demod);

Timestamp TimestampProtoToCore(google::protobuf::Timestamp timestamp);
google::protobuf::Timestamp TimestampCoreToProto(Timestamp timestamp);

std::optional<Event> EventProtoToCore(const Receiver::Event& proto_event);
bool EventCoreToProto(const Event& event, Receiver::Event* proto_event);

Device DeviceProtoToCore(const Receiver::Device& proto_device);

void FftFrameCoreToProto(const FftFrame& frame,
                         Receiver::FftFrame* proto_frame);

} // namespace violetrx

#endif // VIOLETRX_TYPE_CONVERSION_H
