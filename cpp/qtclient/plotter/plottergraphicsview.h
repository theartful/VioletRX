#ifndef PLOTTER_GRAPHICS_VIEW
#define PLOTTER_GRAPHICS_VIEW

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QRect>

class QResizeEvent;
class QGraphicsScene;
class QPaintEvent;
class QWheelEvent;
class QKeyEvent;
class QPainter;

class PlotterGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    PlotterGraphicsView(QGraphicsScene* scene, QWidget* parent = nullptr);
    void translateViewport(qreal dx, qreal dy);
    void setViewport(const QRectF& v, QSize widgetSize);
    void setViewport(const QRectF& v);
    void setFreqSpan(float freqBegin, float freqEnd);
    void setDbSpan(float dbBegin, float dbEnd);
    void setMaxViewport(const QRectF& v);
    QRectF getMaxViewport();
    QRectF getSceneViewport();

public Q_SLOTS:
    void setMinFreqSpan(double);
    void setMaxFreqSpan(double);

Q_SIGNALS:
    void viewportChanged(const QRectF&);
    void maxViewportChanged(const QRectF&);
    void zoomLevelChanged(float level);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void resetTransform();

private:
    QRectF sceneViewport;
    QRectF maxViewport;
    QSizeF minViewportSize;
    QSizeF maxViewportSize;

    float oldWidth;
};

#endif // PLOTTER_GRAPHICS_VIEW
