/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://gqrx.dk/
 *
 * Copyright 2011-2014 Alexandru Csete OZ9AEC.
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef RECEIVER_H
#define RECEIVER_H

#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/wavfile_sink.h>
#include <gnuradio/blocks/wavfile_source.h>
#include <gnuradio/top_block.h>
#include <memory>
#include <osmosdr/source.h>
#include <string>
#include <vector>

#include "core/vfo_channel.h"
#include "dsp/correct_iq_cc.h"
#include "dsp/filter/fir_decim.h"
#include "dsp/multichannel_downconverter.h"
#include "dsp/rx_fft.h"

/**
 * @defgroup DSP Digital signal processing library based on GNU Radio
 */

/**
 * @brief Top-level receiver class.
 * @ingroup DSP
 *
 * This class encapsulates the GNU Radio flow graph for the receiver.
 * Front-ends should only control the receiver through the interface provided
 * by this class.
 */
class receiver : public std::enable_shared_from_this<receiver>
{

public:
    using sptr = std::shared_ptr<receiver>;

    static constexpr int DEFAULT_FFT_SIZE = 8192;

    static sptr make(const std::string& input_device = "",
                     const std::string& audio_device = "", int decimation = 1);

    receiver(const std::string& input_device = "",
             const std::string& audio_device = "", int decimation = 1);
    ~receiver();

    void start();
    void stop();
    bool is_running();
    void set_input_device(const std::string& device);
    void set_output_device(const std::string& device);
    std::string get_input_device() const { return input_devstr; }

    std::vector<std::string> get_antennas(void) const;
    std::string get_antenna(void) const;
    void set_antenna(const std::string& antenna);

    double set_input_rate(double rate);
    double get_input_rate(void) const { return d_input_rate; }

    int set_input_decim(int decim);
    int get_input_decim(void) const { return d_decim; }

    double get_quad_rate(void) const { return d_input_rate / (double)d_decim; }

    double set_analog_bandwidth(double bw);
    double get_analog_bandwidth(void) const;

    void set_iq_swap(bool reversed);
    bool get_iq_swap(void) const;

    void set_dc_cancel(bool enable);
    bool get_dc_cancel(void) const;

    void set_iq_balance(bool enable);
    bool get_iq_balance(void) const;

    double set_rf_freq(double freq_hz);
    double get_rf_freq(void);
    bool get_rf_range(double* start, double* stop, double* step);

    std::vector<std::string> get_gain_names();
    bool get_gain_range(const std::string& name, double* start, double* stop,
                        double* step) const;
    bool set_auto_gain(bool automatic);
    bool get_auto_gain();
    double set_gain(const std::string& name, double value);
    double get_gain(const std::string& name) const;

    double set_freq_corr(double ppm);
    double get_freq_corr() const;
    void set_iq_fft_size(int newsize);
    int iq_fft_size(void) const;
    void set_iq_fft_window(int window_type, bool normalize_energy);
    int get_iq_fft_window() const;
    bool is_iq_fft_window_normalized() const;
    void get_iq_fft_data(float* fftPoints);

    /* I/Q recording and playback */
    bool start_iq_recording(std::string filename);
    bool stop_iq_recording();
    bool is_iq_recording() const { return d_recording_iq; }
    const std::string& get_iq_filename() const { return iq_filename; }
    bool seek_iq_file(long pos);

    vfo_channel::sptr add_vfo_channel();
    void remove_vfo_channel(vfo_channel::sptr);
    const std::vector<vfo_channel::sptr>& get_vfo_channels();

    /* utility functions */
    static std::string escape_filename(std::string filename);

private:
    void connect_all();
    void disconnect_vfo_channels();
    void connect_vfo_channels();

    //! Get a path to a file containing random bytes
    static std::string get_zero_file(void);

private:
    bool d_running;      /*!< Whether receiver is running or not. */
    double d_input_rate; /*!< Input sample rate. */
    double d_decim_rate; /*!< Rate after decimation (input_rate / decim) */
    double d_quad_rate;  /*!< Quadrature rate (after down-conversion) */
    double d_audio_rate; /*!< Audio output rate. */
    int d_decim;         /*!< input decimation. */
    int d_ddc_decim;     /*!< Down-conversion decimation. */
    double d_rf_freq;    /*!< Current RF frequency. */
    bool d_recording_iq; /*!< Whether we are recording I/Q file. */
    bool d_iq_rev;       /*!< Whether I/Q is reversed or not. */
    bool d_dc_cancel;    /*!< Enable automatic DC removal. */
    bool d_iq_balance;   /*!< Enable automatic IQ balance. */

    std::string input_devstr;  /*!< Current input device string. */
    std::string output_devstr; /*!< Current output device string. */
    std::string iq_filename;

    gr::top_block_sptr tb; /*!< The GNU Radio top block. */

    osmosdr::source::sptr src;     /*!< Real time I/Q source. */
    fir_decim_cc_sptr input_decim; /*!< Input decimator. */

    dc_corr_cc_sptr dc_corr; /*!< DC corrector block. */
    iq_swap_cc_sptr iq_swap; /*!< I/Q swapping block. */

    rx_fft_c_sptr iq_fft; /*!< Baseband FFT block. */

    multichannel_downconverter_cc::sptr
        ddc; /*!< Digital down-converter for demod chain. */
    gr::blocks::file_sink::sptr iq_sink; /*!< I/Q file sink. */

    std::vector<vfo_channel::sptr> vfo_channels;
};

#endif // RECEIVER_H
