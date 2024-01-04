#ifndef EVENTS_C
#define EVENTS_C

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

#include "types_c.h"

enum VioletEvent {
    // receiver events
    VIOLET_RECEIVER_UNSUBSCRIBED,
    VIOLET_RECEIVER_SYNC_START,
    VIOLET_RECEIVER_SYNC_END,
    VIOLET_RECEIVER_STARTED,
    VIOLET_RECEIVER_STOPPED,
    VIOLET_RECEIVER_INPUT_DEVICE_CHANGED,
    VIOLET_RECEIVER_ANTENNA_CHANGED,
    VIOLET_RECEIVER_INPUT_RATE_CHANGED,
    VIOLET_RECEIVER_INPUT_DECIM_CHANGED,
    VIOLET_RECEIVER_IQ_SWAP_CHANGED,
    VIOLET_RECEIVER_DC_CANCEL_CHANGED,
    VIOLET_RECEIVER_IQ_BALANCE_CHANGED,
    VIOLET_RECEIVER_RF_FREQ_CHANGED,
    VIOLET_RECEIVER_GAIN_STAGES_CHANGED,
    VIOLET_RECEIVER_ANTENNAS_CHANGED,
    VIOLET_RECEIVER_AUTO_GAIN_CHANGED,
    VIOLET_RECEIVER_GAIN_CHANGED,
    VIOLET_RECEIVER_FREQ_CORR_CHANGED,
    VIOLET_RECEIVER_FFT_SIZE_CHANGED,
    VIOLET_RECEIVER_FFT_WINDOW_CHANGED,
    VIOLET_RECEIVER_IQ_RECORDING_STARTED,
    VIOLET_RECEIVER_IQ_RECORDING_STOPPED,

    // vfo events
    VIOLET_VFO_SYNC_START,
    VIOLET_VFO_SYNC_END,
    VIOLET_VFO_ADDED,
    VIOLET_VFO_REMOVED,
    VIOLET_VFO_DEMOD_CHANGED,
    VIOLET_VFO_OFFSET_CHANGED,
    VIOLET_VFO_CWOFFSET_CHANGED,
    VIOLET_VFO_FILTER_CHANGED,
    VIOLET_VFO_NOISE_BLANKER_ON_CHANGED,
    VIOLET_VFO_NOISE_BLANKER_THRESHOLD_CHANGED,
    VIOLET_VFO_SQL_LEVEL_CHANGED,
    VIOLET_VFO_SQL_ALPHA_CHANGED,
    VIOLET_VFO_AGC_ON_CHANGED,
    VIOLET_VFO_AGC_HANG_CHANGED,
    VIOLET_VFO_AGC_THRESHOLD_CHANGED,
    VIOLET_VFO_AGC_SLOPE_CHANGED,
    VIOLET_VFO_AGC_DECAY_CHANGED,
    VIOLET_VFO_AGC_MANUAL_GAIN_CHANGED,
    VIOLET_VFO_FM_MAXDEV_CHANGED,
    VIOLET_VFO_FM_DEEMPH_CHANGED,
    VIOLET_VFO_AM_DCR_CHANGED,
    VIOLET_VFO_AM_SYNC_DCR_CHANGED,
    VIOLET_VFO_AM_SYNC_PLL_BW_CHANGED,
    VIOLET_VFO_RECORDING_STARTED,
    VIOLET_VFO_RECORDING_STOPPED,
    VIOLET_VFO_SNIFFER_STARTED,
    VIOLET_VFO_SNIFFER_STOPPED,
    VIOLET_VFO_UDP_STREAMING_STARTED,
    VIOLET_VFO_UDP_STREAMING_STOPPED,
    VIOLET_VFO_RDS_DECODER_STARTED,
    VIOLET_VFO_RDS_DECODER_STOPPED,
    VIOLET_VFO_RDS_PARSER_RESET,
    VIOLET_VFO_AUDIO_GAIN_CHANGED,

    VIOLET_EVENT_UNKNOWN,
};

typedef struct {
    VioletEvent event_type;
    int64_t id;
    VioletTimestamp timestamp;
} VioletEventCommon;

typedef struct {
    VioletEvent event_type;
    int64_t id;
    VioletTimestamp timestamp;
    uint64_t handle;
} VioletVfoEventCommon;

typedef struct {
    VioletEventCommon base;
} VioletSyncStart;

typedef struct {
    VioletEventCommon base;
} VioletSyncEnd;

typedef struct {
    VioletEventCommon base;
} VioletStarted;

typedef struct {
    VioletEventCommon base;
} VioletStopped;

typedef struct {
    VioletEventCommon base;
    const char* device;
} VioletInputDeviceChanged;

typedef struct {
    VioletEventCommon base;
    int32_t size;
    const char** antennas;
} VioletAntennasChanged;

typedef struct {
    VioletEventCommon base;
    const char* antenna;
} VioletAntennaChanged;

typedef struct {
    VioletEventCommon base;
    int32_t input_rate;
} VioletInputRateChanged;

typedef struct {
    VioletEventCommon base;
    int32_t decim;
} VioletInputDecimChanged;

