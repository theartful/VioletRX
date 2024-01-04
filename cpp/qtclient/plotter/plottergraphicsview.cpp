#include "plottergraphicsview.h"

#include <QEvent>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QRectF>
#include <QResizeEvent>
#include <QScrollBar>
#include <QWheelEvent>

// PlotterGraphicsView
PlotterGraphicsView::PlotterGraphicsView(QGraphicsScene* scene,
                                         QWidget* parent) :
    QGraphicsView{scene, parent},
    sceneViewport{QPointF{-10e6, -150}, QPointF{10e6, 10}},
    maxViewport{QPointF{0, 15}, QPointF{0, -170}},
    minViewportSize{10e3, 10},
    maxViewportSize{100e8, 200},
    oldWidth{0}
{
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setMouseTracking(true);
    this->setRenderHint(QPainter::Antialiasing);
    this->setContentsMargins(QMargins());
    this->setFrameStyle(QFrame::NoFrame);
    this->setTransformationAnchor(QGraphicsView::NoAnchor);
    this->setResizeAnchor(QGraphicsView::NoAnchor);
    this->setDragMode(QGraphicsView::NoDrag);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setBackgroundBrush(QColor(0xFF1F1D1D));

    this->scene()->setSceneRect(
        QRectF{QPointF{-100e9, -100e9}, QPointF{100e9, 100e9}});

    this->resetTransform();
}

void PlotterGraphicsView::resetTransform()
{
    this->setTransform({1.0, 0.0, 0.0, -1.0, 0.0, 0.0});
}

void PlotterGraphicsView::resizeEvent(QResizeEvent* event)
{
    if (!event->size().isEmpty()) {
        setViewport(sceneViewport, event->size());
    }
}

// prevents the default wheel handler which scrolls the view
void PlotterGraphicsView::wheelEvent(QWheelEvent*) {}

void PlotterGraphicsView::translateViewport(qreal dx, qreal dy)
{
    sceneViewport.translate(dx, dy);
    setViewport(sceneViewport);
}

QRectF PlotterGraphicsView::getSceneViewport() { return sceneViewport; }

void PlotterGraphicsView::setFreqSpan(float freqBegin, float freqEnd)
{
    QRectF v = sceneViewport;
    v.setLeft(freqBegin);
    v.setRight(freqEnd);
    setViewport(v);
}

void PlotterGraphicsView::setDbSpan(float dbBegin, float dbEnd)
{
    QRectF v = sceneViewport;
    v.setTop(dbBegin);
    v.setBottom(dbEnd);
    setViewport(v);
}

void PlotterGraphicsView::setViewport(const QRectF& v, QSize widgetSize)
{
    sceneViewport = v.normalized();

    auto set_width = [](QRectF& rect, double width) {
        float old_width = rect.width();
        rect.setLeft(rect.left() - 0.5 * (width - old_width));
        rect.setRight(rect.right() + 0.5 * (width - old_width));
    };
    auto set_height = [](QRectF& rect, double height) {
        float old_height = rect.height();
        rect.setTop(rect.top() - 0.5 * (height - old_height));
        rect.setBottom(rect.bottom() + 0.5 * (height - old_height));
    };

    if (sceneViewport.width() < minViewportSize.width())
        set_width(sceneViewport, minViewportSize.width());
    else if (sceneViewport.width() > maxViewportSize.width())
        set_width(sceneViewport, maxViewportSize.width());

    if (sceneViewport.height() < minViewportSize.height())
        set_height(sceneViewport, minViewportSize.height());
    else if (sceneViewport.height() > maxViewportSize.height())
        set_height(sceneViewport, maxViewportSize.height());

    qreal tx = 0;
    qreal ty = 0;
    if (maxViewport.width() > 0) {
        if (sceneViewport.width() > maxViewport.width()) {
            sceneViewport.setLeft(maxViewport.left());
            sceneViewport.setRight(maxViewport.right());
        }
        if (sceneViewport.left() < maxViewport.left())
            tx = maxViewport.left() - sceneViewport.left();
        else if (sceneViewport.right() > maxViewport.right())
            tx = maxViewport.right() - sceneViewport.right();
    }
    if (maxViewport.height() > 0) {
        if (sceneViewport.height() > maxViewport.height()) {
            sceneViewport.setTop(maxViewport.top());
            sceneViewport.setBottom(maxViewport.bottom());
        }

        if (sceneViewport.top() < maxViewport.top())
            ty = maxViewport.top() - sceneViewport.top();
        else if (sceneViewport.bottom() > maxViewport.bottom())
            ty = maxViewport.bottom() - sceneViewport.bottom();
    }
    sceneViewport.translate(tx, ty);

    qreal scaleX = widgetSize.width() / sceneViewport.width();
    qreal scaleY = widgetSize.height() / sceneViewport.height();

    resetTransform();
    scale(scaleX, scaleY);
    centerOn(sceneViewport.center());

    Q_EMIT viewportChanged(sceneViewport);

    if (maxViewport.isValid()) {
        float newWidth = sceneViewport.width();

        float oldZoomLevel = maxViewport.width() / oldWidth;
        float newZoomLevel = maxViewport.width() / newWidth;

        static const float ABS_TOLERANCE = 0.1;

        if (std::abs(newZoomLevel - oldZoomLevel) > ABS_TOLERANCE) {
            Q_EMIT zoomLevelChanged(newZoomLevel);
            oldWidth = newWidth;
        }
    }
}

void PlotterGraphicsView::setViewport(const QRectF& v)
{
    setViewport(v, size());
}

void PlotterGraphicsView::setMaxViewport(const QRectF& v)
{
    maxViewport = v.normalized();
    Q_EMIT maxViewportChanged(maxViewport);
}

QRectF PlotterGraphicsView::getMaxViewport() { return maxViewport; }

void PlotterGraphicsView::setMinFreqSpan(double val)
{
    minViewportSize.setWidth(val);
}

void PlotterGraphicsView::setMaxFreqSpan(double val)
{
    maxViewportSize.setWidth(val);
    Q_EMIT maxViewportChanged(maxViewport);
}
