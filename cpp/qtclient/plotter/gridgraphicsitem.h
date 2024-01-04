#ifndef GRID_GRAPHICS_ITEM
#define GRID_GRAPHICS_ITEM

#include <QColor>
#include <QGraphicsItem>

class QWidget;
class QPainter;
class QStyleOptionGraphicsItem;

class GridGraphicsItem : public QGraphicsItem
{
public:
    static constexpr int MARGIN_WIDTH = 35;
    static constexpr int MARGIN_HEIGHT = 30;

    GridGraphicsItem();
    int getXPower5() const;
    int getXPower2() const;
    int getYPower5() const;
    int getYPower2() const;
    float getXUnit() const;
    float getYUnit() const;

    QColor getGridColor() const;
    QColor getLabelsColor() const;
    int getXSpacing() const;
    int getYSpacing() const;

    void setGridColor(const QColor&);
    void setLabelsColor(const QColor&);
    void setSpacing(int, int);

    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
    QRectF boundingRect() const override;
    void updateBoundingRect();

protected:
    QRectF viewportRect(QPainter*) const;
    bool contains(const QPointF& point) const override;

private:
    QColor gridColor;
    QColor labelsColor;

    int x_spacing;
    int y_spacing;
    int x_power5;
    int x_power2;
    int y_power5;
    int y_power2;
    float x_unit;
    float y_unit;

    QRectF cachedBoundingRect;
};

#endif // GRID_GRAPHICS_ITEM
