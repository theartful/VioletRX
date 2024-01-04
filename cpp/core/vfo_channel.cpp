#include <array>
#include <stdexcept>

#include "dsp/multichannel_downconverter.h"
#include "receivers/nbrx.h"
#include "receivers/wfmrx.h"

#include "receiver.h"
#include "vfo_channel.h"

static constexpr double DEFAULT_AUDIO_GAIN = -6.0;

vfo_channel::sptr
vfo_channel::make(multichannel_downconverter_cc::sptr downconverter,
                  int ddc_idx)
{
    return gnuradio::make_block_sptr<vfo_channel>(downconverter, ddc_idx);
}

vfo_channel::vfo_channel(multichannel_downconverter_cc::sptr downconverter,
                         int ddc_idx) :
    gr::hier_block2("vfo_channel",
                    gr::io_signature::make(1, 1, sizeof(gr_complex)),
                    gr::io_signature::make(0, 0, 0)),
    d_ddc_idx(ddc_idx),
    d_quad_rate(48e3),
    d_audio_rate(48000),
    d_demod(RX_DEMOD_OFF),
    d_filter_shape(FILTER_SHAPE_NORMAL),
    d_filter_offset(0.0),
    d_filter_low(0.0),
    d_filter_high(0.0),
    d_cw_offset(0.0),
    d_recording_wav(false),
    d_sniffer_active(false),
    d_udp_streaming(false),
    ddc(downconverter)
{
    rx = make_nbrx(d_quad_rate, d_audio_rate);
    audio_gain0 = gr::blocks::multiply_const_ff::make(0);
    audio_gain1 = gr::blocks::multiply_const_ff::make(0);
    set_af_gain(DEFAULT_AUDIO_GAIN);

    audio_udp_sink = make_udp_sink_f();

    // FIXME: customize audio device!!
    std::string audio_device = "";
    audio_snk = gr::audio::sink::make(d_audio_rate, audio_device, true);

    sniffer = make_sniffer_f();

    null_sink = gr::blocks::null_sink::make(sizeof(std::complex<float>));
    set_demod(RX_DEMOD_OFF, true);
}

void vfo_channel::set_ddc_idx(int idx) { d_ddc_idx = idx; }

int vfo_channel::get_ddc_idx() { return d_ddc_idx; }

bool vfo_channel::set_filter_offset(double offset_hz)
{
    d_filter_offset = offset_hz;
    ddc->set_offset(offset_hz, d_ddc_idx);

    return true;
}

void vfo_channel::set_quad_rate(double quad_rate)
{
    d_quad_rate = quad_rate;

    lock();
    rx->set_quad_rate(d_quad_rate);
    unlock();
}

int vfo_channel::get_quad_rate() { return d_quad_rate; }

bool vfo_channel::set_filter(double low, double high, filter_shape shape)
{
    if ((low >= high) || (std::abs(high - low) < RX_FILTER_MIN_WIDTH))
        return false;

    double trans_width;

    switch (shape) {
    case FILTER_SHAPE_SOFT:
        trans_width = std::abs(high - low) * 0.5;
        break;

    case FILTER_SHAPE_SHARP:
        trans_width = std::abs(high - low) * 0.1;
        break;

    case FILTER_SHAPE_NORMAL:
    default:
        trans_width = std::abs(high - low) * 0.2;
        break;
    }

    rx->set_filter(low, high, trans_width);

    d_filter_low = low;
    d_filter_high = high;
    d_filter_shape = shape;

    return true;
}

/* CW offset can serve as a "BFO" if the GUI needs it */
bool vfo_channel::set_cw_offset(double offset_hz)
{
    d_cw_offset = offset_hz;
    ddc->set_offset(d_filter_offset - d_cw_offset, d_ddc_idx);
    rx->set_cw_offset(d_cw_offset);

    return true;
}

bool vfo_channel::set_nb_on(int nbid, bool on)
{
    if (rx->has_nb())
        rx->set_nb_on(nbid, on);

    return true; // FIXME
}

bool vfo_channel::set_nb_threshold(int nbid, float threshold)
{
    if (rx->has_nb())
        rx->set_nb_threshold(nbid, threshold);

    return true; // FIXME
}

bool vfo_channel::set_sql_level(double level_db)
{
    if (rx->has_sql())
        rx->set_sql_level(level_db);

    return true; // FIXME
}

bool vfo_channel::set_sql_alpha(double alpha)
{
    if (rx->has_sql())
        rx->set_sql_alpha(alpha);

    return true; // FIXME
}

bool vfo_channel::set_agc_on(bool agc_on)
{
    if (rx->has_agc())
        rx->set_agc_on(agc_on);

    return true; // FIXME
}

bool vfo_channel::set_agc_hang(bool use_hang)
{
    if (rx->has_agc())
        rx->set_agc_hang(use_hang);

    return true; // FIXME
}

bool vfo_channel::set_agc_threshold(int threshold)
{
    if (rx->has_agc())
        rx->set_agc_threshold(threshold);

    return true; // FIXME
}

bool vfo_channel::set_agc_slope(int slope)
{
    if (rx->has_agc())
        rx->set_agc_slope(slope);

    return true; // FIXME
}

