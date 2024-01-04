#include "gridgraphicsitem.h"
#include "plottergraphicsview.h"

#include <cmath>
#include <limits>

#include <QGraphicsView>
#include <QPainter>
#include <QVarLengthArray>
#include <QtGlobal>

GridGraphicsItem::GridGraphicsItem() :
    gridColor{QColor::fromRgba(0xFF444242)},
    labelsColor{QColorConstants::White},
    x_spacing{100},
    y_spacing{50}
{
    // speeds up things, since paint will not be called over and over
    setCacheMode(CacheMode::DeviceCoordinateCache);
}

int GridGraphicsItem::getXPower5() const { return x_power5; }
int GridGraphicsItem::getXPower2() const { return x_power2; }
int GridGraphicsItem::getYPower5() const { return y_power5; }
int GridGraphicsItem::getYPower2() const { return y_power2; }
float GridGraphicsItem::getXUnit() const { return x_unit; }
float GridGraphicsItem::getYUnit() const { return y_unit; }
QColor GridGraphicsItem::getGridColor() const { return gridColor; }
QColor GridGraphicsItem::getLabelsColor() const { return labelsColor; }
int GridGraphicsItem::getXSpacing() const { return x_spacing; }
int GridGraphicsItem::getYSpacing() const { return y_spacing; }

void GridGraphicsItem::setGridColor(const QColor& color) { gridColor = color; }

void GridGraphicsItem::setLabelsColor(const QColor& color)
{
    labelsColor = color;
}

void GridGraphicsItem::setSpacing(int x_spacing_, int y_spacing_)
{
    x_spacing = x_spacing_;
    y_spacing = y_spacing_;
    update();
}

static inline qreal horizontalAdvance(const QFontMetrics& fm,
                                      const QString& text)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    return fm.horizontalAdvance(text);
#else
    return fm.boundingRect(text).width();
#endif
}

void GridGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*,
                             QWidget*)
{
    painter->save();

    QTransform worldTransform = painter->transform();
    float scaleX = worldTransform.m11();
    float scaleY = -worldTransform.m22();

    float gridSceneMaxXLen = x_spacing / scaleX;
    float gridSceneMaxYLen = y_spacing / scaleY;

    // 2^power2 * 5^power5 * 1e6 = gridSceneMaxSize
    x_power5 = std::log(gridSceneMaxXLen / 1e6) / std::log(5);
    float pow5l = std::pow(5, x_power5);
    x_power2 = std::log(gridSceneMaxXLen / (1e6 * pow5l)) / std::log(2);
    float pow2l = std::pow(2, x_power2);

    x_unit = pow2l * pow5l * 1e6;

    y_power5 = std::log(gridSceneMaxYLen) / std::log(5.);
    pow5l = std::pow(5, y_power5);
    y_power2 = std::log(gridSceneMaxYLen / pow5l) / std::log(2.);
    pow2l = std::pow(2, y_power2);

    y_unit = pow2l * pow5l;

    QRect pixelViewport = painter->viewport();
    QRectF sceneViewport = viewportRect(painter);

    const QFont& font = painter->font();
    QFontMetrics fm(font);
    qreal txtHeight = fm.height();

    const double marginX = MARGIN_WIDTH / scaleX;
    const double marginY = MARGIN_HEIGHT / scaleY;

    QVarLengthArray<QLineF, 100> linesX;
    QVarLengthArray<QLineF, 100> linesY;

    qreal left = std::ceil((sceneViewport.left() + marginX) / x_unit) * x_unit;
    qreal top = std::ceil((sceneViewport.top() + marginY) / y_unit) * y_unit;

    for (qreal x = left; x < sceneViewport.right(); x += x_unit)
        linesX.append(QLineF(x, sceneViewport.top() + marginY, x,
                             sceneViewport.bottom()));

    for (qreal y = top; y < sceneViewport.bottom(); y += y_unit)
        linesY.append(QLineF(sceneViewport.left() + marginX, y,
                             sceneViewport.right(), y));

    // set up the painter
    QPen gridPen;
    gridPen.setColor(gridColor);
    gridPen.setCosmetic(true);
    gridPen.setStyle(Qt::PenStyle::DotLine);
    painter->setPen(gridPen);

    // draw the grid
    painter->drawLines(linesX.data(), linesX.size());
    painter->drawLines(linesY.data(), linesY.size());

    QPen labelsPen{labelsColor};
    labelsPen.setCosmetic(true);
    painter->setPen(labelsPen);

    painter->resetTransform();

    for (qreal x = left; x < sceneViewport.right(); x += x_unit) {
        QString txt = QString{"%1"}.arg(x / 1e6);
        qreal txtWidth = horizontalAdvance(fm, txt);

        // we assume that there is no shear or rotation, only scale and
        // translation
        int x_pixel = x * scaleX + worldTransform.m31();

        painter->drawText(x_pixel - txtWidth / 2,
                          pixelViewport.bottom() - MARGIN_HEIGHT, txtWidth,
                          MARGIN_HEIGHT, Qt::AlignCenter, txt);
    }

    for (qreal y = top; y < sceneViewport.bottom(); y += y_unit) {
        QString txt = QString{"%1"}.arg(y);

        // we assume that there is no shear or rotation, only scale and
        // translation
        int y_pixel = -y * scaleY + worldTransform.m32();

        painter->drawText(0, y_pixel - txtHeight / 2, MARGIN_WIDTH - 5,
                          txtHeight, Qt::AlignRight | Qt::AlignVCenter, txt);
    }

    painter->restore();
}

QRectF GridGraphicsItem::boundingRect() const { return cachedBoundingRect; }

void GridGraphicsItem::updateBoundingRect()
{
    if (!scene() || scene()->views().isEmpty())
        return;

    prepareGeometryChange();
    // FIXME: should be more general I guess and not assume that we use
    // PlotterGraphicsView?
    PlotterGraphicsView* view =
        qobject_cast<PlotterGraphicsView*>(scene()->views().first());

    cachedBoundingRect = view->getSceneViewport();
    update();
}

QRectF GridGraphicsItem::viewportRect(QPainter* painter) const
{
    QRectF pixelViewport = painter->viewport();
    return painter->transform().inverted().mapRect(pixelViewport);
}

bool GridGraphicsItem::contains(const QPointF&) const { return true; }
