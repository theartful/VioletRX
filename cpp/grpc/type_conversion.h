#ifndef VIOLETRX_TYPE_CONVERSION_H
#define VIOLETRX_TYPE_CONVERSION_H

#include <optional>

#include "async_core/events.h"
#include "receiver.pb.h"

namespace violetrx
{

Receiver::ErrorCode ErrorCodeCoreToProto(ErrorCode code);
ErrorCode ErrorCodeProtoToCore(Receiver::ErrorCode code);

Receiver::WindowType WindowTypeCoreToProto(WindowType code);
WindowType WindowProtoToCore(Receiver::WindowType window);

Receiver::FilterShape FilterShapeCoreToProto(FilterShape filter_shape);
FilterShape FilterShapeProtoToCore(Receiver::FilterShape filter_shape);

Receiver::DemodType DemodCoreToProto(Demod demod);
Demod DemodProtoToCore(Receiver::DemodType demod);

Timestamp TimestampProtoToCore(google::protobuf::Timestamp timestamp);
google::protobuf::Timestamp TimestampCoreToProto(Timestamp timestamp);

Event EventProtoToCore(const Receiver::Event& proto_event);
void EventCoreToProto(const Event& event, Receiver::Event* proto_event);

} // namespace violetrx

#endif // VIOLETRX_TYPE_CONVERSION_H
