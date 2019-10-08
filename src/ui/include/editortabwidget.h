#ifndef EDITORTABWIDGET_H
#define EDITORTABWIDGET_H

#include "EditorNS/editor.h"

#include <QTabWidget>
#include <QWheelEvent>

using namespace EditorNS;

/**
 * @brief A TabWidget used to allow the user to switch between
 *        multiple Editor instances.
 */
class EditorTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit EditorTabWidget(QWidget *parent = nullptr);
    ~EditorTabWidget();

    int indexOf(QSharedPointer<Editor> editor) const;
    int indexOf(QWidget *widget) const;

    int addEditorTab(bool setFocus, const QString &title);
    /**
     * @brief Add a new document, moving it from another EditorTabWidget
     * @param setFocus True to give focus to the new document
     * @param source EditorTabWidget that contains the source document
     * @param tabIndex Tab index, inside \p source, of the document
     * @return Tab index of the new document inside this EditorTabWidget.
     */
    int transferEditorTab(bool setFocus, EditorTabWidget *source, int tabIndex);
    int findOpenEditorByUrl(const QUrl &filename);

    QSharedPointer<Editor> editor(int index) const;
    QSharedPointer<Editor> editor(Editor *editor) const;
    QSharedPointer<Editor> currentEditor();

    /**
     * @brief tabTextFromEditor Returns the tab text of a given Editor, or an empty string if
     *                          the Editor is not part of this tab widget.
     */
    QString tabTextFromEditor(QSharedPointer<Editor> editor);

    qreal zoomFactor() const;
    void setZoomFactor(const qreal &zoomFactor);

    /**
     * @brief deleteIfEmpty Deletes the TabWidget if it has no tabs.
     */
    void deleteIfEmpty();

    /**
     * @brief deleteIfEmpty Deletes the given TabWidget if it has no tabs.
     * @param tabWidget
     */
    static void deleteIfEmpty(EditorTabWidget *tabWidget);

    /**
     * @brief tabText Returns the title of the given Editor or tab index
     */
    QString tabText(Editor* editor) const;
    QString tabText(int index) const;

    /**
     * @brief tabText Sets the title of the given Editor or tab index
     */
    void setTabText(Editor* editor, const QString& text);
    void setTabText(int index, const QString& text);

    int formerTabIndex();

    QString generateTabTitleForUrl(const QUrl &filename) const;

private:

    // Smart pointers to the editors within this TabWidget
    QHash<Editor*, QSharedPointer<Editor>> m_editorPointers;

    qreal m_zoomFactor = 1;

    int m_formerTabIndex = 0;
    int m_mostRecentTabIndex = 0;

    void setTabBarHidden(bool yes);
    void setTabBarHighlight(bool yes);
    void connectEditorSignals(Editor *editor);
    void disconnectEditorSignals(Editor *editor);

    /**
     * @brief Add a new Editor tab (or transfer it from another EditorTabWidget)
     * @param setFocus True if you want to give focus to the new tab
     * @param title Title of the new tab. It's not used if a tab transfer is occurring
     * @param source Container of the tab to transfer. Set it to 0 to create a new tab.
     * @param sourceTabIndex Tab index, within @param source, of the tab to transfer
     * @return Index of the tab
     */
    int rawAddEditorTab(const bool setFocus, const QString &title, EditorTabWidget *source, const int sourceTabIndex);

private slots:
    void on_cleanChanged(bool isClean); 
    void on_editorMouseWheel(QWheelEvent *ev);
    void on_fileNameChanged(const QUrl &, const QUrl &newFileName);
    void on_currentTabChanged(int index);
signals:
    void gotFocus();
    void editorAdded(int index);
    void editorMouseWheel(int tab, QWheelEvent *ev);

public slots:
    void setSavedIcon(int index, bool saved);

protected:
    void mouseReleaseEvent(QMouseEvent *ev);
    void tabRemoved(int);
};

#endif // EDITORTABWIDGET_H