typedef struct {
    VioletEventCommon base;
    bool enabled;
} VioletIqSwapChanged;

typedef struct {
    VioletEventCommon base;
    bool enabled;
} VioletDcCancelChanged;

typedef struct {
    VioletEventCommon base;
    bool enabled;
} VioletIqBalanceChanged;

typedef struct {
    VioletEventCommon base;
    int64_t freq;
} VioletRfFreqChanged;

typedef struct {
    VioletEventCommon base;
    int32_t size;
    const VioletGainStage* stages;
} VioletGainStagesChanged;

typedef struct {
    VioletEventCommon base;
    bool enabled;
} VioletAutoGainChanged;

typedef struct {
    VioletEventCommon base;
    const char* name;
    double value;
} VioletGainChanged;

typedef struct {
    VioletEventCommon base;
    double ppm;
} VioletFreqCorrChanged;

typedef struct {
    VioletEventCommon base;
    int32_t size;
} VioletFftSizeChanged;

typedef struct {
    VioletEventCommon base;
    VioletWindowType window;
} VioletFftWindowChanged;

typedef struct {
    VioletEventCommon base;
    const char* path;
} VioletIqRecordingStarted;

typedef struct {
    VioletEventCommon base;
} VioletIqRecordingStopped;

typedef struct {
    VioletVfoEventCommon base;
} VioletVfoSyncStart;

typedef struct {
    VioletVfoEventCommon base;
} VioletVfoSyncEnd;

typedef struct {
    VioletVfoEventCommon base;
} VioletVfoAdded;

typedef struct {
    VioletVfoEventCommon base;
} VioletVfoRemoved;

typedef struct {
    VioletVfoEventCommon base;
    VioletDemod demod;
} VioletDemodChanged;

typedef struct {
    VioletVfoEventCommon base;
    int32_t offset;
} VioletOffsetChanged;

typedef struct {
    VioletVfoEventCommon base;
    int32_t offset;
} VioletCwOffsetChanged;

typedef struct {
    VioletVfoEventCommon base;
    VioletFilterShape shape;
    int32_t low;
    int32_t high;
} VioletFilterChanged;

typedef struct {
    VioletVfoEventCommon base;
    int32_t nb_id;
    bool enabled;
} VioletNoiseBlankerOnChanged;

typedef struct {
    VioletVfoEventCommon base;
    int32_t nb_id;
    float threshold;
} VioletNoiseBlankerThresholdChanged;

typedef struct {
    VioletVfoEventCommon base;
    double level;
} VioletSqlLevelChanged;

typedef struct {
    VioletVfoEventCommon base;
    double alpha;
} VioletSqlAlphaChanged;

typedef struct {
    VioletVfoEventCommon base;
    bool enabled;
} VioletAgcOnChanged;

typedef struct {
    VioletVfoEventCommon base;
    bool enabled;
} VioletAgcHangChanged;

typedef struct {
    VioletVfoEventCommon base;
    int32_t threshold;
} VioletAgcThresholdChanged;

typedef struct {
    VioletVfoEventCommon base;
    int32_t slope;
} VioletAgcSlopeChanged;

typedef struct {
    VioletVfoEventCommon base;
    int32_t decay;
} VioletAgcDecayChanged;

typedef struct {
    VioletVfoEventCommon base;
    int32_t gain;
} VioletAgcManualGainChanged;

typedef struct {
    VioletVfoEventCommon base;
    float maxdev;
} VioletFmMaxDevChanged;

typedef struct {
    VioletVfoEventCommon base;
    double tau;
} VioletFmDeemphChanged;

typedef struct {
    VioletVfoEventCommon base;
    bool enabled;
} VioletAmDcrChanged;

typedef struct {
    VioletVfoEventCommon base;
    bool enabled;
} VioletAmSyncDcrChanged;

typedef struct {
    VioletVfoEventCommon base;
    float bw;
} VioletAmSyncPllBwChanged;

typedef struct {
    VioletVfoEventCommon base;
    const char* path;
} VioletRecordingStarted;

typedef struct {
    VioletVfoEventCommon base;
} VioletRecordingStopped;

typedef struct {
    VioletVfoEventCommon base;
    int32_t sampleRate;
    int32_t buffSize;
} VioletSnifferStarted;

typedef struct {
    VioletVfoEventCommon base;
} VioletSnifferStopped;

typedef struct {
    VioletVfoEventCommon base;
} VioletRdsDecoderStarted;

typedef struct {
    VioletVfoEventCommon base;
} VioletRdsDecoderStopped;

typedef struct {
    VioletVfoEventCommon base;
} VioletRdsParserReset;

typedef struct {
    VioletVfoEventCommon base;
    const char* host;
    int32_t port;
    bool stereo;
} VioletUdpStreamingStarted;

typedef struct {
    VioletVfoEventCommon base;
} VioletUdpStreamingStopped;

typedef struct {
    VioletVfoEventCommon base;
    float gain;
} VioletAudioGainChanged;

typedef void (*VioletEventCallback)(const VioletEventCommon*, void*);
typedef void (*VioletVfoEventCallback)(const VioletVfoEventCommon*, void*);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // EVENTS_C
