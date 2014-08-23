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
    connect(tabWidget, &EditorTabWidget::currentChanged, this, &TopEditorContainer::on_currentTabChanged);
    // Detect tabWidget switches // FIXME Doesn't work if the user changes
    // tabWidget by clicking directly within one of the editors.
    connect(tabWidget, &EditorTabWidget::gotFocus, this, &TopEditorContainer::on_currentTabWidgetChanged);
    connect(tabWidget, &EditorTabWidget::tabBarClicked, this, &TopEditorContainer::on_currentTabWidgetChanged);

    connect(tabWidget, &EditorTabWidget::customContextMenuRequested, this, &TopEditorContainer::on_customContextMenuRequested);
    connect(tabWidget, &EditorTabWidget::tabCloseRequested, this, &TopEditorContainer::on_tabCloseRequested);
    connect(tabWidget, &EditorTabWidget::editorAdded, this, &TopEditorContainer::on_editorAdded);

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
