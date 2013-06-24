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
#include "appwidesettings.h"

#include <assert.h>

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
#include <Qsci/qsciapis.h>

MainWindow* MainWindow::wMain = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    system_monospace = NULL;
    lexer_factory    = new LexerFactory();

    settings = new QSettings();
    // "container" is the object that contains all the TabWidgets.
    container = new QTabWidgetsContainer(this);
    this->setCentralWidget(container);
    connect(container, SIGNAL(newQsciScintillaqqChildCreated(QsciScintillaqq*)), this, SLOT(_on_newQsciScintillaqqChildCreated(QsciScintillaqq*)));

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

    QTabWidgetqq *firstTabWidget = new QTabWidgetqq(ui->centralWidget->parentWidget());
    container->addWidget(firstTabWidget);

    if(this->layoutDirection() == Qt::RightToLeft) {
        ui->actionText_Direction_RTL->setChecked(true);
    }

    //processCommandLineArgs(QApplication::arguments(), false);

    //Status bar creation
    createStatusBar();

    //Single instance management
    instanceServer = new QLocalServer(this);
    connect(instanceServer, SIGNAL(newConnection()), SLOT(_on_instanceServer_NewConnection()));
    instanceServer->removeServer(INSTANCESERVER_ID);
    instanceServer->listen(INSTANCESERVER_ID);

    //Search Management
    search_engine = new searchengine(this);
    form_search = 0; // Just create the search dialog as needed
    form_preferences = 0;

    //Document monitoring,saving,loading engine for centralized document management.
    document_engine = new docengine(this);

    // The first document in the tabWidget was created by processCommandLineArgs()
    // Now we connect the corresponding signals to our slots.
    connect_tabWidget(firstTabWidget);
}

MainWindow::~MainWindow()
{
    delete lexer_factory;
    delete search_engine;
    delete document_engine;
    delete system_monospace;
    delete ui;
}

void MainWindow::init()
{
    if ( !lexer_factory->init() )
        qDebug() << "cannot initialize lexer factory";
    else
        initialize_languages();
    update_appwide_ui("ALL");
    processCommandLineArgs(QApplication::arguments(), false);

    // Focus on the editor of the first tab
    container->QTabWidgetqqAt(0)->focusQSciScintillaqq()->setFocus();

    // GET SYSTEM FONTS USING DCONF
    QString mono_font_name = generalFunctions::readDConfKey("org.gnome.desktop.interface", "monospace-font-name");
    QString app_font_name  = generalFunctions::readDConfKey("org.gnome.desktop.interface", "font-name");
    qDebug() << "detected " << mono_font_name << " monospace font name";
    qDebug() << "detected " << app_font_name << " font name";

    QRegExp rx("([\\w\\s]+)\\s(\\d{1,3})");
    if ( !mono_font_name.isNull() && !mono_font_name.isEmpty() ) {
        if (rx.indexIn(mono_font_name, 0) != -1) {
            qDebug() << rx.cap(1) << rx.cap(2);
            system_monospace = new QFont(rx.cap(1), rx.cap(2).toInt());
        }
    }

    if (system_monospace == NULL)
        system_monospace = new QFont("Courier New", 10);

    widesettings::apply_monospace_font(system_monospace->family(), system_monospace->pointSize(), NULL);

    if ( !app_font_name.isNull() && !app_font_name.isEmpty() ) {
        if (rx.indexIn(app_font_name, 0) != -1) {
            qDebug() << rx.cap(1) << rx.cap(2);
            QFont system_font(rx.cap(1), rx.cap(2).toInt());
            qApp->setFont( system_font );
        }
    }
}

MainWindow* MainWindow::instance()
{
    // avoid calling MainWindow::instance() inside MainWindow constructor
    static bool recursive_call = false;
    assert( recursive_call == false );

    recursive_call = true;

    if(!wMain){
        wMain = new MainWindow();
    }

    recursive_call = false;

    return wMain;
}

