/*
 *
 * This file is part of the Notepadqq text editor.
 *
 * Copyright(c) 2010 Notepadqq team.
 * http://notepadqq.sourceforge.net/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Qsci/qsciscintilla.h>
#include "qtabwidgetscontainer.h"
#include "frmsrchreplace.h"
#include "frmpreferences.h"
#include "searchengine.h"
#include "docengine.h"
#include "lexerfactory.h"
#include <QSettings>
#include <QLabel>
#include <QSplitter>
#include <QtNetwork/QLocalServer>
#include <QActionGroup>

#define FILETYPES_FILE "filetypes.list"
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // 2-STEP initializer
    void init();

    QTabWidgetsContainer *container;

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

    enum Setting {
        Setting_FIRST,
        Setting_ShowAllCharacters,
        Setting_WordWrap,
        Setting_ShowEndOfLine,
        Setting_ShowWhiteSpaceAndTab,
        Setting_ShowIndentGuide,
        Setting_ShowWrapSymbol,
        Setting_LAST
    };

    int           askIfWantToSave(QsciScintillaqq *sci, int reason);
    int           save(QsciScintillaqq *sci);
    int           saveAs(QsciScintillaqq *sci,bool copy=false);
    int           kindlyTabClose(QsciScintillaqq *sci);
    int           fileAlreadyOpened(const QString & filepath);

    void          createStatusBar();
    void          clearSearchDialog();
    void          update_single_document_ui( QsciScintillaqq* sci );
    void          update_appwide_ui(const char* setting);
    void          connect_tabWidget(QTabWidgetqq *tabWidget);
    void          processCommandLineArgs(QStringList arguments, bool fromExternalMessage);

    QFont*        systemMonospace();
    QString       getSaveDialogDefaultFileName(QsciScintillaqq *sci);
    QSettings*    getSettings();

    LexerFactory* getLexerFactory();
    searchengine* getSearchEngine();

    //Singleton instance of main window class
    static MainWindow* instance();

    //Quick access to certain pointers
    QsciScintillaqq* focused_editor();
    QsciScintillaqq* editor_at_index(int i);
    QTabWidgetqq*    focused_tabWidget();

private:
    static MainWindow* wMain;
    Ui::MainWindow*    ui;
    QLocalServer*      instanceServer;
    QSettings*         settings;

    QFont*             system_monospace;
    QLabel*            statusBar_fileFormat;
    QLabel*            statusBar_lengthInfo;
    QLabel*            statusBar_selectionInfo;
    QLabel*            statusBar_EOLstyle;
    QLabel*            statusBar_textFormat;
    QLabel*            statusBar_overtypeNotify;
    QList<QAction *>   tabContextMenuActions;
    QMenu*             tabContextMenu;

    frmsrchreplace*    form_search;
    frmpreferences*    form_preferences;

    searchengine*      search_engine;
    docengine*         document_engine;
    LexerFactory*      lexer_factory;

    void               closeEvent(QCloseEvent *event);
    void               initialize_languages();

private slots:
    int  _on_tab_close_requested(int index);
    void _on_tabWidget_customContextMenuRequested(QPoint pos);
    void _on_instanceServer_NewConnection();
    void _on_instanceServer_Socket_ReadyRead();
    void _on_newQsciScintillaqqChildCreated(QsciScintillaqq *sci);
    void _on_sci_copyAvailable(bool yes);
    void _on_sci_updateUI();
    void _apply_wide_settings_to_tab(int tab);
    void _on_editor_cursor_position_change(int line, int index);
    void _on_editor_overtype_changed(bool overtype);
    void _on_editor_language_set();
    void _on_editor_new(int index);

    void on_action_New_triggered();
    void on_actionSave_as_triggered();
    void on_actionSave_triggered();
    void on_action_Open_triggered();
    void on_action_Undo_triggered();
    void on_action_Redo_triggered();
    void on_actionCu_t_triggered();
    void on_action_Copy_triggered();
    void on_action_Paste_triggered();
    void on_actionSelect_All_triggered();
    void on_action_Delete_triggered();
    void on_actionClose_triggered();
    void on_actionC_lose_All_triggered();
    void on_actionRestore_Default_Zoom_triggered();
    void on_actionAbout_Notepadqq_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionLaunch_in_Firefox_triggered();
    void on_actionGet_php_help_triggered();
    void on_actionLaunch_in_Chromium_triggered();
    void on_actionGoogle_Search_triggered();
    void on_actionWikipedia_Search_triggered();
    void on_actionCurrent_Full_File_path_to_Clipboard_triggered();
    void on_actionCurrent_Filename_to_Clipboard_triggered();
    void on_actionCurrent_Directory_Path_to_Clipboard_triggered();
    void on_actionSave_All_triggered();
    void on_actionE_xit_triggered();
    void on_actionClose_All_BUT_Current_Document_triggered();
    void on_actionClone_to_Other_View_triggered();
    void on_actionSearch_triggered();
    void on_actionFind_Next_triggered();
    void on_actionFind_Previous_triggered();
    void on_actionSave_a_Copy_As_triggered();
    void on_actionWord_wrap_triggered();
    void on_actionShow_All_Characters_triggered();
    void on_actionUNIX_Format_triggered();
    void on_actionMac_Format_triggered();
    void on_actionWindows_Format_triggered();
    void on_actionReload_from_Disk_triggered();
    void on_actionUPPERCASE_triggered();
    void on_actionLowercase_triggered();
    void on_actionShow_End_of_Line_triggered();
    void on_actionShow_White_Space_and_TAB_triggered();
    void on_actionShow_Indent_Guide_triggered();
    void on_actionShow_Wrap_Symbol_triggered();
    void on_actionPreferences_triggered();

public slots:
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();

};


#endif // MAINWINDOW_H;
