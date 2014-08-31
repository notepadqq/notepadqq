#ifndef EDITORTABWIDGET_H
#define EDITORTABWIDGET_H

#include <QTabWidget>
#include "editor.h"

/**
 * @brief A TabWidget used to allow the user to switch between
 *        multiple Editor instances.
 */
class EditorTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit EditorTabWidget(QWidget *parent = 0);
    int addEditorTab(bool setFocus, QString title);
    /**
     * @brief Add a new document, moving it from another EditorTabWidget
     * @param setFocus True to give focus to the new document
     * @param source EditorTabWidget that contains the source document
     * @param tabIndex Tab index, inside \p source, of the document
     * @return Tab index of the new document inside this EditorTabWidget.
     */
    int transferEditorTab(bool setFocus, EditorTabWidget *source, int tabIndex);
    int findOpenEditorByFileName(QString filename);
    Editor *editor(int index);
    Editor *currentEditor();

private:
    void setTabBarHidden(bool yes);
    void setTabBarHighlight(bool yes);
    void connectEditorSignals(Editor *editor);
    void disconnectEditorSignals(Editor *editor);
    int rawAddEditorTab(bool setFocus, QString title, EditorTabWidget *source, int sourceTabIndex);
private slots:
    void on_cleanChanged(bool isClean);

signals:
    void gotFocus();
    void editorAdded(int index);

public slots:
    void setSavedIcon(int index, bool saved);

};

#endif // EDITORTABWIDGET_H