QFont *MainWindow::systemMonospace()
{
    if ( system_monospace == NULL ) {
        ShrPtrWordsStyle global_overrides =
            getLexerFactory()->getGlobalStyler()->words_stylers_by_name.value(stylename::GLOBAL_OVERRIDE);
        system_monospace = new QFont( global_overrides->font_name, global_overrides->font_size );
    }
    return system_monospace;
}

void MainWindow::processCommandLineArgs(QStringList arguments, bool fromExternalMessage)
{
    bool activateWindow = false;

    if(arguments.count() <= 1)
    {
        QTabWidgetqq *focusedTabWidget = focused_tabWidget();
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
        document_engine->loadDocuments(files, focused_tabWidget());
        activateWindow = true;
    }

    if(fromExternalMessage && activateWindow)
    {
        // Activate the window
        this->activateWindow();
        this->raise();
    }
}

void MainWindow::initialize_languages()
{
    QActionGroup*       actionGroup = new QActionGroup(this);
    QHash<QString,QString>         list        = lexer_factory->languages();
    QMap<QChar,QMenu*>  submenu;

    //Build all the actions
    QHashIterator<QString,QString> lang(list);
    while(lang.hasNext()) {
        lang.next();
        QChar  letter = lang.key().at(0).toUpper();
        QMenu* current_letter = submenu.value(letter,0);
        if(!current_letter) {
            current_letter  = new QMenu(letter,this);
            submenu[letter] = current_letter;
        }
        QAction* newLang = actionGroup->addAction(lang.key());
        current_letter->addAction(newLang);
        newLang->setCheckable(true);
        connect(newLang,SIGNAL(triggered()),this,SLOT(_on_editor_language_set()));

    }

    QMapIterator<QChar,QMenu*> i(submenu);
    while(i.hasNext()) {
        i.next();
        ui->menu_Language->addMenu(i.value());
    }
}

void MainWindow::_on_editor_language_set()
{
    QsciScintillaqq* sci    = focused_editor();
    QAction*         action = qobject_cast<QAction*>(sender());
    if(!action) return;
    if(!sci) return;
    sci->setForcedLanguage(lexer_factory->languages().value(action->text()));
}

void MainWindow::createStatusBar()
{
    QStatusBar* status = this->statusBar();
    QLabel* label;

    status->setFixedHeight(22);

    label = new QLabel("File Format", this);
    label->setFrameShape(QFrame::StyledPanel);
    label->setMinimumWidth(160);
    status->addWidget(label);
    statusBar_fileFormat = label;

    label = new QLabel("Length : 0     Lines : 1", this);
    label->setFrameShape(QFrame::StyledPanel);
    status->addWidget(label);
    statusBar_lengthInfo = label;

    label = new QLabel("Ln : 0     Col : 1     Sel : 0 | 0",this);
    label->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    label->setFrameShape(QFrame::StyledPanel);
    status->addWidget(label);
    statusBar_selectionInfo = label;

    label = new QLabel("EOL",this);
    label->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);
    label->setMinimumWidth(128);
    label->setFrameShape(QFrame::StyledPanel);
    status->addWidget(label);
    statusBar_EOLstyle = label;

    label = new QLabel("Encoding",this);
    label->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);
    label->setMinimumWidth(128);
    label->setFrameShape(QFrame::StyledPanel);
    status->addWidget(label);
    statusBar_textFormat = label;

    label = new QLabel("INS",this);
    label->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);
    label->setMinimumWidth(40);
    label->setFrameShape(QFrame::StyledPanel);
    status->addWidget(label);
    statusBar_overtypeNotify = label;
}

void MainWindow::_on_editor_cursor_position_change(int line, int index)
{
    QsciScintillaqq*    sci = focused_editor();
    int selectionCharacters = 0;
    int selectionLines      = 0;
    int lineFrom = 0,indexFrom = 0, lineTo = 0, indexTo = 0;


    sci->getSelection(&lineFrom,&indexFrom,&lineTo,&indexTo);
    selectionLines      = std::abs(lineFrom-lineTo);
    selectionCharacters = sci->getSelectedTextCount();
    if(selectionCharacters > 0)selectionLines++;

    statusBar_selectionInfo->setText(tr("Ln : %1     Col : %2     Sel : %3 | %4").arg(line+1).arg(index+1).arg(selectionCharacters).arg(selectionLines));

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
    focused_tabWidget()->addNewDocument();
}

