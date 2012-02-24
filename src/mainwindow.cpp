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

#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexerbatch.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercsharp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerd.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexerfortran.h>
#include <Qsci/qscilexerfortran77.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexeridl.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexermakefile.h>
#include <Qsci/qscilexerpascal.h>
#include <Qsci/qscilexerperl.h>
#include <Qsci/qscilexerpostscript.h>
#include <Qsci/qscilexerpov.h>
#include <Qsci/qscilexerproperties.h> /**/
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexerspice.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertcl.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexerverilog.h>
#include <Qsci/qscilexervhdl.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexeryaml.h>

#include <Qsci/qscilexercustom.h>
#include <userlexer.h>

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

    newTabCount = 0;

    settings = new QSettings();

    mainSplitter = new QSplitter(Qt::Horizontal, this);
    this->setCentralWidget(mainSplitter);

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

    QString  axx = QIcon::themeName();

    // Add main QTabWidgetqq
    tabWidget1 = new QTabWidgetqq(ui->centralWidget->parentWidget());
    tabWidget1->setObjectName("tabWidget");
    tabWidget1->setContextMenuPolicy(Qt::CustomContextMenu);
    tabWidget1->setDocumentMode(true);
    tabWidget1->setTabsClosable(true);
    tabWidget1->setMovable(true);
    //tabWidget1->setTabIcon(0, QIcon());
    tabWidget1->setStyleSheet("QTabBar::tab { height: 27px; }");
    //tabWidget1->setIconSize(QSize(12,12));

    connect(tabWidget1, SIGNAL(currentChanged(int)), SLOT(on_tabWidget1_currentChanged(int)));
    connect(tabWidget1, SIGNAL(tabCloseRequested(int)), SLOT(on_tabWidget1_tabCloseRequested(int)));
    connect(tabWidget1, SIGNAL(customContextMenuRequested(QPoint)), SLOT(on_tabWidget1_customContextMenuRequested(QPoint)));
    mainSplitter->addWidget(tabWidget1);

    // Add the second QTabWidgetqq
    tabWidget2 = new QTabWidgetqq(this);
    tabWidget2->setObjectName("tabWidgetBis");
    tabWidget2->setContextMenuPolicy(Qt::CustomContextMenu);
    tabWidget2->setDocumentMode(true);
    tabWidget2->setTabsClosable(true);
    tabWidget2->setMovable(true);

    connect(tabWidget2, SIGNAL(currentChanged(int)), SLOT(on_tabWidget2_currentChanged(int)));
    connect(tabWidget2, SIGNAL(tabCloseRequested(int)), SLOT(on_tabWidget2_tabCloseRequested(int)));
    connect(tabWidget2, SIGNAL(customContextMenuRequested(QPoint)), SLOT(on_tabWidget2_customContextMenuRequested(QPoint)));
    tabWidget2->setVisible(false);
    mainSplitter->addWidget(tabWidget2);

    currentTabWidget = tabWidget1;


    // Context menu
    tabContextMenu = new QMenu;
    tabContextMenuActions.append(ui->actionClose);
    tabContextMenuActions.append(ui->actionClose_All_BUT_Current_Document);
    tabContextMenuActions.append(ui->actionSave);
    tabContextMenuActions.append(ui->actionSave_as);
    tabContextMenuActions.append(ui->actionRename);
    tabContextMenuActions.append(ui->actionDelete_from_Disk);
    tabContextMenuActions.append(ui->actionPrint);
    tabContextMenuActions.append(getSeparator());
    tabContextMenuActions.append(ui->actionMove_to_Other_View);
    tabContextMenuActions.append(ui->actionClone_to_Other_View);
    tabContextMenuActions.append(ui->actionMove_to_New_Instance);
    tabContextMenuActions.append(ui->actionOpen_in_New_Instance);
    tabContextMenu->addActions(tabContextMenuActions);


    // Status bar widgets aspects
    statusBar_fileFormat = new QLabel(fileFormatDescription(""));
    statusBar_fileFormat->setFrameStyle(QFrame::Box | QFrame::Sunken);

    statusBar_lengthInfo = new QLabel("");
    statusBar_lengthInfo->setFrameStyle(QFrame::Box | QFrame::Sunken);

    statusBar_selectionInfo = new QLabel("");
    statusBar_selectionInfo->setFrameStyle(QFrame::Box | QFrame::Sunken);

    statusBar_EOLstyle = new QLabel("");
    statusBar_EOLstyle->setFrameStyle(QFrame::Box | QFrame::Sunken);

    statusBar_textFormat = new QLabel("ANSI");
    statusBar_textFormat->setFrameStyle(QFrame::Box | QFrame::Sunken);

    statusBar_overtypeNotify = new QLabel("");
    statusBar_overtypeNotify->setFrameStyle(QFrame::Box | QFrame::Sunken);

    statusBar()->setSizeGripEnabled(true);
    statusBar()->addWidget(statusBar_fileFormat, 1);
    statusBar()->addPermanentWidget(statusBar_lengthInfo, 1);
    statusBar()->addPermanentWidget(statusBar_selectionInfo, 1);
    statusBar()->addPermanentWidget(statusBar_EOLstyle, 1);
    statusBar()->addPermanentWidget(statusBar_textFormat, 1);
    statusBar()->addPermanentWidget(statusBar_overtypeNotify, 0);

    if(this->layoutDirection() == Qt::RightToLeft) {
        ui->actionText_Direction_RTL->setChecked(true);
    }


    processCommandLineArgs(QApplication::arguments(), false);

    instanceServer = new QLocalServer(this);
    connect(instanceServer, SIGNAL(newConnection()), SLOT(instanceServerNewConnection()));
    instanceServer->removeServer(INSTANCESERVER_ID);
    instanceServer->listen(INSTANCESERVER_ID);

    // Make sure to open at least one tab
    if(currentTabWidget->count() == 0) ui->action_New->trigger();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::instanceServerNewConnection()
{
    while(instanceServer->hasPendingConnections())
    {
        QLocalSocket *client = instanceServer->nextPendingConnection();
        connect(client, SIGNAL(readyRead()), SLOT(instanceSocketReadyRead()));
    }
}

void MainWindow::instanceSocketReadyRead()
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

