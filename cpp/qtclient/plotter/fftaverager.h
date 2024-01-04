#ifndef FFT_AVERAGER_H
#define FFT_AVERAGER_H

#include <QObject>

#include <vector>

class FftAverager : public QObject
{
    Q_OBJECT

public:
    enum PlotScale {
        PLOT_SCALE_DBFS = 0,
        PLOT_SCALE_DBV = 1,
        PLOT_SCALE_DBMW50 = 2
    };

public:
    FftAverager(QObject* parent = nullptr);

public Q_SLOTS:
    void step(const float* fftData, int fftSize);
    void setFftRate(int rate);
    void setFftAvg(float avg);
    void setPlotScale(int scale, bool perHz);
    void setCenterFreq(float);
    void setSampleRate(float);
    void reset();

    const std::vector<float>& iirData();
    const std::vector<float>& data();

private:
    PlotScale plotScale;
    bool plotPerHz;

    std::vector<float> fftData;
    std::vector<float> iirFftData;
    float alpha;
    int fftRate;

    float centerFreq;
    float sampleRate;
};

#endif // FFT_AVERAGER_H
