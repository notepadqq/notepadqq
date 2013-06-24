#include "qtabwidgetscontainer.h"

QTabWidgetsContainer::QTabWidgetsContainer(QWidget *parent) :
    QSplitter(parent)
{
    this->setOrientation(Qt::Horizontal);
}

QTabWidgetqq *QTabWidgetsContainer::focusQTabWidgetqq(bool fallback)
{
    QObject *focus = this->focusWidget();

    if(focus == 0) {
        if(fallback && this->count() > 0)
            return this->QTabWidgetqqAt(0);
        else
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

QTabWidgetqq *QTabWidgetsContainer::addQTabWidgetqq()
{
    QTabWidgetqq *tabWidget = new QTabWidgetqq(this); // this.parentWidget()
    this->addWidget(tabWidget);
    return tabWidget;
}

void QTabWidgetsContainer::setTabBarsMovable( bool on )
{
    for(int i=0;i<count();i++) {
        QTabWidgetqq* tabWidget = QTabWidgetqqAt(i);
        tabWidget->setMovable( on );
    }
}

void QTabWidgetsContainer::setTabBarsHighlight( bool on )
{
    for(int i=0;i<count();i++) {
        QTabWidgetqq* tabWidget = QTabWidgetqqAt(i);
        tabWidget->setTabBarHighlight( on );
    }
}

void QTabWidgetsContainer::setTabBarsVertical( bool on )
{
    for(int i=0;i<count();i++) {
        QTabWidgetqq* tabWidget = QTabWidgetqqAt(i);
        tabWidget->setTabBarVertical( on );
    }
}


void QTabWidgetsContainer::setTabBarsHidden( bool on )
{
    for(int i=0;i<count();i++) {
        QTabWidgetqq* tabWidget = QTabWidgetqqAt(i);
        tabWidget->setTabBarHidden( on );
    }
}


// This method is called from QTabWidgetqq childs, with method QTabWidgetqq::addEditorTab()
void QTabWidgetsContainer::_on_newQsciScintillaqqWidget(QsciScintillaqq *sci)
{
    emit newQsciScintillaqqChildCreated(sci);
}
