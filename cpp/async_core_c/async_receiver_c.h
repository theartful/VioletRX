#ifndef C_ASYNC_RECEIVER_H
#define C_ASYNC_RECEIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "async_core_c/events_c.h"
#include "async_core_c/types_c.h"

VioletReceiver* violet_rx_init();
VioletReceiver* violet_rx_new_ref(VioletReceiver* rx);
void violet_rx_destroy(VioletReceiver* rx);
void violet_vfo_destroy(VioletVfo* vfo);

void violet_rx_start(VioletReceiver* rx, VioletVoidCallback callback,
                     void* userdata);
void violet_rx_stop(VioletReceiver* rx, VioletVoidCallback callback,
                    void* userdata);
void violet_rx_set_input_dev(VioletReceiver* rx, const char*,
                             VioletVoidCallback callback, void* userdata);
void violet_rx_set_antenna(VioletReceiver* rx, const char*,
                           VioletVoidCallback callback, void* userdata);
void violet_rx_set_input_rate(VioletReceiver* rx, int samplerate,
                              VioletIntCallback callback, void* userdata);
void violet_rx_set_input_decim(VioletReceiver* rx, int decim,
                               VioletIntCallback callback, void* userdata);
void violet_rx_set_rf_freq(VioletReceiver* rx, int64_t freq,
                           VioletInt64Callback callback, void* userdata);
void violet_rx_set_iq_swap(VioletReceiver* rx, bool enable,
                           VioletVoidCallback callback, void* userdata);
void violet_rx_set_dc_cancel(VioletReceiver* rx, bool enable,
                             VioletVoidCallback callback, void* userdata);
void violet_rx_set_iq_balance(VioletReceiver* rx, bool enable,
                              VioletVoidCallback callback, void* userdata);
void violet_rx_set_auto_gain(VioletReceiver* rx, bool enable,
                             VioletVoidCallback callback, void* userdata);
void violet_rx_set_gain(VioletReceiver* rx, const char* gain_name, double value,
                        VioletDoubleCallback callback, void* userdata);
void violet_rx_set_freq_corr(VioletReceiver* rx, double ppm,
                             VioletDoubleCallback callback, void* userdata);
void violet_rx_set_fft_size(VioletReceiver* rx, int fftsize,
                            VioletVoidCallback callback, void* userdata);
void violet_rx_set_fft_window(VioletReceiver* rx, VioletWindowType window,
                              bool normalize_energy,
                              VioletVoidCallback callback, void* userdata);
void violet_rx_get_fft_data(VioletReceiver* rx, float* data, int size,
                            VioletFftDataCallback callback, void* userdata);

void violet_rx_add_vfo(VioletReceiver* rx, VioletVfoCallback callback,
                       void* userdata);
void violet_rx_remove_vfo(VioletReceiver* rx, VioletVfo* vfo,
                          VioletVoidCallback callback, void* userdata);

void voilet_rx_synchronize(VioletReceiver* rx, VioletSyncCallback,
                           void* userdata);
char* violet_rx_get_input_dev(VioletReceiver* rx);
char* violet_rx_get_cur_antenna(VioletReceiver* rx);
int violet_rx_get_input_rate(VioletReceiver* rx);
int violet_rx_get_input_decim(VioletReceiver* rx);
bool violet_rx_get_dc_cancel(VioletReceiver* rx);
bool violet_rx_get_iq_balance(VioletReceiver* rx);
bool violet_rx_get_iq_swap(VioletReceiver* rx);
int64_t violet_rx_get_rf_freq(VioletReceiver* rx);
int violet_rx_get_gain_stages_count(VioletReceiver* rx);
VioletGainStage violet_rx_get_gain_stage(VioletReceiver* rx, int idx);
int violet_rx_get_antennas_count(VioletReceiver* rx);
char* violet_rx_get_antenna(VioletReceiver* rx, int idx);
bool violet_rx_get_auto_gain(VioletReceiver* rx);
double violet_rx_get_freq_corr(VioletReceiver* rx);
int violet_rx_get_fft_size(VioletReceiver* rx);
VioletWindowType violet_rx_get_fft_window(VioletReceiver* rx);
int violet_rx_get_vfos_count(VioletReceiver* rx);
VioletVfo* violet_rx_get_vfo(VioletReceiver* rx, int idx);

void violet_rx_subscribe(VioletReceiver* rx, VioletEventHandler handler,
                         void* userdata, VioletConnectionCallback callback,
                         void* userdata2);
void violet_unsubscribe(VioletConnection* connection);

#ifdef __cplusplus
}
#endif

#endif
