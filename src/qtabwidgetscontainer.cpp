#include "qtabwidgetscontainer.h"

QTabWidgetsContainer::QTabWidgetsContainer(QWidget *parent) :
    QSplitter(parent)
{
    this->setOrientation(Qt::Horizontal);
}

QTabWidgetqq *QTabWidgetsContainer::focusQTabWidgetqq()
{
    QObject *focus = this->focusWidget();

    if(focus == 0) {
        return 0;
    } else {
        // Find the first top level focused widget
        while(focus->parent() != this) {
            focus = focus->parent();
        }

        return static_cast<QTabWidgetqq *>(focus);
    }
}

QTabWidgetqq *QTabWidgetsContainer::QTabWidgetqqAt(int index)
{
    QWidget *widget = this->widget(index);

    if(widget == 0) {
        return 0;
    } else {
        // Find the first top level focused widget
        while(widget->parentWidget() != this) {
            widget = widget->parentWidget();
        }

        return static_cast<QTabWidgetqq *>(widget);
    }
}
