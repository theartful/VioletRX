#include <cmath>
#include <limits>

#include "fftaverager.h"

FftAverager::FftAverager(QObject* parent) :
    QObject{parent},
    plotScale{PLOT_SCALE_DBFS},
    plotPerHz{false},
    alpha{1.0},
    fftRate{25},
    centerFreq{0},
    sampleRate{0}
{
}

void FftAverager::setFftRate(int rate) { fftRate = rate; }
void FftAverager::setFftAvg(float avg) { alpha = avg; }
void FftAverager::setPlotScale(int scale, bool perHz)
{
    plotScale = (PlotScale)scale;
    plotPerHz = perHz;
}
void FftAverager::setCenterFreq(float freq) { centerFreq = freq; }
void FftAverager::setSampleRate(float rate) { sampleRate = rate; }
void FftAverager::reset()
{
    fftData.clear();
    iirFftData.clear();
}

void FftAverager::step(const float* fftDataPtr, int fftSize)
{
    // Make sure zeros don't get through to log calcs
    const float fmin = std::numeric_limits<float>::min();

    if (fftSize != (int)fftData.size()) {
        // Reallocate and invalidate IIRs
        reset();
        fftData.resize(fftSize);
        iirFftData.resize(fftSize);
    }

    // For dBFS, define full scale as peak (not RMS). A 1.0 FS peak sine wave
    // is 0 dBFS.
    float _pwr_scale = 1.0 / ((float)fftSize * (float)fftSize);

    // For V, convert peak to RMS (/2). 1V peak corresponds to -3.01 dBV (RMS
    // value is 0.707 * peak).
    if (plotScale == PLOT_SCALE_DBV)
        _pwr_scale *= 1.0 / 2.0;

    // For dBm, the scale is interpreted as V. A 1V peak sine corresponds to
    // 10mW, or 10 dBm. The factor of 2 converts Vpeak to Vrms.
    else if (plotScale == PLOT_SCALE_DBMW50)
        _pwr_scale *= 1000.0 / (2.0 * 50.0);

    // For units of /Hz, rescale by 1/RBW. For V, this results in /sqrt(Hz), and
    // is used for noise spectral density.
    if (plotPerHz && plotScale != PLOT_SCALE_DBFS)
        _pwr_scale *= (float)fftSize / (float)sampleRate;

    const float pwr_scale = _pwr_scale;
    for (int i = 0; i < fftSize; ++i)
        fftData[i] = 10 * std::log10(std::max(fftDataPtr[i] * pwr_scale, fmin));

    // Time constant, taking update rate into account. Attack and decay rate of
    // change in dB/sec should not visibly change with FFT rate.
    const double _a = pow((double)fftRate, -1.75 * (1.0 - alpha));

    // Make the slider vs alpha nonlinear
    const double gamma = 0.7;
    const double a = pow(_a, gamma);

    for (int i = 0; i < fftSize; ++i) {
        const double v = fftData[i];
        const double iir = iirFftData[i];
        iirFftData[i] = iir * (1.0 - a) + v * a;
    }
}

const std::vector<float>& FftAverager::data() { return fftData; }
const std::vector<float>& FftAverager::iirData() { return iirFftData; }
