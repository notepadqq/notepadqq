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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new QSettings();

    container = new QTabWidgetsContainer(this);
    this->setCentralWidget(container);

//    encodeGroup = new QActionGroup(ui->menuEncoding);
//    ui->actionEncode_in_ANSI->setActionGroup(encodeGroup);
//    ui->actionEncode_in_UTF_8_without_BOM->setActionGroup(encodeGroup);
//    ui->actionEncode_in_UTF_8->setActionGroup(encodeGroup);
//    ui->actionEncode_in_UCS_2_Little_Endian->setActionGroup(encodeGroup);
//    ui->actionEncode_in_UCS_2_Big_Endian->setActionGroup(encodeGroup);

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

    QTabWidgetqq *mainTabWidget = new QTabWidgetqq(ui->centralWidget->parentWidget());
    int new_doc = mainTabWidget->addNewDocument();
    connect(mainTabWidget->QSciScintillaqqAt(new_doc), SIGNAL(textChanged()), this, SLOT(on_text_changed()));
    connect(mainTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(on_tab_close_requested(int)));

    container->addWidget(mainTabWidget);

    // Focus on the editor of the first tab
    container->QTabWidgetqqAt(0)->focusQSciScintillaqq()->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_New_triggered()
{
    QTabWidgetqq *focusedTabWidget = container->focusQTabWidgetqq();
    focusedTabWidget->addNewDocument();
}

void MainWindow::on_text_changed()
{

}

int MainWindow::on_tab_close_requested(int index)
{
    int result = MainWindow::tabCloseResult_AlreadySaved;
    QTabWidgetqq *_tabWidget = static_cast<QTabWidgetqq *>(sender());
    QsciScintillaqq *sci = _tabWidget->QSciScintillaqqAt(index);
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
        // Not so good... 0 tabs opened is a bad idea
        ui->action_New->trigger();
    }

    return result;
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
        return writeDocument(sci, sci->fileName(), true);
    }
}

int MainWindow::saveAs(QsciScintillaqq *sci)
{
    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(0, tr("Save as"), getSaveDialogDefaultFileName(sci), tr("Any file (*)"), 0, 0);
    if (filename != "") {
        settings->setValue("lastSelectedDir", QFileInfo(filename).absolutePath());
        // Write
        return writeDocument(sci, filename, true);
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
int MainWindow::writeDocument(QsciScintillaqq *sci, QString filename, bool updateFileName)
{
    QTabWidgetqq *tabWidget = sci->getTabWidget();
    bool retry = true;
    do
    {
        retry = false;
        sci->setFileWatchEnabled(false); //*********************************** <---------------------
        QFile file(filename);
        QFileInfo fi(file);

        if(!sci->write(&file)) {
            // Manage error
            QMessageBox msgBox;
            msgBox.setWindowTitle(QCoreApplication::applicationName());
            msgBox.setText(tr("Error trying to write to \"%1\"").arg(file.fileName()));
            msgBox.setDetailedText(file.errorString());
            msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry);
            msgBox.setDefaultButton(QMessageBox::Retry);
            msgBox.setIcon(QMessageBox::Critical);
            int ret = msgBox.exec();
            if(ret == QMessageBox::Abort) {
                return MainWindow::saveFileResult_Canceled;
                break;
            } else if(ret == QMessageBox::Retry) {
                retry = true;
                continue;
            }
        }

        if(updateFileName) // TODO Use signal
        {
            // Update document's filename
            sci->setFileName(fi.absoluteFilePath());
            sci->setModified(false);
            tabWidget->setTabToolTip(sci->getTabIndex(), sci->fileName());
            sci->autoSyntaxHighlight();

            // Update tab text
            tabWidget->setTabText(sci->getTabIndex(), fi.fileName());
        }
        //updateGui(sci->getTabIndex(), tabWidget1);

        file.close();
        sci->setFileWatchEnabled(true); //******************************** <-------------
    } while (retry);

    return MainWindow::saveFileResult_Saved;
}

void MainWindow::on_actionSave_as_triggered()
{
    saveAs(container->focusQTabWidgetqq()->focusQSciScintillaqq());
}

void MainWindow::on_actionSave_triggered()
{
    save(container->focusQTabWidgetqq()->focusQSciScintillaqq());
}

void MainWindow::openDocuments(QStringList fileNames, QTabWidgetqq *tabWidget)
{
    if(!fileNames.isEmpty())
    {
        settings->setValue("lastSelectedDir", QFileInfo(fileNames[0]).absolutePath());
        // Ok, now open our files
        for (int i = 0; i < fileNames.count(); i++) {

            QFile file(fileNames[i]);
            QFileInfo fi(fileNames[i]);

            int index = tabWidget->addEditorTab(true, fi.fileName());
            QsciScintillaqq* sci = tabWidget->QSciScintillaqqAt(index);

            sci->encoding = generalFunctions::getFileEncoding(fi.absoluteFilePath());

            if (!sci->read(&file, sci->encoding)) {
                // Manage error
                QMessageBox msgBox;
                msgBox.setWindowTitle(QCoreApplication::applicationName());
                msgBox.setText(tr("Error trying to open \"%1\"").arg(fi.fileName()));
                msgBox.setDetailedText(file.errorString());
                msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
                msgBox.setDefaultButton(QMessageBox::Retry);
                msgBox.setIcon(QMessageBox::Critical);
                int ret = msgBox.exec();
                if(ret == QMessageBox::Abort) {
                    tabWidget->removeTab(index);
                    break;
                } else if(ret == QMessageBox::Retry) {
                    tabWidget->removeTab(index);
                    i--;
                    continue;
                } else if(ret == QMessageBox::Ignore) {
                    tabWidget->removeTab(index);
                    continue;
                }
            }

            // If there was only a new empty tab opened, remove it
            if(tabWidget->count() == 2 && tabWidget->QSciScintillaqqAt(0)->isNewEmptyDocument()) {
                tabWidget->removeTab(0);
                index--;
            }

            sci->setFileName(fi.absoluteFilePath());
            sci->setEolMode(sci->guessEolMode());
            sci->setModified(false);
            tabWidget->setTabToolTip(index, sci->fileName());
            sci->autoSyntaxHighlight();

            // updateGui(index, tabWidget1);

            file.close();

            sci->setFocus(Qt::OtherFocusReason);
            //tabWidget1->setFocus();
            //tabWidget1->currentWidget()->setFocus();
        }
    }
}

void MainWindow::on_action_Open_triggered()
{
    // Ask for file names...
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open"), settings->value("lastSelectedDir", ".").toString(), tr("All files (*)"), 0, 0);
    // Set focus here (bug #760308)
    openDocuments(fileNames, container->focusQTabWidgetqq());
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