//Short-circuit function to get the currently active editor/scintilla widget
QsciScintillaqq* MainWindow::focused_editor()
{
    QsciScintillaqq* widget = focused_tabWidget()->focusQSciScintillaqq();
    if( !widget ) return 0;
    return widget;
}

QTabWidgetqq* MainWindow::focused_tabWidget()
{
    QTabWidgetqq* widget = container->focusQTabWidgetqq();
    if( !widget ) return 0;
    return widget;
}

int MainWindow::kindlyTabClose(QsciScintillaqq *sci)
{
    int result = MainWindow::tabCloseResult_AlreadySaved;
    QTabWidgetqq *_tabWidget = sci->tabWidget();
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
            document_engine->removeDocument(filePath);
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
    QTabWidgetqq *tabWidget = sci->tabWidget();

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
        return document_engine->saveDocument(sci,sci->fileName());
    }
}

int MainWindow::saveAs(QsciScintillaqq *sci,bool copy)
{
    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(0, tr("Save as"), getSaveDialogDefaultFileName(sci), tr("Any file (*)"), 0, 0);
    if (filename != "") {
        settings->setValue("lastSelectedDir", QFileInfo(filename).absolutePath());
        // Write
        return document_engine->saveDocument(sci,filename,copy);
    } else {
        return MainWindow::saveFileResult_Canceled;
    }
}

QString MainWindow::getSaveDialogDefaultFileName(QsciScintillaqq *sci)
{
    QString docFileName = sci->fileName();
    QTabWidgetqq *tabWidget = sci->tabWidget();

    if(docFileName == "") {
        return settings->value("lastSelectedDir", ".").toString() + "/" + tabWidget->tabText(sci->getTabIndex());
    } else {
        return docFileName;
    }
}

void MainWindow::on_actionSave_as_triggered()
{
    saveAs(focused_editor());
}

void MainWindow::on_actionSave_a_Copy_As_triggered()
{
    saveAs(focused_editor(),true);
}

void MainWindow::on_actionSave_triggered()
{
    save(focused_editor());
}


void MainWindow::on_action_Open_triggered()
{
    // Ask for file names...
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open"), settings->value("lastSelectedDir", ".").toString(), tr("All files (*)"), 0, 0);
    document_engine->loadDocuments(fileNames, focused_tabWidget());
}

void MainWindow::on_actionReload_from_Disk_triggered()
{
    QsciScintillaqq *sci = focused_editor();
    document_engine->loadDocuments(QStringList(sci->fileName()),sci->tabWidget(),true);
}

void MainWindow::on_action_Undo_triggered()
{
    focused_editor()->undo();
}

void MainWindow::on_action_Redo_triggered()
{
    focused_editor()->redo();
}

void MainWindow::on_actionCu_t_triggered()
{
    focused_editor()->safeCopy();
    focused_editor()->removeSelectedText();
}

void MainWindow::on_action_Copy_triggered()
{
    focused_editor()->safeCopy();
}

void MainWindow::on_action_Paste_triggered()
{
    focused_editor()->paste();
}

void MainWindow::on_actionSelect_All_triggered()
{
    focused_editor()->selectAll(true);
}

void MainWindow::on_action_Delete_triggered()
{
    QsciScintillaqq *sci = focused_editor();
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
    kindlyTabClose(focused_editor());
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
                return;
            }
        }
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
    focused_editor()->zoomIn();
    double zoomLevel = static_cast<double>(focused_editor()->SendScintilla(QsciScintilla::SCI_GETZOOM));
    widesettings::apply_zoom_level(zoomLevel, focused_editor());
}

