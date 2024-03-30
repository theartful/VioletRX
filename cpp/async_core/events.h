#ifndef CORE_EVENTS
#define CORE_EVENTS

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <function2/function2.hpp>

#include "async_core/types.h"

namespace violetrx
{

struct EventCommon {
    static int64_t last_id;

    int64_t id;
    Timestamp timestamp;

    static EventCommon make()
    {
        return EventCommon{last_id++, Timestamp::Now()};
    }
};

struct VfoEventCommon : public EventCommon {
    uint64_t handle;

    static VfoEventCommon make(uint64_t handle)
    {
        return VfoEventCommon{EventCommon::make(), handle};
    }
};

struct SyncStart : public EventCommon {
};
struct SyncEnd : public EventCommon {
};
struct Unsubscribed : public EventCommon {
};
struct Started : public EventCommon {
};
struct Stopped : public EventCommon {
};
struct InputDeviceChanged : public EventCommon {
    std::string device;
};
struct AntennaChanged : public EventCommon {
    std::string antenna;
};
struct InputRateChanged : public EventCommon {
    int32_t input_rate;
};
struct InputDecimChanged : public EventCommon {
    int32_t decim;
};
struct IqSwapChanged : public EventCommon {
    bool enabled;
};
struct DcCancelChanged : public EventCommon {
    bool enabled;
};
struct IqBalanceChanged : public EventCommon {
    bool enabled;
};
struct RfFreqChanged : public EventCommon {
    int64_t freq;
};
struct GainStagesChanged : public EventCommon {
    std::vector<GainStage> stages;
};
struct AntennasChanged : public EventCommon {
    std::vector<std::string> antennas;
};
struct AutoGainChanged : public EventCommon {
    bool enabled;
};
struct GainChanged : public EventCommon {
    std::string name;
    double value;
};
struct FreqCorrChanged : public EventCommon {
    double ppm;
};
struct FftSizeChanged : public EventCommon {
    int32_t size;
};
struct FftWindowChanged : public EventCommon {
    WindowType window;
};
struct IqRecordingStarted : public EventCommon {
    std::string path;
};
struct IqRecordingStopped : public EventCommon {
};
struct VfoSyncStart : public VfoEventCommon {
};
struct VfoSyncEnd : public VfoEventCommon {
};
struct VfoAdded : public VfoEventCommon {
};
struct VfoRemoved : public VfoEventCommon {
};
struct DemodChanged : public VfoEventCommon {
    Demod demod;
};
struct OffsetChanged : public VfoEventCommon {
    int32_t offset;
};
struct CwOffsetChanged : public VfoEventCommon {
    int32_t offset;
};
struct FilterChanged : public VfoEventCommon {
    FilterShape shape;
    int32_t low;
    int32_t high;

