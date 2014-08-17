#ifndef TOPEDITORCONTAINER_H
#define TOPEDITORCONTAINER_H

#include <QSplitter>
#include <include/editortabwidget.h>

/**
 * @brief Contains one or more EditorTabWidgets. This class
 *        allows the user to have multiple tabs displayed at the
 *        same time, splitting the screen space.
 */
class TopEditorContainer : public QSplitter
{
    Q_OBJECT
public:
    explicit TopEditorContainer(QWidget *parent = 0);
    EditorTabWidget *addTabWidget();
    EditorTabWidget *tabWidget(int index);
    EditorTabWidget *currentTabWidget();

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

public slots:

private slots:
    void on_currentTabChanged(int index);
    void on_currentTabWidgetChanged();
    void on_customContextMenuRequested(QPoint point);
    void on_tabCloseRequested(int index);
    void on_editorAdded(int tab);

};

#endif // TOPEDITORCONTAINER_H
