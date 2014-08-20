#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "topeditorcontainer.h"
#include <QLabel>
#include <QSettings>
#include <QCloseEvent>
#include "docengine.h"
#include "frmsearchreplace.h"

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

private slots:
    void refreshEditorUiInfo(Editor *editor);
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

private:
    Ui::MainWindow*     ui;
    TopEditorContainer* topEditorContainer;
    QMenu*              tabContextMenu;
    QList<QAction *>    tabContextMenuActions;
    QLabel*             statusBar_fileFormat;
    QLabel*             statusBar_lengthInfo;
    QLabel*             statusBar_selectionInfo;
    QLabel*             statusBar_EOLstyle;
    QLabel*             statusBar_textFormat;
    QLabel*             statusBar_overtypeNotify;
    QSettings*          settings;
    DocEngine*          docEngine;
    frmSearchReplace*   m_frmSearch;
    void                removeTabWidgetIfEmpty(EditorTabWidget *tabWidget);
    void                createStatusBar();
    int                 askIfWantToSave(EditorTabWidget *tabWidget, int tab, int reason);
    int                 closeTab(EditorTabWidget *tabWidget, int tab);

    /**
     * @brief Save a document. If the document has not an associated path,
     *        open a dialog to ask the user where to save the file.
     * @param tabWidget
     * @param tab
     * @return
     */
    int                 save(EditorTabWidget *tabWidget, int tab);
    int                 saveAs(EditorTabWidget *tabWidget, int tab, bool copy);
    QString             getSaveDialogDefaultFileName(EditorTabWidget *tabWidget, int tab);
    Editor*             currentEditor();
    void                closeEvent(QCloseEvent *event);
    void                processCommandLineArgs(QStringList arguments, bool fromOtherInstance);
};

#endif // MAINWINDOW_H
