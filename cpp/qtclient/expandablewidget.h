#ifndef EXPANDABLEWIDGET_H
#define EXPANDABLEWIDGET_H

#include <QFrame>

class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QLineEdit;
class QSpacerItem;

class ExpandableWidget : public QFrame
{
    Q_OBJECT

public:
    explicit ExpandableWidget(QWidget* widget = nullptr,
                              QWidget* parent = nullptr);
    ~ExpandableWidget();

    static void setExpandFunctionality(QToolButton* expandButton,
                                       QWidget* widget, QObject* parent);

    void setTitle(QString);
    void addToolButton(QToolButton*);

Q_SIGNALS:
    void expanded(bool);

private:
    void onExpandButtonToggled(bool);
    void setupUi();

private:
    QWidget* widget;

    QVBoxLayout* verticalLayout;
    QFrame* frame;
    QHBoxLayout* horizontalLayout;
    QToolButton* expandButton;
    QLineEdit* titleLineEdit;
    QSpacerItem* spacer;
};

#endif // EXPANDABLEWIDGET_H