void MainWindow::processCommandLineArgs(QStringList arguments, bool fromExternalMessage)
{
    bool activateWindow = false;

    if(arguments.count() <= 1)
    {
        // Open an empty tab in this new process
        ui->action_New->trigger();
        activateWindow = true;
    }
    else
    {
        // Open selected files
        QStringList files;
        for(int i = 1; i < arguments.count(); i++)
        {
            //QMessageBox::information(0, "", QString(QApplication::argv()[i]),0);
            files.append(arguments.at(i));
        }
        openNewFile(files);
        activateWindow = true;
    }

    if(fromExternalMessage && activateWindow)
    {
        // Activate the window
        this->activateWindow();
        this->raise();
    }
}

QAction* MainWindow::getSeparator(QObject *parent)
{
    QAction* actionSeparator = new QAction(parent);
    actionSeparator->setSeparator(true);
    return actionSeparator;
}

/**
 * Opens a new document in a new tab
 *
 * @see MainWindow::addEditorTab()
 */
void MainWindow::on_action_New_triggered()
{
    // Installare un event filter per fare in modo che ogni volta che un tabWidget ottiene il focus venga segnato da qualche parte,
    // e di conseguenza avere una variabile contenente l'ultimo tabWidget usato.
    addEditorTab(true, tr("new") + " " + QString::number(++newTabCount), tabWidget1);
}

QTabWidgetqq* MainWindow::lastActiveTabWidget()
{
    if(tabWidget2->hasFocus())
    {
        return tabWidget2;
    } else
    {
        return tabWidget1;
    }
}

/**
 * Closes a tab
 *
 * @param   index   Index of the tab to close
 *
 * @return  A value from enum MainWindow::tabCloseResult.
 */
int MainWindow::on_tabWidget1_tabCloseRequested(int index)
{
    return on_tabWidgetX_tabCloseRequested(index, tabWidget1);
}

int MainWindow::on_tabWidget2_tabCloseRequested(int index)
{
    return on_tabWidgetX_tabCloseRequested(index, tabWidget2);
}

