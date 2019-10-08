#ifndef TOPEDITORCONTAINER_H
#define TOPEDITORCONTAINER_H

#include "EditorNS/editor.h"
#include "editortabwidget.h"

#include <QSplitter>
#include <QWheelEvent>
#include <QtPromise>

#include <functional>
#include <vector>

using namespace QtPromise;

/**
 * @brief Contains one or more EditorTabWidgets. This class
 *        allows the user to have multiple tabs displayed at the
 *        same time, splitting the screen space.
 */
class TopEditorContainer : public QSplitter
{
    Q_OBJECT
public:
    explicit TopEditorContainer(QWidget *parent = nullptr);
    EditorTabWidget *addTabWidget();
    EditorTabWidget *tabWidget(int index);
    EditorTabWidget *currentTabWidget();

    /**
     * @brief Returns either of the two first tabwidgets that is not currently active.
     * @param createIfNotExists creates a second tabwidget if there is only one.
     * @return EditorTabWidget. Returns nullptr if no inactive tab was found.
     */
    EditorTabWidget *inactiveTabWidget(bool createIfNotExists);

    /**
     * @brief Returns the EditorTabWidget that contains a particular Editor
     * @param editor
     * @return EditorTabWidget. Returns 0 if not found.
     */
    EditorTabWidget *tabWidgetFromEditor(QSharedPointer<Editor> editor);
    EditorTabWidget *tabWidgetFromEditor(Editor *editor);

    /**
     * @brief Executes the specified function for each editor in this container.
     * @param backwardIndexes True if you want to get the items in the reverse order
     *                        (useful for example if you're deleting the items
     *                         while iterating over them).
     * @param callback Callback function. It should return true to continue,
     *                 false to break the loop.
     */
    void forEachEditor(bool backwardIndexes, std::function<bool (const int, const int, EditorTabWidget *, QSharedPointer<Editor>)> callback);
    void forEachEditor(std::function<bool (const int, const int, EditorTabWidget *, QSharedPointer<Editor>)> callback);

    /**
     * @brief Executes the specified asynchronous function for each editor in this container, in order.
     *        Every callback can be asynchronous and should call goOn() as soon as it finishes to invoke
     *        the next iteration. If stop() is called, the loop is stopped. Never call both goOn() and
     *        stop() from the same callback.
     * @param backwardIndices True if you want to get the items in the reverse order
     *                        (useful for example if you're deleting the items
     *                         while iterating over them).
     * @param callback
     * @return Returns a promise which is resolved when all the callbacks have finished.
     */
    QPromise<void> forEachEditorAsync(bool backwardIndices, std::function<void (const int tabWidgetId, const int editorId, EditorTabWidget *tabWidget, QSharedPointer<Editor> editor, std::function<void()> goOn, std::function<void()> stop)> callback);

    /**
     * @brief Executes the specified asynchronous function for each editor in this container, concurrently.
     *        Every callback can be asynchronous and should call done() when it finishes.
     * @param callback
     * @return Returns a promise which is resolved when all the callbacks have called done().
     */
    QPromise<void> forEachEditorConcurrent(std::function<void (const int tabWidgetId, const int editorId, EditorTabWidget *tabWidget, QSharedPointer<Editor> editor, std::function<void()> done)> callback);

    std::vector<QSharedPointer<Editor>> getOpenEditors();

    /**
     * @brief Returns the number of editors in all of the TopEditorWidget's children.
     */
    int getNumEditors();

    /**
     * @brief Disconnects all the signals emitted by all the tabWidgets owned by this object.
     *        This method must only be called on exit, as the TopEditorContainer will become
     *        unusable.
     */
    void disconnectAllTabWidgets();

private:
    EditorTabWidget *m_currentTabWidget;

signals:
    /**
     * @brief Emitted when any of the tab widgets switch tab
     * @param tabWidget
     * @param tab
     */
    void currentTabChanged(EditorTabWidget *tabWidget, int tab);

    /**
     * @brief Emitted when the focused tabWidget changes
     * @param tabWidget
     */
    void currentTabWidgetChanged(EditorTabWidget *tabWidget);

    /**
     * @brief Emitted when the focused editor changes, in any of
     *        the tab widgets within this container.
     * @param tabWidget
     * @param tab
     */
    void currentEditorChanged(EditorTabWidget *tabWidget, int tab);

    void customTabContextMenuRequested(QPoint point, EditorTabWidget *tabWidget, int tab);
    void tabCloseRequested(EditorTabWidget *tabWidget, int tab);
    void editorAdded(EditorTabWidget *tabWidget, int tab);
    void editorMouseWheel(EditorTabWidget *tabWidget, int tab, QWheelEvent *ev);
    void tabBarDoubleClicked(EditorTabWidget *tabWidget, int tab);

public slots:

private slots:
    void on_currentTabChanged(int index);
    void on_currentTabWidgetChanged();
    void on_customContextMenuRequested(QPoint point);
    void on_tabCloseRequested(int index);
    void on_editorAdded(int tab);

};

#endif // TOPEDITORCONTAINER_H
