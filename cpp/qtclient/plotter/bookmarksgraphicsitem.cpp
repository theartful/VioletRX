#include "bookmarksgraphicsitem.h"
#include "plottergraphicsview.h"

#include "qtclient/bookmarks.h"

#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>

#include <algorithm>

BookmarksGraphicsItem::BookmarksGraphicsItem(QGraphicsItem* parent) :
    QGraphicsObject(parent)
{
    // speeds up things, since paint will not be called over and over
    setCacheMode(CacheMode::DeviceCoordinateCache);

    setAcceptHoverEvents(true);
}

BookmarksGraphicsItem::~BookmarksGraphicsItem() {}

void BookmarksGraphicsItem::paint(QPainter* painter,
                                  const QStyleOptionGraphicsItem*, QWidget*)
{
    // this is exactly the same implementation in the original CPlotter
    // forgive me father for I am lazy
    painter->save();

    QTransform transform = painter->transform();
    painter->resetTransform();

    QRect pixelViewport = painter->viewport();

    const QFontMetricsF fm(painter->font());
    const qreal fontHeight = fm.ascent() + 1;
    const qreal slant = 5;
    const qreal levelHeight = fontHeight + 5;
    const qreal nLevels = pixelViewport.height() / (levelHeight + slant);

    auto xFromFreq = [&](qint64 freq) {
        return transform.map(QPointF((qreal)freq, 0)).x();
    };

    QVector<int> tagEnd(nLevels + 1);
    collisionRects.clear();
    cachedShape.clear();
    for (int i = 0; i < bookmarks.size(); i++) {
        auto& bookmark = bookmarks[i];

        qreal x = xFromFreq(bookmark.frequency);
        qreal nameWidth = fm.boundingRect(bookmark.name).width();

        int level = 0;
        while (level < nLevels && tagEnd[level] > x)
            level++;

        if (level >= nLevels) {
            level = 0;
            if (tagEnd[level] > x)
                continue; // no overwrite at level 0
        }

        tagEnd[level] = x + nameWidth + slant - 1;

        const auto levelNHeight = level * levelHeight;
        const auto levelNHeightBottom = levelNHeight + fontHeight;
        const auto levelNHeightBottomSlant = levelNHeightBottom + slant;

        collisionRects.append(
            qMakePair(transform.inverted().mapRect(QRectF(
                          x, levelNHeight, nameWidth + slant, fontHeight)),
                      i));
        cachedShape.addRect(collisionRects.back().first);

        QColor color = QColor(bookmark.GetColor());
        color.setAlpha(100);
        // Vertical line
        painter->setPen(QPen(color, Qt::DashLine));
        painter->drawLine(QPointF(x, levelNHeightBottomSlant),
                          QPointF(x, pixelViewport.height()));

        // Horizontal line
        painter->setPen(QPen(color, Qt::SolidLine));
        painter->drawLine(
            QPointF(x + slant, levelNHeightBottom),
            QPointF(x + nameWidth + slant - 1, levelNHeightBottom));
        // Diagonal line
        painter->drawLine(QPointF(x + 1, levelNHeightBottomSlant - 1),
                          QPointF(x + slant - 1, levelNHeightBottom + 1));

        color.setAlpha(255);
        painter->setPen(QPen(color, 2.0, Qt::SolidLine));
        painter->drawText(x + slant, levelNHeight, nameWidth, fontHeight,
                          Qt::AlignVCenter | Qt::AlignHCenter, bookmark.name);
    }

    painter->restore();
}

QRectF BookmarksGraphicsItem::boundingRect() const
{
    return cachedBoundingRect;
}

QPainterPath BookmarksGraphicsItem::shape() const { return cachedShape; }

void BookmarksGraphicsItem::setBookmarks(QList<BookmarkInfo> bookmarks_)
{
    bookmarks = bookmarks_;
    updateBoundingRect();
    update();
}

void BookmarksGraphicsItem::updateBoundingRect()
{
    if (!scene() || scene()->views().isEmpty())
        return;

    prepareGeometryChange();

    // FIXME: should be more general I guess and not assume that we use
    // PlotterGraphicsView?

    // FIXME: this is not a tight boundingRect at all
    PlotterGraphicsView* view =
        qobject_cast<PlotterGraphicsView*>(scene()->views().first());

    cachedBoundingRect = view->getSceneViewport();
    update();
}

bool BookmarksGraphicsItem::contains(const QPointF& point) const
{
    return std::any_of(
        collisionRects.begin(), collisionRects.end(),
        [&](const auto& rect) { return rect.first.contains(point); });
}

void BookmarksGraphicsItem::clearBookmarks() { bookmarks.clear(); }

void BookmarksGraphicsItem::hoverEnterEvent(
    QGraphicsSceneHoverEvent* /* event */)
{
    QGraphicsView* view = this->scene()->views().first();
    view->setCursor(Qt::PointingHandCursor);
}

void BookmarksGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
    QGraphicsView* view = this->scene()->views().first();
    view->unsetCursor();
}

void BookmarksGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        auto it = std::find_if(
            collisionRects.begin(), collisionRects.end(),
            [&](auto& rect) { return rect.first.contains(event->pos()); });

        if (it == collisionRects.end())
            return;

        Q_EMIT bookmarkClicked(bookmarks[it->second]);
    }
}
