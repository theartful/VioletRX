#ifndef VIOLETRX_EVENTS_FORMAT_H
#define VIOLETRX_EVENTS_FORMAT_H

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include "async_core/events.h"

template <>
struct fmt::formatter<violetrx::Timestamp> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::Timestamp& timestamp, FormatContext& ctx) const
    {
        // FIXME: Maybe print time in a better format than seconds and nanos!
        return fmt::format_to(ctx.out(), "Timestamp(seconds={}, nanos={})",
                              timestamp.seconds, timestamp.nanos);
    }
};

template <>
struct fmt::formatter<violetrx::Unsubscribed> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::Unsubscribed& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "Unsubscribed(id={}, time={})", tx.id,
                              tx.timestamp);
    }
};

template <>
struct fmt::formatter<violetrx::Started> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::Started& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "Started(id={}, time={})", tx.id,
                              tx.timestamp);
    }
};
template <>
struct fmt::formatter<violetrx::Stopped> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::Stopped& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "Stopped(id={}, time={})", tx.id,
                              tx.timestamp);
    }
};
template <>
struct fmt::formatter<violetrx::FftSizeChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::FftSizeChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "FftSizeChanged(id={}, time={}, size={})", tx.id,
                              tx.timestamp, tx.size);
    }
};
template <>
struct fmt::formatter<violetrx::FftWindowChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::FftWindowChanged& tx, FormatContext& ctx) const
    {
        // FIXME: WindowType to str
        return fmt::format_to(ctx.out(),
                              "FftWindowChanged(id={}, time={}, window={})",
                              tx.id, tx.timestamp, static_cast<int>(tx.window));
    }
};
template <>
struct fmt::formatter<violetrx::InputDeviceChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::InputDeviceChanged& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "InputDeviceChanged(id={}, time={}, device='{}')",
                              tx.id, tx.timestamp, tx.device);
    }
};
template <>
struct fmt::formatter<violetrx::InputDecimChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::InputDecimChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "InputDecimChanged(id={}, time={}, decim={})",
                              tx.id, tx.timestamp, tx.decim);
    }
};
template <>
struct fmt::formatter<violetrx::InputRateChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::InputRateChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "InputRateChanged(id={}, time={}, rate={})",
                              tx.id, tx.timestamp, tx.input_rate);
    }
};
template <>
struct fmt::formatter<violetrx::IqSwapChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::IqSwapChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "IqSwapChanged(id={}, time={}, enabled={})",
                              tx.id, tx.timestamp, tx.enabled);
    }
};
template <>
struct fmt::formatter<violetrx::AutoGainChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AutoGainChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "AutoGainChanged(id={}, time={}, enabled={})",
                              tx.id, tx.timestamp, tx.enabled);
    }
};
template <>
struct fmt::formatter<violetrx::FreqCorrChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::FreqCorrChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "FreqCorrChanged(id={}, time={}, ppm={})", tx.id,
                              tx.timestamp, tx.ppm);
    }
};
template <>
struct fmt::formatter<violetrx::DcCancelChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::DcCancelChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "DcCancelChanged(id={}, time={}, enabled={})",
                              tx.id, tx.timestamp, tx.enabled);
    }
};
template <>
struct fmt::formatter<violetrx::IqBalanceChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::IqBalanceChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "IqBalanceChanged(id={}, time={}, enabled={})",
                              tx.id, tx.timestamp, tx.enabled);
    }
};
template <>
struct fmt::formatter<violetrx::RfFreqChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::RfFreqChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "RfFreqChanged(id={}, time={}, freq={})", tx.id,
                              tx.timestamp, tx.freq);
    }
};
template <>
struct fmt::formatter<violetrx::GainStage> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::GainStage& stage, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "GainStage(name='{}', start={}, stop={}, step={}, value={})",
            stage.name, stage.start, stage.stop, stage.step, stage.value);
    }
};
template <>
struct fmt::formatter<violetrx::GainStagesChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::GainStagesChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "GainStagesChanged(id={}, time={}, stages=[{}])",
                              tx.id, tx.timestamp, fmt::join(tx.stages, ", "));
    }
};
template <>
struct fmt::formatter<violetrx::AntennaChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AntennaChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "AntennaChanged(id={}, time={}, antenna='{}')",
                              tx.id, tx.timestamp, tx.antenna);
    }
};
template <>
struct fmt::formatter<violetrx::AntennasChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AntennasChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "AntennasChanged(id={}, time={}, antenna=[{}])", tx.id,
            tx.timestamp, fmt::join(tx.antennas, ", "));
    }
};
template <>
struct fmt::formatter<violetrx::VfoAdded> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::VfoAdded& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "VfoAdded(id={}, time={}, handle={})",
                              tx.id, tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::VfoRemoved> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::VfoRemoved& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "VfoRemoved(id={}, time={}, handle={})", tx.id,
                              tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::SyncStart> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::SyncStart& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "SyncStart(id={}, time={})", tx.id,
                              tx.timestamp);
    }
};
template <>
struct fmt::formatter<violetrx::SyncEnd> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::SyncEnd& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "SyncEnd(id={}, time={})", tx.id,
                              tx.timestamp);
    }
};
template <>
struct fmt::formatter<violetrx::VfoSyncStart> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::VfoSyncStart& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "VfoSyncStart(id={}, time={}, handle={})", tx.id,
                              tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::VfoSyncEnd> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::VfoSyncEnd& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "VfoSyncEnd(id={}, time={}, handle={})", tx.id,
                              tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::DemodChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::DemodChanged& tx, FormatContext& ctx) const
    {
        // TODO: demod to str
        return fmt::format_to(
            ctx.out(), "DemodChanged(id={}, time={}, handle={}, demod={})",
            tx.id, tx.timestamp, tx.handle, static_cast<int>(tx.demod));
    }
};
template <>
struct fmt::formatter<violetrx::OffsetChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::OffsetChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "OffsetChanged(id={}, time={}, handle={}, offset={})",
            tx.id, tx.timestamp, tx.handle, tx.offset);
    }
};
template <>
struct fmt::formatter<violetrx::CwOffsetChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::CwOffsetChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "CwOffsetChanged(id={}, time={}, handle={}, offset={})",
            tx.id, tx.timestamp, tx.handle, tx.offset);
    }
};
template <>
struct fmt::formatter<violetrx::FilterChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::FilterChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "FilterChanged(id={}, time={}, handle={}, "
                              "shape={}, low={}, high={})",
                              tx.id, tx.timestamp, tx.handle,
                              static_cast<int>(tx.shape), tx.low, tx.high);
    }
};
template <>
struct fmt::formatter<violetrx::NoiseBlankerOnChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::NoiseBlankerOnChanged& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "NoiseBlankerOnChanged(id={}, time={}, "
                              "handle={}, nb_id={}, enabled={})",
                              tx.id, tx.timestamp, tx.handle, tx.nb_id,
                              tx.enabled);
    }
};
template <>
struct fmt::formatter<violetrx::NoiseBlankerThresholdChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::NoiseBlankerThresholdChanged& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "NoiseBlankerThresholdChanged(id={}, time={}, "
                              "handle={}, nb_id={}, threshold={})",
                              tx.id, tx.timestamp, tx.handle, tx.nb_id,
                              tx.threshold);
    }
};
template <>
struct fmt::formatter<violetrx::AgcOnChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AgcOnChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "AgcOnChanged(id={}, time={}, handle={}, enabled={})",
            tx.id, tx.timestamp, tx.handle, tx.enabled);
    }
};
template <>
struct fmt::formatter<violetrx::AgcHangChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AgcHangChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "AgcHangChanged(id={}, time={}, handle={}, enabled={})",
            tx.id, tx.timestamp, tx.handle, tx.enabled);
    }
};
template <>
struct fmt::formatter<violetrx::AgcThresholdChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AgcThresholdChanged& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "AgcThresholdChanged(id={}, time={}, handle={}, threshold={})",
            tx.id, tx.timestamp, tx.handle, tx.threshold);
    }
};
template <>
struct fmt::formatter<violetrx::AgcSlopeChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AgcSlopeChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "AgcSlopeChanged(id={}, time={}, handle={}, slope={})",
            tx.id, tx.timestamp, tx.handle, tx.slope);
    }
};
template <>
struct fmt::formatter<violetrx::AgcDecayChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AgcDecayChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "AgcDecayChanged(id={}, time={}, handle={}, decay={})",
            tx.id, tx.timestamp, tx.handle, tx.decay);
    }
};
template <>
struct fmt::formatter<violetrx::AgcManualGainChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AgcManualGainChanged& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "AgcManualGainChanged(id={}, time={}, handle={}, gain={})", tx.id,
            tx.timestamp, tx.handle, tx.gain);
    }
};
template <>
struct fmt::formatter<violetrx::FmMaxDevChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::FmMaxDevChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "FmMaxDevChanged(id={}, time={}, handle={}, maxdev={})",
            tx.id, tx.timestamp, tx.handle, tx.maxdev);
    }
};
template <>
struct fmt::formatter<violetrx::FmDeemphChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::FmDeemphChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "FmDeemphChanged(id={}, time={}, handle={}, tau={})",
            tx.id, tx.timestamp, tx.handle, tx.tau);
    }
};
template <>
struct fmt::formatter<violetrx::AmDcrChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AmDcrChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "AmDcrChanged(id={}, time={}, handle={}, enabled={})",
            tx.id, tx.timestamp, tx.handle, tx.enabled);
    }
};
template <>
struct fmt::formatter<violetrx::AmSyncDcrChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AmSyncDcrChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "AmSyncDcrChanged(id={}, time={}, handle={}, enabled={})", tx.id,
            tx.timestamp, tx.handle, tx.enabled);
    }
};
template <>
struct fmt::formatter<violetrx::AmSyncPllBwChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AmSyncPllBwChanged& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "AmSyncPllBwChanged(id={}, time={}, handle={}, bw={})",
            tx.id, tx.timestamp, tx.handle, tx.bw);
    }
};
template <>
struct fmt::formatter<violetrx::SnifferStarted> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::SnifferStarted& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "SnifferStarted(id={}, time={}, handle={}, "
                              "sample_rate={}, size={})",
                              tx.id, tx.timestamp, tx.handle, tx.sample_rate,
                              tx.size);
    }
};
template <>
struct fmt::formatter<violetrx::SnifferStopped> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::SnifferStopped& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "SnifferStopped(id={}, time={}, handle={})",
                              tx.id, tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::RdsDecoderStarted> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::RdsDecoderStarted& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "RdsDecoderStarted(id={}, time={}, handle={})",
                              tx.id, tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::RdsDecoderStopped> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::RdsDecoderStopped& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "RdsDecoderStopped(id={}, time={}, handle={})",
                              tx.id, tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::RdsParserReset> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::RdsParserReset& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "RdsParserReset(id={}, time={}, handle={})",
                              tx.id, tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::UdpStreamingStarted> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::UdpStreamingStarted& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "UdpStreamingStarted(id={}, time={}, "
                              "handle={}, host='{}', port={}, stereo={})",
                              tx.id, tx.timestamp, tx.handle, tx.host, tx.port,
                              tx.stereo);
    }
};
template <>
struct fmt::formatter<violetrx::UdpStreamingStopped> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::UdpStreamingStopped& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "UdpStreamingStopped(id={}, time={}, handle={})",
                              tx.id, tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::SqlLevelChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::SqlLevelChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "SqlLevelChanged(id={}, time={}, handle={}, level={})",
            tx.id, tx.timestamp, tx.handle, tx.level);
    }
};
template <>
struct fmt::formatter<violetrx::SqlAlphaChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::SqlAlphaChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "SqlAlphaChanged(id={}, time={}, handle={}, alpha={})",
            tx.id, tx.timestamp, tx.handle, tx.alpha);
    }
};
template <>
struct fmt::formatter<violetrx::GainChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::GainChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "GainChanged(id={}, time={}, name='{}', value={})",
            tx.id, tx.timestamp, tx.name, tx.value);
    }
};
template <>
struct fmt::formatter<violetrx::RecordingStarted> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::RecordingStarted& tx, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "RecordingStarted(id={}, time={}, handle={}, path='{}')",
            tx.id, tx.timestamp, tx.handle, tx.path);
    }
};
template <>
struct fmt::formatter<violetrx::RecordingStopped> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::RecordingStopped& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "RecordingStopped(id={}, time={}, handle={})",
                              tx.id, tx.timestamp, tx.handle);
    }
};
template <>
struct fmt::formatter<violetrx::IqRecordingStarted> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::IqRecordingStarted& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "IqRecordingStarted(id={}, time={}, path='{}')",
                              tx.id, tx.timestamp, tx.path);
    }
};
template <>
struct fmt::formatter<violetrx::IqRecordingStopped> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::IqRecordingStopped& tx,
                FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "IqRecordingStopped(id={}, time={})",
                              tx.id, tx.timestamp);
    }
};
template <>
struct fmt::formatter<violetrx::AudioGainChanged> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::AudioGainChanged& tx, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "AudioGainChanged(id={}, time={}, gain={})",
                              tx.id, tx.timestamp, tx.gain);
    }
};
template <>
struct fmt::formatter<violetrx::Event> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::Event& tx, FormatContext& ctx) const
    {
        return std::visit(
            [&ctx](auto& t) {
                return fmt::formatter<std::decay_t<decltype(t)>>{}.format(t,
                                                                          ctx);
            },
            tx);
    }
};

template <>
struct fmt::formatter<violetrx::ReceiverEvent> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::ReceiverEvent& tx, FormatContext& ctx) const
    {
        return std::visit(
            [&ctx](const auto& t) {
                return fmt::formatter<std::decay_t<decltype(t)>>{}.format(t,
                                                                          ctx);
            },
            tx);
    }
};

template <>
struct fmt::formatter<violetrx::VfoEvent> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const violetrx::VfoEvent& tx, FormatContext& ctx) const
    {
        return std::visit(
            [&ctx](const auto& t) {
                return fmt::formatter<std::decay_t<decltype(t)>>{}.format(t,
                                                                          ctx);
            },
            tx);
    }
};

#endif
