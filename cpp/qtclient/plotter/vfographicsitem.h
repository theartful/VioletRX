#ifndef FILTER_GRAPHICS_ITEM
#define FILTER_GRAPHICS_ITEM

#include <QGraphicsObject>

class QGraphicsView;

class VFOGraphicsItem : public QGraphicsObject
{
    Q_OBJECT

public:
    VFOGraphicsItem(QGraphicsItem* parent = nullptr);
    ~VFOGraphicsItem();

    void setBackgroundColor(QColor color);
    void setCenterColor(QColor color);

    void setCenterFreq(qint64);
    void setOffset(qint64);
    void setHiLowCutFrequencies(qint64, qint64);
    void getHiLowCutFrequencies(qint64*, qint64*);

    qint64 getOffset() const { return offset; }
    qint64 getCenterFreq() const { return centerFreq; }
    qint64 getBandwidth() const { return hi - lo; }

    void updateBoundingRect();
    void setDemodRanges(qint64 loMin, qint64 loMax, qint64 hiMin, qint64 hiMax,
                        bool symmetric);
    void setActive(bool);
    bool isActive();
    void grabFocus();

Q_SIGNALS:
    void offsetRequestChange(qint64);
    void loHiRequestChange(qint64, qint64);
    void mousePressed(QGraphicsSceneMouseEvent*);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = 0) override;
    QRectF boundingRect() const override;
    QRectF actualBoundingRect() const;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    bool contains(const QPointF& point) const override;

    bool inLeftResizeZone(const QPointF&, const QGraphicsView*) const;
    bool inRightResizeZone(const QPointF&, const QGraphicsView*) const;
    bool inResizeZone(const QPointF&, const QGraphicsView*) const;
    bool inDragZone(const QPointF&, const QGraphicsView*) const;

protected:
    enum ActionType { Dragging, LeftResizing, RightResizing, NoAction };

protected:
    qint64 centerFreq;
    qint64 offset;
    qint64 lo;
    qint64 hi;

    // ranges
    qint64 loMin;
    qint64 loMax;
    qint64 hiMin;
    qint64 hiMax;

    bool symmetric;
    bool active;

    ActionType action;

    QColor backgroundColor;
    QColor centerColor;

    QRectF cachedBoundingRect;

    bool manuallyGrabbed;
};

#endif // FILTER_GRAPHICS_ITEM
