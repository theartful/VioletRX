#ifndef FFT_GRAPHICS_ITEM
#define FFT_GRAPHICS_ITEM

#include <QColor>
#include <QGraphicsItem>
#include <QPixmap>
#include <QRectF>
#include <QVarLengthArray>

class QGraphicsView;

class FFTGraphicsItem : public QGraphicsItem
{
public:
    FFTGraphicsItem(QGraphicsItem* parent = nullptr);
    ~FFTGraphicsItem();

    void setNewFftData(const float* fftData, int fftSize, float rbw,
                       float freqBegin);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = 0) override;
    void setFilled(bool);
    void setFillColor(const QColor&);
    void setFftColor(const QColor&);

    void enableMaxHold(bool enabled);
    void enableMinHold(bool enabled);

    void clearMaxMinHold();

protected:
    void draw();
    QRectF viewportRect() const;
    QGraphicsView* getView();

private:
    const float* fftData;
    int fftSize;

    float rbw;
    float freqBegin;
    float freqEnd;

    bool isFilled;
    QColor fillColor;

    QColor fftColor;
    QPixmap pixmap;
    QRectF pixmap_scene_viewport;
    QRectF pixmap_viewport;

    bool isMaxHold;
    bool isMinHold;

    QColor minHoldColor;
    QColor maxHoldColor;

    static constexpr size_t MAX_POINTS = 16384;
    QVarLengthArray<QPoint, MAX_POINTS> lineBuf;
    QVarLengthArray<QPoint, MAX_POINTS> minHold;
    QVarLengthArray<QPoint, MAX_POINTS> maxHold;
};

#endif // FFT_GRAPHICS_ITEM
