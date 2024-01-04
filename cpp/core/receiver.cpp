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

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include <gnuradio/prefs.h>
#include <gnuradio/top_block.h>
#include <osmosdr/ranges.h>
#include <osmosdr/source.h>

#include "dsp/correct_iq_cc.h"
#include "dsp/filter/fir_decim.h"
#include "dsp/multichannel_downconverter.h"
#include "dsp/rx_fft.h"
#include "receiver.h"

#include <spdlog/spdlog.h>
#include <stdexcept>

static constexpr float TARGET_QUAD_RATE = 280e3;
static constexpr float MAX_NUM_VFO_CHANNELS = -1;

receiver::sptr receiver::make(const std::string& input_device,
                              const std::string& audio_device, int decimation)
{
    return std::make_shared<receiver>(input_device, audio_device, decimation);
}

/**
 * @brief Public constructor.
 * @param input_device Input device specifier.
 * @param audio_device Audio output device specifier,
 *                     e.g. hw:0 when using ALSA or Portaudio.
 */
receiver::receiver(const std::string& input_device,
                   const std::string& audio_device, int decimation) :
    d_running(false),
    d_input_rate(96000.0),
    d_audio_rate(48000),
    d_decim(decimation),
    d_rf_freq(144800000.0),
    d_recording_iq(false),
    d_iq_rev(false),
    d_dc_cancel(false),
    d_iq_balance(false)
{

    tb = gr::make_top_block("gqrx");

    if (input_device.empty()) {
        src = osmosdr::source::make(
            "file=" + escape_filename(get_zero_file()) +
            ",freq=428e6,rate=96000,repeat=true,throttle=true");
    } else {
        input_devstr = input_device;
        src = osmosdr::source::make(input_device);
    }

    // input decimator
    if (d_decim >= 2) {
        try {
            input_decim = make_fir_decim_cc(d_decim);
        } catch (std::range_error& e) {
            spdlog::error("Error creating input decimator {}: {}", d_decim,
                          e.what());
            spdlog::error("Using decimation 1.");
            d_decim = 1;
        }

        d_decim_rate = d_input_rate / (double)d_decim;
    } else {
        d_decim_rate = d_input_rate;
    }

    d_ddc_decim = std::max(1, (int)(d_decim_rate / TARGET_QUAD_RATE));
    d_quad_rate = d_decim_rate / d_ddc_decim;
    ddc = multichannel_downconverter_cc::make(d_ddc_decim, d_decim_rate,
                                              MAX_NUM_VFO_CHANNELS);

    iq_swap = make_iq_swap_cc(false);
    dc_corr = make_dc_corr_cc(d_decim_rate, 1.0);
    iq_fft = make_rx_fft_c(DEFAULT_FFT_SIZE, d_decim_rate,
                           gr::fft::window::WIN_HANN);

    output_devstr = audio_device;

    gr::prefs pref;
    spdlog::debug("Using audio backend: {}",
                  pref.get_string("audio", "audio_module", "N/A"));

    connect_all();
}

receiver::~receiver() { tb->stop(); }

/** Start the receiver. */
void receiver::start()
{
    if (!d_running) {
        tb->start();
        d_running = true;
    }
}

/** Stop the receiver. */
void receiver::stop()
{
    if (d_running) {
        tb->stop();
        tb->wait(); // If the graph is needed to run again, wait() must be
                    // called after stop
        d_running = false;
    }
}

/**
 * @brief Select new input device.
 * @param device
 */
void receiver::set_input_device(const std::string& device)
{
    spdlog::info("Set input device:");
    spdlog::info("  old: {}", input_devstr);
    spdlog::info("  new: {}", device.c_str());

    std::string error = "";

    if (device.empty())
        return;

    input_devstr = device;

    // tb->lock() can hang occasionally
    if (d_running) {
        tb->stop();
        tb->wait();
    }

    if (d_decim >= 2) {
        tb->disconnect(src, 0, input_decim, 0);
        tb->disconnect(input_decim, 0, iq_swap, 0);
    } else {
        tb->disconnect(src, 0, iq_swap, 0);
    }

    src.reset();

    try {
        src = osmosdr::source::make(device);
    } catch (std::exception& x) {
        error = x.what();
        src = osmosdr::source::make(
            "file=" + escape_filename(get_zero_file()) +
            ",freq=428e6,rate=96000,repeat=true,throttle=true");
    }

    if (src->get_sample_rate() != 0)
        set_input_rate(src->get_sample_rate());

    if (d_decim >= 2) {
        tb->connect(src, 0, input_decim, 0);
        tb->connect(input_decim, 0, iq_swap, 0);
    } else {
        tb->connect(src, 0, iq_swap, 0);
    }

    if (d_running)
        tb->start();

    if (error != "") {
        throw std::runtime_error(error);
    }
}

