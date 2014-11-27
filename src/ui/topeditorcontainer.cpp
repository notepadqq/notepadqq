#include "include/topeditorcontainer.h"
#include <QTabBar>

TopEditorContainer::TopEditorContainer(QWidget *parent) :
    QSplitter(parent), m_currentTabWidget(0)
{
    setOrientation(Qt::Horizontal);
}

EditorTabWidget *TopEditorContainer::addTabWidget()
{
    EditorTabWidget *tabWidget = new EditorTabWidget(this);
    addWidget(tabWidget);

    // Detect tab switches
    connect(tabWidget, &EditorTabWidget::currentChanged, this, &TopEditorContainer::on_currentTabChanged);
    connect(tabWidget, &EditorTabWidget::gotFocus, this, &TopEditorContainer::on_currentTabWidgetChanged);
    connect(tabWidget, &EditorTabWidget::tabBarClicked, this, &TopEditorContainer::on_currentTabWidgetChanged);

    connect(tabWidget, &EditorTabWidget::customContextMenuRequested, this, &TopEditorContainer::on_customContextMenuRequested);
    connect(tabWidget, &EditorTabWidget::tabCloseRequested, this, &TopEditorContainer::on_tabCloseRequested);
    connect(tabWidget, &EditorTabWidget::editorAdded, this, &TopEditorContainer::on_editorAdded);
    connect(tabWidget, &EditorTabWidget::editorMouseWheel, this, [=](int tab, QWheelEvent *ev) {
        emit editorMouseWheel(tabWidget, tab, ev);
    });
    connect(tabWidget, &EditorTabWidget::tabBarDoubleClicked, this, [=](int index) {
        emit tabBarDoubleClicked(tabWidget, index);
    });

    return tabWidget;
}

EditorTabWidget *TopEditorContainer::tabWidget(int index)
{
    return dynamic_cast<EditorTabWidget *>(widget(index));
}

EditorTabWidget *TopEditorContainer::currentTabWidget()
{
    return m_currentTabWidget;
}

EditorTabWidget *TopEditorContainer::tabWidgetFromEditor(Editor *editor)
{
    for (int i = 0; i < count(); i++) {
        if (tabWidget(i)->indexOf(editor) > -1)
            return tabWidget(i);
    }
    return 0;
}

void TopEditorContainer::on_currentTabChanged(int index)
{
    EditorTabWidget *tabWidget = dynamic_cast<EditorTabWidget *>(sender());
    if (!tabWidget)
        return;

    m_currentTabWidget = tabWidget;
    emit currentTabChanged(tabWidget, index);
    emit currentEditorChanged(tabWidget, index);
}

void TopEditorContainer::on_currentTabWidgetChanged()
{
    EditorTabWidget *tabWidget = dynamic_cast<EditorTabWidget *>(sender());
    if (!tabWidget)
        return;

    if(m_currentTabWidget != tabWidget) {
        m_currentTabWidget = tabWidget;
        emit currentTabWidgetChanged(tabWidget);
        emit currentEditorChanged(tabWidget, tabWidget->currentIndex());
    }
}

void TopEditorContainer::on_customContextMenuRequested(QPoint point)
{
    EditorTabWidget *tabWidget = dynamic_cast<EditorTabWidget *>(sender());
    if (!tabWidget)
        return;

    int index = tabWidget->tabBar()->tabAt(point);

    if(index != -1)
    {
        tabWidget->setFocus();
        tabWidget->setCurrentIndex(index);

        emit customTabContextMenuRequested(
                 tabWidget->mapToGlobal(point),
                 tabWidget,
                 index);
    }
}

void TopEditorContainer::on_tabCloseRequested(int index)
{
    EditorTabWidget *tabWidget = dynamic_cast<EditorTabWidget *>(sender());
    if (!tabWidget)
        return;

    m_currentTabWidget = tabWidget;
    emit tabCloseRequested(tabWidget, index);
}

void TopEditorContainer::on_editorAdded(int tab)
{
    EditorTabWidget *tabWidget = dynamic_cast<EditorTabWidget *>(sender());
    if (!tabWidget)
        return;

    emit editorAdded(tabWidget, tab);
}

void TopEditorContainer::forEachEditor(std::function<bool (const int, const int, EditorTabWidget *, Editor *)> callback)
{
    forEachEditor(false, callback);
}

void TopEditorContainer::forEachEditor(bool backwardIndexes,
                                       std::function<bool (const int tabWidgetId, const int editorId, EditorTabWidget *tabWidget, Editor *editor)> callback)
{
    if (backwardIndexes) {
        for (int i = count() - 1; i >= 0; i--) {
            EditorTabWidget *tabW = tabWidget(i);
            for (int j = tabW->count() - 1; j >= 0; j--) {
                bool ret = callback(i, j, tabW, tabW->editor(j));
                if (ret == false) return;
            }
        }
    } else {
        for (int i = 0; i < count(); i++) {
            EditorTabWidget *tabW = tabWidget(i);
            for (int j = 0; j < tabW->count(); j++) {
                bool ret = callback(i, j, tabW, tabW->editor(j));
                if (ret == false) return;
            }
        }
    }
}
