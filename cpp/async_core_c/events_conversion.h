#ifndef EVENTS_CONVERSION_H
#define EVENTS_CONVERSION_H

#include "async_core/events.h"
#include "async_core_c/events_c.h"
#include "utility/assert.h"

extern "C" {

#define MAX_VIOLET_EVENT_SIZE 64

typedef struct {
    VioletEvent event_type;
    int64_t id;
    VioletTimestamp timestamp;
    unsigned char data[MAX_VIOLET_EVENT_SIZE];
} VioletEventGeneric;
}

namespace core
{

// The lifetime of CEvent is tied to the event it is constructed from.
class CEvent
{
public:
    CEvent(const Event&);
    CEvent(const ReceiverEvent&);
    CEvent(const VfoEvent&);
    ~CEvent();

    CEvent(const CEvent&) = delete;
    CEvent(CEvent&&) = delete;
    CEvent& operator=(const CEvent&) = delete;

    const VioletEventCommon* inner()
    {
        return reinterpret_cast<const VioletEventCommon*>(&inner_);
    }

    const VioletVfoEventCommon* inner_as_vfo_event()
    {
        VIOLET_ASSERT(inner_.event_type >= VIOLET_VFO_SYNC_START &&
                      inner_.event_type < VIOLET_EVENT_UNKNOWN);

        return reinterpret_cast<const VioletVfoEventCommon*>(&inner_);
    }

private:
    VioletEventGeneric inner_;
};

} // namespace core

#endif // EVENTS_CONVERSION_H