int MainWindow::on_tabWidgetX_tabCloseRequested(int index, QTabWidgetqq * _tabWidget)
{
    int result = MainWindow::tabCloseResult_AlreadySaved;
    // Don't remove the tab if it's the last tab, it's empty, in an unmodified state and it's not associated with a file name.
    // Else, continue.
    if(!(_tabWidget->count() == 1
         && isNewEmptyTab(index))) {

        if(getTextBoxFromIndex(index, _tabWidget)->isModified()) {
            _tabWidget->setCurrentIndex(index);
            int ret = askIfWantToSave(index, askToSaveChangesReason_tabClosing);
            if(ret == QMessageBox::Save) {
                if(save(index) == MainWindow::saveFileResult_Canceled)
                {
                    // hmm... the user canceled the "save dialog". Let's ignore the close event.
                    result = MainWindow::tabCloseResult_Canceled;
                } else if(save(index) == MainWindow::saveFileResult_Saved)
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

/**
 * Detects if a tab is a new empty tab
 *
 * @param   index   Index of the tab
 *
 * @return  true if the tab is a new empty tab, otherwise false
 */
bool MainWindow::isNewEmptyTab(int index)
{
    if(getTextBoxFromIndex(index, tabWidget1)->text() == ""
       && getTextBoxFromIndex(index, tabWidget1)->isModified() == false
       && getTextBoxFromIndex(index, tabWidget1)->fileName() == "" ) {

        return true;
    } else {
        return false;
    }
}

void MainWindow::openNewFile(QStringList fileNames)
{
    if(!fileNames.isEmpty())
    {
        settings->setValue("lastSelectedDir", QFileInfo(fileNames[0]).absolutePath());
        // Ok, now open our files
        for (int i = 0; i < fileNames.count(); i++) {

            QFile file(fileNames[i]);
            QFileInfo fi(fileNames[i]);

            int index = addEditorTab(true, fi.fileName(), tabWidget1);
            QsciScintillaqq* sci = this->getTextBoxFromIndex(index, tabWidget1);

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
                    tabWidget1->removeTab(index);
                    break;
                } else if(ret == QMessageBox::Retry) {
                    tabWidget1->removeTab(index);
                    i--;
                    continue;
                } else if(ret == QMessageBox::Ignore) {
                    tabWidget1->removeTab(index);
                    continue;
                }
            }

            // If there was only a new empty tab opened, remove it
            if(tabWidget1->count() == 2 && isNewEmptyTab(0)) {
                tabWidget1->removeTab(0);
                index--;
            }

            sci->setFileName(fi.absoluteFilePath());
            sci->setEolMode(sci->guessEolMode());
            sci->setModified(false);
            tabWidget1->setTabToolTip(index, sci->fileName());
            autoLexer(sci->fileName(), sci);

            updateGui(index, tabWidget1);

            file.close();

            sci->setFocus(Qt::OtherFocusReason);
            //tabWidget1->setFocus();
            //tabWidget1->currentWidget()->setFocus();
        }
    }
}

/**
 * Opens a file in a new tab
 */
void MainWindow::on_action_Open_triggered()
{
    // Ask for file names...
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open"), settings->value("lastSelectedDir", ".").toString(), tr("All files (*)"), 0, 0);
    // Set focus here (bug #760308)
    openNewFile(fileNames);
}

/**
 * Gets the currently active scintilla document
 *
 * @return  a pointer to the QsciScintillaqq widget currently active
 * @see     MainWindow::getTextBoxFromIndex(int index)
 */
QsciScintillaqq* MainWindow::getCurrentTextBox(QTabWidgetqq * _tabWidget)
{
    return getTextBoxFromIndex(_tabWidget->currentIndex(), tabWidget1);
}

/**
 * Gets the scintilla document located at the specified index
 *
 * @param   index   Index of the tab
 *
 * @return  a pointer to the QsciScintillaqq widget located at the specified index
 * @see     MainWindow::getCurrentTextBox()
 * @see     MainWindow::getIndexFromWidget()
 */
QsciScintillaqq* MainWindow::getTextBoxFromIndex(int index, QTabWidgetqq * _tabWidget)
{
    return static_cast<QsciScintillaqq*>(_tabWidget->widget(index)->children().at(0));
}

/**
 * Adds a tab to the tabWidget
 *
 * @param   setFocus    If true, the new tab will receive focus
 * @param   title       The title of the new tab
 *
 * @return  the index of the new tab
 */
int MainWindow::addEditorTab(bool setFocus, QString title, QTabWidgetqq * _tabWidget)
{

    // Let's add a new tab...
    QWidget *newTab = new QWidget(_tabWidget);
    int index = _tabWidget->addTab(newTab, title);
    if(setFocus) {
        _tabWidget->setCurrentIndex(index);
    }
    _tabWidget->setTabIcon(index, QIcon(":/icons/icons/saved.png"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    // Create textbox
    QsciScintillaqq* sci = new QsciScintillaqq(_tabWidget->widget(index)); //ui->textEdit;

    // Set font
    QFont *f = new QFont("Courier New", 10, -1, false);
    sci->setFont(*f);
    QColor *c = new QColor("#000000"); // DB8B0B
    sci->setColor(*c);
    sci->setCaretForegroundColor(QColor("#5E5E5E"));


    sci->setMarginLineNumbers(1, true);
    sci->setMarginWidth(1, 47);
    sci->setFolding(QsciScintillaqq::BoxedTreeFoldStyle);
    sci->setAutoIndent(true);
    sci->setAutoCompletionThreshold(2);
    sci->setUtf8(true);
    //sci->SendScintilla(QsciScintilla::SCI_SETCODEPAGE, 950);
    //sci->SendScintilla(QsciScintilla::SCI_SETFOLDFLAGS, QsciScintilla::SC_FOLDFLAG_LINEBEFORE_CONTRACTED + QsciScintilla::SC_FOLDFLAG_LINEAFTER_CONTRACTED);

    /*QsciAPIs apis(&lex);
    apis.add("test");
    apis.add("test123");
    apis.add("foobar");
    apis.prepare();
    lex.setAPIs(&apis);*/

    autoLexer(sci->fileName(), sci);

    sci->setBraceMatching(QsciScintillaqq::SloppyBraceMatch);
    sci->setCaretLineVisible(true);
    sci->setCaretLineBackgroundColor(QColor("#E6EBF5"));
    sci->setIndentationGuidesForegroundColor(QColor("#C0C0C0"));

    sci->SendScintilla(QsciScintilla::SCI_INDICSETSTYLE, SELECTOR_DefaultSelectionHighlight, QsciScintilla::INDIC_ROUNDBOX);
    sci->SendScintilla(QsciScintilla::SCI_INDICSETFORE, SELECTOR_DefaultSelectionHighlight, 0x00FF24);
    sci->SendScintilla(QsciScintilla::SCI_INDICSETALPHA, SELECTOR_DefaultSelectionHighlight, 100);
    sci->SendScintilla(QsciScintilla::SCI_INDICSETUNDER, SELECTOR_DefaultSelectionHighlight, true);

    connect(sci, SIGNAL(modificationChanged(bool)), SLOT(on_scintillaModificationChanged(bool)));
    connect(sci, SIGNAL(fileChanged(QString, QsciScintillaqq*)), SLOT(fileChanged(QString, QsciScintillaqq*)));
    connect(sci, SIGNAL(textChanged()), SLOT(on_scintillaTextChanged()));
    connect(sci, SIGNAL(selectionChanged()), SLOT(on_scintillaSelectionChanged()));
    connect(sci, SIGNAL(cursorPositionChanged(int,int)), SLOT(on_scintillaCursorPositionChanged(int,int)));
    connect(sci, SIGNAL(updateUI()), SLOT(on_scintillaUpdateUI()));

    delete f;
    delete c;

    layout->addWidget(sci);
    _tabWidget->widget(index)->setLayout(layout);
    _tabWidget->setDocumentMode(true);
    _tabWidget->setTabToolTip(index, "");

    bool _showallchars = ui->actionShow_All_Characters->isChecked();
    updateScintillaPropertiesForAllTabs();
    if(_showallchars) {
        ui->actionShow_All_Characters->setChecked(true);
        on_actionShow_All_Characters_triggered();
    }

    sci->setFocus();
    // sci->SendScintilla(QsciScintilla::SCI_SETFOCUS, true);
    // sci->SendScintilla(QsciScintilla::SCI_GRABFOCUS);

    updateGui(index, tabWidget1);

    return index;
}

/**
 * Called when a textbox' content is modified
 *
 * @param   m     True if the content is modified, otherwise false
 */
void MainWindow::on_scintillaModificationChanged(bool m)
{
    // Update tab icons...
    QWidget* wid = static_cast<QWidget*>(sender());
    int i = getIndexFromWidget(*(wid));

    if(m) {
          tabWidget1->setTabIcon(i, QIcon(":/icons/icons/unsaved.png"));
    } else {
          tabWidget1->setTabIcon(i, QIcon(":/icons/icons/saved.png"));
    }

    this->updateGui(tabWidget1->currentIndex(), tabWidget1);

}

/**
 * Gets the tab's index relative to the specified widget
 *
 * @param   widget      A child widget of the tab to find
 *
 * @return  The index of the tab containing the specified index. If not found, returns -1.
 * @see     MainWindow::getTextBoxFromIndex()
 */
int MainWindow::getIndexFromWidget(QWidget &widget)
{
    // QsciScintillaqq* _sender = static_cast<QsciScintillaqq*>(sender());
    /*
        -WINDOW \
            TABCONTROL
        -MYTAB \
            SCINTILLA

     */
    /*
    QWidget* temp_widget = &widget;

    while(temp_widget->parentWidget() != tabWidget && temp_widget->parentWidget() != 0)
    {
        QString a = temp_widget->parentWidget()->parentWidget()->objectName();
        temp_widget = temp_widget->parentWidget();
    }

    if(temp_widget->parentWidget() != 0) // parent == tabWidget
    {
        return tabWidget->indexOf(temp_widget); // getTextBoxFromIndex(0)->parentWidget()
    } else {
        return -1;
    }
    */


    for(int i = 0; i < tabWidget1->count(); i++) {
        if(tabWidget1->widget(i) == widget.parentWidget())
        {
            return i;
        }
    }
    return -1;
}


/**
 * Saves the currently active document asking for a new file name.
 *
 * @return  an integer value from enum MainWindow::saveFileResult.
 * @see     MainWindow::on_actionSave_triggered()
 * @see     MainWindow::writeDocument()
 */
int MainWindow::on_actionSave_as_triggered()
{
    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(0, tr("Save as"), getSaveDialogDefaultFileName(tabWidget1->currentIndex()), tr("Any file (*)"), 0, 0);
    if (filename != "") {
        settings->setValue("lastSelectedDir", QFileInfo(filename).absolutePath());
        // Write
        return writeDocument(tabWidget1->currentIndex(), filename, true);
    } else {
        return MainWindow::saveFileResult_Canceled;
    }
}

/**
 * Writes the document's text located at the specified index.
 *
 * @param     index           the index of the document
 * @param     filename        the file name to use
 * @param     updateFileName  if true, updates the file name associated with the respective tab. Tab title and tooltip are updated too.
 *
 * @return    an integer value from enum MainWindow::saveFileResult.
 */
int MainWindow::writeDocument(int index, QString filename, bool updateFileName)
{
    QsciScintillaqq* sci = getTextBoxFromIndex(index, tabWidget1);
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

        if(updateFileName)
        {
            // Update document's filename
            sci->setFileName(fi.absoluteFilePath());
            sci->setModified(false);
            tabWidget1->setTabToolTip(index, sci->fileName());
            autoLexer(sci->fileName(), sci);

            // Update tab text
            tabWidget1->setTabText(index, fi.fileName());
        }
        updateGui(index, tabWidget1);

        file.close();
        sci->setFileWatchEnabled(true); //******************************** <-------------
    } while (retry);

    return MainWindow::saveFileResult_Saved;
}

void MainWindow::autoLexer(QString fullFileName, QsciScintillaqq* parent)
{
    //QString ext = QFileInfo(fullFileName).suffix();
    QFont *f;
    f = new QFont("Courier New", 10, -1, false);

    if(fullFileName != "")
    {

        QString ext = QFileInfo(fullFileName).suffix().toLower();
        if(ext=="cmake")
        {
            QsciLexerCMake lex(parent);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            parent->setLexer(&lex);
        } else if(ext=="cs")
        {
            QsciLexerCSharp lex(parent);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            parent->setLexer(&lex);
        } else if(ext=="css")
        {
            QsciLexerCSS lex(parent);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            parent->setLexer(&lex);
        } else if(ext=="d")
        {
            QsciLexerD lex(parent);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            parent->setLexer(&lex);
        } else if(ext=="diff" || ext=="patch")
        {
            QsciLexerDiff lex(parent);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            parent->setLexer(&lex);
        } else if(ext=="f" || ext=="for" || ext=="f90" || ext=="f95")
        {
            QsciLexerFortran lex(parent);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            parent->setLexer(&lex);
        } else if(ext=="f77")
        {
            QsciLexerFortran77 lex(parent);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            parent->setLexer(&lex);
        }
        else
        {
            // Let's try with mime-types!

            QString fileMime = generalFunctions::getFileMime(fullFileName);
            if(fileMime == "text/html" ||
               fileMime == "text/x-php")
            {
                    QsciLexerHTML lex(parent);

                    lex.setDefaultFont(*f);
                    //lex.setDefaultColor(QColor("#000000"));
                    //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                    lex.setFont(*f);
                    parent->setLexer(&lex); // ** SEGFAULT **
            }
            else if( fileMime == "text/x-c")
            {
                     QsciLexerCPP lex(parent);

                     lex.setDefaultFont(*f);
                     //lex.setDefaultColor(QColor("#000000"));
                     //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                     lex.setFont(*f);
                     parent->setLexer(&lex);
            }
            else if( fileMime == "text/x-shellscript" )
            {
                     QsciLexerBash lex(parent);

                     lex.setDefaultFont(*f);
                     //lex.setDefaultColor(QColor("#000000"));
                     //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                     lex.setFont(*f);
                     parent->setLexer(&lex);
            }
            else if( fileMime == "application/xml" )
            {
                     QsciLexerXML lex(parent);

                     lex.setDefaultFont(*f);
                     //lex.setDefaultColor(QColor("#000000"));
                     //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                     lex.setFont(*f);
                     parent->setLexer(&lex);
            }
            else if( fileMime == "text/x-msdos-batch" )
            {
                     QsciLexerBatch lex(parent);

                     lex.setDefaultFont(*f);
                     //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                     lex.setFont(*f);
                     parent->setLexer(&lex);
            }
            /* else if( ext == "test")
            {
                     userLexer lex(parent);
                     f = new QFont("Courier New", 10, -1, false);
                     lex.setDefaultFont(*f);
                     lex.setDefaultColor(QColor("#000000"));
                     //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                     lex.setFont(*f);
                     parent->setLexer(&lex);
            } */
            else
            {
                    // Plain text
                    parent->setLexer(0);
            }

        }

    } else
    {
        parent->setLexer(0);
    }

    if(parent->lexer() != 0)
    {
        //
    }

}

QString MainWindow::fileFormatDescription(QString filename)
{
    QString ext = QFileInfo(filename).suffix();
    return settings->value("format/" + ext + "/description", tr("Normal text file")).toString();
}

void MainWindow::on_tabWidget1_currentChanged(int index)
{
    on_tabWidgetX_currentChanged(index, tabWidget1);
}

void MainWindow::on_tabWidget2_currentChanged(int index)
{
    on_tabWidgetX_currentChanged(index, tabWidget2);
}

void MainWindow::on_tabWidgetX_currentChanged(int index, QTabWidgetqq * _tabWidget)
{
    if(scintillaWasCreated(index, _tabWidget))
    {
        // Set focus
        getTextBoxFromIndex(index, _tabWidget)->setFocus(Qt::OtherFocusReason);
        getTextBoxFromIndex(index, _tabWidget)->SendScintilla(QsciScintilla::SCI_SCROLLCARET);
    }

    updateGui(index, _tabWidget);
}

bool MainWindow::scintillaWasCreated(int index, QTabWidgetqq *_tabWidget)
{
    if(index >= 0 && _tabWidget->widget(index)->children().count() > 0)
        return true;
    else
        return false;
}

void MainWindow::updateGui(int index, QTabWidgetqq * _tabWidget)
{
    // Update elements only if the 'scintilla' object is already created
    if(scintillaWasCreated(index, _tabWidget))
    {
        QsciScintillaqq *sci = getTextBoxFromIndex(index, _tabWidget);
        QString filename = sci->fileName();
        QString descr = fileFormatDescription(filename);
        statusBar_fileFormat->setText(descr);

        QString modified_sign = "";
        if(sci->isModified()) modified_sign = "*";
        if(sci->fileName() == "")
        {
            this->setWindowTitle(modified_sign + _tabWidget->tabText(index) + " - " + QCoreApplication::applicationName());
            ui->actionReload_from_Disk->setEnabled(false);
        } else {
            this->setWindowTitle(modified_sign + sci->fileName() + " - " + QCoreApplication::applicationName());
            ui->actionReload_from_Disk->setEnabled(true);
        }

        ui->actionSave->setEnabled(sci->isModified());

        on_scintillaTextChanged(); // Update statusbar (lengthInfo)

        // Update statusbar (selectionInfo):
        int lineFrom, indexFrom;
        sci->getCursorPosition(&lineFrom, &indexFrom);
        on_scintillaCursorPositionChanged(lineFrom, indexFrom);

        // Update statusbar (EOL format)
        QString EOL_name = "?";
        ui->actionWindows_Format->setEnabled(true);
        ui->actionUNIX_Format->setEnabled(true);
        ui->actionMac_Format->setEnabled(true);
        switch(sci->eolMode())
        {
            case QsciScintilla::EolWindows:
                 EOL_name = "Dos\\Windows";
                 ui->actionWindows_Format->setEnabled(false);
                 break;
            case QsciScintilla::EolMac:
                 EOL_name = "Macintosh";
                 ui->actionMac_Format->setEnabled(false);
                 break;
            case QsciScintilla::EolUnix:
                 EOL_name = "UNIX";
                 ui->actionUNIX_Format->setEnabled(false);
                 break;
        }

        this->statusBar_EOLstyle->setText(EOL_name);
        updateOvertypeLabel();
    }
}

void MainWindow::updateOvertypeLabel()
{
    if(getCurrentTextBox(tabWidget1)->overType())
    {
        statusBar_overtypeNotify->setText(tr("OVR"));
    }
    else
    {
        statusBar_overtypeNotify->setText(tr("INS"));
    }
}

void MainWindow::on_scintillaUpdateUI()
{
    updateOvertypeLabel();
    statusBar_textFormat->setText(getCurrentTextBox(tabWidget1)->encoding);
}

void MainWindow::on_actionReload_from_Disk_triggered()
{
    reloadFromDisk(this->getCurrentTextBox(tabWidget1)->fileName(), true, getCurrentTextBox(tabWidget1));
}

bool MainWindow::reloadFromDisk(QString filename, bool askConfirm, QsciScintillaqq* sci)
{
    if(filename != "")
    {
        int ret = QMessageBox::Yes;

        if(askConfirm == true && sci->isModified())
        {
            QMessageBox msgBox_ask;
            msgBox_ask.setWindowTitle(QCoreApplication::applicationName());
            msgBox_ask.setText(tr("Are you sure you want to reload the current file and lose the changes made in Notepadqq?"));
            msgBox_ask.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox_ask.setDefaultButton(QMessageBox::No);
            msgBox_ask.setIcon(QMessageBox::Question);
            ret = msgBox_ask.exec();
        }

        if(ret == QMessageBox::Yes) {
            QFile file(filename);
            QFileInfo fi(filename);

            bool retry = false;
            bool aborted = false;
            do
            {
                retry = false;
                if (!sci->read(&file)) {
                    // Manage error
                    QMessageBox msgBox;
                    msgBox.setWindowTitle(QCoreApplication::applicationName());
                    msgBox.setText(tr("Error trying to open \"%1\"").arg(fi.fileName()));
                    msgBox.setDetailedText(file.errorString());
                    msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry);
                    msgBox.setDefaultButton(QMessageBox::Retry);
                    msgBox.setIcon(QMessageBox::Critical);
                    int ret = msgBox.exec();
                    if(ret == QMessageBox::Abort) {
                        aborted = true;
                        break;
                    } else if(ret == QMessageBox::Retry) {
                        retry = true;
                        continue;
                    }
                }
            } while (retry);

            file.close();

            if(!aborted)
            {
                sci->setEolMode(sci->guessEolMode());
                sci->setModified(false);
                updateGui(tabWidget1->currentIndex(), tabWidget1);
                return true;
            } else
            {
                return false;
            }
        }
    }
    return false;
}

/**
 * Saves the currently active document. If the document is not already associated with a file name, ask for it.
 *
 * @return  an integer value from enum MainWindow::saveFileResult.
 * @see     MainWindow::on_actionSave_as_triggered()
 * @see     MainWindow::save()
 */
int MainWindow::on_actionSave_triggered()
{
    return save(tabWidget1->currentIndex());
}

/**
 * Saves the document located at the specified index. If the document is not already associated with a file name, ask for it.
 *
 * @param     index   the index of the document to save
 *
 * @return  an integer value from enum MainWindow::saveFileResult.
 * @see     MainWindow::on_actionSave_as_triggered()
 * @see     MainWindow::writeDocument()
 */
int MainWindow::save(int index)
{
    if(getTextBoxFromIndex(index, tabWidget1)->fileName() == "")
    {
        // Call "save as"
        return on_actionSave_as_triggered();
    } else {
        return writeDocument(tabWidget1->currentIndex(), getCurrentTextBox(tabWidget1)->fileName(), true);
    }
}

void MainWindow::on_actionAbout_Notepadqq_triggered()
{
    frmAbout *_about;
    _about = new frmAbout(this);
    _about->exec();

    delete _about;
}

QString MainWindow::getSaveDialogDefaultFileName(int index)
{

    QString docFileName = getTextBoxFromIndex(index, tabWidget1)->fileName();

    if(docFileName == "") {
        return settings->value("lastSelectedDir", ".").toString() + "/" + tabWidget1->tabText(index);
    } else {
        return docFileName;
    }
}

int MainWindow::on_actionSave_a_Copy_As_triggered()
{
    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(0, tr("Save a copy as"), getSaveDialogDefaultFileName(tabWidget1->currentIndex()), tr("Any file (*)"), 0, 0);
    if (filename != "") {
        settings->setValue("lastSelectedDir", QFileInfo(filename).absolutePath());
        // Write
        return writeDocument(tabWidget1->currentIndex(), filename, false);
    } else {
        return MainWindow::saveFileResult_Canceled;
    }
}

int MainWindow::askIfWantToSave(int index, int reason)
{
    QMessageBox msgBox;
    QString file;
    QsciScintillaqq* sci = getTextBoxFromIndex(index, tabWidget1);

    if (sci->fileName() == "")
    {
        file = Qt::escape(tabWidget1->tabText(index));
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    for(int i = 0; i < tabWidget1->count(); i++) {
        if(getTextBoxFromIndex(i, tabWidget1)->isModified()) {
            tabWidget1->setCurrentIndex(i);
            int ret = askIfWantToSave(i, askToSaveChangesReason_tabClosing);
            if(ret == QMessageBox::Save) {
                if(save(i) == MainWindow::saveFileResult_Canceled)
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

void MainWindow::on_actionE_xit_triggered()
{
    this->close();
}

void MainWindow::on_actionSave_All_triggered()
{
    for(int i = 0; i < tabWidget1->count(); i++) {
        if(getTextBoxFromIndex(i, tabWidget1)->isModified()) {
            save(i);
        }
    }
}

void MainWindow::fileChanged(const QString &path, QsciScintillaqq* sender)
{
    tabWidget1->setCurrentIndex(getIndexFromWidget(*sender));

    QMessageBox msgBox;

    msgBox.setWindowTitle(QCoreApplication::applicationName());

    msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
    // TODO: Check if file exists!!!
    msgBox.setText("<h3>" + tr("The document «%1» has been modified.").arg(path) + "</h3>");
    msgBox.setInformativeText(tr("Do you want to reload it? If you do, you'll lose any changes you made to the file from within Notepadqq."));
    msgBox.setButtonText(QMessageBox::Open, tr("Reload"));
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setEscapeButton(QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Warning);

    msgBox.exec();

    if(msgBox.standardButton(msgBox.clickedButton()) == QMessageBox::Open)
    {
        // TODO: Reload file from disk
        reloadFromDisk(path, false, sender);
    }
}

QTabWidgetqq* MainWindow::tabWidgetFromObject(QObject * obj)
{
    return static_cast<QTabWidgetqq *>(obj);
}

void MainWindow::on_tabWidget1_customContextMenuRequested(QPoint pos)
{
    on_tabWidgetX_customContextMenuRequested(pos, tabWidget1);
}

void MainWindow::on_tabWidget2_customContextMenuRequested(QPoint pos)
{
    on_tabWidgetX_customContextMenuRequested(pos, tabWidget2);
}

void MainWindow::on_tabWidgetX_customContextMenuRequested(QPoint pos, QTabWidgetqq * _tabWidget)
{
    int index = _tabWidget->getTabIndexAt(pos);

    if(index != -1)
    {
        _tabWidget->setCurrentIndex(index);
        //QMessageBox::information(this,"Context menu :: debug",QString::number(index),QMessageBox::Ok);

        tabContextMenu->exec(_tabWidget->mapToGlobal(pos));
    }
}

void MainWindow::on_actionClose_triggered()
{
    this->on_tabWidget1_tabCloseRequested(tabWidget1->currentIndex());
}

void MainWindow::on_actionClose_All_BUT_Current_Document_triggered()
{
    QWidget* dontRemove = tabWidget1->currentWidget();
    while(tabWidget1->count() != 1) {
        int tabToRemove = tabWidget1->count()-1;
        // Well, we must ensure that we're not removing our lucky tab...
        // And while our tab is being in the way, try other indices (<-- cool translation by an Italian guy)
        while(tabWidget1->widget(tabToRemove) == dontRemove)
        {
            tabToRemove--;
        }
        int result = this->on_tabWidget1_tabCloseRequested(tabToRemove);
        if(result == MainWindow::tabCloseResult_Canceled)
        {
            // Stop all.
            break;
        }
    }
}

void MainWindow::on_actionC_lose_All_triggered()
{
    int max = tabWidget1->count();
    for(int i = 0; i < max; i++)
    {
        // Always remove the tab at position 0
        int result = this->on_tabWidget1_tabCloseRequested(0);
        if(result == MainWindow::tabCloseResult_Canceled)
        {
            // Stop all.
            break;
        }
    }
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_action_Copy_triggered()
{
    getCurrentTextBox(tabWidget1)->copy();
}

void MainWindow::on_action_Paste_triggered()
{
    getCurrentTextBox(tabWidget1)->paste();
}

void MainWindow::on_actionCu_t_triggered()
{
    getCurrentTextBox(tabWidget1)->cut();
}

void MainWindow::on_actionSelect_All_triggered()
{
    getCurrentTextBox(tabWidget1)->selectAll(true);
}

void MainWindow::on_scintillaTextChanged()
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
    statusBar_lengthInfo->setText(QString(tr("length: %1   lines: %2")).arg(
                                         QString::number(sci->text().length())
                                        ,QString::number(sci->lines())
                                        ));
}

void MainWindow::on_scintillaSelectionChanged()
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);

    // Highlight selection
    int lineFrom, indexFrom, lineTo, indexTo;
    sci->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
    int line, pos;
    sci->getCursorPosition(&line, &pos);


    // Clear highlights
    clearHighlightOfSelection(getIndexFromWidget(*sci));

    if(sci->selectedText().length() > 0  &&  lineFrom == lineTo)
    {
        // Select 1 char before and 1 char after the selected string
        int sel_posStart = sci->positionFromLineIndex(lineFrom, indexFrom);
        int sel_posEnd = sci->positionFromLineIndex(lineTo, indexTo);
        QString selExtended = sci->text().mid(sel_posStart - 1, sci->selectedText().length() + 2);

        bool isWord = true;

        bool startCharIsWordChar = sci->isWordCharacter(selExtended.at(0).toLatin1());
        bool endCharIsWordChar = sci->isWordCharacter(selExtended.at(selExtended.length()-1).toLatin1());
        if(sel_posStart == 0) startCharIsWordChar = !startCharIsWordChar;
        if(sel_posEnd == sci->text().length()) endCharIsWordChar = !endCharIsWordChar;

        if(startCharIsWordChar || endCharIsWordChar)
        {
            isWord = false;
        }

        if(isWord)
        {
            sci->highlightTextRecurrence(QsciScintilla::SCFIND_WHOLEWORD, sci->selectedText(), 0, sci->text().toUtf8().size(), SELECTOR_DefaultSelectionHighlight);
        }
    }
}

void MainWindow::clearHighlightOfSelection(int index)
{
    QsciScintillaqq *sci = getTextBoxFromIndex(index, tabWidget1);
    sci->SendScintilla(QsciScintilla::SCI_SETINDICATORCURRENT, SELECTOR_DefaultSelectionHighlight);
    sci->SendScintilla(QsciScintilla::SCI_INDICATORCLEARRANGE, 0, sci->text().length() * 2);
}

void MainWindow::on_scintillaCursorPositionChanged(int line, int pos)
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
    statusBar_selectionInfo->setText(QString(tr("Ln: %1   Col: %2   Sel: %3")).arg(
                                          QString::number(line + 1)
                                         ,QString::number(pos + 1)
                                         ,QString::number(sci->selectedText().length())
                                         ));

}

int MainWindow::getMultibyteTextLength(QTabWidgetqq * _tabWidget, int lineFrom, int indexFrom, int lineTo, int indexTo)
{
    QsciScintillaqq *sci = getCurrentTextBox(_tabWidget);
    int from = sci->positionFromLineIndex(lineFrom, indexFrom);
    int to = sci->positionFromLineIndex(lineTo, indexTo);
    return qAbs(to - from);
}

void MainWindow::on_actionWindows_Format_triggered()
{
    getCurrentTextBox(tabWidget1)->convertEols(QsciScintilla::EolWindows);
    getCurrentTextBox(tabWidget1)->setEolMode(QsciScintilla::EolWindows);
    updateGui(tabWidget1->currentIndex(), tabWidget1);
}

void MainWindow::on_actionUNIX_Format_triggered()
{
    getCurrentTextBox(tabWidget1)->convertEols(QsciScintilla::EolUnix);
    getCurrentTextBox(tabWidget1)->setEolMode(QsciScintilla::EolUnix);
    updateGui(tabWidget1->currentIndex(), tabWidget1);
}

void MainWindow::on_actionMac_Format_triggered()
{
    getCurrentTextBox(tabWidget1)->convertEols(QsciScintilla::EolMac);
    getCurrentTextBox(tabWidget1)->setEolMode(QsciScintilla::EolMac);
    updateGui(tabWidget1->currentIndex(), tabWidget1);
}

void MainWindow::on_actionShow_End_of_Line_triggered()
{
    updateScintillaPropertiesForAllTabs();
}

void MainWindow::on_action_Undo_triggered()
{
    getCurrentTextBox(tabWidget1)->undo();
}

void MainWindow::on_action_Redo_triggered()
{
    getCurrentTextBox(tabWidget1)->redo();
}

void MainWindow::on_action_Delete_triggered()
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
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
    sci->removeSelectedText();
}

void MainWindow::on_actionShow_White_Space_and_TAB_triggered()
{
    updateScintillaPropertiesForAllTabs();
}

void MainWindow::updateScintillaPropertiesForAllTabs()
{
    ui->actionShow_All_Characters->setChecked(false);
    for(int i = 0; i < tabWidget1->count(); i++)
    {
        if(this->scintillaWasCreated(i, tabWidget1))
        {
            if(ui->actionShow_White_Space_and_TAB->isChecked())
            {
                getTextBoxFromIndex(i, tabWidget1)->setWhitespaceVisibility(QsciScintilla::WsVisible);
            } else {
                getTextBoxFromIndex(i, tabWidget1)->setWhitespaceVisibility(QsciScintilla::WsInvisible);
            }

            getTextBoxFromIndex(i, tabWidget1)->setEolVisibility(ui->actionShow_End_of_Line->isChecked());

            if(ui->actionWord_wrap->isChecked())
            {
                getTextBoxFromIndex(i, tabWidget1)->setWrapMode(QsciScintilla::WrapWord);
            } else {
                getTextBoxFromIndex(i, tabWidget1)->setWrapMode(QsciScintilla::WrapNone);
            }

            if(ui->actionShow_Wrap_Symbol->isChecked())
            {
                getTextBoxFromIndex(i, tabWidget1)->setWrapVisualFlags(QsciScintilla::WrapFlagByText, QsciScintilla::WrapFlagNone, 5);
            } else {
                getTextBoxFromIndex(i, tabWidget1)->setWrapVisualFlags(QsciScintilla::WrapFlagNone);
            }

            getTextBoxFromIndex(i, tabWidget1)->setIndentationGuides(ui->actionShow_Indent_Guide->isChecked());

            if(ui->actionText_Direction_RTL->isChecked())
            {
                getTextBoxFromIndex(i, tabWidget1)->setLayoutDirection(Qt::RightToLeft);
            } else {
                getTextBoxFromIndex(i, tabWidget1)->setLayoutDirection(Qt::LeftToRight);
            }
        }
    }
}

void MainWindow::on_actionShow_All_Characters_triggered()
{
    if(ui->actionShow_All_Characters->isChecked())
    {
        ui->actionShow_End_of_Line->setChecked(false);
        ui->actionShow_White_Space_and_TAB->setChecked(false);
    }

    for(int i = 0; i < tabWidget1->count(); i++)
    {
        if(this->scintillaWasCreated(i, tabWidget1))
        {
            if(ui->actionShow_All_Characters->isChecked())
            {
                getTextBoxFromIndex(i, tabWidget1)->setWhitespaceVisibility(QsciScintilla::WsVisible);
                getTextBoxFromIndex(i, tabWidget1)->setEolVisibility(true);
            } else {
                getTextBoxFromIndex(i, tabWidget1)->setWhitespaceVisibility(QsciScintilla::WsInvisible);
                getTextBoxFromIndex(i, tabWidget1)->setEolVisibility(false);
            }
        }
    }
}

void MainWindow::on_actionWord_wrap_triggered()
{
    updateScintillaPropertiesForAllTabs();
}

void MainWindow::on_actionShow_Wrap_Symbol_triggered()
{
    updateScintillaPropertiesForAllTabs();
}

void MainWindow::on_actionShow_Indent_Guide_triggered()
{
    updateScintillaPropertiesForAllTabs();
}

void MainWindow::on_actionText_Direction_RTL_triggered()
{
    updateScintillaPropertiesForAllTabs();
}

void MainWindow::on_actionCurrent_Full_File_path_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).absoluteFilePath());
    } else {
        QWidget *wid = static_cast<QWidget*>(sci);
        QApplication::clipboard()->setText(tabWidget1->tabText(getIndexFromWidget(*(wid))));
    }
}