bool vfo_channel::set_agc_decay(int decay_ms)
{
    if (rx->has_agc())
        rx->set_agc_decay(decay_ms);

    return true; // FIXME
}

bool vfo_channel::set_agc_manual_gain(int gain)
{
    if (rx->has_agc())
        rx->set_agc_manual_gain(gain);

    return true; // FIXME
}

bool vfo_channel::set_fm_maxdev(float maxdev_hz)
{
    if (rx->has_fm())
        rx->set_fm_maxdev(maxdev_hz);

    return true;
}

bool vfo_channel::set_fm_deemph(double tau)
{
    if (rx->has_fm())
        rx->set_fm_deemph(tau);

    return true;
}

bool vfo_channel::set_am_dcr(bool enabled)
{
    if (rx->has_am())
        rx->set_am_dcr(enabled);

    return true;
}

bool vfo_channel::set_amsync_dcr(bool enabled)
{
    if (rx->has_amsync())
        rx->set_amsync_dcr(enabled);

    return true;
}

bool vfo_channel::set_amsync_pll_bw(float pll_bw)
{
    if (rx->has_amsync())
        rx->set_amsync_pll_bw(pll_bw);

    return true;
}

bool vfo_channel::set_demod(rx_demod demod, bool force)
{
    if (!force && demod == d_demod)
        return true;

    lock();
    disconnect_all();

    bool ret = true;

    switch (demod) {
    case RX_DEMOD_OFF:
        connect_all(RX_CHAIN_NONE);
        break;

    case RX_DEMOD_NONE:
        connect_all(RX_CHAIN_NBRX);
        rx->set_demod(nbrx::NBRX_DEMOD_NONE);
        break;

    case RX_DEMOD_AM:
        connect_all(RX_CHAIN_NBRX);
        rx->set_demod(nbrx::NBRX_DEMOD_AM);
        break;

    case RX_DEMOD_AMSYNC:
        connect_all(RX_CHAIN_NBRX);
        rx->set_demod(nbrx::NBRX_DEMOD_AMSYNC);
        break;

    case RX_DEMOD_NFM:
        connect_all(RX_CHAIN_NBRX);
        rx->set_demod(nbrx::NBRX_DEMOD_FM);
        break;

    case RX_DEMOD_WFM_M:
        connect_all(RX_CHAIN_WFMRX);
        rx->set_demod(wfmrx::WFMRX_DEMOD_MONO);
        break;

    case RX_DEMOD_WFM_S:
        connect_all(RX_CHAIN_WFMRX);
        rx->set_demod(wfmrx::WFMRX_DEMOD_STEREO);
        break;

    case RX_DEMOD_WFM_S_OIRT:
        connect_all(RX_CHAIN_WFMRX);
        rx->set_demod(wfmrx::WFMRX_DEMOD_STEREO_UKW);
        break;

    case RX_DEMOD_SSB:
        connect_all(RX_CHAIN_NBRX);
        rx->set_demod(nbrx::NBRX_DEMOD_SSB);
        break;

    default:
        ret = false;
        break;
    }

    d_demod = demod;
    unlock();

    return ret;
}

void vfo_channel::connect_all(rx_chain type)
{
    // RX demod chain
    switch (type) {
    case RX_CHAIN_NBRX:
        if (rx->name() != "NBRX") {
            rx.reset();
            rx = make_nbrx(d_quad_rate, d_audio_rate);
        }
        break;

    case RX_CHAIN_WFMRX:
        if (rx->name() != "WFMRX") {
            rx.reset();
            rx = make_wfmrx(d_quad_rate, d_audio_rate);
        }
        break;

    default:
        break;
    }

    // Audio path (if there is a receiver)
    if (type != RX_CHAIN_NONE) {
        connect(self(), 0, rx, 0);
        connect(rx, 0, audio_udp_sink, 0);
        connect(rx, 1, audio_udp_sink, 1);
        connect(rx, 0, audio_gain0, 0);
        connect(rx, 1, audio_gain1, 0);
        connect(audio_gain0, 0, audio_snk, 0);
        connect(audio_gain1, 0, audio_snk, 1);

        if (d_recording_wav) {
            connect(rx, 0, wav_sink, 0);
            connect(rx, 1, wav_sink, 1);
        }
        if (d_sniffer_active) {
            connect(rx, 0, sniffer_rr, 0);
            connect(sniffer_rr, 0, sniffer, 0);
        }
    } else {
        connect(self(), 0, null_sink, 0);
    }
}

bool vfo_channel::set_af_gain(float gain_db)
{
    d_af_gain = gain_db;

    /* convert dB to factor */
    float k = powf(10.0f, gain_db / 20.0f);
    audio_gain0->set_k(k);
    audio_gain1->set_k(k);

    return true;
}

