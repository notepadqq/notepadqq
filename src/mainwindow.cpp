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
    container->addWidget(firstTabWidget);

    //Build the status bar.
    createStatusBar();

    if(this->layoutDirection() == Qt::RightToLeft) {
        ui->actionText_Direction_RTL->setChecked(true);
    }

    //processCommandLineArgs(QApplication::arguments(), false);

    instanceServer = new QLocalServer(this);
    connect(instanceServer, SIGNAL(newConnection()), SLOT(_on_instanceServer_NewConnection()));
    instanceServer->removeServer(INSTANCESERVER_ID);
    instanceServer->listen(INSTANCESERVER_ID);

    // Build the search dialog for later use
    searchDialog = new frmsrchreplace(this);
    searchDialog->hide();
    se = new searchengine();

    //Document monitoring,saving,loading engine for centralized document management.
    de = new docengine(this);

    // The first document in the tabWidget was created by processCommandLineArgs()
    // Now we connect the corresponding signals to our slots.
    connect_tabWidget(firstTabWidget);
}

MainWindow::~MainWindow()
{
    delete lexer_factory;
    delete system_monospace;
    delete ui;
}

void MainWindow::init()
{
    if ( !lexer_factory->init() )
        qDebug() << "cannot initialize lexer factory";
    else
        initLanguages();

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

    // APPLY WIDE SETTINGS TO ALL OPEN TABS
    for ( int i = 0; i < container->count(); ++i ) {
        QTabWidgetqq* tqq = qobject_cast<QTabWidgetqq*>( container->widget(i) );
        if ( !tqq ) continue;
        for ( int j = 0; j < tqq->count(); ++j ) {
            QsciScintillaqq* sqq = tqq->QSciScintillaqqAt(j);
            if ( !sqq ) continue;
            widesettings::apply_settings(sqq);
            widesettings::apply_single_document_settings(sqq);
            update_single_document_ui(sqq);

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

void MainWindow::initLanguages()
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
        connect(newLang,SIGNAL(triggered()),this,SLOT(setLanguage()));

    }

    QMapIterator<QChar,QMenu*> i(submenu);
    while(i.hasNext()) {
        i.next();
        ui->menu_Language->addMenu(i.value());
    }
}


//Really need a better way of doing this outside of pulling button text so the menus can look a little a better cosmetically....
//Probably QSignalMapper is the way to go here instead.
void MainWindow::setLanguage()
{
    QsciScintillaqq* sci    = getFocusedEditor();
    QAction*         action = qobject_cast<QAction*>(sender());
    if(!action) return;
    if(!sci) return;
    sci->setForcedLanguage(lexer_factory->languages().value(action->text()));
}

/*
void MainWindow::setLanguage(QString lang)
{

}*/

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
    label->setMinimumWidth(32);
    label->setFrameShape(QFrame::StyledPanel);
    status->addWidget(label);
    statusBar_overtypeNotify = label;
}

void MainWindow::_on_editor_cursor_position_change(int line, int index)
{
    QsciScintillaqq*    sci = getFocusedEditor();
    int selectionCharacters = 0;
    int selectionLines      = 0;
    int lineFrom = 0,indexFrom = 0, lineTo = 0, indexTo = 0;


    sci->getSelection(&lineFrom,&indexFrom,&lineTo,&indexTo);
    selectionLines      = std::abs(lineFrom-lineTo);
    selectionCharacters = selectionLines+std::abs(indexFrom-indexTo);
    if(selectionCharacters > 0)selectionLines++;

    statusBar_selectionInfo->setText(tr("Ln : %1     Col : %2     Sel : %3 | %4").arg(line+1).arg(index+1).arg(selectionCharacters).arg(selectionLines));

}

void MainWindow::_on_editor_keyrelease(QKeyEvent* e)
{
    QsciScintillaqq* sci = qobject_cast<QsciScintillaqq*>(sender());
    switch(e->key())
    {
    case Qt::Key_Insert:
        updateTypingMode(sci->overType());
        break;
    }
}

