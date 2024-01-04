#ifndef TYPES_C
#define TYPES_C

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct VioletTimestamp {
    uint64_t seconds;
    uint32_t nanos;
} VioletTimestamp;

typedef struct VioletGainStage {
    const char* name;
    double start;
    double stop;
    double step;
    double value;
} VioletGainStage;

// see gr::fft::window
enum VioletWindowType {
    VIOLET_WINDOW_HAMMING = 0,
    VIOLET_WINDOW_HANN = 1,
    VIOLET_WINDOW_BLACKMAN = 2,
    VIOLET_WINDOW_RECTANGULAR = 3,
    VIOLET_WINDOW_KAISER = 4,
    VIOLET_WINDOW_BLACKMAN_HARRIS = 5,
    VIOLET_WINDOW_BARTLETT = 6,
    VIOLET_WINDOW_FLATTOP = 7,
    VIOLET_WINDOW_NUTTALL = 8,
    VIOLET_WINDOW_NUTTALL_CFD = 9,
    VIOLET_WINDOW_WELCH = 10,
    VIOLET_WINDOW_PARZEN = 11,
    VIOLET_WINDOW_EXPONENTIAL = 12,
    VIOLET_WINDOW_RIEMANN = 13,
    VIOLET_WINDOW_GAUSSIAN = 14,
    VIOLET_WINDOW_TUKEY = 15,
};

enum VioletFilterShape {
    VIOLET_FILTER_SHAPE_SOFT = 0,
    VIOLET_FILTER_SHAPE_NORMAL = 1,
    VIOLET_FILTER_SHAPE_SHARP = 2
};

enum VioletDemod {
    VIOLET_DEMOD_OFF = 0,
    VIOLET_DEMOD_RAW = 1,
    VIOLET_DEMOD_AM = 2,
    VIOLET_DEMOD_AM_SYNC = 3,
    VIOLET_DEMOD_LSB = 4,
    VIOLET_DEMOD_USB = 5,
    VIOLET_DEMOD_CWL = 6,
    VIOLET_DEMOD_CWU = 7,
    VIOLET_DEMOD_NFM = 8,
    VIOLET_DEMOD_WFM_MONO = 9,
    VIOLET_DEMOD_WFM_STEREO = 10,
    VIOLET_DEMOD_WFM_STEREO_OIRT = 11,
    VIOLET_DEMOD_LAST = 12
};

struct VioletFilter {
    VioletFilterShape shape;
    int64_t low;
    int64_t high;
};

struct VioletFilterRange {
    int64_t lowMin;
    int64_t lowMax;
    int64_t highMin;
    int64_t highMax;
    bool symmetric;
};

typedef int VioletError;
typedef void VioletReceiver;
typedef void VioletVfo;
typedef void VioletConnection;
typedef void (*VioletVoidCallback)(VioletError, void*);
typedef void (*VioletIntCallback)(VioletError, int, void*);
typedef void (*VioletFloatCallback)(VioletError, double, void*);
typedef void (*VioletDoubleCallback)(VioletError, double, void*);
typedef void (*VioletInt64Callback)(VioletError, int64_t, void*);
typedef void (*VioletVfoCallback)(VioletError, VioletVfo*, void*);
typedef void (*VioletFftDataCallback)(VioletError, VioletTimestamp, int64_t,
                                      int, float*, int, void*);
typedef void (*VioletSyncCallback)(VioletError, void*);

#ifdef __cplusplus
}
#endif

#endif