void MainWindow::on_actionCurrent_Filename_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).fileName());
    } else {
        QWidget *wid = static_cast<QWidget*>(sci);
        QApplication::clipboard()->setText(tabWidget1->tabText(getIndexFromWidget(*(wid))));
    }
}

void MainWindow::on_actionCurrent_Directory_Path_to_Clipboard_triggered()
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
    if(sci->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(sci->fileName()).absolutePath());
    } else {
        QApplication::clipboard()->setText("");
    }
}

void MainWindow::on_actionIncrease_Line_Indent_triggered()
{          
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
    int lineFrom, indexFrom, lineTo, indexTo;
    sci->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

    if(lineFrom == -1 || lineTo == -1)
    {
        sci->getCursorPosition(&lineFrom, &indexFrom);
        if(lineFrom == -1 || lineTo == -1)
        {
            sci->insertAt("\t", lineFrom, indexFrom);
            sci->setCursorPosition(lineFrom, indexFrom+1);
        }
    }
    else if(lineFrom == lineTo)
    {
        // Insert a normal tab
        sci->insertAt("\t", lineFrom, indexFrom);
        sci->setCursorPosition(lineFrom, indexFrom+1);
    }
    else
    {
        for(int i = lineFrom; i <= lineTo; i++)
        {
            sci->indent(i);
        }
    }
}

