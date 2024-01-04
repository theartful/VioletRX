#ifndef WATERFALL_H
#define WATERFALL_H

#include <QImage>
#include <QWidget>

class QPaintEvent;
class QResizeEvent;

class Waterfall : public QWidget
{
    Q_OBJECT

public:
    using Colormap = std::array<QRgb, 256>;

    Waterfall(QWidget* parent = nullptr);
    void setNewFftData(const float* fftData, int fftSize);

    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    quint64 getTimeResolution() const;
    void setTimeSpan(quint64 spanMs);
    void resetCurrentLine();

public Q_SLOTS:
    void setSampleRate(float sampleRate);
    void setCenterFreq(float centerFreq);
    void setColormap(const Colormap&);
    void setColormapFromName(const QString&);
    void setViewport(const QRectF& viewport);
    void setFreqRange(float min, float max);
    void setPowerRange(float min, float max);

Q_SIGNALS:
    void sizeChanged(const QSize&);

private:
    void draw(const float* fftData, int fftSize, const QRectF& viewport);
    void updateMicroPerLine(int height);

private:
    std::vector<float> accumulatedFftData;

    float centerFreq;
    float sampleRate;

    QImage img;
    int lineY;

    quint64 spanMs;
    quint64 microPerLine;        // microseconds to spend on a single line
    quint64 lastLineTime;        // the time of last time we drew a line
    quint64 remMicroSpentOnLine; // the remainder of excess micro seconds spent
                                 // on the last line (always < microPerLine)
    bool nextFftIsFirstInLine;

    Colormap colormap;

    QRectF viewport;
};

#endif // WATERFALL_H
