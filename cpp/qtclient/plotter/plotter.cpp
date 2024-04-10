#include <algorithm>

#include "bookmarksgraphicsitem.h"
#include "fftgraphicsitem.h"
#include "gridgraphicsitem.h"
#include "plotter.h"
#include "plottergraphicsview.h"
#include "vfographicsitem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStackedLayout>

PanZoomEventFilter::PanZoomEventFilter(PlotterGraphicsView* view) :
    PlotterEventFilter{view}, action{None}, zoomOnMouse{true}
{
}

bool PanZoomEventFilter::inXDragZone(const QPoint& p)
{
    return QRect{QPoint{GridGraphicsItem::MARGIN_WIDTH,
                        view->height() - GridGraphicsItem::MARGIN_HEIGHT},
                 QPoint{view->width(), view->height()}}
        .contains(p);
}
bool PanZoomEventFilter::inYDragZone(const QPoint& p)
{
    return QRect{QPoint{0, 0},
                 QPoint{GridGraphicsItem::MARGIN_WIDTH,
                        view->height() - GridGraphicsItem::MARGIN_HEIGHT}}
        .contains(p);
}

bool PanZoomEventFilter::mousePressEvent(QMouseEvent* event)
{
    if (action != None)
        return true;

    if (event->button() == Qt::LeftButton) {
        if (inXDragZone(event->pos())) {
            action = XDrag;
            view->setCursor(Qt::ClosedHandCursor);
            initialPos = event->pos();
            return true;
        } else if (inYDragZone(event->pos())) {
            action = YDrag;
            view->setCursor(Qt::ClosedHandCursor);
            initialPos = event->pos();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

PlotterEventFilter::PlotterEventFilter(PlotterGraphicsView* view_) :
    QObject{view_}, view{view_}
{
}

void PlotterEventFilter::setView(PlotterGraphicsView* view_)
{
    view = view_;
    setParent(view_);
}

bool PlotterEventFilter::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        return this->mouseMoveEvent(mouseEvent);
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        return this->mousePressEvent(mouseEvent);
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        return this->mouseReleaseEvent(mouseEvent);
    } else if (event->type() == QEvent::Wheel) {
        QWheelEvent* mouseEvent = static_cast<QWheelEvent*>(event);
        return this->mouseWheelEvent(mouseEvent);
    } else {
        return QObject::eventFilter(object, event);
    }
}

bool PanZoomEventFilter::mouseReleaseEvent(QMouseEvent* event)
{
    if (action != None && event->button() == Qt::LeftButton) {
        view->unsetCursor();
        action = None;
        return true;
    }
    return false;
}

bool PanZoomEventFilter::mouseMoveEvent(QMouseEvent* event)
{
    if (action != None) {
        QPoint pos = event->pos();
        QLineF mappedLine{view->mapToScene(pos), view->mapToScene(initialPos)};
        if (action == XDrag)
            translateViewport(mappedLine.dx(), 0);
        else if (action == YDrag)
            translateViewport(0, mappedLine.dy());
        initialPos = pos;
        return true;
    } else {
        return inXDragZone(event->pos()) || inYDragZone(event->pos());
    }
}

bool PanZoomEventFilter::mouseWheelEvent(QWheelEvent* event)
{
    const QRectF X_DRAG_ZONE{
        QPoint{GridGraphicsItem::MARGIN_WIDTH,
               view->height() - GridGraphicsItem::MARGIN_HEIGHT},
        QPoint{view->width(), view->height()}};

    const QRectF Y_DRAG_ZONE{
        QPoint{0, 0}, QPoint{GridGraphicsItem::MARGIN_WIDTH,
                             view->height() - GridGraphicsItem::MARGIN_HEIGHT}};

    const QPointF mousePosition = event->position();

    if (action == None && event->angleDelta().y() != 0) {
        if (X_DRAG_ZONE.contains(mousePosition)) {
            auto viewport = view->getSceneViewport();
            float factor = event->angleDelta().y() > 0 ? -0.08 : 0.08;
            float dWidth = factor * viewport.width();

            if (zoomOnMouse) {
                float pos_x =
                    view->mapToScene(
                            QPoint{static_cast<int>(mousePosition.x()),
                                   static_cast<int>(mousePosition.y())})
                        .x();

                viewport.setLeft(viewport.left() -
                                 dWidth * (pos_x - viewport.left()) /
                                     view->getSceneViewport().width());
                viewport.setRight(viewport.right() +
                                  dWidth * (viewport.right() - pos_x) /
                                      view->getSceneViewport().width());
            } else {
                viewport.setLeft(viewport.left() - 0.5 * dWidth);
                viewport.setRight(viewport.right() + 0.5 * dWidth);
            }

            setViewport(viewport);
            return true;

        } else if (Y_DRAG_ZONE.contains(mousePosition)) {
            auto viewport = view->getSceneViewport();
            float factor = event->angleDelta().y() > 0 ? -0.08 : 0.08;

            float pos_y =
                view->mapToScene(QPoint{static_cast<int>(mousePosition.x()),
                                        static_cast<int>(mousePosition.y())})
                    .y();

            float dHeight = factor * viewport.height();

            viewport.setTop(viewport.top() - dHeight *
                                                 (pos_y - viewport.top()) /
                                                 viewport.height());
            viewport.setBottom(viewport.bottom() +
                               dHeight * (viewport.bottom() - pos_y) /
                                   viewport.height());

            setViewport(viewport);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void PanZoomEventFilter::setViewport(const QRectF& viewport)
{
    view->setViewport(viewport);
}

void PanZoomEventFilter::translateViewport(qreal dx, qreal dy)
{
    view->translateViewport(dx, dy);
}

class PlotterFallbackHandler : public QGraphicsItem
{
public:
    PlotterFallbackHandler(Plotter* plotter_) :
        QGraphicsItem(), plotter{plotter_}, state{None}
    {
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*,
               QWidget*) override
    {
        if (state == LeftButtonDragging) {
            QPen pen;
            pen.setStyle(Qt::PenStyle::DotLine);
            pen.setCosmetic(true);
            pen.setColor(QColorConstants::White);
            painter->setPen(pen);
            painter->drawRect(QRectF(initialPos, currentPos));
        }
    }

    QRectF boundingRect() const override
    {
        // FIXME: YUCK
        return QRectF(QPointF(-1e50, -1e50), QPointF(1e50, 1e50));
    }
    bool contains(const QPointF&) const override { return true; }

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (event->button() == Qt::RightButton) {
            if (state == None) {
                Q_EMIT plotter->gridMouseRightButtonClicked(event->scenePos(),
                                                            event->screenPos());
            } else {
                state = None;
                initialPos = {};
                initialPosInPixels = {};
                currentPos = {};
                plotter->repaint();
            }
        } else if (event->button() == Qt::LeftButton) {
            initialPos = event->scenePos();
            initialPosInPixels = event->screenPos();

            currentPos = {};
            state = LeftButtonClicked;
        }
    }

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton) {
            Q_EMIT plotter->gridMouseLeftButtonDoubleClicked(
                event->scenePos(), event->screenPos());
        }
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (state == LeftButtonClicked) {
            state = LeftButtonDragging;
        }

        if (state == LeftButtonDragging) {
            currentPos = event->scenePos();
            plotter->repaint();
        }
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (state == LeftButtonDragging) {
            currentPos = event->scenePos();

            static constexpr int MIN_ZOOM_PIXELS_WIDTH = 10;

            int width =
                std::abs(event->screenPos().x() - initialPosInPixels.x());

            if (width >= MIN_ZOOM_PIXELS_WIDTH) {
                if (initialPos.x() < currentPos.x())
                    plotter->setFreqSpan(initialPos.x(), currentPos.x());
                else
                    plotter->setFreqSpan(currentPos.x(), initialPos.x());
            }

            state = None;
            initialPos = {};
            currentPos = {};
            plotter->repaint();
        }
    }

private:
    enum State { LeftButtonClicked, LeftButtonDragging, None };

private:
    Plotter* plotter;
    State state;
    QPoint initialPosInPixels;
    QPointF initialPos;
    QPointF currentPos;
};

static constexpr qreal FGI_ZVALUE = 3;
static constexpr qreal FFT_ZVALUE = 4;
static constexpr qreal GRID_ZVALUE = 6;
static constexpr qreal BOOKMARKS_ZVALUE = 6;
static constexpr qreal MIN_ZVALUE = -1000;

Plotter::Plotter(QWidget* parent) :
    QWidget{parent},
    centerFreq{0},
    sampleRate{0},
    running{false},
    bookmarksEnabled{false},
    bandplanEnabled{false}
{
    scene = new QGraphicsScene(this);
    view = new PlotterGraphicsView(scene, this);
    fft = new FFTGraphicsItem();
    grid = new GridGraphicsItem();
    bookmarks = new BookmarksGraphicsItem();

    auto fallbackHandler = new PlotterFallbackHandler(this);

    fft->setZValue(FFT_ZVALUE);
    grid->setZValue(GRID_ZVALUE);
    bookmarks->setZValue(BOOKMARKS_ZVALUE);
    fallbackHandler->setZValue(MIN_ZVALUE);

    scene->addItem(fft);
    scene->addItem(grid);
    scene->addItem(fallbackHandler);

    layout = new QStackedLayout{this};
    layout->addWidget(view);

    addEventFilter(new PanZoomEventFilter());

    connect(view, &PlotterGraphicsView::viewportChanged, this,
            &Plotter::viewportChanged);

    connect(view, &PlotterGraphicsView::zoomLevelChanged, this,
            &Plotter::zoomLevelChanged);

    connect(view, &PlotterGraphicsView::viewportChanged, this,
            &Plotter::onViewportChanged);

    centerFreq = view->getSceneViewport().center().x();
}

void Plotter::setViewport(const QRectF& viewport)
{
    view->setViewport(viewport);
}

void Plotter::setFreqSpan(float freqBegin, float freqEnd)
{
    view->setFreqSpan(freqBegin, freqEnd);
}

void Plotter::translateViewport(qreal dx, qreal dy)
{
    view->translateViewport(dx, dy);
}

void Plotter::setDbSpan(float dbBegin, float dbEnd)
{
    view->setDbSpan(dbBegin, dbEnd);
}

void Plotter::setMaxViewport(const QRectF& viewport)
{
    view->setMaxViewport(viewport);
}

QRectF Plotter::viewport() { return view->getSceneViewport(); }

void Plotter::setNewFftData(const float* fftDataPtr, int fftSize)
{
    double rbw = (double)sampleRate / fftSize;
    fft->setNewFftData(fftDataPtr, fftSize, rbw, freqFrom());
}

VFOGraphicsItem* Plotter::addVFO()
{
    auto vfo = new VFOGraphicsItem();
    vfo->setCenterFreq(centerFreq);

    vfo->setZValue(FGI_ZVALUE);
    scene->addItem(vfo);

    vfo->updateBoundingRect();

    vfos.push_back(vfo);
    return vfos.back();
}

void Plotter::removeVFO(VFOGraphicsItem* vfo)
{
    scene->removeItem(vfo);
    auto it = std::find(vfos.begin(), vfos.end(), vfo);
    vfos.erase(it);

    vfo->deleteLater();
}

void Plotter::zoomOnXAxis(float level)
{
    QRectF sceneViewport = view->getSceneViewport();
    qreal newWidth = sampleRate / level;
    qreal right = sceneViewport.center().x() + newWidth * 0.5;
    qreal left = sceneViewport.center().x() - newWidth * 0.5;

    sceneViewport.setLeft(left);
    sceneViewport.setRight(right);

    setViewport(sceneViewport);
}

void Plotter::moveToCenterFreq()
{
    QRectF sceneViewport = view->getSceneViewport();
    sceneViewport.moveCenter(QPointF(centerFreq, sceneViewport.center().y()));

    setViewport(sceneViewport);
}

void Plotter::setCenterFreq(qint64 centerFreq_)
{
    // FIXME: should probably use some sort of fequals
    if (centerFreq == centerFreq_)
        return;

    auto oldCenterFreq = centerFreq;
    centerFreq = centerFreq_;

    QRectF curMaxViewport = view->getMaxViewport();

    setMaxViewport(QRectF{QPointF{(qreal)freqFrom(), curMaxViewport.top()},
                          QPointF{(qreal)freqTo(), curMaxViewport.bottom()}});

    float dx = centerFreq - oldCenterFreq;
    view->translateViewport(dx, 0);

    for (VFOGraphicsItem* vfo : vfos) {
        vfo->setCenterFreq(centerFreq);
    }
}

void Plotter::setSampleRate(qint64 sampleRate_)
{
    if (sampleRate == sampleRate_)
        return;

    sampleRate = sampleRate_;
    QRectF curMaxViewport = view->getMaxViewport();

    setMaxViewport(QRectF{QPointF{(qreal)freqFrom(), curMaxViewport.top()},
                          QPointF{(qreal)freqTo(), curMaxViewport.bottom()}});
}

void Plotter::setMinFreqSpan(float minBw) { view->setMinFreqSpan(minBw); }

void Plotter::setMaxFreqSpan(float maxBw) { view->setMaxFreqSpan(maxBw); }

void Plotter::enableFftFill(bool val) { fft->setFilled(val); }
void Plotter::setFftFillColor(const QColor& val) { fft->setFillColor(val); }
void Plotter::setFftColor(const QColor& val) { fft->setFftColor(val); }

void Plotter::addEventFilter(PlotterEventFilter* filter)
{
    filter->setView(view);
    view->viewport()->installEventFilter(filter);
    eventFilters.push_back(std::move(filter));
}

void Plotter::clearEventFilters()
{
    for (PlotterEventFilter* filter : eventFilters) {
        view->viewport()->removeEventFilter(filter);
        delete filter;
    }
    eventFilters.clear();
}

void Plotter::onViewportChanged(const QRectF&)
{
    updateItemsBoundingRects();
    fft->clearMaxMinHold();
}

void Plotter::resizeEvent(QResizeEvent* /* event */)
{
    fft->clearMaxMinHold();
    updateItemsBoundingRects();
}

void Plotter::updateItemsBoundingRects()
{
    for (auto& vfo : vfos) {
        vfo->updateBoundingRect();
    }

    grid->updateBoundingRect();
    bookmarks->updateBoundingRect();
}

void Plotter::setGridColor(const QColor& color) { grid->setGridColor(color); }
QColor Plotter::gridColor() const { return grid->getGridColor(); }
QColor Plotter::labelsColor() const { return grid->getLabelsColor(); }
void Plotter::setLabelsColor(const QColor& color)
{
    grid->setLabelsColor(color);
}

QGraphicsScene* Plotter::getScene() { return scene; }

float Plotter::currentXZoomLevel()
{
    return sampleRate / view->getSceneViewport().width();
}

void Plotter::enableMaxHold(bool enabled) { fft->enableMaxHold(enabled); }
void Plotter::enableMinHold(bool enabled) { fft->enableMinHold(enabled); }

void Plotter::setRunningState(bool value)
{
    if (value && !running)
        fft->clearMaxMinHold();

    running = value;
}

void Plotter::resetHorizontalZoom()
{
    QRectF viewport = view->getSceneViewport();
    viewport.setLeft(centerFreq - sampleRate / 2.0);
    viewport.setRight(centerFreq + sampleRate / 2.0);

    setViewport(viewport);
}

void Plotter::enableBookmarks(bool enabled)
{
    if (enabled == bookmarksEnabled)
        return;

    bookmarksEnabled = enabled;

    if (enabled) {
        scene->addItem(bookmarks);
        refreshBookmarks();
    } else {
        scene->removeItem(bookmarks);
    }
}

void Plotter::enableBandplans(bool enabled)
{
    if (enabled == bandplanEnabled)
        return;

    // TODO
}

void Plotter::refreshBookmarks()
{
    if (!bookmarksEnabled)
        return;

    // QList<BookmarkInfo> items =
    //     Bookmarks::Get().getBookmarksInRange(freqFrom(), freqTo());
    // bookmarks->setBookmarks(items);
}

qint64 Plotter::freqFrom() { return centerFreq - sampleRate / 2.0; }
qint64 Plotter::freqTo() { return centerFreq + sampleRate / 2.0; }

Plotter::~Plotter()
{
    // not really necessary since we set the parent of event filters to "view",
    // and when the view is deleted, it will delete its children
    for (auto& eventFilter : eventFilters) {
        delete eventFilter;
    }
    eventFilters.clear();

    // not really necessary since the scene will be automatically deleted, and
    // it will delete all associated qgraphicsitems
    for (auto& vfo : vfos) {
        delete vfo;
    }
    vfos.clear();

    delete fft;
    delete grid;

    // but this is necessary if bookmarksEnabled is false, since it will not be
    // in the scene
    delete bookmarks;

    // I think order matters here: the scene has to be deleted after the items
    delete scene;
}
