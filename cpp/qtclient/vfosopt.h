#ifndef VFOSOPT_H
#define VFOSOPT_H

// FIXME: what a horrible confusing naming
// "vfoopt" and "vfosopt"

#include <QWidget>

class ReceiverModel;
class VFOChannelModel;
class QVBoxLayout;
class QScrollArea;
class QSettings;
class QToolButton;

class VfosOpt : public QWidget
{
    Q_OBJECT

public:
    VfosOpt(ReceiverModel* rxModel, QWidget* parent = nullptr);

private:
    void onVfoAdded(VFOChannelModel*);

private:
    ReceiverModel* rxModel;

    QWidget* scrollAreaContents;
    QScrollArea* scrollArea;
    QVBoxLayout* verticalLayout;
    QToolButton* addVfoButton;

    int vfoCount;
};

#endif // VFOSOPT_H
