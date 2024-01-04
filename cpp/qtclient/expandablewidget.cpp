#include "expandablewidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>

ExpandableWidget::ExpandableWidget(QWidget* widget_, QWidget* parent) :
    QFrame(parent), widget(widget_)
{
    setupUi();

    if (widget) {
        verticalLayout->addWidget(widget);
        widget->setVisible(expandButton->isChecked());
    }

    connect(expandButton, &QToolButton::toggled, this,
            &ExpandableWidget::onExpandButtonToggled);
}

void ExpandableWidget::setupUi()
{
    if (this->objectName().isEmpty())
        this->setObjectName("ExpandableWidget");

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());

    this->setSizePolicy(sizePolicy);
    this->setFrameShape(QFrame::StyledPanel);
    this->setFrameShadow(QFrame::Raised);

    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(0);
    verticalLayout->setObjectName("verticalLayout");
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    frame = new QFrame(this);
    frame->setObjectName("frame");
    sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
    frame->setSizePolicy(sizePolicy);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    horizontalLayout = new QHBoxLayout(frame);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setObjectName("horizontalLayout");
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    expandButton = new QToolButton(frame);
    expandButton->setObjectName("expandButton");
    expandButton->setStyleSheet("QToolButton { border: none; }");
    expandButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    expandButton->setArrowType(Qt::ArrowType::RightArrow);
    expandButton->setCheckable(true);
    expandButton->setChecked(false);
    expandButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    horizontalLayout->addWidget(expandButton);

    verticalLayout->addWidget(frame);
}

ExpandableWidget::~ExpandableWidget() {}

void ExpandableWidget::setTitle(QString title) { expandButton->setText(title); }

void ExpandableWidget::onExpandButtonToggled(bool toggled)
{
    if (!widget)
        return;

    expandButton->setArrowType(toggled ? Qt::ArrowType::DownArrow
                                       : Qt::ArrowType::RightArrow);
    widget->setVisible(toggled);

    Q_EMIT expanded(toggled);
}

void ExpandableWidget::setExpandFunctionality(QToolButton* expandButton,
                                              QWidget* widget, QObject* parent)
{
    widget->setVisible(expandButton->isChecked());

    connect(expandButton, &QToolButton::toggled, parent, [=](bool toggled) {
        if (!widget)
            return;

        expandButton->setArrowType(toggled ? Qt::ArrowType::DownArrow
                                           : Qt::ArrowType::RightArrow);
        widget->setVisible(toggled);
    });
}

void ExpandableWidget::addToolButton(QToolButton* button)
{
    horizontalLayout->addWidget(button);
}
