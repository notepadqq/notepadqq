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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "frmabout.h"
#include "frmsrchreplace.h"
#include "searchengine.h"
#include "constants.h"
#include "generalfunctions.h"
#include "qtabwidgetscontainer.h"

#include <Qsci/qsciapis.h>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QSettings>
#include <QLabel>
#include <QMessageBox>
#include <QtGui/QCloseEvent>
#include <QClipboard>
#include <QtNetwork/QLocalSocket>
#include <QTextCodec>
#include <QTextDocument>
#include <QTemporaryFile>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>

MainWindow* MainWindow::wMain = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new QSettings();
    // "container" is the object that contains all the TabWidgets.
    container = new QTabWidgetsContainer(this);
    this->setCentralWidget(container);
    connect(container, SIGNAL(newQsciScintillaqqChildCreated(QsciScintillaqq*)), this, SLOT(_on_newQsciScintillaqqChildCreated(QsciScintillaqq*)));

//    encodeGroup = new QActionGroup(ui->menuEncoding);
//    ui->actionEncode_in_ANSI->setActionGroup(encodeGroup);
//    ui->actionEncode_in_UTF_8_without_BOM->setActionGroup(encodeGroup);
//    ui->actionEncode_in_UTF_8->setActionGroup(encodeGroup);
//    ui->actionEncode_in_UCS_2_Little_Endian->setActionGroup(encodeGroup);
//    ui->actionEncode_in_UCS_2_Big_Endian->setActionGroup(encodeGroup);

    // Let's assign icons to our actions
    ui->action_New->setIcon(QIcon::fromTheme("document-new", QIcon(ui->action_New->icon())));
    ui->action_Open->setIcon(QIcon::fromTheme("document-open", QIcon(ui->action_Open->icon())));
    ui->actionSave->setIcon(QIcon::fromTheme("document-save", QIcon(ui->actionSave->icon())));
    ui->actionSave_All->setIcon(QIcon::fromTheme("document-save-all", QIcon(ui->actionSave_All->icon())));
    ui->actionPrint_Now->setIcon(QIcon::fromTheme("document-print", QIcon(ui->actionPrint_Now->icon())));
    ui->actionCu_t->setIcon(QIcon::fromTheme("edit-cut", QIcon(ui->actionCu_t->icon())));
    ui->action_Copy->setIcon(QIcon::fromTheme("edit-copy", QIcon(ui->action_Copy->icon())));
    ui->action_Paste->setIcon(QIcon::fromTheme("edit-paste", QIcon(ui->action_Paste->icon())));
    ui->action_Undo->setIcon(QIcon::fromTheme("edit-undo", QIcon(ui->action_Undo->icon())));
    ui->action_Redo->setIcon(QIcon::fromTheme("edit-redo", QIcon(ui->action_Redo->icon())));
    ui->actionZoom_In->setIcon(QIcon::fromTheme("zoom-in", QIcon(ui->actionZoom_In->icon())));
    ui->actionZoom_Out->setIcon(QIcon::fromTheme("zoom-out", QIcon(ui->actionZoom_Out->icon())));
    ui->actionRestore_Default_Zoom->setIcon(QIcon::fromTheme("zoom-original", QIcon(ui->actionRestore_Default_Zoom->icon())));
    ui->action_Start_Recording->setIcon(QIcon::fromTheme("media-record", QIcon(ui->action_Start_Recording->icon())));
    ui->action_Stop_Recording->setIcon(QIcon::fromTheme("media-playback-stop", QIcon(ui->action_Stop_Recording->icon())));
    ui->action_Playback->setIcon(QIcon::fromTheme("media-playback-start", QIcon(ui->action_Playback->icon())));
    ui->actionRun_a_Macro_Multiple_Times->setIcon(QIcon::fromTheme("media-seek-forward", QIcon(ui->actionRun_a_Macro_Multiple_Times->icon())));
    ui->actionPreferences->setIcon(QIcon::fromTheme("preferences-other", QIcon(ui->actionPreferences->icon())));
    ui->actionSearch->setIcon(QIcon::fromTheme("edit-find",QIcon(ui->actionSearch->icon())));
    ui->actionFind_Next->setIcon(QIcon::fromTheme("go-next",QIcon(ui->actionFind_Next->icon())));
    ui->actionFind_Previous->setIcon(QIcon::fromTheme("go-previous",QIcon(ui->actionFind_Previous->icon())));


    // Context menu initialization
    tabContextMenu = new QMenu;
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    tabContextMenuActions.append(ui->actionClose);
    tabContextMenuActions.append(ui->actionClose_All_BUT_Current_Document);
    tabContextMenuActions.append(ui->actionSave);
    tabContextMenuActions.append(ui->actionSave_as);
    tabContextMenuActions.append(ui->actionRename);
    tabContextMenuActions.append(ui->actionDelete_from_Disk);
    tabContextMenuActions.append(ui->actionPrint);
    tabContextMenuActions.append(separator);
    tabContextMenuActions.append(ui->actionMove_to_Other_View);
    tabContextMenuActions.append(ui->actionClone_to_Other_View);
    tabContextMenuActions.append(ui->actionMove_to_New_Instance);
    tabContextMenuActions.append(ui->actionOpen_in_New_Instance);
    tabContextMenu->addActions(tabContextMenuActions);

    // Ok, now it's time to create or first tabWidget.
    QTabWidgetqq *firstTabWidget = new QTabWidgetqq(ui->centralWidget->parentWidget());
    // The first document in the tabWidget will be created by processCommandLineArgs()
    // Now we connect the corresponding signals to our slots.
    connect(firstTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(_on_tab_close_requested(int)));
    connect(firstTabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(_on_tabWidget_customContextMenuRequested(QPoint)));

    container->addWidget(firstTabWidget);

    if(this->layoutDirection() == Qt::RightToLeft) {
        ui->actionText_Direction_RTL->setChecked(true);
    }

    processCommandLineArgs(QApplication::arguments(), false);

    instanceServer = new QLocalServer(this);
    connect(instanceServer, SIGNAL(newConnection()), SLOT(_on_instanceServer_NewConnection()));
    instanceServer->removeServer(INSTANCESERVER_ID);
    instanceServer->listen(INSTANCESERVER_ID);

    // Focus on the editor of the first tab
    container->QTabWidgetqqAt(0)->focusQSciScintillaqq()->setFocus();

    // Build the search dialog for later use
    searchDialog = new frmsrchreplace(this);
    searchDialog->hide();
    se = new searchengine();

    //Document monitoring,saving,loading engine for centralized document management.
    de = new docengine(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow* MainWindow::instance()
{
    if(!wMain){
        wMain = new MainWindow();
    }
    return wMain;
}


void MainWindow::processCommandLineArgs(QStringList arguments, bool fromExternalMessage)
{
    bool activateWindow = false;

    if(arguments.count() <= 1)
    {
        QTabWidgetqq *focusedTabWidget = container->focusQTabWidgetqq();
        focusedTabWidget->addNewDocument();
        activateWindow = true;
    }
    else
    {
        // Open selected files
        QStringList files;
        for(int i = 1; i < arguments.count(); i++)
        {
            files.append(arguments.at(i));
        }
        de->loadDocuments(files, container->focusQTabWidgetqq());
        activateWindow = true;
    }

    if(fromExternalMessage && activateWindow)
    {
        // Activate the window
        this->activateWindow();
        this->raise();
    }
}

// Custom context menu for right-clicks on the tab title
void MainWindow::_on_tabWidget_customContextMenuRequested(QPoint pos)
{
    QTabWidgetqq *_tabWidget = static_cast<QTabWidgetqq *>(sender());
    int index = _tabWidget->getTabIndexAt(pos);

    if(index != -1)
    {
        _tabWidget->setCurrentIndex(index);
        tabContextMenu->exec(_tabWidget->mapToGlobal(pos));
    }
}

void MainWindow::on_action_New_triggered()
{
    QTabWidgetqq *focusedTabWidget = container->focusQTabWidgetqq();
    focusedTabWidget->addNewDocument();
}

void MainWindow::_on_text_changed()
{

}

int MainWindow::kindlyTabClose(QsciScintillaqq *sci)
{
    int result = MainWindow::tabCloseResult_AlreadySaved;
    QTabWidgetqq *_tabWidget = sci->getTabWidget();
    int index = sci->getTabIndex();
    // Don't remove the tab if it's the last tab, it's empty, in an unmodified state and it's not associated with a file name.
    // Else, continue.
    if(!(_tabWidget->count() == 1
         && sci->isNewEmptyDocument())) {

        if(_tabWidget->QSciScintillaqqAt(index)->isModified()) {
            _tabWidget->setCurrentIndex(index);
            int ret = askIfWantToSave(sci, askToSaveChangesReason_tabClosing);
            if(ret == QMessageBox::Save) {
                if(save(sci) == MainWindow::saveFileResult_Canceled)
                {
                    // hmm... the user canceled the "save dialog". Let's ignore the close event.
                    result = MainWindow::tabCloseResult_Canceled;
                } else if(save(sci) == MainWindow::saveFileResult_Saved)
                {
                    _tabWidget->removeTab(index);
                    result = MainWindow::tabCloseResult_Saved;
                }
            } else if(ret == QMessageBox::Discard) {
                // Don't save
                _tabWidget->removeTab(index);
                result = MainWindow::tabCloseResult_NotSaved;
            } else if(ret == QMessageBox::Cancel) {
                // Don't save and don't close
                result = MainWindow::tabCloseResult_Canceled;
            }
        } else {
            // It's already saved: we can remove it safely.
            _tabWidget->removeTab(index);
            result = MainWindow::tabCloseResult_AlreadySaved;
        }
    }

    if(_tabWidget->count() == 0) {
        /* Not so good... 0 tabs opened is a bad idea
         * So, if there are more than one TabWidgets opened (split-screen)
         * then we completely remove this one. Otherwise, we add a new empty tab.
        */
        if(container->count() > 1) {
            _tabWidget->deleteLater();
        } else {
            _tabWidget->addNewDocument();
        }
    }

    return result;
}

int MainWindow::_on_tab_close_requested(int index)
{
    QTabWidgetqq *_tabWidget = static_cast<QTabWidgetqq *>(sender());
    QsciScintillaqq *sci = _tabWidget->QSciScintillaqqAt(index);

    //Clean up document monitoring, if possible
    QString filePath(sci->fileName());
    int retval = kindlyTabClose(sci);
    switch(retval) {
        case MainWindow::tabCloseResult_Saved:
        case MainWindow::tabCloseResult_NotSaved:
        case MainWindow::tabCloseResult_AlreadySaved:
            de->removeDocument(filePath);
            break;
        default:
            break;
    }
    return retval;
}

int MainWindow::askIfWantToSave(QsciScintillaqq *sci, int reason)
{
    QMessageBox msgBox;
    QString file;
    QTabWidgetqq *tabWidget = sci->getTabWidget();

    if (sci->fileName() == "")
    {
        file = Qt::escape(tabWidget->tabText(sci->getTabIndex()));
    } else {
        file = Qt::escape(QFileInfo(sci->fileName()).fileName());
    }
    msgBox.setWindowTitle(QCoreApplication::applicationName());

    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch(reason)
    {
    case askToSaveChangesReason_generic:
        msgBox.setText("<h3>" + tr("Do you want to save changes to «%1»?").arg(file) + "</h3>");
        msgBox.setButtonText(QMessageBox::Discard, tr("Don't Save"));
        break;
    case askToSaveChangesReason_tabClosing:
        msgBox.setText("<h3>" + tr("Do you want to save changes to «%1» before closing?").arg(file) + "</h3>");
        break;
    }

    msgBox.setInformativeText(tr("If you don't save the changes you made, you'll lose them forever."));
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setEscapeButton(QMessageBox::Cancel);

    QPixmap img = QIcon::fromTheme("document-save", QIcon(ui->actionSave->icon())).pixmap(64,64).scaled(64,64,Qt::KeepAspectRatio, Qt::SmoothTransformation);
    msgBox.setIconPixmap(img);

    msgBox.exec();

    return msgBox.standardButton(msgBox.clickedButton());
}

/**
* Saves the document located at the specified index. If the document is not already associated with a file name, ask for it.
*
* @param index the index of the document to save
*
* @return an integer value from enum MainWindow::saveFileResult.
* @see MainWindow::on_actionSave_as_triggered()
* @see MainWindow::writeDocument()
*/
int MainWindow::save(QsciScintillaqq *sci)
{
    if(sci->fileName() == "")
    {
        // Call "save as"
        return saveAs(sci);
    } else {
        return de->saveDocument(sci,sci->fileName());
    }
}

int MainWindow::saveAs(QsciScintillaqq *sci,bool copy)
{
    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(0, tr("Save as"), getSaveDialogDefaultFileName(sci), tr("Any file (*)"), 0, 0);
    if (filename != "") {
        settings->setValue("lastSelectedDir", QFileInfo(filename).absolutePath());
        // Write
        return de->saveDocument(sci,filename,copy);
    } else {
        return MainWindow::saveFileResult_Canceled;
    }
}

QString MainWindow::getSaveDialogDefaultFileName(QsciScintillaqq *sci)
{
    QString docFileName = sci->fileName();
    QTabWidgetqq *tabWidget = sci->getTabWidget();

    if(docFileName == "") {
        return settings->value("lastSelectedDir", ".").toString() + "/" + tabWidget->tabText(sci->getTabIndex());
    } else {
        return docFileName;
    }
}

/**
* Writes the document's text located at the specified index.
*
* @param index the index of the document
* @param filename the file name to use
* @param updateFileName if true, updates the file name associated with the respective tab. Tab title and tooltip are updated too.
*
* @return an integer value from enum MainWindow::saveFileResult.
*/
//int MainWindow::writeDocument(QsciScintillaqq *sci, QString filename)
//{
//    QTabWidgetqq *tabWidget = sci->getTabWidget();
//    if(!de->saveDocument(sci,filename)) {
//        return MainWindow::saveFileResult_Canceled;
//    }

//    return MainWindow::saveFileResult_Saved;
//}

void MainWindow::on_actionSave_as_triggered()
{
    saveAs(container->focusQTabWidgetqq()->focusQSciScintillaqq());
}

void MainWindow::on_actionSave_a_Copy_As_triggered()
{
    saveAs(container->focusQTabWidgetqq()->focusQSciScintillaqq(),true);
}

void MainWindow::on_actionSave_triggered()
{
    save(container->focusQTabWidgetqq()->focusQSciScintillaqq());
}


void MainWindow::on_action_Open_triggered()
{
    // Ask for file names...
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open"), settings->value("lastSelectedDir", ".").toString(), tr("All files (*)"), 0, 0);
    // A patch for bug #760308
    QWidget* foc = focusWidget();
    de->loadDocuments(fileNames, container->focusQTabWidgetqq());
    foc->setFocus();
}

void MainWindow::on_action_Undo_triggered()
{
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->undo();
}

void MainWindow::on_action_Redo_triggered()
{
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->redo();
}

void MainWindow::on_actionCu_t_triggered()
{
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->cut();
}

void MainWindow::on_action_Copy_triggered()
{
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->copy();
}

void MainWindow::on_action_Paste_triggered()
{
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->paste();
}

void MainWindow::on_actionSelect_All_triggered()
{
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->selectAll(true);
}

void MainWindow::on_action_Delete_triggered()
{
    QsciScintillaqq *sci = container->focusQTabWidgetqq()->focusQSciScintillaqq();
    int lineFrom, indexFrom;
    sci->getCursorPosition(&lineFrom, &indexFrom);

    if(sci->selectedText().length() == 0)
    {
        // If nothing is selected, select next letter
        int pos = sci->positionFromLineIndex(lineFrom, indexFrom);
        int lineTo, indexTo;
        sci->lineIndexFromPosition(++pos, &lineTo, &indexTo);
        sci->setSelection(lineFrom, indexFrom, lineTo, indexTo);
    }

    // Remove all the selected text
    sci->removeSelectedText();
}

void MainWindow::on_actionClose_triggered()
{
    kindlyTabClose(container->focusQTabWidgetqq()->focusQSciScintillaqq());
}

void MainWindow::on_actionC_lose_All_triggered()
{
    for(int i = 0; i < container->count(); i++) {
        QTabWidgetqq *tabWidget = container->QTabWidgetqqAt(i);
        int tab_count = tabWidget->count();
        for(int j = 0; j < tab_count; j++) {
            // Always remove the tab at position 0, 'cause we're sure there's always something at pos 0.
            QsciScintillaqq *sci = tabWidget->QSciScintillaqqAt(0);
            int result = kindlyTabClose(sci);
            if(result == MainWindow::tabCloseResult_Canceled)
            {
                // Abort! Stop everything.
                goto End;
            }
        }
    }

    End:
        return;
}

void MainWindow::on_actionZoom_In_triggered()
{
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->zoomIn();
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->syncZoom();
}

void MainWindow::on_actionZoom_Out_triggered()
{
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->zoomOut();
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->syncZoom();
}

void MainWindow::on_actionRestore_Default_Zoom_triggered()
{
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->zoomTo(0);
    container->focusQTabWidgetqq()->focusQSciScintillaqq()->syncZoom();
}

void MainWindow::on_actionAbout_Notepadqq_triggered()
{
    frmAbout *_about;
    _about = new frmAbout(this);
    _about->exec();

    _about->deleteLater();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    for(int i = 0; i < container->count(); i++) {
        QTabWidgetqq *tabWidget = container->QTabWidgetqqAt(i);
        int tab_count = tabWidget->count();
        for(int j = 0; j < tab_count; j++) {
            QsciScintillaqq *sci = tabWidget->QSciScintillaqqAt(j);
            if(sci->isModified()) {
                tabWidget->setCurrentIndex(j);
                sci->setFocus();
                int ret = askIfWantToSave(sci, askToSaveChangesReason_tabClosing);
                if(ret == QMessageBox::Save) {
                    if(save(sci) == MainWindow::saveFileResult_Canceled)
                    {
                        // hmm... the user canceled the "save dialog". Let's stop the close event.
                        event->ignore();
                        break;
                    }
                } else if(ret == QMessageBox::Discard) {
                    // Don't save
                } else if(ret == QMessageBox::Cancel) {
                    event->ignore();
                    break;
                }
            }
        }
    }
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_actionLaunch_in_Firefox_triggered()
{
    QProcess::startDetached("firefox", QStringList(container->focusQTabWidgetqq()->focusQSciScintillaqq()->fileName()));
}

void MainWindow::on_actionGet_php_help_triggered()
{
    QDesktopServices::openUrl(QUrl("http://php.net/" + QUrl::toPercentEncoding(container->focusQTabWidgetqq()->focusQSciScintillaqq()->selectedText())));
}

void MainWindow::on_actionLaunch_in_Chromium_triggered()
{
    QProcess::startDetached("chromium-browser", QStringList(container->focusQTabWidgetqq()->focusQSciScintillaqq()->fileName()));
}

void MainWindow::on_actionGoogle_Search_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.google.com/search?q=" + QUrl::toPercentEncoding(container->focusQTabWidgetqq()->focusQSciScintillaqq()->selectedText())));
}

void MainWindow::on_actionWikipedia_Search_triggered()
{
    QDesktopServices::openUrl(QUrl("http://en.wikipedia.org/w/index.php?title=Special%3ASearch&search=" + QUrl::toPercentEncoding(container->focusQTabWidgetqq()->focusQSciScintillaqq()->selectedText())));
}

void MainWindow::on_actionCurrent_Full_File_path_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = container->focusQTabWidgetqq()->focusQSciScintillaqq();
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).absoluteFilePath());
    } else {
        QApplication::clipboard()->setText(sci->getTabWidget()->tabText(sci->getTabIndex()));
    }
}

