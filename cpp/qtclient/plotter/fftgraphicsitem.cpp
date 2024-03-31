#include "fftgraphicsitem.h"

#include <QGraphicsView>
#include <QPainter>
#include <QScrollBar>
#include <QVarLengthArray>

#include <algorithm>

FFTGraphicsItem::FFTGraphicsItem(QGraphicsItem* parent) :
    QGraphicsItem{parent},
    fftData{nullptr},
    fftSize{0},
    rbw{0},
    freqBegin{0},
    freqEnd{0},
    isFilled{false},
    fillColor{255, 255, 255, 50},
    fftColor{QColorConstants::White},
    isMaxHold{false},
    isMinHold{false},
    minHoldColor{QColor::fromRgba(0x50FFFFFF)},
    maxHoldColor{QColor::fromRgba(0x50FFFFFF)}
{
}

void FFTGraphicsItem::setNewFftData(const float* fftData_, int fftSize_,
                                    float rbw_, float freqBegin_)
{
    if (!fftData_)
        return;

    if (freqBegin != freqBegin_ || rbw != rbw_ || fftSize != fftSize_) {
        prepareGeometryChange();

        rbw = rbw_;
        fftSize = fftSize_;

        freqBegin = freqBegin_;
        freqEnd = freqBegin + rbw * fftSize;
    }

    fftData = fftData_;

    draw();
    update();
}

void FFTGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*,
                            QWidget*)
{
    if (pixmap_scene_viewport != viewportRect() && fftData) {
        draw();
    }

    painter->drawPixmap(pixmap_scene_viewport, pixmap, pixmap_viewport);
}

QRectF FFTGraphicsItem::viewportRect() const
{
    QList<QGraphicsView*> views = scene()->views();
    if (views.size() == 0)
        return {};
    // assumes the first view is the right one
    QGraphicsView* view = views.first();
    return view->mapToScene(QRect{0, 0, view->width(), view->height()})
        .boundingRect();
}

