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
#include "qsciscintillaqq.h"
#include "qtabwidgetqq.h"
#include <QSettings>
#include <QLabel>
#include <QSplitter>
#include <QtNetwork/QLocalServer>
#include <QActionGroup>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QSplitter * mainSplitter;
    QTabWidgetqq * tabWidget1;
    QTabWidgetqq * tabWidget2;
    QsciScintillaqq * getCurrentTextBox(QTabWidgetqq * _tabWidget);
    QsciScintillaqq * getTextBoxFromIndex(int index, QTabWidgetqq * _tabWidget);
    QMenu * tabContextMenu;
    QAction * getSeparator(QObject *parent = 0);
    QList<QAction *> tabContextMenuActions;
    int addEditorTab(bool setFocus, QString name, QTabWidgetqq * _tabWidget);
    void autoLexer(QString filename, QsciScintillaqq* parent);
    QString fileFormatDescription(QString filename);
    void updateGui(int index, QTabWidgetqq * _tabWidget);
    bool isNewEmptyTab(int index);
    int writeDocument(int index, QString filename, bool updateFileName);
    int askIfWantToSave(int index);
    bool scintillaWasCreated(int index, QTabWidgetqq * _tabWidget);
    void openNewFile(QStringList fileNames);
    int save(int index);
    QString getSaveDialogDefaultFileName(int index);
    int getIndexFromWidget(QWidget &widget);
    QTabWidgetqq* tabWidgetFromObject(QObject * obj);
    QTabWidgetqq* lastActiveTabWidget();
    QTabWidgetqq* currentTabWidget;
    int getMultibyteTextLength(QTabWidgetqq * _tabWidget, int lineFrom, int indexFrom, int lineTo, int indexTo);
    void encodeIn(QString _enc, bool _bom, QsciScintillaqq * sci);
    void convertTo(QString _enc, bool _bom, QsciScintillaqq * sci);

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


private:
    Ui::MainWindow *ui;
    int newTabCount; // Number for the "new x" label on new documents' tabs.
    QSettings * settings;
    QLabel * statusBar_fileFormat;
    QLabel * statusBar_lengthInfo;
    QLabel * statusBar_selectionInfo;
    QLabel * statusBar_EOLstyle;
    QLabel * statusBar_textFormat;
    QLabel * statusBar_overtypeNotify;
    QLocalServer * instanceServer;
    void closeEvent(QCloseEvent *event);
    // QActionGroup *encodeGroup;

private slots:
    void on_actionEncode_in_UTF_16BE_UCS_2_Big_Endian_triggered();
    void on_actionEncode_in_UTF_8_triggered();
    void on_actionEncode_in_Windows_1252_triggered();
    void on_actionEncode_in_UTF_8_without_BOM_triggered();
    void on_actionISO_8859_6_triggered();
    void on_actionUTF_8_with_BOM_triggered();
    void on_actionUTF_16LE_triggered();
    void on_actionUTF_16BE_triggered();
    void on_actionWindows_1252_ANSI_triggered();
    void on_actionUTF_8_triggered();
    void on_actionLowercase_triggered();
    void on_actionUPPERCASE_triggered();
    void on_action_Start_Recording_triggered();
    void on_actionClone_to_Other_View_triggered();
    void on_actionRestore_Default_Zoom_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionZoom_In_triggered();
    void on_actionDecrease_Line_Indent_triggered();
    void on_actionIncrease_Line_Indent_triggered();
    void on_actionCurrent_Directory_Path_to_Clipboard_triggered();
    void on_actionCurrent_Filename_to_Clipboard_triggered();
    void on_actionCurrent_Full_File_path_to_Clipboard_triggered();
    void on_actionText_Direction_RTL_triggered();
    void on_actionShow_Indent_Guide_triggered();
    void on_actionShow_Wrap_Symbol_triggered();
    void on_actionWord_wrap_triggered();
    void on_actionShow_All_Characters_triggered();
    void on_actionShow_White_Space_and_TAB_triggered();
    void on_action_Delete_triggered();
    void on_action_Redo_triggered();
    void on_action_Undo_triggered();
    void on_actionShow_End_of_Line_triggered();
    void on_actionMac_Format_triggered();
    void on_actionUNIX_Format_triggered();
    void on_actionWindows_Format_triggered();
    void on_actionSelect_All_triggered();
    void on_actionCu_t_triggered();
    void on_action_Paste_triggered();
    void on_action_Copy_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionC_lose_All_triggered();
    void on_actionClose_All_BUT_Current_Document_triggered();
    void on_actionClose_triggered();
    void on_actionSave_All_triggered();
    void on_actionE_xit_triggered();
    int on_actionSave_a_Copy_As_triggered();
    void on_actionAbout_Notepadqq_triggered();
    int on_actionSave_triggered();
    void on_actionReload_from_Disk_triggered();
    int on_actionSave_as_triggered();
    void on_action_Open_triggered();
    void on_action_New_triggered();
    void fileChanged(const QString &path);
    void on_scintillaTextChanged();
    void on_scintillaSelectionChanged();
    void on_scintillaCursorPositionChanged(int line, int pos);
    void on_scintillaUpdateUI();
    void on_scintillaModificationChanged(bool m);
    void updateScintillaPropertiesForAllTabs();
    void updateOvertypeLabel();
    void instanceServerNewConnection();
    void instanceSocketReadyRead();
    void processCommandLineArgs(QStringList arguments, bool externalMessage);
    void clearHighlightOfSelection(int index);

    void on_tabWidget1_currentChanged(int index);
    void on_tabWidget2_currentChanged(int index);
    void on_tabWidgetX_currentChanged(int index, QTabWidgetqq * _tabWidget);
    int on_tabWidget1_tabCloseRequested(int index);
    int on_tabWidget2_tabCloseRequested(int index);
    int on_tabWidgetX_tabCloseRequested(int index, QTabWidgetqq * _tabWidget);
    void on_tabWidget1_customContextMenuRequested(QPoint pos);
    void on_tabWidget2_customContextMenuRequested(QPoint pos);
    void on_tabWidgetX_customContextMenuRequested(QPoint pos, QTabWidgetqq * _tabWidget);
};

#endif // MAINWINDOW_H;
