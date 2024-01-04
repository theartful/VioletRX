#include "vfosopt.h"
#include "vfoopt.h"
#include "expandablewidget.h"
#include "receiver_model.h"

#include <QColorDialog>
#include <QDialog>
#include <QInputDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

VfosOpt::VfosOpt(ReceiverModel* rxModel_, QWidget* parent) :
    QWidget(parent), rxModel(rxModel_), vfoCount(0)
{
    QVBoxLayout* topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 4);

    scrollArea = new QScrollArea(this);
    scrollArea->setVerticalScrollBarPolicy(
        Qt::ScrollBarPolicy::ScrollBarAlwaysOn);

    scrollAreaContents = new QWidget();
    verticalLayout = new QVBoxLayout(scrollAreaContents);
    verticalLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(scrollAreaContents);
    scrollArea->setWidgetResizable(true);
    topLayout->addWidget(scrollArea);

    addVfoButton = new QToolButton(this);
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/icons/add.svg"), QSize(),
                 QIcon::Normal, QIcon::Off);
    addVfoButton->setIcon(icon);
    addVfoButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    addVfoButton->setText("Add VFO");
    topLayout->addWidget(addVfoButton);

    connect(addVfoButton, &QToolButton::clicked, rxModel,
            &ReceiverModel::addVFOChannel);
    connect(rxModel, &ReceiverModel::vfoAdded, this, &VfosOpt::onVfoAdded);

    for (VFOChannelModel* vfo : rxModel->vfoChannels())
        onVfoAdded(vfo);
}

void VfosOpt::onVfoAdded(VFOChannelModel* vfo)
{
    VfoOpt* vfoOpt = new VfoOpt(vfo, scrollAreaContents);
    ExpandableWidget* widget = new ExpandableWidget(vfoOpt, scrollAreaContents);

    QToolButton* exitButton = new QToolButton(this);
    exitButton->setStyleSheet("QToolButton { border: none; }");
    exitButton->setObjectName("exitButton");
    QIcon icon1;
    icon1.addFile(QString::fromUtf8(":/icons/icons/close.svg"), QSize(),
                  QIcon::Normal, QIcon::Off);
    exitButton->setIcon(icon1);
    exitButton->setToolTip("Disconnect VFO channel");

    QToolButton* renameButton = new QToolButton(this);
    renameButton->setStyleSheet("QToolButton { border: none; }");
    renameButton->setObjectName("renameButton");
    QIcon icon2;
    icon1.addFile(QString::fromUtf8(":/icons/icons/rename.svg"), QSize(),
                  QIcon::Normal, QIcon::Off);
    renameButton->setIcon(icon1);
    renameButton->setToolTip("Rename VFO channel");

    QToolButton* focusButton = new QToolButton(this);
    focusButton->setObjectName("focusButton");
    QIcon icon3;
    icon1.addFile(QString::fromUtf8(":/icons/icons/focused.svg"), QSize(),
                  QIcon::Normal, QIcon::On);
    icon1.addFile(QString::fromUtf8(":/icons/icons/unfocused.svg"), QSize(),
                  QIcon::Normal, QIcon::Off);
    focusButton->setIcon(icon1);
    focusButton->setToolTip("Focus on this VFO channel"); // FIXME: YUCK
    focusButton->setStyleSheet("QToolButton { border: none; }");
    focusButton->setCheckable(true);
    focusButton->setChecked(vfo->isActive());

    QToolButton* colorButton = new QToolButton(this);
    QPixmap pixmap(15, 15);
    pixmap.fill(vfo->color());
    QIcon icon4(pixmap);
    colorButton->setIcon(icon4);
    colorButton->setToolTip("Change VFO color");
    colorButton->setStyleSheet("QToolButton { border: none; }");

    widget->addToolButton(renameButton);
    widget->addToolButton(colorButton);
    widget->addToolButton(focusButton);
    widget->addToolButton(exitButton);

    if (vfo->name().isEmpty()) {
        vfo->setName(QString("VFO Channel #%1").arg(vfoCount++));
    }
    widget->setTitle(vfo->name());

    connect(focusButton, &QToolButton::toggled, this,
            [vfo](bool checked) { vfo->setActive(checked); });

    connect(vfo, &VFOChannelModel::activeStatusChanged, this,
            [focusButton](bool status) {
                focusButton->blockSignals(true);
                focusButton->setChecked(status);
                focusButton->blockSignals(false);
            });

    connect(exitButton, &QToolButton::clicked, this,
            [vfo] { vfo->parentModel()->removeVFOChannel(vfo); });

    connect(renameButton, &QToolButton::clicked, this, [vfo, this] {
        bool ok = false;
        QString name =
            QInputDialog::getText(this, "Enter VFO name", "Name",
                                  QLineEdit::Normal, vfo->name(), &ok);

        if (ok) {
            name = name.trimmed();
            if (!name.isEmpty()) {
                vfo->setName(name);
            }
        }
    });

    connect(colorButton, &QToolButton::clicked, this, [this, vfo]() {
        QColorDialog* colorDialog = new QColorDialog(this);

        connect(colorDialog, &QColorDialog::finished, this,
                [color = vfo->color(), vfo, colorDialog](int result) {
                    if (result == QDialog::Rejected) {
                        // restore color
                        vfo->setColor(color);
                    }

                    colorDialog->deleteLater();
                });

        connect(colorDialog, &QColorDialog::currentColorChanged, this,
                [vfo](QColor color) { vfo->setColor(color); });

        colorDialog->setCurrentColor(vfo->color());
        colorDialog->open();
    });

    connect(vfo, &VFOChannelModel::removed, this, [this, widget]() {
        verticalLayout->removeWidget(widget);
        widget->deleteLater();
    });

    connect(vfo, &VFOChannelModel::nameChanged, this,
            [widget](const QString& name) { widget->setTitle(name); });

    connect(vfo, &VFOChannelModel::colorChanged, this,
            [colorButton](const QColor& color) {
                QPixmap pixmap(15, 15);
                pixmap.fill(color);
                QIcon icon4(pixmap);
                colorButton->setIcon(icon4);
            });

    verticalLayout->addWidget(widget);
}
