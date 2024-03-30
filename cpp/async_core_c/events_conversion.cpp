#include "events_conversion.h"

namespace violetrx
{

namespace
{

template <typename T>
VioletEventGeneric to_generic_event(const T& c_event)
{
    static_assert(sizeof(T) <= sizeof(VioletEventGeneric));

    VioletEventGeneric generic_event;
    std::memcpy(&generic_event, &c_event, sizeof(T));

    return generic_event;
}

VioletEventCommon to_event_base(const EventCommon& event,
                                VioletEvent event_type)
{
    return VioletEventCommon{.event_type = event_type,
                             .id = event.id,
                             .timestamp = {.seconds = event.timestamp.seconds,
                                           .nanos = event.timestamp.nanos}};
}

VioletVfoEventCommon to_vfo_event_base(const VfoEventCommon& event,
                                       VioletEvent event_type)
{
    return VioletVfoEventCommon{
        .event_type = event_type,
        .id = event.id,
        .timestamp = {.seconds = event.timestamp.seconds,
                      .nanos = event.timestamp.nanos},
        .handle = event.handle};
}

VioletEventGeneric event_cpp_to_c(const SyncStart& event)
{
    return to_generic_event(VioletSyncStart{
        .base = to_event_base(event, VIOLET_RECEIVER_SYNC_START)});
}

VioletEventGeneric event_cpp_to_c(const SyncEnd& event)
{
    return to_generic_event(
        VioletSyncEnd{.base = to_event_base(event, VIOLET_RECEIVER_SYNC_END)});
}

VioletEventGeneric event_cpp_to_c(const Unsubscribed& event)
{
    return to_generic_event(
        VioletSyncEnd{.base = to_event_base(event, VIOLET_RECEIVER_UNSUBSCRIBED)});
}

VioletEventGeneric event_cpp_to_c(const Started& event)
{
    return to_generic_event(
        VioletStarted{.base = to_event_base(event, VIOLET_RECEIVER_STARTED)});
}

VioletEventGeneric event_cpp_to_c(const Stopped& event)
{
    return to_generic_event(
        VioletStopped{.base = to_event_base(event, VIOLET_RECEIVER_STOPPED)});
}

VioletEventGeneric event_cpp_to_c(const InputDeviceChanged& event)
{
    return to_generic_event(VioletInputDeviceChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_INPUT_DEVICE_CHANGED),
        .device = event.device.c_str()});
}

VioletEventGeneric event_cpp_to_c(const AntennaChanged& event)
{
    return to_generic_event(VioletAntennaChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_INPUT_DEVICE_CHANGED),
        .antenna = event.antenna.c_str()});
}

VioletEventGeneric event_cpp_to_c(const InputRateChanged& event)
{
    return to_generic_event(VioletInputRateChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_INPUT_RATE_CHANGED),
        .input_rate = event.input_rate});
}

VioletEventGeneric event_cpp_to_c(const InputDecimChanged& event)
{
    return to_generic_event(VioletInputDecimChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_INPUT_DECIM_CHANGED),
        .decim = event.decim});
}

VioletEventGeneric event_cpp_to_c(const IqSwapChanged& event)
{
    return to_generic_event(VioletIqSwapChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_IQ_SWAP_CHANGED),
        .enabled = event.enabled});
}

VioletEventGeneric event_cpp_to_c(const DcCancelChanged& event)
{
    return to_generic_event(VioletDcCancelChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_DC_CANCEL_CHANGED),
        .enabled = event.enabled});
}

VioletEventGeneric event_cpp_to_c(const IqBalanceChanged& event)
{
    return to_generic_event(VioletIqBalanceChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_IQ_BALANCE_CHANGED),
        .enabled = event.enabled});
}

VioletEventGeneric event_cpp_to_c(const RfFreqChanged& event)
{
    return to_generic_event(VioletRfFreqChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_RF_FREQ_CHANGED),
        .freq = event.freq});
}