void MainWindow::on_actionZoom_Out_triggered()
{
    focused_editor()->zoomOut();
    double zoomLevel = static_cast<double>(focused_editor()->SendScintilla(QsciScintilla::SCI_GETZOOM));
    widesettings::apply_zoom_level(zoomLevel, focused_editor());
}

void MainWindow::on_actionRestore_Default_Zoom_triggered()
{
    widesettings::apply_zoom_level(0,focused_editor());
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
    QProcess::startDetached("firefox", QStringList(focused_editor()->fileName()));
}

void MainWindow::on_actionGet_php_help_triggered()
{
    QDesktopServices::openUrl(QUrl("http://php.net/" + QUrl::toPercentEncoding(focused_editor()->selectedText())));
}

void MainWindow::on_actionLaunch_in_Chromium_triggered()
{
    QProcess::startDetached("chromium-browser", QStringList(focused_editor()->fileName()));
}

void MainWindow::on_actionGoogle_Search_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.google.com/search?q=" + QUrl::toPercentEncoding(focused_editor()->selectedText())));
}

void MainWindow::on_actionWikipedia_Search_triggered()
{
    QDesktopServices::openUrl(QUrl("http://en.wikipedia.org/w/index.php?title=Special%3ASearch&search=" + QUrl::toPercentEncoding(focused_editor()->selectedText())));
}

void MainWindow::on_actionCurrent_Full_File_path_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = focused_editor();
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).absoluteFilePath());
    } else {
        QApplication::clipboard()->setText(sci->tabWidget()->tabText(sci->getTabIndex()));
    }
}

void MainWindow::on_actionCurrent_Filename_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = focused_editor();
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).fileName());
    } else {
        QApplication::clipboard()->setText(sci->tabWidget()->tabText(sci->getTabIndex()));
    }
}

void MainWindow::on_actionCurrent_Directory_Path_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = focused_editor();
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
    connect(sci, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(_on_editor_cursor_position_change(int,int)));
    connect(sci, SIGNAL(overtypeChanged(bool)), this, SLOT(_on_editor_overtype_changed(bool)));
}

void MainWindow::_on_editor_overtype_changed(bool overtype)
{
    statusBar_overtypeNotify->setText(overtype ? "OVR" : "INS");
}

void MainWindow::_on_editor_new(int index)
{
    QsciScintillaqq* sci = focused_tabWidget()->QSciScintillaqqAt(index);
    if( !sci )
        return;
    widesettings::apply_settings(sci);
    widesettings::apply_single_document_settings(sci);
    update_single_document_ui(sci);
    sci->setModified(false);
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

    statusBar_lengthInfo->setText(tr("Length : %1     Lines : %2").arg(sci->length()).arg(sci->lines()));
}

void MainWindow::on_actionE_xit_triggered()
{
    this->close();
}

void MainWindow::on_actionClose_All_BUT_Current_Document_triggered()
{
    QTabWidgetqq *keep_tabWidget = focused_tabWidget();
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
                        return;
                    }
                }
            }

        } else {

            for(int j = cur_tabwidget->count() - 1; j >= 0; j--) {
                QsciScintillaqq *cur_sci = cur_tabwidget->QSciScintillaqqAt(j);
                int result = kindlyTabClose(cur_sci);
                if(result == MainWindow::tabCloseResult_Canceled) {
                    return;
                }
            }

            cur_tabwidget->deleteLater();
        }
    }
}