/**
 * @brief Select new audio output device.
 * @param device
 */
void receiver::set_output_device(const std::string& /* device */)
{
    // FIXME
}

/** Get a list of available antenna connectors. */
std::vector<std::string> receiver::get_antennas(void) const
{
    return src->get_antennas();
}

std::string receiver::get_antenna(void) const { return src->get_antenna(); }

/** Select antenna connector. */
void receiver::set_antenna(const std::string& antenna)
{
    if (!antenna.empty()) {
        src->set_antenna(antenna);
    }
}

/**
 * @brief Set new input sample rate.
 * @param rate The desired input rate
 * @return The actual sample rate set or 0 if there was an error with the
 *         device.
 */
double receiver::set_input_rate(double rate)
{
    double current_rate;
    bool rate_has_changed;

    current_rate = src->get_sample_rate();
    rate_has_changed = !(rate == current_rate ||
                         std::abs(rate - current_rate) <
                             std::abs(std::min(rate, current_rate)) *
                                 std::numeric_limits<double>::epsilon());

    tb->lock();
    try {
        d_input_rate = src->set_sample_rate(rate);
    } catch (std::runtime_error& e) {
        d_input_rate = 0;
    }

    if (d_input_rate == 0) {
        // This can be the case when no device is attached and gr-osmosdr
        // puts in a null_source with rate 100 ksps or if the rate has not
        // changed
        if (rate_has_changed) {
            std::cerr << std::endl;
            std::cerr << "Failed to set RX input rate to " << rate << std::endl;
            std::cerr << "Your device may not be working properly."
                      << std::endl;
            std::cerr << std::endl;
        }
        d_input_rate = rate;
    }

    d_decim_rate = d_input_rate / (double)d_decim;
    d_ddc_decim = std::max(1, (int)(d_decim_rate / TARGET_QUAD_RATE));
    d_quad_rate = d_decim_rate / d_ddc_decim;
    dc_corr->set_sample_rate(d_decim_rate);
    ddc->set_decim_and_samp_rate(d_ddc_decim, d_decim_rate);

    for (auto& vfo : vfo_channels)
        vfo->set_quad_rate(d_quad_rate);

    iq_fft->set_quad_rate(d_decim_rate);
    tb->unlock();

    return d_input_rate;
}

/** Set input decimation */
int receiver::set_input_decim(int decim)
{
    if (decim == d_decim)
        return d_decim;

    if (d_running) {
        tb->stop();
        tb->wait();
    }

    if (d_decim >= 2) {
        tb->disconnect(src, 0, input_decim, 0);
        tb->disconnect(input_decim, 0, iq_swap, 0);
    } else {
        tb->disconnect(src, 0, iq_swap, 0);
    }

    input_decim.reset();
    d_decim = decim;
    if (d_decim >= 2) {
        try {
            input_decim = make_fir_decim_cc(d_decim);
        } catch (std::range_error& e) {
            spdlog::error("Error opening creating input decimator {}: {}",
                          d_decim, e.what());
            spdlog::error("Using decimation 1.");
            d_decim = 1;
        }

        d_decim_rate = d_input_rate / (double)d_decim;
    } else {
        d_decim_rate = d_input_rate;
    }

    // update quadrature rate
    d_ddc_decim = std::max(1, (int)(d_decim_rate / TARGET_QUAD_RATE));
    d_quad_rate = d_decim_rate / d_ddc_decim;
    dc_corr->set_sample_rate(d_decim_rate);
    ddc->set_decim_and_samp_rate(d_ddc_decim, d_decim_rate);

    for (auto& vfo : vfo_channels)
        vfo->set_quad_rate(d_quad_rate);

    iq_fft->set_quad_rate(d_decim_rate);

    if (d_decim >= 2) {
        tb->connect(src, 0, input_decim, 0);
        tb->connect(input_decim, 0, iq_swap, 0);
    } else {
        tb->connect(src, 0, iq_swap, 0);
    }

#ifdef CUSTOM_AIRSPY_KERNELS
    if (input_devstr.find("airspy") != std::string::npos)
        src->set_bandwidth(d_decim_rate);
#endif

    if (d_running)
        tb->start();

    return d_decim;
}