void MainWindow::on_actionDecrease_Line_Indent_triggered()
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
    int lineFrom, indexFrom, lineTo, indexTo;
    sci->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

    if(lineFrom == -1 || lineTo == -1)
    {
        sci->getCursorPosition(&lineFrom, &indexFrom);
        lineTo = lineFrom;
        indexTo = indexFrom;
    }

    if(!(lineFrom == -1 || lineTo == -1))
    {
        for(int i = lineFrom; i <= lineTo; i++)
        {
            sci->unindent(i);
        }
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
    //getCurrentTextBox(tabWidget1)->SendScintilla(QsciScintilla::SCI_ZOOMIN);
    getCurrentTextBox(tabWidget1)->zoomIn();
}

void MainWindow::on_actionZoom_Out_triggered()
{
    //getCurrentTextBox(tabWidget1)->SendScintilla(QsciScintilla::SCI_ZOOMOUT);
    getCurrentTextBox(tabWidget1)->zoomOut();
}

void MainWindow::on_actionRestore_Default_Zoom_triggered()
{
    //getCurrentTextBox(tabWidget1)->SendScintilla(QsciScintilla::SCI_SETZOOM, 0);
    getCurrentTextBox(tabWidget1)->zoomTo(0);
}

void MainWindow::on_actionClone_to_Other_View_triggered()
{
    tabWidget2->setVisible(true);
    int i = addEditorTab(true, "-", tabWidget2);
    getTextBoxFromIndex(i, tabWidget2)->setDocument(getCurrentTextBox(tabWidget1)->document());
}

