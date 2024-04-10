#include <spdlog/spdlog.h>
#include <variant>

#include "type_conversion.h"

namespace violetrx
{

Receiver::ErrorCode ErrorCodeCoreToProto(ErrorCode code)
{
    // FIXME
    return static_cast<Receiver::ErrorCode>(code);
}

WindowType WindowProtoToCore(Receiver::WindowType window)
{
    // FIXME
    return static_cast<WindowType>(window);
}

FilterShape FilterShapeProtoToCore(Receiver::FilterShape filter_shape)
{
    // FIXME
    return static_cast<FilterShape>(filter_shape);
}

Demod DemodProtoToCore(Receiver::DemodType demod)
{
    // FIXME
    return static_cast<Demod>(demod);
}

google::protobuf::Timestamp TimestampCoreToProto(Timestamp timestamp)
{
    google::protobuf::Timestamp result;
    result.set_nanos(timestamp.nanos);
    result.set_seconds(timestamp.seconds);

    return result;
}

ErrorCode ErrorCodeProtoToCore(Receiver::ErrorCode code)
{
    // FIXME
    return static_cast<ErrorCode>(code);
}

Receiver::WindowType WindowCoreToProto(WindowType code)
{
    // FIXME
    return static_cast<Receiver::WindowType>(code);
}

Receiver::FilterShape FilterShapeCoreToProto(FilterShape filter_shape)
{
    // FIXME
    return static_cast<Receiver::FilterShape>(filter_shape);
}

Receiver::DemodType DemodCoreToProto(Demod demod)
{
    // FIXME
    return static_cast<Receiver::DemodType>(demod);
}

Timestamp TimestampProtoToCore(google::protobuf::Timestamp timestamp)
{
    Timestamp result;
    result.nanos = timestamp.nanos();
    result.seconds = timestamp.seconds();

    return result;
}

void FftFrameCoreToProto(const FftFrame& frame, Receiver::FftFrame* proto_frame)
{
    int fft_size = frame.fft_points.size();
    proto_frame->mutable_data()->Reserve(fft_size);
    float* data = proto_frame->mutable_data()->mutable_data();
    std::memcpy(data, frame.fft_points.data(), sizeof(float) * fft_size);
    proto_frame->mutable_data()->AddNAlreadyReserved(fft_size);

    proto_frame->set_allocated_timestamp(
        new google::protobuf::Timestamp(TimestampCoreToProto(frame.timestamp)));

    proto_frame->set_center_freq(frame.center_freq);
    proto_frame->set_sample_rate(frame.sample_rate);

    return;
}

Device DeviceProtoToCore(const Receiver::Device& proto_device)
{
    Device result;
    result.label = proto_device.label();
    result.devstr = proto_device.devstr();

    return result;
}

std::optional<Event> EventProtoToCore(const Receiver::Event& proto_event)
{
    std::optional<Event> event = std::nullopt;

    EventCommon ec;
    ec.id = proto_event.id();
    ec.timestamp = TimestampProtoToCore(proto_event.timestamp());

    switch (proto_event.tx_case()) {
    case Receiver::Event::TxCase::kSyncStart:
        event = SyncStart{ec};
        break;
    case Receiver::Event::TxCase::kSyncEnd:
        event = SyncEnd{ec};
        break;
    case Receiver::Event::TxCase::kUnsubscribed:
        event = Unsubscribed{ec};
        break;
    case Receiver::Event::TxCase::kRxStarted:
        event = Started{ec};
        break;
    case Receiver::Event::TxCase::kRxStopped:
        event = Stopped{ec};
        break;
    case Receiver::Event::TxCase::kInputDevChanged:
        event =
            InputDeviceChanged{ec, proto_event.input_dev_changed().device()};
        break;
    case Receiver::Event::TxCase::kAntennaChanged:
        event = AntennaChanged{ec, proto_event.antenna_changed().antenna()};
        break;
    case Receiver::Event::TxCase::kInputRateChanged:
        event = InputRateChanged{
            ec, static_cast<int32_t>(proto_event.input_rate_changed().rate())};
        break;
    case Receiver::Event::TxCase::kInputDecimChanged:
        event = InputDecimChanged{
            ec,
            static_cast<int32_t>(proto_event.input_decim_changed().decim())};
        break;
    case Receiver::Event::TxCase::kIqSwapChanged:
        event = IqSwapChanged{ec, proto_event.iq_swap_changed().enabled()};
        break;
    case Receiver::Event::TxCase::kDcCancelChanged:
        event = DcCancelChanged{ec, proto_event.dc_cancel_changed().enabled()};
        break;
    case Receiver::Event::TxCase::kIqBalanceChanged:
        event =
            IqBalanceChanged{ec, proto_event.iq_balance_changed().enabled()};
        break;
    case Receiver::Event::TxCase::kRfFreqChanged:
        event = RfFreqChanged{
            ec, static_cast<int64_t>(proto_event.rf_freq_changed().freq())};
        break;
    case Receiver::Event::TxCase::kGainStagesChanged: {
        const auto& specific_proto_event = proto_event.gain_stages_changed();
        std::vector<GainStage> gain_stages;
        gain_stages.reserve(specific_proto_event.stages_size());

        for (const auto& proto_stage : specific_proto_event.stages()) {
            gain_stages.push_back(GainStage{.name = proto_stage.name(),
                                            .start = proto_stage.start(),
                                            .stop = proto_stage.stop(),
                                            .step = proto_stage.step(),
                                            .value = proto_stage.value()});
        }

        event = GainStagesChanged{{ec}, gain_stages};
        break;
    }
    case Receiver::Event::TxCase::kAntennasChanged: {
        const auto& specific_proto_event = proto_event.antennas_changed();
        std::vector<std::string> antennas;
        antennas.reserve(specific_proto_event.antennas_size());

        for (const auto& antenna : specific_proto_event.antennas()) {
            antennas.push_back(antenna);
        }

        event = AntennasChanged{{ec}, antennas};
        break;
    }
    case Receiver::Event::TxCase::kAutoGainChanged:
        event = AutoGainChanged{ec, proto_event.auto_gain_changed().enabled()};
        break;
    case Receiver::Event::TxCase::kGainChanged:
        event = GainChanged{ec, proto_event.gain_changed().name(),
                            proto_event.gain_changed().value()};
        break;
    case Receiver::Event::TxCase::kFreqCorrChanged:
        event = FreqCorrChanged{ec, proto_event.freq_corr_changed().ppm()};
        break;
    case Receiver::Event::TxCase::kFftSizeChanged:
        event = FftSizeChanged{
            ec, static_cast<int32_t>(proto_event.fft_size_changed().size())};
        break;
    case Receiver::Event::TxCase::kFftWindowChanged:
        event = FftWindowChanged{
            ec, WindowProtoToCore(proto_event.fft_window_changed().window())};
        break;
    case Receiver::Event::TxCase::kVfoAdded:
        event = VfoAdded{VfoEventCommon{ec, proto_event.vfo_added().handle()}};
        break;
    case Receiver::Event::TxCase::kVfoRemoved:
        event =
            VfoRemoved{VfoEventCommon{ec, proto_event.vfo_removed().handle()}};
        break;
    case Receiver::Event::TxCase::kDemodChanged:
        event = DemodChanged{
            VfoEventCommon{ec, proto_event.demod_changed().handle()},
            DemodProtoToCore(proto_event.demod_changed().demod())};
        break;
    case Receiver::Event::TxCase::kOffsetChanged:
        event = OffsetChanged{
            VfoEventCommon{ec, proto_event.offset_changed().handle()},
            proto_event.offset_changed().offset()};
        break;
    case Receiver::Event::TxCase::kCwOffsetChanged:
        event = CwOffsetChanged{
            VfoEventCommon{ec, proto_event.cw_offset_changed().handle()},
            proto_event.cw_offset_changed().offset()};
        break;
    case Receiver::Event::TxCase::kFilterChanged:
        event = FilterChanged{
            VfoEventCommon{ec, proto_event.filter_changed().handle()},
            FilterShapeProtoToCore(proto_event.filter_changed().shape()),
            proto_event.filter_changed().low(),
            proto_event.filter_changed().high(),
        };
        break;
    case Receiver::Event::TxCase::kNbOnChanged:
        event = NoiseBlankerOnChanged{
            VfoEventCommon{ec, proto_event.nb_on_changed().handle()},
            proto_event.nb_on_changed().id(),
            proto_event.nb_on_changed().enabled(),
        };
        break;
    case Receiver::Event::TxCase::kNbThresholdChanged:
        event = NoiseBlankerThresholdChanged{
            VfoEventCommon{ec, proto_event.nb_threshold_changed().handle()},
            proto_event.nb_threshold_changed().id(),
            proto_event.nb_threshold_changed().threshold(),
        };
        break;
    case Receiver::Event::TxCase::kSqlLevelChanged:
        event = SqlLevelChanged{
            VfoEventCommon{ec, proto_event.sql_level_changed().handle()},
            proto_event.sql_level_changed().level(),
        };
        break;
    case Receiver::Event::TxCase::kSqlAlphaChanged:
        event = SqlAlphaChanged{
            VfoEventCommon{ec, proto_event.sql_alpha_changed().handle()},
            proto_event.sql_alpha_changed().alpha(),
        };
        break;
    case Receiver::Event::TxCase::kAgcOnChanged:
        event = AgcOnChanged{
            VfoEventCommon{ec, proto_event.agc_on_changed().handle()},
            proto_event.agc_on_changed().enabled(),
        };
        break;
    case Receiver::Event::TxCase::kAgcHangChanged:
        event = AgcHangChanged{
            VfoEventCommon{ec, proto_event.agc_hang_changed().handle()},
            proto_event.agc_hang_changed().enabled(),
        };
        break;
    case Receiver::Event::TxCase::kAgcThresholdChanged:
        event = AgcThresholdChanged{
            VfoEventCommon{ec, proto_event.agc_threshold_changed().handle()},
            proto_event.agc_threshold_changed().threshold(),
        };
        break;
    case Receiver::Event::TxCase::kAgcSlopeChanged:
        event = AgcSlopeChanged{
            VfoEventCommon{ec, proto_event.agc_slope_changed().handle()},
            proto_event.agc_slope_changed().slope(),
        };
        break;
    case Receiver::Event::TxCase::kAgcDecayChanged:
        event = AgcDecayChanged{
            VfoEventCommon{ec, proto_event.agc_decay_changed().handle()},
            proto_event.agc_decay_changed().decay(),
        };
        break;
    case Receiver::Event::TxCase::kAgcManualGainChanged:
        event = AgcManualGainChanged{
            VfoEventCommon{ec, proto_event.agc_manual_gain_changed().handle()},
            proto_event.agc_manual_gain_changed().gain(),
        };
        break;
    case Receiver::Event::TxCase::kFmMaxdevChanged:
        event = FmMaxDevChanged{
            VfoEventCommon{ec, proto_event.fm_maxdev_changed().handle()},
            proto_event.fm_maxdev_changed().maxdev(),
        };
        break;
    case Receiver::Event::TxCase::kFmDeemphChanged:
        event = FmDeemphChanged{
            VfoEventCommon{ec, proto_event.fm_deemph_changed().handle()},
            proto_event.fm_deemph_changed().tau(),
        };
        break;
    case Receiver::Event::TxCase::kAmDcrChanged:
        event = AmDcrChanged{
            VfoEventCommon{ec, proto_event.am_dcr_changed().handle()},
            proto_event.am_dcr_changed().enabled(),
        };
        break;
    case Receiver::Event::TxCase::kAmSyncDcrChanged:
        event = AmSyncDcrChanged{
            VfoEventCommon{ec, proto_event.am_sync_dcr_changed().handle()},
            proto_event.am_sync_dcr_changed().enabled(),
        };
        break;
    case Receiver::Event::TxCase::kAmSyncPllBwChanged:
        event = AmSyncPllBwChanged{
            VfoEventCommon{ec, proto_event.am_sync_pll_bw_changed().handle()},
            proto_event.am_sync_pll_bw_changed().bw(),
        };
        break;
    case Receiver::Event::TxCase::kRecordingStarted:
        event = RecordingStarted{
            VfoEventCommon{ec, proto_event.recording_started().handle()},
            proto_event.recording_started().path(),
        };
        break;
    case Receiver::Event::TxCase::kRecordingStopped:
        event = RecordingStopped{
            VfoEventCommon{ec, proto_event.recording_stopped().handle()}};
        break;
    case Receiver::Event::TxCase::kSnifferStarted:
        event = SnifferStarted{
            VfoEventCommon{ec, proto_event.sniffer_started().handle()},
            static_cast<int>(proto_event.sniffer_started().sample_rate()),
            static_cast<int>(proto_event.sniffer_started().size()),
        };
        break;
    case Receiver::Event::TxCase::kSnifferStopped:
        event = SnifferStopped{
            VfoEventCommon{ec, proto_event.sniffer_stopped().handle()},
        };
        break;
    case Receiver::Event::TxCase::kRdsDecoderStarted:
        event = RdsDecoderStarted{
            VfoEventCommon{ec, proto_event.rds_decoder_started().handle()},
        };
        break;
    case Receiver::Event::TxCase::kRdsDecoderStopped:
        event = RdsDecoderStopped{
            VfoEventCommon{ec, proto_event.rds_decoder_stopped().handle()},
        };
        break;
    case Receiver::Event::TxCase::kRdsParserReset:
        event = RdsParserReset{
            VfoEventCommon{ec, proto_event.rds_parser_reset().handle()},
        };
        break;
    case Receiver::Event::TxCase::TX_NOT_SET:
        spdlog::error("EventProtoToCore: unexpected event type: {}",
                      (int)proto_event.tx_case());
        event = {};
        break;
    }

    return event;
}

bool EventCoreToProto(const Event& event, Receiver::Event* proto_event)
{
    std::visit(
        [&](auto e) {
            proto_event->set_id(e.id);
            proto_event->set_allocated_timestamp(
                new google::protobuf::Timestamp(
                    TimestampCoreToProto(e.timestamp)));
        },
        event);

    proto_event->clear_tx();
    return std::visit( //
        Visitor{
            [&](const SyncStart&) {
                auto* proto_specific_event = new Receiver::SyncStart();
                proto_event->set_allocated_sync_start(proto_specific_event);
                return true;
            },
            [&](const SyncEnd&) {
                auto* proto_specific_event = new Receiver::SyncEnd();
                proto_event->set_allocated_sync_end(proto_specific_event);
                return true;
            },
            [&](const Unsubscribed&) {
                auto* proto_specific_event = new Receiver::Unsubscribed();
                proto_event->set_allocated_unsubscribed(proto_specific_event);
                return true;
            },
            [&](const Started&) {
                auto* proto_specific_event = new Receiver::Started();
                proto_event->set_allocated_rx_started(proto_specific_event);
                return true;
            },
            [&](const Stopped&) {
                auto* proto_specific_event = new Receiver::Stopped();
                proto_event->set_allocated_rx_stopped(proto_specific_event);
                return true;
            },
            [&](const InputDeviceChanged& ev) {
                auto* proto_specific_event = new Receiver::InputDeviceChanged();
                proto_specific_event->set_device(ev.device);

                proto_event->set_allocated_input_dev_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AntennaChanged& ev) {
                auto* proto_specific_event = new Receiver::AntennaChanged();
                proto_specific_event->set_antenna(ev.antenna);

                proto_event->set_allocated_antenna_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AntennasChanged& ev) {
                auto* proto_specific_event = new Receiver::AntennasChanged();
                *proto_specific_event->mutable_antennas() = {
                    ev.antennas.begin(), ev.antennas.end()};

                proto_event->set_allocated_antennas_changed(
                    proto_specific_event);
                return true;
            },
            [&](const InputRateChanged& ev) {
                auto* proto_specific_event = new Receiver::InputRateChanged();
                proto_specific_event->set_rate(ev.input_rate);

                proto_event->set_allocated_input_rate_changed(
                    proto_specific_event);
                return true;
            },
            [&](const RfFreqChanged& ev) {
                auto* proto_specific_event = new Receiver::RfFreqChanged();
                proto_specific_event->set_freq(ev.freq);

                proto_event->set_allocated_rf_freq_changed(
                    proto_specific_event);
                return true;
            },
            [&](const InputDecimChanged& ev) {
                auto* proto_specific_event = new Receiver::InputDecimChanged();
                proto_specific_event->set_decim(ev.decim);

                proto_event->set_allocated_input_decim_changed(
                    proto_specific_event);
                return true;
            },
            [&](const IqSwapChanged& ev) {
                auto* proto_specific_event = new Receiver::IqSwapChanged();
                proto_specific_event->set_enabled(ev.enabled);

                proto_event->set_allocated_iq_swap_changed(
                    proto_specific_event);
                return true;
            },
            [&](const IqBalanceChanged& ev) {
                auto* proto_specific_event = new Receiver::IqBalanceChanged();
                proto_specific_event->set_enabled(ev.enabled);

                proto_event->set_allocated_iq_balance_changed(
                    proto_specific_event);
                return true;
            },
            [&](const DcCancelChanged& ev) {
                auto* proto_specific_event = new Receiver::DcCancelChanged();
                proto_specific_event->set_enabled(ev.enabled);

                proto_event->set_allocated_dc_cancel_changed(
                    proto_specific_event);
                return true;
            },
            [&](const GainStagesChanged& ev) {
                auto* proto_specific_event = new Receiver::GainStagesChanged();
                for (const auto& gain_stage : ev.stages) {
                    auto* proto_stage = proto_specific_event->add_stages();
                    proto_stage->set_name(gain_stage.name);
                    proto_stage->set_start(gain_stage.start);
                    proto_stage->set_stop(gain_stage.stop);
                    proto_stage->set_step(gain_stage.step);
                    proto_stage->set_value(gain_stage.value);
                }

                proto_event->set_allocated_gain_stages_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AutoGainChanged& ev) {
                auto* proto_specific_event = new Receiver::AutoGainChanged();
                proto_specific_event->set_enabled(ev.enabled);

                proto_event->set_allocated_auto_gain_changed(
                    proto_specific_event);
                return true;
            },
            [&](const GainChanged& ev) {
                auto* proto_specific_event = new Receiver::GainChanged();
                proto_specific_event->set_name(ev.name);
                proto_specific_event->set_value(ev.value);

                proto_event->set_allocated_gain_changed(proto_specific_event);
                return true;
            },
            [&](const FftWindowChanged& ev) {
                auto* proto_specific_event = new Receiver::FftWindowChanged();
                proto_specific_event->set_window(WindowCoreToProto(ev.window));

                proto_event->set_allocated_fft_window_changed(
                    proto_specific_event);
                return true;
            },
            [&](const FftSizeChanged& ev) {
                auto* proto_specific_event = new Receiver::FftSizeChanged();
                proto_specific_event->set_size(ev.size);

                proto_event->set_allocated_fft_size_changed(
                    proto_specific_event);
                return true;
            },
            [&](const FreqCorrChanged& ev) {
                auto* proto_specific_event = new Receiver::FreqCorrChanged();
                proto_specific_event->set_ppm(ev.ppm);

                proto_event->set_allocated_freq_corr_changed(
                    proto_specific_event);
                return true;
            },
            [&](const IqRecordingStarted&) {
                // FIXME
                spdlog::error(
                    "EventCoreToProto: IqRecordingStarted doesn't have "
                    "an equivalent proto type!");
                return false;
            },
            [&](const IqRecordingStopped&) {
                // FIXME
                spdlog::error(
                    "EventCoreToProto: IqRecordingStopped doesn't have "
                    "an equivalent proto type!");
                return false;
            },
            [&](const VfoAdded& ev) {
                auto* proto_specific_event = new Receiver::VfoAdded();
                proto_specific_event->set_handle(ev.handle);

                proto_event->set_allocated_vfo_added(proto_specific_event);
                return true;
            },
            [&](const VfoRemoved& ev) {
                auto* proto_specific_event = new Receiver::VfoRemoved();
                proto_specific_event->set_handle(ev.handle);

                proto_event->set_allocated_vfo_removed(proto_specific_event);
                return true;
            },
            [&](const VfoSyncStart&) { return false; },
            [&](const VfoSyncEnd&) { return false; },
            [&](const DemodChanged& ev) {
                auto* proto_specific_event = new Receiver::DemodChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_demod(DemodCoreToProto(ev.demod));

                proto_event->set_allocated_demod_changed(proto_specific_event);
                return true;
            },
            [&](const OffsetChanged& ev) {
                auto* proto_specific_event = new Receiver::OffsetChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_offset(ev.offset);

                proto_event->set_allocated_offset_changed(proto_specific_event);
                return true;
            },
            [&](const CwOffsetChanged& ev) {
                auto* proto_specific_event = new Receiver::CwOffsetChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_offset(ev.offset);

                proto_event->set_allocated_cw_offset_changed(
                    proto_specific_event);
                return true;
            },
            [&](const FilterChanged& ev) {
                auto* proto_specific_event = new Receiver::FilterChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_low(ev.low);
                proto_specific_event->set_high(ev.high);
                proto_specific_event->set_shape(
                    FilterShapeCoreToProto(ev.shape));

                proto_event->set_allocated_filter_changed(proto_specific_event);
                return true;
            },
            [&](const NoiseBlankerOnChanged& ev) {
                auto* proto_specific_event =
                    new Receiver::NoiseBlankerOnChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_id(ev.id);
                proto_specific_event->set_enabled(ev.enabled);

                proto_event->set_allocated_nb_on_changed(proto_specific_event);
                return true;
            },
            [&](const NoiseBlankerThresholdChanged& ev) {
                auto* proto_specific_event =
                    new Receiver::NoiseBlankerThresholdChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_id(ev.id);
                proto_specific_event->set_threshold(ev.threshold);

                proto_event->set_allocated_nb_threshold_changed(
                    proto_specific_event);
                return true;
            },
            [&](const SqlLevelChanged& ev) {
                auto* proto_specific_event = new Receiver::SqlLevelChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_level(ev.level);

                proto_event->set_allocated_sql_level_changed(
                    proto_specific_event);
                return true;
            },
            [&](const SqlAlphaChanged& ev) {
                auto* proto_specific_event = new Receiver::SqlAlphaChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_alpha(ev.alpha);

                proto_event->set_allocated_sql_alpha_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AgcOnChanged& ev) {
                auto* proto_specific_event = new Receiver::AgcOnChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_enabled(ev.enabled);

                proto_event->set_allocated_agc_on_changed(proto_specific_event);
                return true;
            },
            [&](const AgcHangChanged& ev) {
                auto* proto_specific_event = new Receiver::AgcHangChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_enabled(ev.enabled);

                proto_event->set_allocated_agc_hang_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AgcThresholdChanged& ev) {
                auto* proto_specific_event =
                    new Receiver::AgcThresholdChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_threshold(ev.threshold);

                proto_event->set_allocated_agc_threshold_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AgcSlopeChanged& ev) {
                auto* proto_specific_event = new Receiver::AgcSlopeChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_slope(ev.slope);

                proto_event->set_allocated_agc_slope_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AgcDecayChanged& ev) {
                auto* proto_specific_event = new Receiver::AgcDecayChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_decay(ev.decay);

                proto_event->set_allocated_agc_decay_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AgcManualGainChanged& ev) {
                auto* proto_specific_event =
                    new Receiver::AgcManualGainChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_gain(ev.gain);

                proto_event->set_allocated_agc_manual_gain_changed(
                    proto_specific_event);
                return true;
            },
            [&](const FmMaxDevChanged& ev) {
                auto* proto_specific_event = new Receiver::FmMaxDevChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_maxdev(ev.maxdev);

                proto_event->set_allocated_fm_maxdev_changed(
                    proto_specific_event);
                return true;
            },
            [&](const FmDeemphChanged& ev) {
                auto* proto_specific_event = new Receiver::FmDeemphChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_tau(ev.tau);

                proto_event->set_allocated_fm_deemph_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AmDcrChanged& ev) {
                auto* proto_specific_event = new Receiver::AmDcrChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_enabled(ev.enabled);

                proto_event->set_allocated_am_dcr_changed(proto_specific_event);
                return true;
            },
            [&](const AmSyncDcrChanged& ev) {
                auto* proto_specific_event = new Receiver::AmSyncDcrChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_enabled(ev.enabled);

                proto_event->set_allocated_am_sync_dcr_changed(
                    proto_specific_event);
                return true;
            },
            [&](const AmSyncPllBwChanged& ev) {
                auto* proto_specific_event = new Receiver::AmSyncPllBwChanged();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_bw(ev.bw);

                proto_event->set_allocated_am_sync_pll_bw_changed(
                    proto_specific_event);
                return true;
            },
            [&](const RecordingStarted& ev) {
                auto* proto_specific_event = new Receiver::RecordingStarted();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_path(ev.path);

                proto_event->set_allocated_recording_started(
                    proto_specific_event);
                return true;
            },
            [&](const RecordingStopped& ev) {
                auto* proto_specific_event = new Receiver::RecordingStopped();
                proto_specific_event->set_handle(ev.handle);

                proto_event->set_allocated_recording_stopped(
                    proto_specific_event);
                return true;
            },
            [&](const SnifferStarted& ev) {
                auto* proto_specific_event = new Receiver::SnifferStarted();
                proto_specific_event->set_handle(ev.handle);
                proto_specific_event->set_sample_rate(ev.sample_rate);
                proto_specific_event->set_size(ev.size);

                proto_event->set_allocated_sniffer_started(
                    proto_specific_event);
                return true;
            },
            [&](const SnifferStopped& ev) {
                auto* proto_specific_event = new Receiver::SnifferStopped();
                proto_specific_event->set_handle(ev.handle);

                proto_event->set_allocated_sniffer_stopped(
                    proto_specific_event);
                return true;
            },
            [&](const UdpStreamingStarted&) {
                spdlog::error(
                    "EventCoreToProto: UdpStreamingStarted doesn't have "
                    "an equivalent proto type!");
                return false;
            },
            [&](const UdpStreamingStopped&) {
                spdlog::error(
                    "EventCoreToProto: UdpStreamingStopped doesn't have "
                    "an equivalent proto type!");
                return false;
            },
            [&](const RdsDecoderStarted& ev) {
                auto* proto_specific_event = new Receiver::RdsDecoderStarted();
                proto_specific_event->set_handle(ev.handle);

                proto_event->set_allocated_rds_decoder_started(
                    proto_specific_event);
                return true;
            },
            [&](const RdsDecoderStopped& ev) {
                auto* proto_specific_event = new Receiver::RdsDecoderStopped();
                proto_specific_event->set_handle(ev.handle);

                proto_event->set_allocated_rds_decoder_stopped(
                    proto_specific_event);
                return true;
            },
            [&](const RdsParserReset& ev) {
                auto* proto_specific_event = new Receiver::RdsParserReset();
                proto_specific_event->set_handle(ev.handle);

                proto_event->set_allocated_rds_parser_reset(
                    proto_specific_event);
                return true;
            },
            [&](const AudioGainChanged&) {
                // FIXME: Audio should be client side
                spdlog::error("EventCoreToProto: AudioGainChanged doesn't have "
                              "an equivalent proto type!");
                return false;
            },
        },
        event);
}

} // namespace violetrx