/**
 * @brief Set new analog bandwidth.
 * @param bw The new bandwidth.
 * @return The actual bandwidth.
 */
double receiver::set_analog_bandwidth(double bw)
{
    return src->set_bandwidth(bw);
}

/** Get current analog bandwidth. */
double receiver::get_analog_bandwidth(void) const
{
    return src->get_bandwidth();
}

/** Set I/Q reversed. */
void receiver::set_iq_swap(bool reversed)
{
    if (reversed == d_iq_rev)
        return;

    d_iq_rev = reversed;
    iq_swap->set_enabled(d_iq_rev);
}

/**
 * @brief Get current I/Q reversed setting.
 * @retval true I/Q swappign is enabled.
 * @retval false I/Q swapping is disabled.
 */
bool receiver::get_iq_swap(void) const { return d_iq_rev; }

/**
 * @brief Enable/disable automatic DC removal in the I/Q stream.
 * @param enable Whether DC removal should enabled or not.
 */
void receiver::set_dc_cancel(bool enable)
{
    if (enable == d_dc_cancel)
        return;

    d_dc_cancel = enable;

    // until we have a way to switch on/off
    // inside the dc_corr_cc we do a reconf
    for (auto& vfo : vfo_channels)
        vfo->set_demod(vfo->get_demod(), true);
}

/**
 * @brief Get auto DC cancel status.
 * @retval true  Automatic DC removal is enabled.
 * @retval false Automatic DC removal is disabled.
 */
bool receiver::get_dc_cancel(void) const { return d_dc_cancel; }

/**
 * @brief Enable/disable automatic I/Q balance.
 * @param enable Whether automatic I/Q balance should be enabled.
 */
void receiver::set_iq_balance(bool enable)
{
    if (enable == d_iq_balance)
        return;

    d_iq_balance = enable;

    src->set_iq_balance_mode(enable ? 2 : 0);
}

/**
 * @brief Get auto I/Q balance status.
 * @retval true  Automatic I/Q balance is enabled.
 * @retval false Automatic I/Q balance is disabled.
 */
bool receiver::get_iq_balance(void) const { return d_iq_balance; }

/**
 * @brief Set RF frequency.
 * @param freq_hz The desired frequency in Hz.
 * @return false if an error occurs, e.g. the frequency is out of range.
 * @sa get_rf_freq()
 */
double receiver::set_rf_freq(double freq_hz)
{
    src->set_center_freq(freq_hz);
    d_rf_freq = src->get_center_freq();

    return d_rf_freq;
}

/**
 * @brief Get RF frequency.
 * @return The current RF frequency.
 * @sa set_rf_freq()
 */
double receiver::get_rf_freq(void)
{
    d_rf_freq = src->get_center_freq();

    return d_rf_freq;
}

/**
 * @brief Get the RF frequency range of the current input device.
 * @param start The lower limit of the range in Hz.
 * @param stop  The upper limit of the range in Hz.
 * @param step  The frequency step in Hz.
 * @returns true if the range could be retrieved, false if an error has
 * occurred.
 */
bool receiver::get_rf_range(double* start, double* stop, double* step)
{
    osmosdr::freq_range_t range;

    range = src->get_freq_range();

    // currently range is empty for all but E4000
    if (!range.empty()) {
        if (range.start() < range.stop()) {
            *start = range.start();
            *stop = range.stop();
            *step = range.step(); /** FIXME: got 0 for rtl-sdr? **/

            return true;
        }
    }

    return false;
}

/** Get the names of available gain stages. */
std::vector<std::string> receiver::get_gain_names()
{
    return src->get_gain_names();
}