    Filter filter() const { return Filter{shape, low, high}; }
};
struct NoiseBlankerOnChanged : public VfoEventCommon {
    int nb_id;
    bool enabled;
};
struct NoiseBlankerThresholdChanged : public VfoEventCommon {
    int nb_id;
    float threshold;
};
struct SqlLevelChanged : public VfoEventCommon {
    double level;
};
struct SqlAlphaChanged : public VfoEventCommon {
    double alpha;
};
struct AgcOnChanged : public VfoEventCommon {
    bool enabled;
};
struct AgcHangChanged : public VfoEventCommon {
    bool enabled;
};
struct AgcThresholdChanged : public VfoEventCommon {
    int threshold;
};
struct AgcSlopeChanged : public VfoEventCommon {
    int slope;
};
struct AgcDecayChanged : public VfoEventCommon {
    int decay;
};
struct AgcManualGainChanged : public VfoEventCommon {
    int gain;
};
struct FmMaxDevChanged : public VfoEventCommon {
    float maxdev;
};
struct FmDeemphChanged : public VfoEventCommon {
    double tau;
};
struct AmDcrChanged : public VfoEventCommon {
    bool enabled;
};
struct AmSyncDcrChanged : public VfoEventCommon {
    bool enabled;
};
struct AmSyncPllBwChanged : public VfoEventCommon {
    float bw;
};
struct RecordingStarted : public VfoEventCommon {
    std::string path;
};
struct RecordingStopped : public VfoEventCommon {
};
struct SnifferStarted : public VfoEventCommon {
    int sample_rate;
    int size;
};
struct SnifferStopped : public VfoEventCommon {
};
struct RdsDecoderStarted : public VfoEventCommon {
};
struct RdsDecoderStopped : public VfoEventCommon {
};
struct RdsParserReset : public VfoEventCommon {
};
struct UdpStreamingStarted : public VfoEventCommon {
    std::string host;
    int port;
    bool stereo;
};
struct UdpStreamingStopped : public VfoEventCommon {
};

// FIXME: audio should be client side, and hence this should be removed
struct AudioGainChanged : public VfoEventCommon {
    float gain;
};

using ReceiverEvent =
    std::variant<SyncStart, SyncEnd, Unsubscribed, Started, Stopped,
                 InputDeviceChanged, AntennaChanged, InputRateChanged,
                 InputDecimChanged, IqSwapChanged, DcCancelChanged,
                 IqBalanceChanged, RfFreqChanged, GainStagesChanged,
                 AntennasChanged, AutoGainChanged, GainChanged, FreqCorrChanged,
                 FftSizeChanged, FftWindowChanged, IqRecordingStarted,
                 IqRecordingStopped, VfoAdded, VfoRemoved>;

using VfoEvent = std::variant<
    VfoSyncStart, VfoSyncEnd, DemodChanged, OffsetChanged, CwOffsetChanged,
    FilterChanged, NoiseBlankerOnChanged, NoiseBlankerThresholdChanged,
    SqlLevelChanged, SqlAlphaChanged, AgcOnChanged, AgcHangChanged,
    AgcThresholdChanged, AgcSlopeChanged, AgcDecayChanged, AgcManualGainChanged,
    FmMaxDevChanged, FmDeemphChanged, AmDcrChanged, AmSyncDcrChanged,
    AmSyncPllBwChanged, RecordingStarted, RecordingStopped, SnifferStarted,
    SnifferStopped, UdpStreamingStarted, UdpStreamingStopped, RdsDecoderStarted,
    RdsDecoderStopped, RdsParserReset, AudioGainChanged, VfoRemoved>;

using Event = std::variant<
    SyncStart, SyncEnd, Unsubscribed, Started, Stopped, InputDeviceChanged,
    AntennaChanged, InputRateChanged, InputDecimChanged, IqSwapChanged,
    DcCancelChanged, IqBalanceChanged, RfFreqChanged, GainStagesChanged,
    AntennasChanged, AutoGainChanged, GainChanged, FreqCorrChanged,
    FftSizeChanged, FftWindowChanged, IqRecordingStarted, IqRecordingStopped,
    VfoAdded, VfoRemoved, AudioGainChanged, VfoSyncStart, VfoSyncEnd,
    DemodChanged, OffsetChanged, CwOffsetChanged, FilterChanged,
    NoiseBlankerOnChanged, NoiseBlankerThresholdChanged, SqlLevelChanged,
    SqlAlphaChanged, AgcOnChanged, AgcHangChanged, AgcThresholdChanged,
    AgcSlopeChanged, AgcDecayChanged, AgcManualGainChanged, FmMaxDevChanged,
    FmDeemphChanged, AmDcrChanged, AmSyncDcrChanged, AmSyncPllBwChanged,
    RecordingStarted, RecordingStopped, SnifferStarted, SnifferStopped,
    UdpStreamingStarted, UdpStreamingStopped, RdsDecoderStarted,
    RdsDecoderStopped, RdsParserReset>;

inline constexpr bool IsReceiverEvent(const Event& event)
{
    return std::visit(
        [](const auto& specific_event) {
            using EventType = std::decay_t<decltype(specific_event)>;
            return std::is_convertible_v<EventType, ReceiverEvent>;
        },
        event);
}

inline constexpr bool IsVfoEvent(const Event& event)
{
    return std::visit(
        [](const auto& specific_event) {
            using EventType = std::decay_t<decltype(specific_event)>;
            return std::is_convertible_v<EventType, VfoEvent>;
        },
        event);
}

inline constexpr Event ToGeneralEvent(ReceiverEvent event)
{
    return std::visit(
        [](auto&& specific_event) -> Event { return specific_event; },
        std::move(event));
}

inline constexpr Event ToGeneralEvent(VfoEvent event)
{
    return std::visit(
        [](auto&& specific_event) -> Event { return specific_event; },
        std::move(event));
}

inline constexpr std::optional<ReceiverEvent> ToReceiverEvent(Event event)
{
    return std::visit(
        [](auto&& specific_event) -> std::optional<ReceiverEvent> {
            using EventType = std::decay_t<decltype(specific_event)>;

            if constexpr (std::is_convertible_v<EventType, ReceiverEvent>) {
                return ReceiverEvent(std::move(specific_event));
            } else {
                return std::nullopt;
            }
        },
        std::move(event));
}

inline constexpr std::optional<VfoEvent> ToVfoEvent(Event event)
{
    return std::visit(
        [](auto&& specific_event) -> std::optional<VfoEvent> {
            using EventType = std::decay_t<decltype(specific_event)>;

            if constexpr (std::is_convertible_v<EventType, VfoEvent>) {
                return VfoEvent(std::move(specific_event));
            } else {
                return std::nullopt;
            }
        },
        std::move(event));
}

using EventHandler = fu2::function<void(const Event&)>;
using ReceiverEventHandler = fu2::function<void(const ReceiverEvent&)>;
using VfoEventHandler = fu2::function<void(const VfoEvent&)>;

template <class... Ts>
struct Visitor : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
Visitor(Ts...) -> Visitor<Ts...>;

static_assert(IsReceiverEvent(Event{SyncStart{}}));
static_assert(!IsReceiverEvent(Event{VfoSyncStart{}}));

static_assert(IsVfoEvent(Event{VfoSyncStart{}}));
static_assert(!IsVfoEvent(Event{SyncStart{}}));

static_assert(!ToVfoEvent(Event{SyncStart{}}).has_value());
static_assert(ToVfoEvent(Event{VfoSyncStart{}}).has_value());

static_assert(ToReceiverEvent(Event{SyncStart{}}).has_value());
static_assert(!ToReceiverEvent(Event{VfoSyncStart{}}).has_value());

} // namespace violetrx

#endif
