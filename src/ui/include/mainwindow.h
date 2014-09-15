#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "topeditorcontainer.h"
#include <QLabel>
#include <QSettings>
#include <QCloseEvent>
#include "docengine.h"
#include "frmsearchreplace.h"
#include <functional>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /**
     * Describes the result of a save process. For example, if the user cancels the save dialog, \p saveFileResult_Canceled is returned.
     */
    enum saveFileResult {
         saveFileResult_Saved       /** The file was saved  */
        ,saveFileResult_Canceled    /** The save process was canceled */
    };

    /**
     * Describes the result of a tab closing process.
     */
    enum tabCloseResult {
         tabCloseResult_Saved        /** The tab was closed and the file was saved */
        ,tabCloseResult_NotSaved     /** The tab was closed and the file wasn't saved */
        ,tabCloseResult_AlreadySaved /** The tab was closed and the file was already saved */
        ,tabCloseResult_Canceled     /** The close process was canceled */
    };

    /**
     * Reasons for asking to save changes
     */
    enum askToSaveChangesReason {
         askToSaveChangesReason_tabClosing  /** The tab is closing */
        ,askToSaveChangesReason_generic     /** Generic reason */
    };

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
private slots:
    void refreshEditorUiInfo(Editor *editor);
    void refreshEditorUiCursorInfo(Editor *editor);
    void on_action_New_triggered();
    void on_customTabContextMenuRequested(QPoint point, EditorTabWidget *tabWidget, int tabIndex);
    void on_actionMove_to_Other_View_triggered();
    void on_action_Open_triggered();
    void on_tabCloseRequested(EditorTabWidget* tabWidget, int tab);
    void on_actionSave_triggered();
    void on_actionSave_as_triggered();
    void on_actionSave_a_Copy_As_triggered();
    void on_action_Copy_triggered();
    void on_action_Paste_triggered();
    void on_actionCu_t_triggered();
    void on_currentEditorChanged(EditorTabWidget* tabWidget, int tab);
    void on_editorAdded(EditorTabWidget* tabWidget, int tab);
    void on_cursorActivity();
    void on_action_Delete_triggered();
    void on_actionSelect_All_triggered();
    void on_actionAbout_Notepadqq_triggered();
    void on_actionAbout_Qt_triggered();
    void on_action_Undo_triggered();
    void on_action_Redo_triggered();
    void on_actionE_xit_triggered();
    void on_actionSearch_triggered();
    void setCurrentEditorLanguage(QString language);
    void on_actionCurrent_Full_File_path_to_Clipboard_triggered();
    void on_actionCurrent_Filename_to_Clipboard_triggered();
    void on_actionCurrent_Directory_Path_to_Clipboard_triggered();
    void on_actionPreferences_triggered();
    void on_actionClose_triggered();
    void on_actionC_lose_All_triggered();
    void on_fileOnDiskChanged(EditorTabWidget *tabWidget, int tab, bool removed);
    void on_actionReplace_triggered();
    void on_actionPlain_text_triggered();
    void on_currentLanguageChanged(QString id, QString name);
    void on_actionRestore_Default_Zoom_triggered();
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_editorMouseWheel(EditorTabWidget *tabWidget, int tab, QWheelEvent *ev);
    void on_actionUPPERCASE_triggered();
    void on_actionLowercase_triggered();
    void on_actionClose_All_BUT_Current_Document_triggered();
    void on_actionSave_All_triggered();
    void on_bannerRemoved(QWidget *banner);
    void on_documentSaved(EditorTabWidget *tabWidget, int tab);
    void on_documentReloaded(EditorTabWidget *tabWidget, int tab);
    void on_actionReload_from_Disk_triggered();
    void on_actionFind_Next_triggered();
    void on_actionFind_Previous_triggered();
    void on_actionRename_triggered();

private:
    Ui::MainWindow*     ui;
    TopEditorContainer* m_topEditorContainer;
    DocEngine*          m_docEngine;
    QMenu*              m_tabContextMenu;
    QList<QAction *>    m_tabContextMenuActions;
    QLabel*             m_statusBar_fileFormat;
    QLabel*             m_statusBar_lengthInfo;
    QLabel*             m_statusBar_selectionInfo;
    QLabel*             m_statusBar_EOLstyle;
    QLabel*             m_statusBar_textFormat;
    QLabel*             m_statusBar_overtypeNotify;
    QSettings*          m_settings;
    frmSearchReplace*   m_frmSearchReplace = 0;
    void                removeTabWidgetIfEmpty(EditorTabWidget *tabWidget);
    void                createStatusBar();
    int                 askIfWantToSave(EditorTabWidget *tabWidget, int tab, int reason);

    /**
     * @brief Removes the specified tab. Doesn't remove the tab if it's the
     *        last tab, it's empty, in an unmodified state and it's not
     *        associated with a file name.
     *        If the document inside the tab is in a modified state, asks
     *        the user to save the changes.
     *        In addition, it ensures that the window won't remain without
     *        any tab opened, and that there won't be any empty EditorTabWidget
     * @param tabWidget
     * @param tab
     * @param remove Set this to false if you want to manually remove the tab from
     *               the tabWidget.
     * @param force Set this to true to close the tab without ever asking the user
     *              to save changes.
     * @return tabCloseResult
     */
    int                 closeTab(EditorTabWidget *tabWidget, int tab, bool remove, bool force);
    int                 closeTab(EditorTabWidget *tabWidget, int tab);

    /**
     * @brief Save a document. If the document has not an associated path,
     *        open a dialog to ask the user where to save the file.
     * @param tabWidget
     * @param tab
     * @return a saveFileResult
     */
    int                 save(EditorTabWidget *tabWidget, int tab);
    int                 saveAs(EditorTabWidget *tabWidget, int tab, bool copy);
    QUrl                getSaveDialogDefaultFileName(EditorTabWidget *tabWidget, int tab);
    Editor*             currentEditor();
    void                openCommandLineProvidedUrls();
    void                setupLanguagesMenu();
    void                transformSelectedText(std::function<QString (const QString &)> func);
    void                restoreWindowSettings();
    void                loadIcons();
};

#endif // MAINWINDOW_H