void MainWindow::on_actionCurrent_Filename_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = container->focusQTabWidgetqq()->focusQSciScintillaqq();
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).fileName());
    } else {
        QApplication::clipboard()->setText(sci->getTabWidget()->tabText(sci->getTabIndex()));
    }
}

void MainWindow::on_actionCurrent_Directory_Path_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = container->focusQTabWidgetqq()->focusQSciScintillaqq();
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).absolutePath());
    } else {
        QApplication::clipboard()->setText("");
    }
}

void MainWindow::on_actionSave_All_triggered()
{
    for(int i = 0; i < container->count(); i++) {
        QTabWidgetqq *tabWidget = container->QTabWidgetqqAt(i);
        int tab_count = tabWidget->count();
        for(int j = 0; j < tab_count; j++) {
            QsciScintillaqq *sci = tabWidget->QSciScintillaqqAt(j);
            save(sci);
        }
    }
}

void MainWindow::_on_instanceServer_NewConnection()
{
    while(instanceServer->hasPendingConnections())
    {
        QLocalSocket *client = instanceServer->nextPendingConnection();
        connect(client, SIGNAL(readyRead()), SLOT(_on_instanceServer_Socket_ReadyRead()));
    }
}

void MainWindow::_on_instanceServer_Socket_ReadyRead()
{
    QLocalSocket *instanceSocket = static_cast<QLocalSocket*>(sender());
    QString message = "";
    QByteArray ar_raw;

    if(instanceSocket->bytesAvailable() > 0) {
        ar_raw = instanceSocket->readAll();
        message = QString::fromUtf8(ar_raw);


        if(message == "NEW_CLIENT")
        {
            instanceSocket->write(QString("HELLO").toUtf8());
            instanceSocket->waitForBytesWritten(400);
        }
        else
        {
            // Command line args
            QDataStream args(&ar_raw,QIODevice::ReadOnly);
            QStringList remote_args;
            args >> remote_args;
            processCommandLineArgs(remote_args, true);
        }
    }
}