/**
 * @brief Get gain range for a specific stage.
 * @param[in]  name The name of the gain stage.
 * @param[out] start Lower limit for this gain setting.
 * @param[out] stop  Upper limit for this gain setting.
 * @param[out] step  The resolution for this gain setting.
 *
 * This function returns the range for the requested gain stage.
 */
bool receiver::get_gain_range(const std::string& name, double* start,
                              double* stop, double* step) const
{
    osmosdr::gain_range_t range;

    range = src->get_gain_range(name);
    *start = range.start();
    *stop = range.stop();
    *step = range.step();

    return true;
}

double receiver::set_gain(const std::string& name, double value)
{
    return src->set_gain(value, name);
}

double receiver::get_gain(const std::string& name) const
{
    return src->get_gain(name);
}

/**
 * @brief Set RF gain.
 * @param gain_rel The desired relative gain between 0.0 and 1.0 (use -1 for
 *                 AGC where supported).
 * @return false if an error occurs, e.g. the gain is out of valid range.
 */
bool receiver::set_auto_gain(bool automatic)
{
    return src->set_gain_mode(automatic);
}

bool receiver::get_auto_gain() { return src->get_gain_mode(); }

double receiver::set_freq_corr(double ppm) { return src->set_freq_corr(ppm); }
double receiver::get_freq_corr() const { return src->get_freq_corr(); }

/** Set new FFT size. */
void receiver::set_iq_fft_size(int newsize) { iq_fft->set_fft_size(newsize); }

int receiver::iq_fft_size() const { return iq_fft->fft_size(); }

void receiver::set_iq_fft_window(int window_type, bool normalize_energy)
{
    iq_fft->set_window_type(window_type, normalize_energy);
}
int receiver::get_iq_fft_window() const { return iq_fft->get_window_type(); }
bool receiver::is_iq_fft_window_normalized() const
{
    return iq_fft->is_normalized();
}

/** Get latest baseband FFT data. */
void receiver::get_iq_fft_data(float* fftPoints)
{
    iq_fft->get_fft_data(fftPoints);
}

/**
 * @brief Start I/Q data recorder.
 * @param filename The filename where to record.
 */
bool receiver::start_iq_recording(std::string filename)
{
    if (d_recording_iq) {
        spdlog::info("{}: already recording", __func__);
        return false;
    }

    try {
        iq_sink = gr::blocks::file_sink::make(sizeof(gr_complex),
                                              filename.c_str(), true);
    } catch (std::runtime_error& e) {
        spdlog::error("{}: couldn't open I/Q file ({})", __func__, filename);
        return false;
    }

    iq_filename = std::move(filename);

    tb->lock();
    if (d_decim >= 2)
        tb->connect(input_decim, 0, iq_sink, 0);
    else
        tb->connect(src, 0, iq_sink, 0);
    d_recording_iq = true;
    tb->unlock();

    return true;
}

/** Stop I/Q data recorder. */
bool receiver::stop_iq_recording()
{
    if (!d_recording_iq) {
        /* error: we are not recording */
        return false;
    }

    tb->lock();
    iq_sink->close();

    if (d_decim >= 2)
        tb->disconnect(input_decim, 0, iq_sink, 0);
    else
        tb->disconnect(src, 0, iq_sink, 0);

    tb->unlock();
    iq_sink.reset();
    d_recording_iq = false;

    return true;
}

/**
 * @brief Seek to position in IQ file source.
 * @param pos Byte offset from the beginning of the file.
 */
bool receiver::seek_iq_file(long pos)
{
    tb->lock();
    bool status = src->seek(pos, SEEK_SET);
    tb->unlock();

    return status;
}

/** Convenience function to connect all blocks. */
void receiver::connect_all()
{
    gr::basic_block_sptr b;

    // Setup source
    b = src;

    // Pre-processing
    if (d_decim >= 2) {
        tb->connect(b, 0, input_decim, 0);
        b = input_decim;
    }

    if (d_recording_iq) {
        // We record IQ with minimal pre-processing
        tb->connect(b, 0, iq_sink, 0);
    }

    tb->connect(b, 0, iq_swap, 0);
    b = iq_swap;

    if (d_dc_cancel) {
        tb->connect(b, 0, dc_corr, 0);
        b = dc_corr;
    }

    // Visualization
    tb->connect(b, 0, iq_fft, 0);

    // vfo channels
    tb->connect(b, 0, ddc, 0);
    for (auto& vfo : vfo_channels) {
        tb->connect(ddc, vfo->get_ddc_idx(), vfo, 0);
    }
}

