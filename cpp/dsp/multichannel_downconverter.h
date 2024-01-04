#ifndef MULTICHANNEL_DOWNCONVERTER_H
#define MULTICHANNEL_DOWNCONVERTER_H

#include <gnuradio/blocks/rotator.h>
#include <gnuradio/blocks/rotator_cc.h>
#include <gnuradio/fft/fft.h>
#include <gnuradio/filter/fft_filter.h>
#include <gnuradio/hier_block2.h>
#include <gnuradio/sync_decimator.h>

class multichannel_downconverter_cc : public gr::sync_decimator
{
public:
    using sptr = std::shared_ptr<multichannel_downconverter_cc>;
    static sptr make(int decim, double samp_rate, int num_channels,
                     int nthreads = 1);

    multichannel_downconverter_cc(int decim, double samp_rate, int num_channels,
                                  int nthreads);
    ~multichannel_downconverter_cc();

    void set_decim_and_samp_rate(int decim, double samp_rate);
    void set_offset(double offset, int idx);

    int work(int noutput_items, gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;

private:
    void filter(int noutput_items, gr_vector_const_void_star& input_items,
                gr_vector_void_star& output_items);

private:
    void connect_all();
    void update_proto_taps();
    void update_phase_inc();

    void compute_sizes(int ntaps);
    void build_composite_taps(size_t idx);
    int tailsize() const { return d_ntaps - 1; }

private:
    struct channel_data {
        bool updated = true;
        double offset = 0.0;
        double prev_offset = 0.0;
        volk::vector<gr_complex> xformed_taps;
        std::vector<gr_complex> tail;
        gr::blocks::rotator rotator;
        gr_complex* output;
    };

private:
    int d_num_channels;
    unsigned int d_decim;
    double d_samp_rate;

    std::vector<float> d_proto_taps;
    std::vector<gr_complex> d_composite_taps;

    std::vector<channel_data> d_channels_data;

    // fft filter stuff
    int d_ntaps;
    int d_nsamples;
    int d_fftsize; // fftsize = ntaps + nsamples - 1
    std::unique_ptr<gr::fft::fft_complex_fwd> d_fwdfft; // forward "plan"
    std::unique_ptr<gr::fft::fft_complex_rev> d_invfft; // inverse "plan"
    int d_nthreads; // number of FFTW threads to use
};

#endif // MULTICHANNEL_DOWNCONVERTER_H
