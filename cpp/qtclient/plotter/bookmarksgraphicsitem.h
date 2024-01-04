#ifndef BOOKMARKS_GRAPHICS_ITEM
#define BOOKMARKS_GRAPHICS_ITEM

#include <QGraphicsObject>

class BookmarkInfo;

class BookmarksGraphicsItem : public QGraphicsObject
{
    Q_OBJECT

public:
    BookmarksGraphicsItem(QGraphicsItem* parent = nullptr);
    ~BookmarksGraphicsItem();

    void setBookmarks(QList<BookmarkInfo>);
    void clearBookmarks();
    void updateBoundingRect();

Q_SIGNALS:
    void bookmarkClicked(const BookmarkInfo& bookmark);

protected:
    bool contains(const QPointF& point) const override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = 0) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    void updateGeometry();

private:
    QList<BookmarkInfo> bookmarks;
    QList<QPair<QRectF, qint64>> collisionRects;

    QRectF cachedBoundingRect;
    QPainterPath cachedShape;
};

#endif // BOOKMARKS_GRAPHICS_ITEM
