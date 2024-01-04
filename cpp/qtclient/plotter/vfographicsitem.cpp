#include "vfographicsitem.h"

#include <limits>

#include <QColor>
#include <QEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QRect>
#include <qrgba64.h>

static constexpr uint DEFAULT_BACKGROUND_COLOR = 0x4AA0A0A4;
static constexpr uint DEFAULT_CENTER_COLOR = 0xFFFF7171;

static constexpr uint RESIZE_PIXEL_MARGIN = 10;
static constexpr uint CENTER_PIXEL_MARGIN = 10;

VFOGraphicsItem::VFOGraphicsItem(QGraphicsItem* parent) :
    QGraphicsObject{parent},
    centerFreq{0},
    offset{0},
    lo{0},
    hi{0},
    loMin{0},
    loMax{0},
    hiMin{0},
    hiMax{0},
    symmetric{true},
    active{false},
    action{NoAction},
    backgroundColor{QColor::fromRgba(DEFAULT_BACKGROUND_COLOR)},
    centerColor{QColor::fromRgba(DEFAULT_CENTER_COLOR)},
    manuallyGrabbed{false}
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
}

void VFOGraphicsItem::setBackgroundColor(QColor color)
{
    backgroundColor = color;
    update();
}

void VFOGraphicsItem::setCenterColor(QColor color)
{
    centerColor = color;
    update();
}

QRectF VFOGraphicsItem::boundingRect() const { return cachedBoundingRect; }

QRectF VFOGraphicsItem::actualBoundingRect() const
{
    return QRectF(QPointF(centerFreq + offset + lo, -1000),
                  QPointF(centerFreq + offset + hi, 100));
}

void VFOGraphicsItem::updateBoundingRect()
{
    prepareGeometryChange();
    cachedBoundingRect = actualBoundingRect();

    constexpr qreal MIN_PIXEL_WIDTH = 10;

    if (this->scene() && !this->scene()->views().isEmpty()) {
        QGraphicsView* view = this->scene()->views().first();
        qreal freqPerPixel = view->transform().inverted().m11();

        qreal min_width = MIN_PIXEL_WIDTH * freqPerPixel;

        if (cachedBoundingRect.width() < min_width) {
            cachedBoundingRect.setLeft(centerFreq + offset - min_width / 2);
            cachedBoundingRect.setRight(centerFreq + offset + min_width / 2);
        }
    }
}

bool VFOGraphicsItem::contains(const QPointF& point) const
{
    return point.x() >= lo && point.x() <= hi;
}

void VFOGraphicsItem::setCenterFreq(qint64 freq)
{
    centerFreq = freq;

    updateBoundingRect();
    update();
}

void VFOGraphicsItem::setOffset(qint64 o)
{
    offset = o;

    updateBoundingRect();
    update();
}
void VFOGraphicsItem::setHiLowCutFrequencies(qint64 l, qint64 h)
{
    lo = l;
    hi = h;

    updateBoundingRect();
    update();
}

void VFOGraphicsItem::getHiLowCutFrequencies(qint64* l, qint64* h)
{
    *l = lo;
    *h = hi;
}

void VFOGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*,
                            QWidget*)
{
    QRectF rect = actualBoundingRect();
    QColor bg = backgroundColor;
    if (!active)
        bg.setAlphaF(bg.alphaF() * 0.5);

    painter->fillRect(rect, bg);

    QPen pen;
    pen.setColor(centerColor);
    pen.setCosmetic(true);
    painter->setPen(pen);

    painter->drawLine(QLineF(centerFreq + offset, rect.top(),
                             centerFreq + offset, rect.bottom()));
}

void VFOGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QGraphicsView* view = this->scene()->views().first();
        QPointF pos = event->pos();

        if (inDragZone(pos, view)) {
            action = Dragging;
            view->setCursor(Qt::OpenHandCursor);
        } else if (inLeftResizeZone(pos, view)) {
            action = LeftResizing;
            view->setCursor(Qt::SizeHorCursor);
        } else if (inResizeZone(pos, view)) {
            action = RightResizing;
            view->setCursor(Qt::SizeHorCursor);
        }
    }

    Q_EMIT mousePressed(event);
}

void VFOGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        action = NoAction;
        QGraphicsView* view = this->scene()->views().first();
        view->unsetCursor();
    }

    if (manuallyGrabbed) {
        manuallyGrabbed = false;
        ungrabMouse();
    }
}

bool VFOGraphicsItem::inDragZone(const QPointF& pos,
                                 const QGraphicsView* view) const
{
    qint64 centerMargin =
        std::min(CENTER_PIXEL_MARGIN / view->transform().m11(),
                 0.33 * boundingRect().width());

    return (pos.x() > centerFreq + offset - centerMargin &&
            pos.x() < centerFreq + offset + centerMargin);
}

bool VFOGraphicsItem::inLeftResizeZone(const QPointF& pos,
                                       const QGraphicsView* view) const
{
    qint64 resizeMargin =
        std::min(RESIZE_PIXEL_MARGIN / view->transform().m11(),
                 0.16 * boundingRect().width());
    return (pos.x() > boundingRect().left() &&
            pos.x() < boundingRect().left() + resizeMargin);
}

bool VFOGraphicsItem::inRightResizeZone(const QPointF& pos,
                                        const QGraphicsView* view) const
{
    qint64 resizeMargin =
        std::min(RESIZE_PIXEL_MARGIN / view->transform().m11(),
                 0.16 * boundingRect().width());
    return (pos.x() < boundingRect().right() &&
            pos.x() > boundingRect().right() - resizeMargin);
}

bool VFOGraphicsItem::inResizeZone(const QPointF& pos,
                                   const QGraphicsView* view) const
{
    return inLeftResizeZone(pos, view) || inRightResizeZone(pos, view);
}

void VFOGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    auto pos = event->pos();

    switch (action) {
    case Dragging:
        if (event->buttons() | Qt::LeftButton) {
            Q_EMIT offsetRequestChange(pos.x() - centerFreq);
        }
        break;
    case LeftResizing:
        if (event->buttons() | Qt::LeftButton) {
            qint64 newLo =
                qBound(loMin, ((qint64)pos.x() - centerFreq) - offset, loMax);
            if (symmetric) {
                Q_EMIT loHiRequestChange(newLo, -newLo);
            } else {
                Q_EMIT loHiRequestChange(newLo, hi);
            }
        }
        break;
    case RightResizing:
        if (event->buttons() | Qt::LeftButton) {
            qint64 newHi =
                qBound(hiMin, ((qint64)pos.x() - centerFreq) - offset, hiMax);
            if (symmetric) {
                Q_EMIT loHiRequestChange(-newHi, newHi);
            } else {
                Q_EMIT loHiRequestChange(lo, newHi);
            }
        }
        break;
    case NoAction:
        break;
    default:
        throw std::logic_error("VFOGraphicsItem action has invalid value! This "
                               "should never happen!");
    }
}

void VFOGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
    // TODO maybe?
}

void VFOGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
    QGraphicsView* view = this->scene()->views().first();
    action = NoAction;
    view->unsetCursor();
}

void VFOGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    auto pos = event->pos();
    QGraphicsView* view = this->scene()->views().first();

    if (inDragZone(pos, view)) {
        view->setCursor(Qt::OpenHandCursor);
    } else if (inResizeZone(pos, view)) {
        view->setCursor(Qt::SizeHorCursor);
    } else {
        view->unsetCursor();
    }
}

void VFOGraphicsItem::setDemodRanges(qint64 lmin, qint64 lmax, qint64 hmin,
                                     qint64 hmax, bool sym)
{
    loMin = lmin;
    loMax = lmax;
    hiMin = hmin;
    hiMax = hmax;
    symmetric = sym;
}

void VFOGraphicsItem::setActive(bool val)
{
    active = val;
    update();
}
bool VFOGraphicsItem::isActive() { return active; }

void VFOGraphicsItem::grabFocus()
{
    action = Dragging;
    manuallyGrabbed = true;

    QGraphicsView* view = this->scene()->views().first();
    view->setCursor(Qt::OpenHandCursor);

    grabMouse();
}

VFOGraphicsItem::~VFOGraphicsItem() {}
