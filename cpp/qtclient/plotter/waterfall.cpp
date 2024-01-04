#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>

#include "waterfall.h"
#include "waterfall_colormaps.h"

#include <QColor>
#include <QPaintEvent>
#include <QPainter>
#include <QDateTime>

Waterfall::Waterfall(QWidget* parent) :
    QWidget{parent},
    img{},
    lineY{0},
    spanMs{0},
    microPerLine{0},
    lastLineTime{0},
    remMicroSpentOnLine{0},
    nextFftIsFirstInLine(true),
    colormap{getColormap("gqrx")}
{
}

void Waterfall::paintEvent(QPaintEvent*)
{
    QPainter painter{this};
    painter.translate(0, height());
    painter.scale(1, -1);

    int lineYImg = int((float(lineY) / img.height()) * height() + 0.5);

    painter.drawImage(
        QRect{QPoint{0, height() - lineYImg}, QSize{width(), lineYImg}}, img,
        QRect{QPoint{0, 0}, QSize{img.width(), lineY}});

    painter.drawImage(
        QRect{QPoint{0, 0}, QSize{width(), height() - lineYImg}}, img,
        QRect{QPoint{0, lineY}, QSize{img.width(), img.height() - lineY}});
}

static inline quint64 nowInMicro()
{
    using namespace std::chrono;
    // should I use high resolution clock?
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch())
        .count();
}

void Waterfall::setNewFftData(const float* fftData, int fftSize)
{
    if (microPerLine == 0) {
        draw(fftData, fftSize, viewport);
        return;
    }

    if (fftSize != (int)accumulatedFftData.size()) {
        resetCurrentLine();
        accumulatedFftData.resize(fftSize);
    }

    if (nextFftIsFirstInLine) {
        nextFftIsFirstInLine = false;
        std::copy_n(fftData, fftSize, accumulatedFftData.begin());
    } else {
        for (int i = 0; i < fftSize; i++) {
            accumulatedFftData[i] = qMax(accumulatedFftData[i], fftData[i]);
        }
    }

    if (lastLineTime == 0) {
        lastLineTime = nowInMicro();
    }

    quint64 now = nowInMicro();

    quint64 microSpentOnLine = remMicroSpentOnLine + (now - lastLineTime);

    if (microSpentOnLine > microPerLine) {
        // we repeat the same line if we spent too much time so that we maintain
        // the requested span
        //
        // this repeating can be made to be more efficient by caching the line,
        // and then repeating it instead of recalculating it every time, but it
        // doesn't really matter
        while (microSpentOnLine >= microPerLine) {
            draw(accumulatedFftData.data(), fftSize, viewport);

            microSpentOnLine -= microPerLine;
        }

        remMicroSpentOnLine = microSpentOnLine;
        lastLineTime = now;
        nextFftIsFirstInLine = true;
    }
}

void Waterfall::draw(const float* fftData, int fftSize, const QRectF& viewport)
{
    if (img.size().isEmpty())
        return;

    float freqBegin = centerFreq - sampleRate / 2.0;
    float rbw = sampleRate / fftSize;

    float pixelPerFreq = img.width() / float(viewport.width());

    int begin_index = qMax(0, int((viewport.left() - freqBegin) / rbw));
    int end_index =
        qMin(fftSize - 1, int((viewport.right() - freqBegin) / rbw));

    auto get_pixel = [&](float freq) -> int {
        return qMax(0, int((freq - viewport.left()) * pixelPerFreq));
    };

    int curX = get_pixel(freqBegin + begin_index * rbw);
    float curVal = std::numeric_limits<float>::lowest();

    int pixelPerFftSample = static_cast<int>(std::ceil(pixelPerFreq * rbw));

    uchar* line = img.scanLine(lineY);
    std::fill_n(line, img.bytesPerLine(), 0);
    for (int i = begin_index; i <= end_index; i++) {
        int x = get_pixel(freqBegin + i * rbw);
        if (x == curX) {
            curVal = qMax(curVal, fftData[i]);
        } else {
            int colorIdx = qBound(
                0, int(((curVal - viewport.top()) / viewport.height()) * 256),
                255);

            int maxX = qMin(int(curX + pixelPerFftSample), img.width() - 1);
            for (int j = curX; j <= maxX; j++)
                line[j] = colorIdx;

            curVal = fftData[i];
            curX = x;
        }
    }

    lineY = (lineY + 1) % img.height();
    update();
}

void Waterfall::resizeEvent(QResizeEvent* event)
{
    if (img.isNull()) {
        img = QImage{event->size(), QImage::Format_Indexed8};
        for (int i = 0; i < 256; i++)
            img.setColor(i, colormap[i]);
        img.fill(0);
    } else {
        if (!event->size().isEmpty()) {
            lineY = int((float(lineY) / img.height()) * event->size().height() +
                        0.5) %
                    event->size().height();
            img = img.scaled(event->size());
        }
    }

    updateMicroPerLine(event->size().height());

    Q_EMIT sizeChanged(event->size());
}

void Waterfall::updateMicroPerLine(int height)
{
    if (height > 0)
        microPerLine = spanMs * 1000 / height;
    else
        microPerLine = 0;
}

void Waterfall::setSampleRate(float sampleRate_) { sampleRate = sampleRate_; }
void Waterfall::setCenterFreq(float centerFreq_) { centerFreq = centerFreq_; }

void Waterfall::setColormap(const Colormap& colormap_)
{
    colormap = colormap_;

    for (int i = 0; i < 256; i++)
        img.setColor(i, colormap[i]);
}

void Waterfall::setColormapFromName(const QString& name)
{
    setColormap(getColormap(name));
}

void Waterfall::setTimeSpan(quint64 spanMs_)
{
    spanMs = spanMs_;
    updateMicroPerLine(height());

    resetCurrentLine();
}

void Waterfall::resetCurrentLine()
{
    lastLineTime = 0;
    remMicroSpentOnLine = 0;
    nextFftIsFirstInLine = true;
}

quint64 Waterfall::getTimeResolution() const { return microPerLine / 1000; }

void Waterfall::setViewport(const QRectF& viewport_) { viewport = viewport_; }
void Waterfall::setFreqRange(float min, float max)
{
    viewport.setLeft(min);
    viewport.setRight(max);
}
void Waterfall::setPowerRange(float min, float max)
{
    viewport.setTop(min);
    viewport.setBottom(max);
}