VioletEventGeneric event_cpp_to_c(const GainStagesChanged& event)
{
    int size = std::ssize(event.stages);
    VioletGainStage* c_stages =
        (VioletGainStage*)malloc(sizeof(VioletGainStage) * size);

    for (int i = 0; i < size; i++) {
        const auto& stage = event.stages[i];
        c_stages[i].name = stage.name.c_str();
        c_stages[i].start = stage.start;
        c_stages[i].stop = stage.stop;
        c_stages[i].step = stage.step;
        c_stages[i].value = stage.value;
    }

    return to_generic_event(VioletGainStagesChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_GAIN_STAGES_CHANGED),
        .size = size,
        .stages = c_stages});
}

VioletEventGeneric event_cpp_to_c(const AntennasChanged& event)
{
    int size = std::ssize(event.antennas);
    const char** antennas =
        (const char**)malloc(sizeof(char*) * event.antennas.size());

    for (int i = 0; i < size; i++) {
        const std::string& antenna_str = event.antennas[i];
        antennas[i] = antenna_str.c_str();
    }

    return to_generic_event(VioletAntennasChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_ANTENNAS_CHANGED),
        .size = size,
        .antennas = antennas});
}

VioletEventGeneric event_cpp_to_c(const AutoGainChanged& event)
{
    return to_generic_event(VioletAutoGainChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_AUTO_GAIN_CHANGED),
        .enabled = event.enabled});
}

VioletEventGeneric event_cpp_to_c(const GainChanged& event)
{
    return to_generic_event(VioletGainChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_GAIN_CHANGED),
        .name = event.name.c_str(),
        .value = event.value});
}

VioletEventGeneric event_cpp_to_c(const FreqCorrChanged& event)
{
    return to_generic_event(VioletFreqCorrChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_FREQ_CORR_CHANGED),
        .ppm = event.ppm});
}

VioletEventGeneric event_cpp_to_c(const FftSizeChanged& event)
{
    return to_generic_event(VioletFftSizeChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_FREQ_CORR_CHANGED),
        .size = event.size});
}

VioletEventGeneric event_cpp_to_c(const FftWindowChanged& event)
{
    return to_generic_event(VioletFftWindowChanged{
        .base = to_event_base(event, VIOLET_RECEIVER_FREQ_CORR_CHANGED),
        .window = (VioletWindowType)event.window});
}

VioletEventGeneric event_cpp_to_c(const IqRecordingStarted& event)
{
    return to_generic_event(VioletIqRecordingStarted{
        .base = to_event_base(event, VIOLET_RECEIVER_IQ_RECORDING_STARTED),
        .path = event.path.c_str()});
}

VioletEventGeneric event_cpp_to_c(const IqRecordingStopped& event)
{
    return to_generic_event(VioletIqRecordingStopped{
        .base = to_event_base(event, VIOLET_RECEIVER_IQ_RECORDING_STOPPED)});
}

VioletEventGeneric event_cpp_to_c(const VfoSyncStart& event)
{
    return to_generic_event(VioletVfoSyncStart{
        .base = to_vfo_event_base(event, VIOLET_VFO_SYNC_START)});
}

VioletEventGeneric event_cpp_to_c(const VfoSyncEnd& event)
{
    return to_generic_event(VioletVfoSyncEnd{
        .base = to_vfo_event_base(event, VIOLET_VFO_SYNC_END)});
}

VioletEventGeneric event_cpp_to_c(const VfoAdded& event)
{
    return to_generic_event(
        VioletVfoAdded{.base = to_vfo_event_base(event, VIOLET_VFO_ADDED)});
}

VioletEventGeneric event_cpp_to_c(const VfoRemoved& event)
{
    return to_generic_event(
        VioletVfoRemoved{.base = to_vfo_event_base(event, VIOLET_VFO_REMOVED)});
}

VioletEventGeneric event_cpp_to_c(const DemodChanged& event)
{
    return to_generic_event(VioletDemodChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_DEMOD_CHANGED),
        .demod = (VioletDemod)event.demod});
}

VioletEventGeneric event_cpp_to_c(const OffsetChanged& event)
{
    return to_generic_event(VioletOffsetChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_OFFSET_CHANGED),
        .offset = event.offset});
}

VioletEventGeneric event_cpp_to_c(const CwOffsetChanged& event)
{
    return to_generic_event(VioletCwOffsetChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_CWOFFSET_CHANGED),
        .offset = event.offset});
}

