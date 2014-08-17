#ifndef EDITORTABWIDGET_H
#define EDITORTABWIDGET_H

#include <QTabWidget>
#include <include/editor.h>

/**
 * @brief A TabWidget used to allow the user to switch between
 *        multiple Editor instances.
 */
class EditorTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit EditorTabWidget(QWidget *parent = 0);
    Editor getTabDriver(int index);
    int addEditorTab(bool setFocus, QString title);
    // Add a tab, cloning it from the specified source.
    int transferEditorTab(bool setFocus, EditorTabWidget *source, int tabIndex);
    int findOpenEditorByFileName(QString filename);
    Editor *editor(int index);
    Editor *currentEditor();

private:
    void setTabBarHidden(bool yes);
    void setTabBarHighlight(bool yes);
    void setTabBarVertical(bool yes);

private slots:
    void on_contentChanged();
    void on_cleanChanged(bool isClean);

signals:
    void gotFocus();
    void editorAdded(int index);

public slots:
    void setSavedIcon(int index, bool saved);

};

#endif // EDITORTABWIDGET_H