void MainWindow::_on_newQsciScintillaqqChildCreated(QsciScintillaqq *sci)
{
    connect(sci, SIGNAL(copyAvailable(bool)), this, SLOT(_on_sci_copyAvailable(bool)));
    connect(sci, SIGNAL(SCN_UPDATEUI(int)), this, SLOT(_on_sci_updateUI()));
}

void MainWindow::_on_sci_copyAvailable(bool yes)
{
    // TODO Call me on every tab switch!!
    QsciScintillaqq *sci = static_cast<QsciScintillaqq *>(sender());
    if(sci->hasFocus()) {
        ui->actionCu_t->setEnabled(yes);
        ui->action_Copy->setEnabled(yes);
    }
}

void MainWindow::_on_sci_updateUI()
{
    QsciScintillaqq *sci = static_cast<QsciScintillaqq *>(sender());
    if(sci->hasFocus()) {
        //this->setWindowTitle(QString::number(QDateTime::currentDateTime().toTime_t()));
        ui->action_Undo->setEnabled(sci->SendScintilla(QsciScintillaBase::SCI_CANUNDO));
        ui->action_Redo->setEnabled(sci->SendScintilla(QsciScintillaBase::SCI_CANREDO));
        ui->action_Paste->setEnabled(sci->SendScintilla(QsciScintillaBase::SCI_CANPASTE));
    }
}

