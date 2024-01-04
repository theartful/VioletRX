#ifndef C_ASYNC_VFO
#define C_ASYNC_VFO

#if __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "async_core_c/events_c.h"
#include "async_core_c/types_c.h"

typedef void (*VioletSnifferDataCallback)(VioletError, float*, int, void*);
typedef void (*VioletRdsDataCallback)(VioletError, const char*, int, void*);

/* filter */
void violet_vfo_set_filter_offset(VioletVfo* vfo, int64_t offset,
                                  VioletVoidCallback callback, void* userdata);
void violet_vfo_set_filter(VioletVfo* vfo, int64_t low, int64_t high,
                           VioletFilterShape filter_shape,
                           VioletVoidCallback callback, void* userdata);
void violet_vfo_set_cw_offset(VioletVfo* vfo, int64_t offset,
                              VioletVoidCallback callback, void* userdata);
void violet_vfo_set_demod(VioletVfo* vfo, VioletDemod demod,
                          VioletVoidCallback callback, void* userdata);
void violet_vfo_get_signal_pwr(VioletVfo* vfo, VioletFloatCallback callback,
                               void* userdata);

/* Noise blanker */
void violet_vfo_set_noise_blanker(VioletVfo* vfo, int id, bool on,
                                  VioletVoidCallback callback, void* userdata);
void violet_vfo_set_noise_threshold(VioletVfo* vfo, int id, float threshold,
                                    VioletVoidCallback callback,
                                    void* userdata);

/* Sql parameter */
void violet_vfo_set_sql_level(VioletVfo* vfo, double level_db,
                              VioletVoidCallback callback, void* userdata);
void violet_vfo_set_sql_alpha(VioletVfo* vfo, double alpha,
                              VioletVoidCallback callback, void* userdata);

/* AGC */
void violet_vfo_set_agc_on(VioletVfo* vfo, bool on, VioletVoidCallback callback,
                           void* userdata);
void violet_vfo_set_agc_hang(VioletVfo* vfo, bool on,
                             VioletVoidCallback callback, void* userdata);
void violet_vfo_set_agc_threshold(VioletVfo* vfo, int threshold,
                                  VioletVoidCallback callback, void* userdata);
void violet_vfo_set_agc_slope(VioletVfo* vfo, int slope,
                              VioletVoidCallback callback, void* userdata);
void violet_vfo_set_agc_decay(VioletVfo* vfo, int decay,
                              VioletVoidCallback callback, void* userdata);
void violet_vfo_set_agc_manual_gain(VioletVfo* vfo, int gain,
                                    VioletVoidCallback callback,
                                    void* userdata);

/* FM parameters */
void violet_vfo_set_fm_maxdev(VioletVfo* vfo, float maxdev,
                              VioletVoidCallback callback, void* userdata);
void violet_vfo_set_fm_deemph(VioletVfo* vfo, double tau,
                              VioletVoidCallback callback, void* userdata);

/* AM parameters */
void violet_vfo_set_am_dcr(VioletVfo* vfo, bool enable,
                           VioletVoidCallback callback, void* userdata);

/* AM-Sync parameters */
void violet_vfo_set_am_sync_dcr(VioletVfo* vfo, bool enable,
                                VioletVoidCallback callback, void* userdata);
void violet_vfo_set_am_sync_pll_bw(VioletVfo* vfo, float bw,
                                   VioletVoidCallback callback, void* userdata);

/* Audio Recording */
void violet_vfo_start_audio_recording(VioletVfo* vfo, const char* filename,
                                      VioletVoidCallback callback,
                                      void* userdata);
void violet_vfo_stop_audio_recording(VioletVfo* vfo,
                                     VioletVoidCallback callback,
                                     void* userdata);
void violet_vfo_set_audio_gain(VioletVfo* vfo, float,
                               VioletVoidCallback callback, void* userdata);

/* sample sniffer */
void violet_vfo_start_sniffer(VioletVfo* vfo, int samplerate, int buffsize,
                              VioletVoidCallback callback, void* userdata);
void violet_vfo_stop_sniffer(VioletVfo* vfo, VioletVoidCallback callback,
                             void* userdata);
void violet_vfo_get_sniffer_data(VioletVfo* vfo, float* data, int size,
                                 VioletSnifferDataCallback callback,
                                 void* userdata);

/* rds functions */
void violet_vfo_get_rds_data(VioletVfo* vfo, VioletRdsDataCallback callback,
                             void* userdata);
void violet_vfo_start_rds_decoder(VioletVfo* vfo, VioletVoidCallback callback,
                                  void* userdata);
void violet_vfo_stop_rds_decoder(VioletVfo* vfo, VioletVoidCallback callback,
                                 void* userdata);
void violet_vfo_reset_rds_parser(VioletVfo* vfo, VioletVoidCallback callback,
                                 void* userdata);

uint64_t violet_vfo_get_id(VioletVfo* vfo);
VioletFilterRange violet_vfo_get_filter_range(VioletVfo* vfo,
                                              VioletDemod demod);

/* Sync API: getters can only be called inside a successful callback function */
void violet_vfo_synchronize(VioletVfo* vfo, VioletSyncCallback, void* userdata);
VioletDemod violet_vfo_get_demod(VioletVfo* vfo);
int64_t violet_vfo_get_offset(VioletVfo* vfo);
VioletFilter violet_vfo_get_filter(VioletVfo* vfo);
int64_t violet_vfo_get_cw_offset(VioletVfo* vfo);
float violet_vfo_get_fm_maxdev(VioletVfo* vfo);
double violet_vfo_get_fm_deemph(VioletVfo* vfo);
bool violet_vfo_get_am_dcr(VioletVfo* vfo);
bool violet_vfo_get_am_sync_dcr(VioletVfo* vfo);
float violet_vfo_get_am_sync_pll_bw(VioletVfo* vfo);
bool violet_vfo_is_audio_recording(VioletVfo* vfo);
char* violet_vfo_recording_filename(VioletVfo* vfo);
bool violet_vfo_is_udp_streaming(VioletVfo* vfo);
bool violet_vfo_is_sniffing(VioletVfo* vfo);
bool violet_vfo_is_rds_decoder_active(VioletVfo* vfo);
bool violet_vfo_is_agc_on(VioletVfo* vfo);
bool violet_vfo_is_agc_hang_on(VioletVfo* vfo);
int violet_vfo_get_agc_threshold(VioletVfo* vfo);
int violet_vfo_get_agc_slope(VioletVfo* vfo);
int violet_vfo_get_agc_decay(VioletVfo* vfo);
int violet_vfo_get_agc_manual_gain(VioletVfo* vfo);
double violet_vfo_get_sql_level(VioletVfo* vfo);
double violet_vfo_get_sql_alpha(VioletVfo* vfo);
bool violet_vfo_is_noise_blanker1_on(VioletVfo* vfo);
bool violet_vfo_is_noise_blanker2_on(VioletVfo* vfo);
float violet_vfo_get_noise_blanker1_threshold(VioletVfo* vfo);
float violet_vfo_get_noise_blanker2_threshold(VioletVfo* vfo);

VioletConnection* violet_vfo_subscribe(VioletVfo* vfo,
                                       VioletVfoEventCallback callback,
                                       void* userdata);
}

#if __cplusplus
#endif
#endif
