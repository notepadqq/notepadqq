#include "include/topeditorcontainer.h"
#include <QTabBar>

TopEditorContainer::TopEditorContainer(QWidget *parent) :
    QSplitter(parent), m_currentTabWidget(0)
{
    this->setOrientation(Qt::Horizontal);
}

EditorTabWidget *TopEditorContainer::addTabWidget()
{
    EditorTabWidget *tabWidget = new EditorTabWidget(this);
    this->addWidget(tabWidget);

    // Detect tab switches
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(on_currentTabChanged(int)));
    // Detect tabWidget switches
    connect(tabWidget, SIGNAL(gotFocus()), this, SLOT(on_currentTabWidgetChanged()));
    connect(tabWidget, SIGNAL(tabBarClicked(int)), this, SLOT(on_currentTabWidgetChanged()));

    connect(tabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequested(QPoint)));

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(on_tabCloseRequested(int)));
    connect(tabWidget, SIGNAL(editorAdded(int)), this, SLOT(on_editorAdded(int)));

    return tabWidget;
}

EditorTabWidget *TopEditorContainer::tabWidget(int index)
{
    return (EditorTabWidget *)this->widget(index);
}

EditorTabWidget *TopEditorContainer::currentTabWidget()
{
    return this->m_currentTabWidget;
}

void TopEditorContainer::on_currentTabChanged(int index)
{
    EditorTabWidget *tabWidget = (EditorTabWidget *)sender();
    this->m_currentTabWidget = tabWidget;
    emit this->currentTabChanged(tabWidget, index);
    emit this->currentEditorChanged(tabWidget, index);
}

void TopEditorContainer::on_currentTabWidgetChanged()
{
    EditorTabWidget *tabWidget = (EditorTabWidget *)sender();

    if(m_currentTabWidget != tabWidget) {
        this->m_currentTabWidget = tabWidget;
        emit this->currentTabWidgetChanged(tabWidget);
        emit this->currentEditorChanged(tabWidget, tabWidget->currentIndex());
    }
}

void TopEditorContainer::on_customContextMenuRequested(QPoint point)
{
    EditorTabWidget *tabWidget = static_cast<EditorTabWidget *>(sender());
    int index = tabWidget->tabBar()->tabAt(point);

    if(index != -1)
    {
        tabWidget->setCurrentIndex(index);

        emit this->customTabContextMenuRequested(
                    tabWidget->mapToGlobal(point),
                    tabWidget,
                    index);
    }
}

void TopEditorContainer::on_tabCloseRequested(int index)
{
    EditorTabWidget *tabWidget = (EditorTabWidget *)sender();
    this->m_currentTabWidget = tabWidget;
    emit this->tabCloseRequested(tabWidget, index);
}

void TopEditorContainer::on_editorAdded(int tab)
{
    EditorTabWidget *tabWidget = (EditorTabWidget *)sender();
    emit this->editorAdded(tabWidget, tab);
}