std::string receiver::escape_filename(std::string filename)
{
    std::stringstream ss1;
    std::stringstream ss2;

    ss1 << std::quoted(filename, '\'', '\\');
    ss2 << std::quoted(ss1.str(), '\'', '\\');
    return ss2.str();
}

bool receiver::is_running() { return d_running; }

vfo_channel::sptr receiver::add_vfo_channel()
{
    if (MAX_NUM_VFO_CHANNELS > 0 && vfo_channels.size() >= MAX_NUM_VFO_CHANNELS)
        return nullptr;

    vfo_channel::sptr vfo = vfo_channel::make(ddc, vfo_channels.size());

    vfo->set_parent_receiver(weak_from_this());
    vfo->set_quad_rate(d_quad_rate);

    if (d_running) {
        tb->stop();
        tb->wait();
    }

    disconnect_vfo_channels();
    vfo_channels.push_back(vfo);
    connect_vfo_channels();

    if (d_running) {
        tb->start();
    }

    return vfo;
}

void receiver::remove_vfo_channel(vfo_channel::sptr vfo)
{
    auto it = std::find(vfo_channels.begin(), vfo_channels.end(), vfo);
    if (it == vfo_channels.end())
        return;

    tb->lock();

    disconnect_vfo_channels();

    vfo->set_parent_receiver({});
    vfo_channels.erase(it);

    connect_vfo_channels();

    tb->unlock();
}

void receiver::disconnect_vfo_channels()
{
    for (auto& vfo : vfo_channels) {
        tb->disconnect(ddc, vfo->get_ddc_idx(), vfo, 0);
    }
}

void receiver::connect_vfo_channels()
{
    int ddc_idx = 0;
    for (auto& vfo : vfo_channels) {
        ddc->set_offset(vfo->get_filter_offset(), ddc_idx);
        tb->connect(ddc, ddc_idx, vfo, 0);
        vfo->set_ddc_idx(ddc_idx++);
    }
}

const std::vector<vfo_channel::sptr>& receiver::get_vfo_channels()
{
    return vfo_channels;
}

#ifdef _WIN32
#    include "windows.h"
#    include <codecvt>
#endif

std::string receiver::get_zero_file(void)
{
    static std::filesystem::path path;

    if (path.empty()) {
        path = "/dev/zero";
        if (!std::filesystem::exists(path)) {
            FILE* file = nullptr;
#ifdef __linux__
            auto temp_dir = std::filesystem::temp_directory_path();
            std::string temp_path = temp_dir / "violetrx_XXXXXX";
            int fd = mkstemp((char*)temp_path.c_str());
            if (fd == 0) {
                throw std::runtime_error(
                    "Could not generate a temporary file!");
            }
            file = fdopen(fd, "wb+");
            path = temp_path;
#elif _WIN32
            auto temp_dir = std::filesystem::temp_directory_path();
            std::wstring temp_path = temp_dir / "VioletRx_XXXXXX";
            int err =
                _wmktemp_s((wchar_t*)temp_path.c_str(), temp_path.size() + 1);
            if (err != 0) {
                throw std::runtime_error(
                    "Could not generate a temporary file!");
            }

            path = temp_path;
            file = _wfopen(path.c_str(), L"wb+");
#else
            path = std::tmpnam(nullptr);
            file = std::fopen(path.c_str(), "wb+");
#endif

            if (path.empty() || file == nullptr) {
                throw std::runtime_error(
                    "Could not generate a temporary file!");
            }

            int buffer[1024] = {};
            std::fwrite(&buffer, sizeof(buffer), 1, file);
            std::fclose(file);

            struct TempFile {
                ~TempFile()
                {
                    std::filesystem::remove(path);
                    std::cout << "Deleted random file " << path << std::endl;
                }

                std::filesystem::path path;
            };

            static TempFile tempFile{path};
            std::cout << "Created random file " << path << std::endl;
        }
    }

#ifdef _WIN32
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(path);
#else
    return path;
#endif
}