void MainWindow::on_action_Start_Recording_triggered()
{

}


void MainWindow::encodeIn(QString _enc, bool _bom, QsciScintillaqq * sci)
{
    QTemporaryFile file;

//    QString enc = sci->encoding;
//    bool BOM = sci->BOM;

    if (sci->write(&file)) { // Write our text to a temp file...
        file.close();

        if(sci->read(&file, _enc)) // ... and reopen our text, but encoded as if it was "_enc"
        {
            sci->encoding = _enc;
            sci->BOM = _bom;
        } else qDebug(file.errorString().toAscii());
    }
    file.close();

//    sci->encoding = enc;
//    sci->BOM = BOM;
    qDebug(file.fileName().toUtf8());
    QFile::remove(file.fileName());
}

void MainWindow::on_actionUPPERCASE_triggered()
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
    if(sci->selectedText().length() > 0)
    {
        sci->beginUndoAction();

        QString transf = sci->selectedText().toUpper();

        int lineFrom, indexFrom, lineTo, indexTo;
        sci->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        sci->removeSelectedText();

        int lineFromC, indexFromC;
        sci->getCursorPosition(&lineFromC, &indexFromC);

        sci->insertAt(transf, lineFromC, indexFromC);
        sci->setSelection(lineFrom, indexFrom, lineTo, indexTo);

        sci->endUndoAction();
    }
}

