#ifndef NEW_PLOTTER_H
#define NEW_PLOTTER_H

#include <QList>
#include <QObject>
#include <QWidget>

class QGraphicsScene;
class QGraphicsSceneMouseEvent;
class QMouseEvent;
class QResizeEvent;
class QStackedLayout;
class FFTGraphicsItem;
class VFOGraphicsItem;
class PlotterGraphicsView;
class GridGraphicsItem;
class BandplanGraphicsItem;
class BookmarksGraphicsItem;

class PlotterEventFilter : public QObject
{
    Q_OBJECT

public:
    PlotterEventFilter(PlotterGraphicsView* view = nullptr);
    virtual ~PlotterEventFilter() {}
    bool eventFilter(QObject* object, QEvent* event) override;
    void setView(PlotterGraphicsView* view);

protected:
    virtual bool mousePressEvent(QMouseEvent*) { return false; }
    virtual bool mouseReleaseEvent(QMouseEvent*) { return false; }
    virtual bool mouseMoveEvent(QMouseEvent*) { return false; }
    virtual bool mouseWheelEvent(QWheelEvent*) { return false; }

protected:
    PlotterGraphicsView* view;
};

class PanZoomEventFilter : public PlotterEventFilter
{
    Q_OBJECT

public:
    PanZoomEventFilter(PlotterGraphicsView* view = nullptr);
    virtual ~PanZoomEventFilter() override {}

    enum ActionType {
        XDrag,
        YDrag,
        None,
    };

    void setZoomOnMouse(bool val) { zoomOnMouse = val; }

protected:
    bool mousePressEvent(QMouseEvent* event) override;
    bool mouseReleaseEvent(QMouseEvent* event) override;
    bool mouseMoveEvent(QMouseEvent* event) override;
    bool mouseWheelEvent(QWheelEvent* event) override;
    virtual void setViewport(const QRectF& viewport);
    virtual void translateViewport(qreal dx, qreal dy);

    bool inXDragZone(const QPoint&);
    bool inYDragZone(const QPoint&);

private:
    QPoint initialPos;
    ActionType action;
    bool zoomOnMouse;
};

class Plotter : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(
        QColor gridColor READ gridColor WRITE setGridColor DESIGNABLE true)
    Q_PROPERTY(QColor labelsColor READ labelsColor WRITE setLabelsColor
                   DESIGNABLE true)

public:
    enum PlotScale {
        PLOT_SCALE_DBFS = 0,
        PLOT_SCALE_DBV = 1,
        PLOT_SCALE_DBMW50 = 2
    };

public:
    Plotter(QWidget* parent = nullptr);
    ~Plotter();

    QGraphicsScene* getScene();
    QRectF viewport();
    float currentXZoomLevel();

public Q_SLOTS:
    void zoomOnXAxis(float level);
    void resetHorizontalZoom();
    void moveToCenterFreq();
    void setDbSpan(float dbBegin, float dbEnd);
    void setCenterFreq(qint64 centerFreq);
    void setSampleRate(qint64 sampleRate);
    void setNewFftData(const float* fftData, int fftSize);
    void setViewport(const QRectF& viewport);
    void setMaxViewport(const QRectF& viewport);

    const QList<VFOGraphicsItem*>& getVfos() { return vfos; }
    VFOGraphicsItem* addVFO();
    void removeVFO(VFOGraphicsItem*);

    void setFreqSpan(float freqBegin, float freqEnd);
    void translateViewport(qreal dx, qreal dy);
    void setMinFreqSpan(float minBw);
    void setMaxFreqSpan(float maxBw);
    void enableFftFill(bool);
    void setFftColor(const QColor&);
    void setFftFillColor(const QColor&);
    void addEventFilter(PlotterEventFilter*);
    void clearEventFilters();

    void enableMaxHold(bool enabled);
    void enableMinHold(bool enabled);
    void setRunningState(bool);

    void enableBookmarks(bool enabled);
    void enableBandplans(bool enabeld);

    void refreshBookmarks();

Q_SIGNALS:
    void viewportChanged(const QRectF&);
    void zoomLevelChanged(float level);
    void gridMouseRightButtonClicked(QPointF scenePos, QPoint screenPos);
    void gridMouseLeftButtonDoubleClicked(QPointF scenePos, QPoint screenPos);

public:
    // grid params
    void setGridColor(const QColor&);
    void setLabelsColor(const QColor&);

    QColor gridColor() const;
    QColor labelsColor() const;

private:
    void onViewportChanged(const QRectF&);
    void updateItemsBoundingRects();

    void resizeEvent(QResizeEvent* event) override;

    qint64 freqFrom();
    qint64 freqTo();

private:
    QGraphicsScene* scene;
    PlotterGraphicsView* view;
    FFTGraphicsItem* fft;
    GridGraphicsItem* grid;
    // BandplanGraphicsItem* bandplan;
    BookmarksGraphicsItem* bookmarks;

    QStackedLayout* layout;
    QList<PlotterEventFilter*> eventFilters;
    QList<VFOGraphicsItem*> vfos;

    qint64 centerFreq;
    qint64 sampleRate;

    bool running;
    bool bookmarksEnabled;
    bool bandplanEnabled;
};

#endif