VioletEventGeneric event_cpp_to_c(const FilterChanged& event)
{
    return to_generic_event(VioletFilterChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_CWOFFSET_CHANGED),
        .shape = (VioletFilterShape)event.shape,
        .low = event.low,
        .high = event.high,
    });
}

VioletEventGeneric event_cpp_to_c(const NoiseBlankerOnChanged& event)
{
    return to_generic_event(VioletNoiseBlankerOnChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_NOISE_BLANKER_ON_CHANGED),
        .nb_id = event.nb_id,
        .enabled = event.enabled,
    });
}

VioletEventGeneric event_cpp_to_c(const NoiseBlankerThresholdChanged& event)
{
    return to_generic_event(VioletNoiseBlankerThresholdChanged{
        .base = to_vfo_event_base(event,
                                  VIOLET_VFO_NOISE_BLANKER_THRESHOLD_CHANGED),
        .nb_id = event.nb_id,
        .threshold = event.threshold,
    });
}

VioletEventGeneric event_cpp_to_c(const SqlLevelChanged& event)
{
    return to_generic_event(VioletSqlLevelChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_SQL_LEVEL_CHANGED),
        .level = event.level,
    });
}

VioletEventGeneric event_cpp_to_c(const SqlAlphaChanged& event)
{
    return to_generic_event(VioletSqlAlphaChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_SQL_ALPHA_CHANGED),
        .alpha = event.alpha,
    });
}

VioletEventGeneric event_cpp_to_c(const AgcOnChanged& event)
{
    return to_generic_event(VioletAgcOnChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AGC_ON_CHANGED),
        .enabled = event.enabled,
    });
}

VioletEventGeneric event_cpp_to_c(const AgcHangChanged& event)
{
    return to_generic_event(VioletAgcHangChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AGC_HANG_CHANGED),
        .enabled = event.enabled,
    });
}

VioletEventGeneric event_cpp_to_c(const AgcThresholdChanged& event)
{
    return to_generic_event(VioletAgcThresholdChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AGC_THRESHOLD_CHANGED),
        .threshold = event.threshold,
    });
}

VioletEventGeneric event_cpp_to_c(const AgcSlopeChanged& event)
{
    return to_generic_event(VioletAgcSlopeChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AGC_SLOPE_CHANGED),
        .slope = event.slope,
    });
}

VioletEventGeneric event_cpp_to_c(const AgcDecayChanged& event)
{
    return to_generic_event(VioletAgcDecayChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AGC_DECAY_CHANGED),
        .decay = event.decay,
    });
}

VioletEventGeneric event_cpp_to_c(const AgcManualGainChanged& event)
{
    return to_generic_event(VioletAgcManualGainChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AGC_MANUAL_GAIN_CHANGED),
        .gain = event.gain,
    });
}

VioletEventGeneric event_cpp_to_c(const FmMaxDevChanged& event)
{
    return to_generic_event(VioletFmMaxDevChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_FM_MAXDEV_CHANGED),
        .maxdev = event.maxdev,
    });
}

VioletEventGeneric event_cpp_to_c(const FmDeemphChanged& event)
{
    return to_generic_event(VioletFmDeemphChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_FM_MAXDEV_CHANGED),
        .tau = event.tau,
    });
}

VioletEventGeneric event_cpp_to_c(const AmDcrChanged& event)
{
    return to_generic_event(VioletAmDcrChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AM_DCR_CHANGED),
        .enabled = event.enabled,
    });
}

VioletEventGeneric event_cpp_to_c(const AmSyncDcrChanged& event)
{
    return to_generic_event(VioletAmSyncDcrChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AM_SYNC_DCR_CHANGED),
        .enabled = event.enabled,
    });
}

VioletEventGeneric event_cpp_to_c(const AmSyncPllBwChanged& event)
{
    return to_generic_event(VioletAmSyncPllBwChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AM_SYNC_PLL_BW_CHANGED),
        .bw = event.bw,
    });
}

