#include "include/topeditorcontainer.h"

#include <QTabBar>

TopEditorContainer::TopEditorContainer(QWidget *parent) :
    QSplitter(parent), m_currentTabWidget(0)
{
    setOrientation(Qt::Horizontal);

    //Always add a first tabWidget to the container.
    //This ensures m_currentTagWidget is never null
    m_currentTabWidget = addTabWidget();
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

    //Resize all panes to be equally big.
    const int currentViewCount = count();
    const int tabSize = contentsRect().width() / currentViewCount;

    QList<int> sizes;
    for(int i=0; i<currentViewCount; ++i)
        sizes << tabSize;

    setSizes( sizes );

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

EditorTabWidget *TopEditorContainer::inactiveTabWidget(bool createIfNotExists)
{
    const int currentViewCount = count();

    if(currentViewCount >= 2) {
        //Two view panes are open. Pick the one not currently active.
        int viewId = widget(1)==currentTabWidget() ? 0 : 1;
        return tabWidget(viewId);
    }

    //Only one view pane is open.
    if(createIfNotExists)
        return addTabWidget();
    else
        return nullptr;
}

EditorTabWidget *TopEditorContainer::tabWidgetFromEditor(QSharedPointer<Editor> editor)
{
    return tabWidgetFromEditor(editor.data());
}

EditorTabWidget *TopEditorContainer::tabWidgetFromEditor(Editor* editor)
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

void TopEditorContainer::forEachEditor(std::function<bool (const int, const int, EditorTabWidget *, QSharedPointer<Editor>)> callback)
{
    forEachEditor(false, callback);
}

std::vector<QSharedPointer<Editor>> TopEditorContainer::getOpenEditors()
{
    std::vector<QSharedPointer<Editor>> editors;

    for (int i = 0; i < count(); i++) {
        EditorTabWidget *tabW = tabWidget(i);
        for (int j = 0; j < tabW->count(); j++) {
            editors.push_back(tabW->editor(j));
        }
    }

    return editors;
}

int TopEditorContainer::getNumEditors()
{
    int total = 0;

    for(int i = 0; i < count(); ++i)
        total += tabWidget(i)->count();

    return total;
}

void TopEditorContainer::disconnectAllTabWidgets()
{
    for (int i = 0; i < count(); ++i) {
        tabWidget(i)->disconnect();
    }
}

void TopEditorContainer::forEachEditor(bool backwardIndexes,
                                       std::function<bool (const int tabWidgetId, const int editorId, EditorTabWidget *tabWidget, QSharedPointer<Editor> editor)> callback)
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

QPromise<void> TopEditorContainer::forEachEditorAsync(bool backwardIndices,
                                                    std::function<void (const int tabWidgetId, const int editorId, EditorTabWidget *tabWidget, QSharedPointer<Editor> editor, std::function<void()> goOn, std::function<void()> stop)> callback)
{
    return QPromise<void>([=](const auto& resolve, const auto&) {

        if (backwardIndices) {
            std::function<std::function<void()>(int,int)> iteration = [=](int i, int j) {
                return [=]() {
                    if (i < 0) {
                        resolve();
                        return;
                    }
                    if (j < 0) {
                        if (i-1 >= 0) {
                            iteration(i-1, this->tabWidget(i-1)->count() - 1);
                        }
                        return;
                    }

                    EditorTabWidget *tabW = this->tabWidget(i);
                    callback(i, j, tabW, tabW->editor(j), iteration(i, j-1), [resolve](){ resolve(); });
                };
            };

            iteration(this->count() - 1, this->tabWidget(0)->count() - 1)();

        } else {
            std::function<std::function<void()>(int,int)> iteration = [=](int i, int j) {
                return [=]() {
                    if (i >= this->count()) {
                        resolve();
                        return;
                    }
                    if (j >= this->tabWidget(i)->count()) {
                        iteration(i+1, 0);
                        return;
                    }

                    EditorTabWidget *tabW = this->tabWidget(i);
                    callback(i, j, tabW, tabW->editor(j), iteration(i, j+1), [resolve](){ resolve(); });
                };
            };

            iteration(0, 0)();
        }
    });
}

QPromise<void> TopEditorContainer::forEachEditorConcurrent(std::function<void (const int tabWidgetId, const int editorId, EditorTabWidget *tabWidget, QSharedPointer<Editor> editor, std::function<void()> done)> callback)
{
    return QPromise<void>([=](const auto& resolve, const auto&) {

        // Collect all the indices we're going to use
        std::vector<std::pair<int,int>> indices;
        for (int i = 0; i < this->count(); i++) {
            EditorTabWidget *tabW = this->tabWidget(i);
            for (int j = 0; j < tabW->count(); j++) {
                indices.push_back(std::make_pair(i, j));
            }
        }

        // Counts the number of iterations
        std::shared_ptr<int> cnt = std::make_shared<int>(indices.size());

        for (const auto& idx : indices) {
            int i = idx.first;
            int j = idx.second;
            callback(i, j, this->tabWidget(i), this->tabWidget(i)->editor(j), [cnt, resolve](){
                (*cnt)--;
                if (*cnt == 0) {
                    resolve();
                }
            });
        }

    });
}
