#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "include/topeditorcontainer.h"
#include <QLabel>
#include <QSettings>
#include <QCloseEvent>
#include "docengine.h"
#include "include/Search/frmsearchreplace.h"
#include <functional>
#include "QtPrintSupport/QPrinter"
#include "include/Search/filesearchresultswidget.h"
#include "include/Extensions/extension.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &workingDirectory, const QStringList &arguments, QWidget *parent = 0);
    explicit MainWindow(const QStringList &arguments, QWidget *parent = 0);
    ~MainWindow();

    static QList<MainWindow *> instances();
    static MainWindow * lastActiveInstance();

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

    TopEditorContainer *topEditorContainer();

    void openCommandLineProvidedUrls(const QString &workingDirectory, const QStringList &arguments);

    Editor*   currentEditor();
    QSharedPointer<Editor> currentEditorSharedPtr();
    QAction*  addExtensionMenuItem(QString extensionId, QString text);
    void showExtensionsMenu(bool show);
    void updateShortcuts();
    QList<QAction*> getActions();
    QString getDefaultShortcut(QString actionName);

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void keyPressEvent(QKeyEvent *ev);
    void changeEvent(QEvent *e);

private slots:
    void refreshEditorUiInfo(Editor *editor);
    void refreshEditorUiCursorInfo(Editor *editor);
    void on_action_New_triggered();
    void on_customTabContextMenuRequested(QPoint point, EditorTabWidget *tabWidget, int tabIndex);
    void on_actionMove_to_Other_View_triggered();
    void on_action_Open_triggered();
    void on_actionOpen_Folder_triggered();
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
    void on_documentLoaded(EditorTabWidget *tabWidget, int tab, bool wasAlreadyOpened);
    void on_actionReload_from_Disk_triggered();
    void on_actionFind_Next_triggered();
    void on_actionFind_Previous_triggered();
    void on_actionRename_triggered();
    void on_actionWord_wrap_toggled(bool on);
    void on_actionEmpty_Recent_Files_List_triggered();
    void on_actionOpen_All_Recent_Files_triggered();
    void on_actionUNIX_Format_triggered();
    void on_actionWindows_Format_triggered();
    void on_actionMac_Format_triggered();
    void on_actionUTF_8_triggered();
    void on_actionUTF_8_without_BOM_triggered();
    void on_actionUTF_16BE_triggered();
    void on_actionUTF_16LE_triggered();
    void on_actionInterpret_as_UTF_8_triggered();
    void on_actionInterpret_as_UTF_8_without_BOM_triggered();
    void on_actionInterpret_as_UTF_16BE_UCS_2_Big_Endian_triggered();
    void on_actionInterpret_as_UTF_16LE_UCS_2_Little_Endian_triggered();
    void on_actionShow_Tabs_triggered(bool on);
    void on_actionConvert_to_triggered();
    void on_actionIndentation_Default_settings_triggered();
    void on_actionIndentation_Custom_triggered();
    void on_actionReload_file_interpreted_as_triggered();
    void on_actionInterpret_as_triggered();
    void on_actionPrint_triggered();
    void on_actionPrint_Now_triggered();
    void on_actionLaunch_in_Firefox_triggered();
    void on_actionLaunch_in_Chromium_triggered();
    void on_actionLaunch_in_Chrome_triggered();
    void on_actionGet_php_help_triggered();
    void on_actionGoogle_Search_triggered();
    void on_actionWikipedia_Search_triggered();
    void on_actionOpen_a_New_Window_triggered();
    void on_actionOpen_in_New_Window_triggered();
    void on_actionMove_to_New_Window_triggered();
    void on_actionOpen_file_triggered();
    void on_actionOpen_in_another_window_triggered();
    void on_tabBarDoubleClicked(EditorTabWidget *tabWidget, int tab);
    void on_actionFind_in_Files_triggered();
    void on_actionDelete_Line_triggered();
    void on_actionDuplicate_Line_triggered();
    void on_actionMove_Line_Up_triggered();
    void on_actionMove_Line_Down_triggered();
    void on_fileSearchResultFinished(FileSearchResult::SearchResult result);
    void on_resultMatchClicked(const FileSearchResult::FileResult &file, const FileSearchResult::Result &match);
    void on_actionTrim_Trailing_Space_triggered();
    void on_actionTrim_Leading_Space_triggered();
    void on_actionTrim_Leading_and_Trailing_Space_triggered();
    void on_actionEOL_to_Space_triggered();
    void on_actionTAB_to_Space_triggered();
    void on_actionSpace_to_TAB_All_triggered();
    void on_actionSpace_to_TAB_Leading_triggered();
    void on_editorUrlsDropped(QList<QUrl> urls);
    void on_actionGo_to_line_triggered();
    void on_actionInstall_Extension_triggered();
    void on_actionFull_Screen_toggled(bool on);
    void on_actionShow_End_of_Line_triggered(bool on);
    void on_actionShow_All_Characters_toggled(bool on);
    void on_actionShow_Spaces_triggered(bool on);

private:
    static QList<MainWindow*> m_instances;
    Ui::MainWindow*       ui;
    TopEditorContainer*   m_topEditorContainer;
    DocEngine*            m_docEngine;
    QMenu*                m_tabContextMenu;
    QList<QAction *>      m_tabContextMenuActions;
    QLabel*               m_statusBar_fileFormat;
    QLabel*               m_statusBar_length_lines;
    QLabel*               m_statusBar_curPos;
    QLabel*               m_statusBar_selection;
    QLabel*               m_statusBar_EOLstyle;
    QLabel*               m_statusBar_textFormat;
    QLabel*               m_statusBar_overtypeNotify;
    QSettings*            m_settings;
    frmSearchReplace*     m_frmSearchReplace = 0;
    bool                  m_overwrite = false; // Overwrite mode vs Insert mode
    FileSearchResultsWidget* m_fileSearchResultsWidget;
    QString               m_workingDirectory;
    QMap<QSharedPointer<Extensions::Extension>, QMenu*> m_extensionMenus;
    QMap<QString,QString>* m_defaultShortcuts;

    void                defaultShortcuts();
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
    void                setupLanguagesMenu();
    void                transformSelectedText(std::function<QString (const QString &)> func);
    void                restoreWindowSettings();
    void                loadIcons();
    void                updateRecentDocsInMenu();
    void                convertEditorEncoding(Editor *editor, QTextCodec *codec, bool bom);
    void                toggleOverwrite();
    void                checkIndentationMode(Editor *editor);
    bool                reloadWithWarning(EditorTabWidget *tabWidget, int tab, QTextCodec *codec, bool bom);
    QStringList         currentWordOrSelections();
    QString             currentWordOrSelection();
    void                currentWordOnlineSearch(const QString &searchUrl);
    QString             getNewDocumentName();

    /**
     * @brief Workaround for this bug: https://bugs.launchpad.net/ubuntu/+source/appmenu-qt5/+bug/1313248
     */
    void                fixKeyboardShortcuts();
    void                instantiateFrmSearchReplace();
    QUrl                stringToUrl(QString fileName, QString workingDirectory = QString());

    /**
     * @brief Initialize UI from settings
     */
    void initUI();

    /**
     * @brief Update symbol options using parameter `on` and Show_All_Characters toggle status.
     * @param on  `true` or `false` based on the calling element's toggle status.
     * @return bool: `true` if `on` is `false` and Show_All_Characters is checked. False otherwise.
     *               On a `true` return, default symbol saving behavior is modified.
     */
    bool updateSymbols(bool on);
};

#endif // MAINWINDOW_H
