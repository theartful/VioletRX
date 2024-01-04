#ifndef VFO_CHANNEL
#define VFO_CHANNEL

#include <chrono>
#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/wavfile_sink.h>
#include <gnuradio/sync_block.h>

#include "core/interfaces/udp_sink_f.h"
#include "dsp/multichannel_downconverter.h"
#include "dsp/resampler_xx.h"
#include "dsp/sniffer_f.h"

#include "receivers/receiver_base.h"

#include <gnuradio/audio/sink.h>

class receiver;

class vfo_channel : public gr::hier_block2
{
public:
    using sptr = std::shared_ptr<vfo_channel>;
    static vfo_channel::sptr
    make(multichannel_downconverter_cc::sptr downconverter, int idx);

    /** Supported receiver types */
    enum rx_chain {
        RX_CHAIN_NONE = 0, /*!< No receiver, just spectrum analyzer */
        RX_CHAIN_NBRX = 1, /*!< Narrow band receiver (AM, FM, SSB) */
        RX_CHAIN_WFMRX = 2 /*!< Wide band FM receiver (for broadcast) */
    };

    /** Available demodulators */
    enum rx_demod {
        RX_DEMOD_OFF = 0,        /*!< No receiver */
        RX_DEMOD_NONE = 1,       /*!< No demod Raw I/Q to audio */
        RX_DEMOD_AM = 2,         /*!< Amplitude modulation */
        RX_DEMOD_NFM = 3,        /*!< Frequency modulation */
        RX_DEMOD_WFM_M = 4,      /*!< Frequency modulation (wide, mono) */
        RX_DEMOD_WFM_S = 5,      /*!< Frequency modulation (wide, stereo) */
        RX_DEMOD_WFM_S_OIRT = 6, /*!< Frequency modulation (wide, stereo oirt)*/
        RX_DEMOD_SSB = 7,        /*!< Single Side Band */
        RX_DEMOD_AMSYNC = 8 /*!< Amplitude modulation (synchronous demod) */
    };

    /** Filter shape (convenience wrappers for "transition width") */
    enum filter_shape {
        FILTER_SHAPE_SOFT = 0,   /*!< Soft: Transition band is TBD of width */
        FILTER_SHAPE_NORMAL = 1, /*!< Normal: Transition band is TBD of width */
        FILTER_SHAPE_SHARP = 2   /*!< Sharp: Transition band is TBD of width */
    };

    struct udp_stream_params {
        std::string host;
        int port;
        bool stereo;
    };

    struct sniffer_params {
        int samplerate;
        int buffsize;
    };

    vfo_channel(multichannel_downconverter_cc::sptr downconverter, int idx);
    ~vfo_channel();

    void set_ddc_idx(int idx);
    int get_ddc_idx();

    bool set_filter_offset(double offset_hz);
    bool set_filter(double low, double high, filter_shape shape);
    bool set_cw_offset(double offset_hz);
    float get_signal_pwr() const;

    void set_quad_rate(double quad_rate);
    int get_quad_rate();

    /* Noise blanker */
    bool set_nb_on(int nbid, bool on);
    bool set_nb_threshold(int nbid, float threshold);

    /* Squelch parameter */
    bool set_sql_level(double level_db);
    bool set_sql_alpha(double alpha);

    /* AGC */
    bool set_agc_on(bool agc_on);
    bool set_agc_hang(bool use_hang);
    bool set_agc_threshold(int threshold);
    bool set_agc_slope(int slope);
    bool set_agc_decay(int decay_ms);
    bool set_agc_manual_gain(int gain);

    /* FM parameters */
    bool set_fm_maxdev(float maxdev_hz);
    bool set_fm_deemph(double tau);

    /* AM parameters */
    bool set_am_dcr(bool enabled);

    /* AM-Sync parameters */
    bool set_amsync_dcr(bool enabled);
    bool set_amsync_pll_bw(float pll_bw);

    bool set_demod(rx_demod demod, bool force = false);
    rx_demod get_demod() { return d_demod; }

    /* Audio recording */
    bool set_af_gain(float gain_db);
    float get_af_gain() const { return d_af_gain; }
    bool start_audio_recording(std::string filename);
    bool stop_audio_recording();
    bool is_recording_audio() const { return d_recording_wav; }
    std::string get_recording_filename() const { return recording_filename; }

    /* UDP streaming */
    bool start_udp_streaming(const std::string& host, int port, bool stereo);
    bool stop_udp_streaming();
    bool is_udp_streaming() const { return d_udp_streaming; }
    udp_stream_params get_udp_stream_params() const { return d_udp_params; }

    /* sample sniffer */
    bool start_sniffer(int samplrate, int buffsize);
    bool stop_sniffer();
    int get_sniffer_buffsize();
    void get_sniffer_data(float* outbuff, int& num);
    bool is_snifffer_active(void) const { return d_sniffer_active; }
    sniffer_params get_sniffer_params() const { return d_sniffer_params; }

    /* rds functions */
    void get_rds_data(std::string& outbuff, int& num);
    void start_rds_decoder(void);
    void stop_rds_decoder();
    bool is_rds_decoder_active(void) const;
    void reset_rds_parser(void);

    /* properties */
    double get_filter_low() const { return d_filter_low; }
    double get_filter_high() const { return d_filter_high; }
    filter_shape get_filter_shape() const { return d_filter_shape; }
    double get_filter_offset() const { return d_filter_offset; }
    double get_cw_offset() const { return d_cw_offset; }

    void set_parent_receiver(std::weak_ptr<receiver> rx_);
    std::shared_ptr<receiver> get_parent_receiver();

protected:
    void connect_all(rx_chain type);
    bool is_running();

protected:
    int d_ddc_idx;

    double d_quad_rate;  /*!< Quadrature rate (after down-conversion) */
    double d_audio_rate; /*!< Audio output rate */

    rx_demod d_demod;            /*!< Current demodulator */
    filter_shape d_filter_shape; /*!< Current filter shape */
    double d_filter_offset;      /*!< Current filter offset */
    double d_filter_low;         /*!< Current filter low */
    double d_filter_high;        /*!< Current filter low */
    double d_cw_offset;          /*!< CW offset */

    bool d_recording_wav;
    bool d_sniffer_active;
    bool d_udp_streaming;

    float d_af_gain;

    std::string recording_filename;
    udp_stream_params d_udp_params;
    sniffer_params d_sniffer_params;

    gr::blocks::null_sink::sptr null_sink;
    multichannel_downconverter_cc::sptr ddc;
    receiver_base_cf_sptr rx; /*!< receiver */

    // recording
    gr::blocks::file_sink::sptr iq_sink;     /*!< I/Q file sink */
    gr::blocks::wavfile_sink::sptr wav_sink; /*!< WAV file sink for recording */

    udp_sink_f_sptr
        audio_udp_sink;           /*!< UDP sink to stream audio over network */
    sniffer_f_sptr sniffer;       /*!< Sample sniffer for data decoders */
    resampler_ff_sptr sniffer_rr; /*!< Sniffer resampler */

    gr::blocks::multiply_const_ff::sptr audio_gain0; /*!< Audio gain block */
    gr::blocks::multiply_const_ff::sptr audio_gain1; /*!< Audio gain block */

    gr::audio::sink::sptr audio_snk; /*!< gr audio sink */

    std::weak_ptr<receiver> parent_rx;
};

#endif // VFO_CHANNEL