void MainWindow::updateTypingMode(bool yes)
{
    statusBar_overtypeNotify->setText(yes ? "OVR" : "INS");
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
    int index = focusedTabWidget->addNewDocument();
    QsciScintillaqq* sci = focusedTabWidget->QSciScintillaqqAt(index);
    if( !sci ) return;
    widesettings::apply_settings(sci);
    widesettings::apply_single_document_settings(sci);
    update_single_document_ui(sci);
}

void MainWindow::_on_text_changed()
{

}

//Short-circuit function to get the currently active editor/scintilla widget
QsciScintillaqq* MainWindow::getFocusedEditor()
{
    QsciScintillaqq* sci = container->focusQTabWidgetqq()->focusQSciScintillaqq();
    if( !sci ) return 0;
    return sci;
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
    QTabWidgetqq *tabWidget = sci->tabWidget();

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
    saveAs(getFocusedEditor());
}

void MainWindow::on_actionSave_a_Copy_As_triggered()
{
    saveAs(getFocusedEditor(),true);
}

void MainWindow::on_actionSave_triggered()
{
    save(getFocusedEditor());
}


void MainWindow::on_action_Open_triggered()
{
    // Ask for file names...
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open"), settings->value("lastSelectedDir", ".").toString(), tr("All files (*)"), 0, 0);
    de->loadDocuments(fileNames, container->focusQTabWidgetqq());

    update_single_document_ui(getFocusedEditor());
}

void MainWindow::on_actionReload_from_Disk_triggered()
{
    QsciScintillaqq *sci = getFocusedEditor();
    de->loadDocuments(QStringList(sci->fileName()),sci->tabWidget(),true);
}

void MainWindow::on_action_Undo_triggered()
{
    getFocusedEditor()->undo();
}

void MainWindow::on_action_Redo_triggered()
{
    getFocusedEditor()->redo();
}

void MainWindow::on_actionCu_t_triggered()
{
    getFocusedEditor()->safeCopy();
    getFocusedEditor()->removeSelectedText();
}

void MainWindow::on_action_Copy_triggered()
{
    getFocusedEditor()->safeCopy();
}

void MainWindow::on_action_Paste_triggered()
{
    getFocusedEditor()->paste();
}

void MainWindow::on_actionSelect_All_triggered()
{
    getFocusedEditor()->selectAll(true);
}

void MainWindow::on_action_Delete_triggered()
{
    QsciScintillaqq *sci = getFocusedEditor();
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
    kindlyTabClose(getFocusedEditor());
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
    getFocusedEditor()->zoomIn();
    double zoomLevel = static_cast<double>(getFocusedEditor()->SendScintilla(QsciScintilla::SCI_GETZOOM));
    widesettings::apply_zoom_level(zoomLevel, getFocusedEditor());
}

void MainWindow::on_actionZoom_Out_triggered()
{
    getFocusedEditor()->zoomOut();
    double zoomLevel = static_cast<double>(getFocusedEditor()->SendScintilla(QsciScintilla::SCI_GETZOOM));
    widesettings::apply_zoom_level(zoomLevel, getFocusedEditor());
}

void MainWindow::on_actionRestore_Default_Zoom_triggered()
{
    widesettings::apply_zoom_level(0,getFocusedEditor());
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
    QProcess::startDetached("firefox", QStringList(getFocusedEditor()->fileName()));
}

void MainWindow::on_actionGet_php_help_triggered()
{
    QDesktopServices::openUrl(QUrl("http://php.net/" + QUrl::toPercentEncoding(getFocusedEditor()->selectedText())));
}

void MainWindow::on_actionLaunch_in_Chromium_triggered()
{
    QProcess::startDetached("chromium-browser", QStringList(getFocusedEditor()->fileName()));
}

void MainWindow::on_actionGoogle_Search_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.google.com/search?q=" + QUrl::toPercentEncoding(getFocusedEditor()->selectedText())));
}

void MainWindow::on_actionWikipedia_Search_triggered()
{
    QDesktopServices::openUrl(QUrl("http://en.wikipedia.org/w/index.php?title=Special%3ASearch&search=" + QUrl::toPercentEncoding(getFocusedEditor()->selectedText())));
}