//Update UI elements for settings that apply across the board
void MainWindow::update_appwide_ui(const char* s)
{
    const char* ALL = "ALL";
    if(s == widesettings::SETTING_SHOW_ALL_CHARS||s == ALL)
        ui->actionShow_All_Characters->setChecked(settings->value(widesettings::SETTING_SHOW_ALL_CHARS,false).toBool());
    if(s == widesettings::SETTING_WRAP_MODE||s == ALL)
        ui->actionWord_wrap->setChecked(settings->value(widesettings::SETTING_WRAP_MODE,false).toBool());
    if(s == widesettings::SETTING_SHOW_END_OF_LINE||s == ALL)
        ui->actionShow_End_of_Line->setChecked(settings->value(widesettings::SETTING_SHOW_END_OF_LINE,false).toBool());
    if(s == widesettings::SETTING_SHOW_WHITE_SPACE||s == ALL)
        ui->actionShow_White_Space_and_TAB->setChecked(settings->value(widesettings::SETTING_SHOW_WHITE_SPACE,false).toBool());
    if(s == widesettings::SETTING_SHOW_INDENT_GUIDE||s == ALL)
        ui->actionShow_Indent_Guide->setChecked(settings->value(widesettings::SETTING_SHOW_INDENT_GUIDE,true).toBool());
    if(s == widesettings::SETTING_WRAP_SYMBOL||s == ALL)
        ui->actionShow_Wrap_Symbol->setChecked(settings->value(widesettings::SETTING_WRAP_SYMBOL,false).toBool());
    if(s == widesettings::SETTING_TABBAR_HIDE||s == ALL)
        container->setTabBarsHidden(settings->value(widesettings::SETTING_TABBAR_HIDE,false).toBool());
    if(s == widesettings::SETTING_TABBAR_VERTICAL||s == widesettings::SETTING_TABBAR_REDUCE||s == ALL)
        container->setTabBarsVertical(settings->value(widesettings::SETTING_TABBAR_VERTICAL,false).toBool());
    if(s == widesettings::SETTING_TABBAR_MOVABLE||s == ALL)
        container->setTabBarsMovable(settings->value(widesettings::SETTING_TABBAR_MOVABLE,true).toBool());
    if(s == widesettings::SETTING_TABBAR_HIGHLIGHT||s == ALL)
        container->setTabBarsHighlight(settings->value(widesettings::SETTING_TABBAR_HIGHLIGHT,true).toBool());
}

void MainWindow::on_actionClone_to_Other_View_triggered()
{
    //QsciScintillaqq* focus_sci = getFocusedEditor();
    if(focused_tabWidget() == container->QTabWidgetqqAt(0)) {
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
    if(!form_search) {
        form_search = new frmsrchreplace(this);
    }
    form_search->show();
}

void MainWindow::clearSearchDialog()
{
    form_search->deleteLater();
    form_search = 0;
}

void MainWindow::connect_tabWidget(QTabWidgetqq *tabWidget)
{
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(_on_tab_close_requested(int)));
    connect(tabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(_on_tabWidget_customContextMenuRequested(QPoint)));
    connect(tabWidget, SIGNAL(currentChanged(int)), SLOT(_apply_wide_settings_to_tab(int)));
    connect(tabWidget, SIGNAL(documentAdded(int)), this, SLOT(_on_editor_new(int)));
}

searchengine* MainWindow::getSearchEngine()
{
    return search_engine;
}

void MainWindow::on_actionFind_Next_triggered()
{
    if(search_engine->getForward()) {
        search_engine->findString();
    }else {
        search_engine->setForward(true);
        search_engine->setNewSearch(true);
        search_engine->findString();
    }
}

void MainWindow::on_actionFind_Previous_triggered()
{
    if(!search_engine->getForward()) {
        search_engine->findString();
    }else {
        search_engine->setForward(false);
        search_engine->setNewSearch(true);
        search_engine->findString();
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    if(!form_preferences)
        form_preferences = new frmpreferences(this);
    form_preferences->show();
}

QSettings* MainWindow::getSettings()
{
    return settings;
}

LexerFactory* MainWindow::getLexerFactory()
{
    return lexer_factory;
}

void MainWindow::on_actionWord_wrap_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = focused_editor();
    if ( !sci || !widesettings::toggle_word_wrap(sci) ) return;
}