void MainWindow::on_actionE_xit_triggered()
{
    this->close();
}

void MainWindow::on_actionClose_All_BUT_Current_Document_triggered()
{
    QTabWidgetqq *keep_tabWidget = container->focusQTabWidgetqq();
    QsciScintillaqq* keep_sci = keep_tabWidget->focusQSciScintillaqq();

    for(int i = container->count() - 1; i >= 0; i--) {
        QTabWidgetqq *cur_tabwidget = container->QTabWidgetqqAt(i);
        if(cur_tabwidget == keep_tabWidget) {

            for(int j = cur_tabwidget->count() - 1; j >= 0; j--) {
                QsciScintillaqq *cur_sci = cur_tabwidget->QSciScintillaqqAt(j);
                if(cur_sci == keep_sci)
                {
                    // Do nothing
                } else {
                    cur_tabwidget->setCurrentIndex(j);
                    cur_sci->setFocus();
                    int result = kindlyTabClose(cur_sci);
                    if(result == MainWindow::tabCloseResult_Canceled) {
                        goto Dengo;
                    }
                }
            }

        } else {

            for(int j = cur_tabwidget->count() - 1; j >= 0; j--) {
                QsciScintillaqq *cur_sci = cur_tabwidget->QSciScintillaqqAt(j);
                int result = kindlyTabClose(cur_sci);
                if(result == MainWindow::tabCloseResult_Canceled) {
                    goto Dengo;
                }
            }

            cur_tabwidget->deleteLater();
        }
    }

    // Screw good practice. How bad can it be?
    // http://xkcd.com/292/
    Dengo:
        return;
}