void MainWindow::on_actionCurrent_Full_File_path_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = getFocusedEditor();
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).absoluteFilePath());
    } else {
        QApplication::clipboard()->setText(sci->tabWidget()->tabText(sci->getTabIndex()));
    }
}

void MainWindow::on_actionCurrent_Filename_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = getFocusedEditor();
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).fileName());
    } else {
        QApplication::clipboard()->setText(sci->tabWidget()->tabText(sci->getTabIndex()));
    }
}

void MainWindow::on_actionCurrent_Directory_Path_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = getFocusedEditor();
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
    connect(sci,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(_on_editor_cursor_position_change(int,int)));
    connect(sci,SIGNAL(keyReleased(QKeyEvent*)),this,SLOT(_on_editor_keyrelease(QKeyEvent*)));
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
    //QsciScintillaqq* focus_sci = getFocusedEditor();
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
    connect(tabWidget, SIGNAL(currentChanged(int)), SLOT(_apply_wide_settings_to_tab(int)));
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

LexerFactory* MainWindow::getLexerFactory()
{
    return lexer_factory;
}

void MainWindow::on_actionWord_wrap_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = getFocusedEditor();
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
    if((sci->encoding() == "UTF-8") && (!sci->BOM()) ){
        statusBar_textFormat->setText("ANSI as UTF-8");
    }else {
        statusBar_textFormat->setText(sci->encoding());
    }

    ui->actionShow_All_Characters->setChecked(settings->value(widesettings::SETTING_SHOW_ALL_CHARS,true).toBool());
    ui->actionWord_wrap->setChecked(settings->value(widesettings::SETTING_WRAP_MODE,true).toBool());
    ui->actionShow_End_of_Line->setChecked(settings->value(widesettings::SETTING_SHOW_END_OF_LINE,true).toBool());
    ui->actionShow_White_Space_and_TAB->setChecked(settings->value(widesettings::SETTING_SHOW_WHITE_SPACE,true).toBool());
    ui->actionShow_Indent_Guide->setChecked(settings->value(widesettings::SETTING_SHOW_INDENT_GUIDE,true).toBool());
    ui->actionShow_Wrap_Symbol->setChecked(settings->value(widesettings::SETTING_WRAP_SYMBOL,true).toBool());

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
    QsciScintillaqq *sci = getFocusedEditor();
    if ( !sci || !widesettings::toggle_invisible_chars(sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionWindows_Format_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = getFocusedEditor();
    if ( !sci || !widesettings::set_eol_mode(QsciScintilla::EolWindows, sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionMac_Format_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = getFocusedEditor();
    if ( !sci || !widesettings::set_eol_mode(QsciScintilla::EolMac, sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionUNIX_Format_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = getFocusedEditor();
    if ( !sci || !widesettings::set_eol_mode(QsciScintilla::EolUnix, sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionUPPERCASE_triggered()
{
    getFocusedEditor()->SendScintilla(QsciScintilla::SCI_UPPERCASE);
}

void MainWindow::on_actionLowercase_triggered()
{
    getFocusedEditor()->SendScintilla(QsciScintilla::SCI_LOWERCASE);
}

void MainWindow::on_actionShow_White_Space_and_TAB_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = getFocusedEditor();
    if ( !sci || !widesettings::toggle_white_space(sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionShow_End_of_Line_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = getFocusedEditor();
    if ( !sci || !widesettings::toggle_end_of_line(sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionShow_Indent_Guide_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = getFocusedEditor();
    if ( !sci || !widesettings::toggle_indent_guide(sci) ) return;
    update_single_document_ui(sci);
}

void MainWindow::on_actionShow_Wrap_Symbol_triggered()
{
    // APPLY TO CURRENT TAB
    QsciScintillaqq *sci = getFocusedEditor();
    if ( !sci || !widesettings::toggle_wrap_symbol(sci) ) return;
    update_single_document_ui(sci);
}