bool vfo_channel::start_audio_recording(std::string filename)
{
    if (d_recording_wav) {
        /* error - we are already recording */
        d_logger->warn("Can not start audio recorder (already recording)");
        return false;
    }

    if (d_demod == RX_DEMOD_OFF) {
        d_logger->warn("Can not start audio recorder (channel is off)");
        return false;
    }

    if (!is_running()) {
        d_logger->warn("Can not start audio recorder (receiver not running)");
        return false;
    }

    // if this fails, we don't want to go and crash now, do we
    try {
        wav_sink = gr::blocks::wavfile_sink::make(
            filename.c_str(), 2, (unsigned int)d_audio_rate,
            gr::blocks::FORMAT_WAV, gr::blocks::FORMAT_PCM_16);
    } catch (std::runtime_error& e) {
        d_logger->error("Error opening {}: {}", filename, e.what());
        return false;
    }

    lock();
    connect(rx, 0, wav_sink, 0);
    connect(rx, 1, wav_sink, 1);
    unlock();

    d_recording_wav = true;
    d_logger->info("Recording audio to {}", filename);

    recording_filename = std::move(filename);

    return true;
}

/** Stop WAV file recorder. */
bool vfo_channel::stop_audio_recording()
{
    if (!d_recording_wav) {
        /* error: we are not recording */
        d_logger->error("Can not stop audio recorder (not recording)");
        return false;
    }

    // not strictly necessary to lock but I think it is safer
    lock();
    wav_sink->close();
    disconnect(rx, 0, wav_sink, 0);
    disconnect(rx, 1, wav_sink, 1);

    unlock();
    wav_sink.reset();
    d_recording_wav = false;

    d_logger->info("Audio recorder stopped");
    return true;
}

void vfo_channel::set_parent_receiver(std::weak_ptr<receiver> rx_)
{
    parent_rx = rx_;
}

bool vfo_channel::is_running()
{
    auto parent_rx_locked = parent_rx.lock();
    return parent_rx_locked && parent_rx_locked->is_running();
}

std::shared_ptr<receiver> vfo_channel::get_parent_receiver()
{
    return parent_rx.lock();
}

/**
 * @brief Get current signal power.
 * @param dbfs Whether to use dbfs or absolute power.
 * @return The current signal power.
 *
 * This method returns the current signal power detected by the receiver. The
 * detector is located after the band pass filter. The full scale is 1.0
 */
float vfo_channel::get_signal_pwr() const { return rx->get_signal_level(); }

void vfo_channel::get_rds_data(std::string& outbuff, int& num)
{
    rx->get_rds_data(outbuff, num);
}

void vfo_channel::start_rds_decoder(void)
{
    if (is_running()) {
        lock();
        rx->start_rds_decoder();
        unlock();
    } else {
        rx->start_rds_decoder();
    }
}

void vfo_channel::stop_rds_decoder(void)
{
    if (is_running()) {
        lock();
        rx->stop_rds_decoder();
        unlock();
    } else {
        rx->stop_rds_decoder();
    }
}

bool vfo_channel::is_rds_decoder_active(void) const
{
    return rx->is_rds_decoder_active();
}

void vfo_channel::reset_rds_parser(void) { rx->reset_rds_parser(); }

bool vfo_channel::start_udp_streaming(const std::string& host, int port,
                                      bool stereo)
{
    audio_udp_sink->start_streaming(host, port, stereo);
    d_udp_streaming = true;
    d_udp_params = {host, port, stereo};
    return true;
}
bool vfo_channel::stop_udp_streaming()
{
    audio_udp_sink->stop_streaming();
    d_udp_streaming = false;
    return true;
}

/**
 * @brief Start data sniffer.
 * @param buffsize The buffer that should be used in the sniffer.
 * @return STATUS_OK if the sniffer was started, STATUS_ERROR if the sniffer is
 * already in use.
 */
bool vfo_channel::start_sniffer(int samprate, int buffsize)
{
    if (d_sniffer_active) {
        /* sniffer already in use */
        return false;
    }

    sniffer->set_buffer_size(buffsize);
    sniffer_rr = make_resampler_ff((float)samprate / (float)d_audio_rate);
    lock();
    connect(rx, 0, sniffer_rr, 0);
    connect(sniffer_rr, 0, sniffer, 0);
    unlock();
    d_sniffer_active = true;

    d_sniffer_params = { samprate, buffsize };

    return true;
}

/**
 * @brief Stop data sniffer.
 * @return STATUS_ERROR i the sniffer is not currently active.
 */
bool vfo_channel::stop_sniffer()
{
    if (!d_sniffer_active) {
        return false;
    }

    lock();
    disconnect(rx, 0, sniffer_rr, 0);

    // Temporary workaround for https://github.com/gnuradio/gnuradio/issues/5436
    disconnect(ddc, 0, rx, 0);
    connect(ddc, 0, rx, 0);
    // End temporary workaronud

    disconnect(sniffer_rr, 0, sniffer, 0);
    unlock();
    d_sniffer_active = false;

    /* delete resampler */
    sniffer_rr.reset();

    return true;
}

/** Get sniffer data. */
void vfo_channel::get_sniffer_data(float* outbuff, int& num)
{
    sniffer->get_samples(outbuff, num);
}

int vfo_channel::get_sniffer_buffsize() { return sniffer->buffer_size(); }

vfo_channel::~vfo_channel()
{
    d_logger->debug("~vfo_channel ({})", fmt::ptr(this));
}
