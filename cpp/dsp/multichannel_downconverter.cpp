#include <gnuradio/filter/firdes.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/math.h>
#include <gnuradio/sync_decimator.h>

#include "dsp/multichannel_downconverter.h"

#define LPF_CUTOFF 120e3

multichannel_downconverter_cc::sptr
multichannel_downconverter_cc::make(int decim, double samp_rate,
                                    int num_channels, int nthreads)
{
    return gnuradio::make_block_sptr<multichannel_downconverter_cc>( decim, samp_rate, num_channels, nthreads);
}

multichannel_downconverter_cc::multichannel_downconverter_cc(int decimation,
                                                             double samp_rate,
                                                             int num_channels,
                                                             int nthreads) :
    gr::sync_decimator(
        "multichannel_downconverter_cc",
        gr::io_signature::make(1, 1, sizeof(gr_complex)),
        gr::io_signature::make(0, num_channels, sizeof(gr_complex)),
        decimation),
    d_num_channels(num_channels),
    d_decim(decimation),
    d_fftsize(0),
    d_nthreads(nthreads)
{
    if (d_num_channels > 0)
        d_channels_data.resize(d_num_channels);

    set_decim_and_samp_rate(decimation, samp_rate);
}

void multichannel_downconverter_cc::set_decim_and_samp_rate(int decim,
                                                            double samp_rate)
{
    d_samp_rate = samp_rate;
    d_decim = decim;

    set_decimation(decim);

    if (d_decim > 1) {
        // init d_proto_taps
        double out_rate = d_samp_rate / decimation();
        d_proto_taps = gr::filter::firdes::low_pass(
            1.0, d_samp_rate, LPF_CUTOFF, out_rate - 2 * LPF_CUTOFF);

        // init taps for each channel
        compute_sizes(d_proto_taps.size());
    }

    if (d_num_channels > 0) {
        for (int i = 0; i < d_num_channels; i++) {
            build_composite_taps(i);
            d_channels_data[i].updated = false;
        }
    }
}

void multichannel_downconverter_cc::build_composite_taps(size_t idx)
{
    auto& channel_data = d_channels_data[idx];
    float fwT0 = 2 * GR_M_PI * channel_data.offset / d_samp_rate;

    if (d_decim > 1) {
        // initialize taps
        d_composite_taps.resize(d_proto_taps.size());

        for (size_t i = 0; i < d_proto_taps.size(); i++) {
            d_composite_taps[i] =
                d_proto_taps[i] * exp(gr_complex(0, i * fwT0));
        }

        // initialize tail
        std::vector<gr_complex>& tail = channel_data.tail;
        tail.resize(tailsize());
        std::fill(tail.begin(), tail.end(), 0);

        gr_complex* in = d_fwdfft->get_inbuf();
        gr_complex* out = d_fwdfft->get_outbuf();

        float scale = 1.0 / d_fftsize;

        // Compute forward xform of taps.
        // Copy taps into first ntaps slots, then pad with zeros
        for (int i = 0; i < d_ntaps; i++)
            in[i] = d_composite_taps[i] * scale;

        for (int i = d_ntaps; i < d_fftsize; i++)
            in[i] = 0;

        d_fwdfft->execute(); // do the xform

        // now copy output to d_xformed_taps
        for (int i = 0; i < d_fftsize; i++) {
            channel_data.xformed_taps[i] = out[i];
        }
    }

    gr::blocks::rotator& rot = channel_data.rotator;

    // In order to avoid phase jumps during a retune, adjust the phase
    // of the rotator. Phase delay of a symmetric, odd length FIR is (N-1)/2.
    // Scale phase delay by delta omega to get the difference in phase response
    // caused by retuning. Subtract from the current rotator phase.

    gr_complex phase = rot.phase();
    phase /= std::abs(phase);
    float delta_freq = channel_data.offset - channel_data.prev_offset;
    float delta_omega = 2.0 * GR_M_PI * delta_freq / d_samp_rate;
    float delta_phase = -delta_omega * (d_proto_taps.size() - 1) / 2.0;
    phase *= exp(gr_complex(0, delta_phase));
    rot.set_phase(phase);
    channel_data.prev_offset = channel_data.offset;

    rot.set_phase_incr(exp(gr_complex(0, -fwT0 * decimation())));
}