void MainWindow::on_actionClone_to_Other_View_triggered()
{
    QWidget *cur_widget = container->focusQTabWidgetqq()->currentWidget();
    //QsciScintillaqq* focus_sci = container->focusQTabWidgetqq()->focusQSciScintillaqq();
    if(container->focusQTabWidgetqq() == container->QTabWidgetqqAt(0)) {
        if(container->count() < 2) {
            QTabWidgetqq *tabWidget = container->addQTabWidgetqq();
            connect_tabWidget(tabWidget);

            //int index = tabWidget->addEditorTab(true, "???");
            //tabWidget->addTab(cur_widget, "gmm");
            //QsciScintillaqq* sci = tabWidget->QSciScintillaqqAt(index);
        } else {
            //
        }
    }
}

void MainWindow::on_actionSearch_triggered()
{
    searchDialog->show();
}

void MainWindow::connect_tabWidget(QTabWidgetqq *tabWidget)
{
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(_on_tab_close_requested(int)));
    connect(tabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(_on_tabWidget_customContextMenuRequested(QPoint)));
}

searchengine* MainWindow::getSearchEngine()
{
    return se;
}

void MainWindow::on_actionFind_Next_triggered()
{
    if(se->getForward()) {
        se->findString();
    }else {
        se->setForward(true);
        se->setNewSearch(true);
        se->findString();
    }
}

void MainWindow::on_actionFind_Previous_triggered()
{
    if(!se->getForward()) {
        se->findString();
    }else {
        se->setForward(false);
        se->setNewSearch(true);
        se->findString();
    }
}

QSettings* MainWindow::getSettings()
{
    return settings;
}