void MainWindow::on_actionLowercase_triggered()
{
    QsciScintillaqq *sci = getCurrentTextBox(tabWidget1);
    if(sci->selectedText().length() > 0)
    {
        sci->beginUndoAction();

        QString transf = sci->selectedText().toLower();

        int lineFrom, indexFrom, lineTo, indexTo;
        sci->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

        sci->removeSelectedText();

        int lineFromC, indexFromC;
        sci->getCursorPosition(&lineFromC, &indexFromC);

        sci->insertAt(transf, lineFromC, indexFromC);
        sci->setSelection(lineFrom, indexFrom, lineTo, indexTo);

        sci->endUndoAction();
    }
}

void MainWindow::convertTo(QString _enc, bool _bom, QsciScintillaqq * sci)
{
    sci->encoding = _enc;
    sci->BOM = _bom;

    // Set the text to a modified state (but the undo list will be cleaned)
    QString t = sci->text();
    sci->setText(QString::number(random()));
    sci->setText(t);
}

void MainWindow::on_actionUTF_8_triggered()
{
    convertTo("UTF-8", false, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionWindows_1252_ANSI_triggered()
{
    convertTo("Windows-1252", false, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionUTF_16BE_triggered()
{
    convertTo("UTF-16BE", true, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionUTF_16LE_triggered()
{
    convertTo("UTF-16LE", true, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionUTF_8_with_BOM_triggered()
{
    convertTo("UTF-8", true, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionISO_8859_6_triggered()
{
    convertTo("ISO-8859-6", false, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionEncode_in_UTF_8_without_BOM_triggered()
{
    statusBar_textFormat->setText("UTF-8");
    encodeIn("UTF-8", false, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionEncode_in_Windows_1252_triggered()
{
    statusBar_textFormat->setText("Windows-1252");
    encodeIn("Windows-1252", false, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionEncode_in_UTF_8_triggered()
{
    statusBar_textFormat->setText("UTF-8");
    encodeIn("UTF-8", true, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionEncode_in_UTF_16BE_UCS_2_Big_Endian_triggered()
{
    statusBar_textFormat->setText("UTF-16 Big Endian");
    encodeIn("UTF-16BE", true, getCurrentTextBox(tabWidget1));
}

void MainWindow::on_actionLaunch_in_Firefox_triggered()
{
    QProcess::startDetached("firefox", QStringList(getCurrentTextBox(tabWidget1)->fileName()));
}

void MainWindow::on_actionGet_php_help_triggered()
{
    QDesktopServices::openUrl(QUrl("http://php.net/" + QUrl::toPercentEncoding(getCurrentTextBox(tabWidget1)->selectedText())));
}

void MainWindow::on_actionLaunch_in_Chromium_triggered()
{
    QProcess::startDetached("chromium-browser", QStringList(getCurrentTextBox(tabWidget1)->fileName()));
}

void MainWindow::on_actionGoogle_Search_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.google.com/search?q=" + QUrl::toPercentEncoding(getCurrentTextBox(tabWidget1)->selectedText())));
}

void MainWindow::on_actionWikipedia_Search_triggered()
{
    QDesktopServices::openUrl(QUrl("http://en.wikipedia.org/w/index.php?title=Special%3ASearch&search=" + QUrl::toPercentEncoding(getCurrentTextBox(tabWidget1)->selectedText())));
}