VioletEventGeneric event_cpp_to_c(const RecordingStarted& event)
{
    return to_generic_event(VioletRecordingStarted{
        .base = to_vfo_event_base(event, VIOLET_VFO_RECORDING_STARTED),
        .path = event.path.c_str(),
    });
}

VioletEventGeneric event_cpp_to_c(const RecordingStopped& event)
{
    return to_generic_event(VioletRecordingStopped{
        .base = to_vfo_event_base(event, VIOLET_VFO_RECORDING_STOPPED),
    });
}

VioletEventGeneric event_cpp_to_c(const SnifferStarted& event)
{
    return to_generic_event(VioletSnifferStarted{
        .base = to_vfo_event_base(event, VIOLET_VFO_SNIFFER_STARTED),
        .sample_rate = event.sample_rate,
        .size = event.size,
    });
}

VioletEventGeneric event_cpp_to_c(const SnifferStopped& event)
{
    return to_generic_event(VioletSnifferStopped{
        .base = to_vfo_event_base(event, VIOLET_VFO_SNIFFER_STOPPED),
    });
}

VioletEventGeneric event_cpp_to_c(const RdsDecoderStarted& event)
{
    return to_generic_event(VioletRdsDecoderStarted{
        .base = to_vfo_event_base(event, VIOLET_VFO_RDS_DECODER_STARTED),
    });
}

VioletEventGeneric event_cpp_to_c(const RdsDecoderStopped& event)
{
    return to_generic_event(VioletRdsDecoderStarted{
        .base = to_vfo_event_base(event, VIOLET_VFO_RDS_DECODER_STOPPED),
    });
}

VioletEventGeneric event_cpp_to_c(const RdsParserReset& event)
{
    return to_generic_event(VioletRdsDecoderStarted{
        .base = to_vfo_event_base(event, VIOLET_VFO_RDS_PARSER_RESET),
    });
}

VioletEventGeneric event_cpp_to_c(const UdpStreamingStarted& event)
{
    return to_generic_event(VioletUdpStreamingStarted{
        .base = to_vfo_event_base(event, VIOLET_VFO_UDP_STREAMING_STARTED),
        .host = event.host.c_str(),
        .port = event.port,
        .stereo = event.stereo,
    });
}

VioletEventGeneric event_cpp_to_c(const UdpStreamingStopped& event)
{
    return to_generic_event(VioletUdpStreamingStopped{
        .base = to_vfo_event_base(event, VIOLET_VFO_UDP_STREAMING_STOPPED),
    });
}

VioletEventGeneric event_cpp_to_c(const AudioGainChanged& event)
{
    return to_generic_event(VioletAudioGainChanged{
        .base = to_vfo_event_base(event, VIOLET_VFO_AUDIO_GAIN_CHANGED),
        .gain = event.gain,
    });
}

}; // namespace

CEvent::CEvent(const Event& event)
{
    inner_ = std::visit(
        [](const auto& specific_event) {
            return event_cpp_to_c(specific_event);
        },
        event);
}
CEvent::CEvent(const ReceiverEvent& event)
{
    inner_ = std::visit(
        [](const auto& specific_event) {
            return event_cpp_to_c(specific_event);
        },
        event);
}

CEvent::CEvent(const VfoEvent& event)
{
    inner_ = std::visit(
        [](const auto& specific_event) {
            return event_cpp_to_c(specific_event);
        },
        event);
}

CEvent::~CEvent()
{
    switch (inner_.event_type) {
    case VIOLET_RECEIVER_GAIN_STAGES_CHANGED: {

        VioletGainStagesChanged* specific_event =
            reinterpret_cast<VioletGainStagesChanged*>(&inner_);

        // std::free doesn't work with const pointers as normally const pointers
        // indicate that they're not allocated
        std::free((void*)specific_event->stages);
        break;
    }
    case VIOLET_RECEIVER_ANTENNAS_CHANGED: {

        VioletAntennasChanged* specific_event =
            reinterpret_cast<VioletAntennasChanged*>(&inner_);

        // std::free doesn't work with const pointers as normally const pointers
        // indicate that they're not allocated
        std::free((void*)specific_event->antennas);
        break;
    }
    default:
        // DO NOTHING
        break;
    }
}

} // namespace violetrx