void multichannel_downconverter_cc::compute_sizes(int ntaps)
{
    d_ntaps = ntaps;
    d_fftsize = (int)(2 * pow(2.0, ceil(log(double(ntaps)) / log(2.0))));
    d_nsamples = d_fftsize - d_ntaps + 1;

    d_fwdfft =
        std::make_unique<gr::fft::fft_complex_fwd>(d_fftsize, d_nthreads);
    d_invfft =
        std::make_unique<gr::fft::fft_complex_rev>(d_fftsize, d_nthreads);

    set_output_multiple(d_nsamples);
}

void multichannel_downconverter_cc::set_offset(double offset, int idx)
{
    if (d_num_channels == -1 && idx >= (int)d_channels_data.size()) {
        d_channels_data.resize(idx + 1);
        for (auto& channel_data : d_channels_data)
            channel_data.xformed_taps.resize(d_fftsize);
    }

    d_channels_data[idx].offset = offset;
    d_channels_data[idx].updated = true;
}

int multichannel_downconverter_cc::work(int noutput_items,
                                        gr_vector_const_void_star& input_items,
                                        gr_vector_void_star& output_items)
{
    if (output_items.size() != d_channels_data.size()) {
        d_channels_data.resize(output_items.size());
        for (auto& channel_data : d_channels_data)
            channel_data.xformed_taps.resize(d_fftsize);
    }

    for (size_t idx = 0; idx < output_items.size(); idx++) {
        if (d_channels_data[idx].updated) {
            build_composite_taps(idx);
            d_channels_data[idx].updated = false;
        }
    }

    // first we perform band pass filter if decimation > 1
    if (d_decim > 1) {
        filter(noutput_items, input_items, output_items);
    }

    // then we rotate
    for (size_t idx = 0; idx < output_items.size(); idx++) {
        gr_complex* out = (gr_complex*)output_items[idx];
        gr_complex* in = (d_decim > 1) ? out : (gr_complex*)input_items[0];

        d_channels_data[idx].rotator.rotateN(out, in, noutput_items);
    }

    return noutput_items;
}

void multichannel_downconverter_cc::filter(
    int noutput_items, gr_vector_const_void_star& input_items,
    gr_vector_void_star& output_items)
{
    gr_complex* input = (gr_complex*)input_items[0];
    for (size_t i = 0; i < output_items.size(); i++)
        d_channels_data[i].output = (gr_complex*)output_items[i];

    int ninput_items = noutput_items * decimation();

    int dec_ctr = 0;
    for (int i = 0; i < ninput_items; i += d_nsamples) {
        memcpy(d_fwdfft->get_inbuf(), &input[i],
               d_nsamples * sizeof(gr_complex));

        for (int j = d_nsamples; j < d_fftsize; j++)
            d_fwdfft->get_inbuf()[j] = 0;

        d_fwdfft->execute(); // compute fwd xform

        int old_dec_ctr = dec_ctr;
        for (size_t idx = 0; idx < output_items.size(); idx++) {
            auto& channel_data = d_channels_data[idx];

            gr_complex*& output = channel_data.output;

            gr_complex* a = d_fwdfft->get_outbuf();
            gr_complex* b = channel_data.xformed_taps.data();
            gr_complex* c = d_invfft->get_inbuf();

            volk_32fc_x2_multiply_32fc_a(c, a, b, d_fftsize);

            d_invfft->execute(); // compute inv xform

            // add in the overlapping tail
            for (int j = 0; j < tailsize(); j++)
                d_invfft->get_outbuf()[j] += channel_data.tail[j];

            // copy nsamples to output
            int j = old_dec_ctr;
            for (; j < d_nsamples; j += decimation()) {
                *output++ = d_invfft->get_outbuf()[j];
            }
            dec_ctr = (j - d_nsamples);

            // stash the tail
            if (!channel_data.tail.empty()) {
                memcpy(channel_data.tail.data(),
                       d_invfft->get_outbuf() + d_nsamples,
                       tailsize() * sizeof(gr_complex));
            }
        }
    }
}

multichannel_downconverter_cc::~multichannel_downconverter_cc() {}