void FFTGraphicsItem::draw()
{
    static constexpr int BIN_SIZE_IN_PIXELS = 1;

    QGraphicsView* view = getView();

    if (pixmap.width() < view->size().width() ||
        pixmap.height() < view->size().height()) {
        pixmap = QPixmap{view->size()};
    }

    pixmap.fill(QColorConstants::Transparent);

    QPainter painter{&pixmap};
    painter.translate(0, view->height());
    painter.scale(1, -1);

    float pixelLen =
        QLineF{view->mapToScene(QPoint{0, 0}), view->mapToScene(QPoint{1, 0})}
            .length();
    float binSize = BIN_SIZE_IN_PIXELS * pixelLen;

    QRectF viewport = viewportRect();
    pixmap_scene_viewport = viewport;

    pixmap_viewport = QRectF(0, 0, view->width(), view->height());

    auto get_bin = [&](float freq) {
        return int((freq - freqBegin) / binSize + 0.5);
    };
    auto bin_to_x = [&](int bin) { return bin * binSize + freqBegin; };

    int begin_index = qMax(0, int((viewport.left() - freqBegin) / rbw));
    int end_index =
        qMin(fftSize - 1, int((viewport.right() - freqBegin) / rbw) + 1);

    QPen pen;
    pen.setColor(fftColor);
    pen.setCosmetic(true);
    painter.setPen(pen);

    constexpr float MIN_DB = -200;

    int bin = get_bin(freqBegin + begin_index * rbw);
    float freq = bin_to_x(bin);
    float prev_freq = freq - (freq - bin_to_x(bin - 1)) / 2.0;
    float next_freq = freq + (bin_to_x(bin + 1) - freq) / 2.0;

    QPointF curPoint{freq, MIN_DB};

    lineBuf.clear();
    for (int i = begin_index; i <= end_index; i++) {
        int newBin = get_bin(freqBegin + i * rbw);
        freq = bin_to_x(bin);

        // if we're in the same bin
        if (bin == newBin) {
            curPoint.setY(qMax(curPoint.y(), (qreal)fftData[i]));
        } else {
            QPointF p1{prev_freq, curPoint.y()};
            QPointF p2{next_freq, curPoint.y()};

            // we draw in pixel coordinates
            QPoint p1FromScene = view->mapFromScene(p1);
            QPoint p2FromScene = view->mapFromScene(p2);

            lineBuf.append(p1FromScene);
            lineBuf.append(p2FromScene);

            // if (isFilled) {
            //     QRect rect{p1FromScene,
            //                QSize{p2FromScene.x() - p1FromScene.x(), 1000}};
            //     painter.fillRect(rect, fillColor);
            // }

            prev_freq = next_freq;
            next_freq = freq + (bin_to_x(bin + 1) - freq) / 2.0;
            curPoint = {freq, fftData[i]};
        }

        bin = newBin;
    }
    // last point
    QPointF p1{prev_freq, curPoint.y()};
    QPointF p2{next_freq, curPoint.y()};

    QPoint p1FromScene = view->mapFromScene(p1);
    QPoint p2FromScene = view->mapFromScene(p2);

    lineBuf.append(p1FromScene);
    lineBuf.append(p2FromScene);

    // if (isFilled) {
    //     QRect rect{p1FromScene, QSize{p2FromScene.x() - p1FromScene.x(),
    //     1000}}; painter.fillRect(rect, fillColor);
    // }

    if (isMinHold) {
        if (minHold.size() != lineBuf.size()) {
            minHold = lineBuf;
        } else {
            for (qsizetype i = 0; i < lineBuf.size(); i++) {
                // higher y means lower power
                minHold[i].setY(qMax(minHold[i].y(), lineBuf[i].y()));
            }
        }
    }

    if (isMaxHold) {
        if (maxHold.size() != lineBuf.size()) {
            maxHold = lineBuf;
        } else {
            for (qsizetype i = 0; i < lineBuf.size(); i++) {
                // lower y means higher power
                maxHold[i].setY(qMin(maxHold[i].y(), lineBuf[i].y()));
            }
        }
    }

    if (isFilled) {
        lineBuf.append(QPoint(lineBuf.last().x(), view->height()));
        lineBuf.append(QPoint(lineBuf.first().x(), view->height()));

        painter.setBrush(fillColor);
        painter.drawPolygon(lineBuf.data(), lineBuf.size());
    } else {
        painter.drawPolyline(lineBuf.data(), lineBuf.size());
    }

    pen.setColor(minHoldColor);
    painter.setPen(pen);
    painter.drawPolyline(minHold.data(), minHold.size());

    pen.setColor(maxHoldColor);
    painter.setPen(pen);
    painter.drawPolyline(maxHold.data(), maxHold.size());
}

QGraphicsView* FFTGraphicsItem::getView()
{
    QList<QGraphicsView*> views = scene()->views();
    if (views.size() == 0)
        return nullptr;
    else
        return views.first();
}

QRectF FFTGraphicsItem::boundingRect() const
{
    static constexpr float MIN_DB = -1000;
    static constexpr float MAX_DB = 1000;

    return QRectF{QPointF{freqBegin, MIN_DB}, QPointF{freqEnd, MAX_DB}};
}

void FFTGraphicsItem::setFilled(bool val) { isFilled = val; }
void FFTGraphicsItem::setFillColor(const QColor& val) { fillColor = val; }
void FFTGraphicsItem::setFftColor(const QColor& val)
{
    fftColor = val;
    minHoldColor = val;
    minHoldColor.setAlpha(80);
    maxHoldColor = val;
    maxHoldColor.setAlpha(80);
}

void FFTGraphicsItem::enableMaxHold(bool enabled)
{
    isMaxHold = enabled;
    if (!isMaxHold) {
        maxHold.clear();
    }
}
void FFTGraphicsItem::enableMinHold(bool enabled)
{
    isMinHold = enabled;
    if (!isMinHold) {
        minHold.clear();
    }
}
void FFTGraphicsItem::clearMaxMinHold()
{
    maxHold.clear();
    minHold.clear();
}

FFTGraphicsItem::~FFTGraphicsItem() {}