void MainWindow::update_single_document_ui( QsciScintillaqq* sci )
{
    if(!sci) {
        return;
    }
    QsciScintilla::EolMode eol = sci->guessEolMode();
    if(eol == -1) {
        eol = static_cast<QsciScintilla::EolMode>(
                    settings->value(widesettings::SETTING_EOL_MODE,QsciScintilla::EolUnix).toInt() );
    }
    ui->actionWindows_Format->setChecked( (eol == QsciScintilla::EolWindows ) ? true : false );
    ui->actionWindows_Format->setDisabled( (eol == QsciScintilla::EolWindows ) ? true : false );
    ui->actionUNIX_Format->setChecked( (eol == QsciScintilla::EolUnix ) ? true : false);
    ui->actionUNIX_Format->setDisabled( (eol == QsciScintilla::EolUnix ) ? true : false );
    ui->actionMac_Format->setChecked( (eol == QsciScintilla::EolMac ) ? true : false);
    ui->actionMac_Format->setDisabled( (eol == QsciScintilla::EolMac ) ? true : false );

    switch(eol) {
    case QsciScintilla::EolWindows:
        statusBar_EOLstyle->setText(tr("DOS/Windows"));
        break;
    case QsciScintilla::EolUnix:
        statusBar_EOLstyle->setText(tr("UNIX"));
        break;
    case QsciScintilla::EolMac:
        statusBar_EOLstyle->setText(tr("Mac"));
        break;
     default:
        break;

    }

    QString fileType = generalFunctions::getFileType(sci->fileName());
    statusBar_fileFormat->setText(fileType);
    if((sci->encoding() == "UTF-8") && (!sci->BOM()) )
        statusBar_textFormat->setText("ANSI as UTF-8");
    else
        statusBar_textFormat->setText(sci->encoding());

    sci->updateLineMargin();
}

void MainWindow::_apply_wide_settings_to_tab( int index )
{
    qDebug() << "switched to tab " << index;

    QTabWidgetqq *_tabWidget = qobject_cast<QTabWidgetqq *>(sender());
    QsciScintillaqq *sci = _tabWidget->QSciScintillaqqAt(index);
    if ( !sci ) return;
    widesettings::apply_settings(sci);
    update_single_document_ui(sci);
}

void MainWindow::on_actionShow_All_Characters_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = focused_editor();
    if ( !sci || !widesettings::toggle_invisible_chars(sci) ) return;
    update_single_document_ui(sci);
    update_appwide_ui(widesettings::SETTING_SHOW_END_OF_LINE);
    update_appwide_ui(widesettings::SETTING_SHOW_WHITE_SPACE);
    update_appwide_ui(widesettings::SETTING_SHOW_ALL_CHARS);
}

void MainWindow::on_actionWindows_Format_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = focused_editor();
    if ( !sci || !widesettings::set_eol_mode(QsciScintilla::EolWindows, sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionMac_Format_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = focused_editor();
    if ( !sci || !widesettings::set_eol_mode(QsciScintilla::EolMac, sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionUNIX_Format_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = focused_editor();
    if ( !sci || !widesettings::set_eol_mode(QsciScintilla::EolUnix, sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionUPPERCASE_triggered()
{
    focused_editor()->SendScintilla(QsciScintilla::SCI_UPPERCASE);
}

void MainWindow::on_actionLowercase_triggered()
{
    focused_editor()->SendScintilla(QsciScintilla::SCI_LOWERCASE);
}

void MainWindow::on_actionShow_White_Space_and_TAB_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = focused_editor();
    if ( !sci || !widesettings::toggle_white_space(sci) ) return;
    update_single_document_ui(sci);
    update_appwide_ui(widesettings::SETTING_SHOW_ALL_CHARS);
}

void MainWindow::on_actionShow_End_of_Line_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = focused_editor();
    if ( !sci || !widesettings::toggle_end_of_line(sci) ) return;
    update_single_document_ui(sci);
    update_appwide_ui(widesettings::SETTING_SHOW_ALL_CHARS);

}

void MainWindow::on_actionShow_Indent_Guide_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = focused_editor();
    if ( !sci || !widesettings::toggle_indent_guide(sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionShow_Wrap_Symbol_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = focused_editor();
    if ( !sci || !widesettings::toggle_wrap_symbol(sci) ) return;
    update_single_document_ui(sci);
}
