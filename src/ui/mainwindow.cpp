#include "include/mainwindow.h"
#include "ui_mainwindow.h"
#include "include/editor.h"
#include "include/editortabwidget.h"
#include "include/frmabout.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // FIXME Set /usr/share/themes QIcon::setThemeSearchPaths();

    ui->setupUi(this);

    this->settings = new QSettings();

    this->topEditorContainer = new TopEditorContainer(this);
    this->setCentralWidget(this->topEditorContainer);

    docEngine = new DocEngine(settings, topEditorContainer);

    // Assign (where possible) system theme icons to our actions.
    // If a system icon doesn't exist, fallback on the already assigned icon.
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

    connect(this->topEditorContainer,
            SIGNAL(customTabContextMenuRequested(QPoint,EditorTabWidget*,int)),
            this,
            SLOT(on_customTabContextMenuRequested(QPoint,EditorTabWidget*,int)));

    connect(this->topEditorContainer,
            SIGNAL(tabCloseRequested(EditorTabWidget*,int)),
            this,
            SLOT(on_tabCloseRequested(EditorTabWidget*,int)));

    connect(this->topEditorContainer,
            SIGNAL(currentEditorChanged(EditorTabWidget*,int)),
            this,
            SLOT(on_currentEditorChanged(EditorTabWidget*,int)));

    connect(this->topEditorContainer,
            SIGNAL(editorAdded(EditorTabWidget*,int)),
            this,
            SLOT(on_editorAdded(EditorTabWidget*,int)));

    this->createStatusBar();



    this->processCommandLineArgs(QApplication::arguments(), false);

    // DEBUG: Add a second tabWidget
    //this->topEditorContainer->addTabWidget()->addEditorTab(false, "test");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createStatusBar()
{
    QStatusBar* status = this->statusBar();
    QLabel* label;

    status->setFixedHeight(22);

    label = new QLabel("File Format", this);
    label->setMinimumWidth(160);
    status->addWidget(label);
    statusBar_fileFormat = label;

    label = new QLabel("Length : 0     Lines : 1", this);
    status->addWidget(label);
    statusBar_lengthInfo = label;

    label = new QLabel("Ln : 0     Col : 1     Sel : 0 | 0", this);
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    status->addWidget(label);
    statusBar_selectionInfo = label;

    label = new QLabel("EOL", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    label->setMinimumWidth(128);
    status->addWidget(label);
    statusBar_EOLstyle = label;

    label = new QLabel("Encoding", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    label->setMinimumWidth(128);
    status->addWidget(label);
    statusBar_textFormat = label;

    label = new QLabel("INS", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    label->setMinimumWidth(40);
    status->addWidget(label);
    statusBar_overtypeNotify = label;
}

void MainWindow::processCommandLineArgs(QStringList arguments, bool fromOtherInstance)
{
    bool activateWindow = false;

    if(arguments.count() <= 1)
    {
        // Open a new empty document
        ui->action_New->trigger();
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

        EditorTabWidget *tabW = this->topEditorContainer->currentTabWidget();
        // Make sure we have a tabWidget: if not, create it.
        if(tabW == 0) {
            tabW = this->topEditorContainer->addTabWidget();
        }
        docEngine->loadDocuments(files, tabW, false);
        activateWindow = true;
    }

    if(fromOtherInstance && activateWindow)
    {
        // Activate the window
        this->activateWindow();
        this->raise();
    }
}

void MainWindow::on_action_New_triggered()
{
    static int num = 1; // FIXME maybe find smarter way
    EditorTabWidget *tabW = this->topEditorContainer->currentTabWidget();

    // Make sure we have a tabWidget: if not, create it.
    if(tabW == 0) {
        tabW = this->topEditorContainer->addTabWidget();
    }

    tabW->addEditorTab(true, tr("new %1").arg(num));
    num++;
}

void MainWindow::on_customTabContextMenuRequested(QPoint point, EditorTabWidget *tabWidget, int tabIndex) {
    this->tabContextMenu->exec(point);
}

void MainWindow::on_actionMove_to_Other_View_triggered()
{
    EditorTabWidget *curTabWidget = this->topEditorContainer->currentTabWidget();
    EditorTabWidget *newTabWidget;

    if(this->topEditorContainer->count() >= 2) {
        int viewId = 1;
        if(this->topEditorContainer->widget(1) == curTabWidget) {
            viewId = 0;
        }

        newTabWidget = (EditorTabWidget *)this->topEditorContainer->widget(viewId);

    } else {
        newTabWidget = this->topEditorContainer->addTabWidget();
    }

    newTabWidget->transferEditorTab(true, curTabWidget, curTabWidget->currentIndex());

    this->removeTabWidgetIfEmpty(curTabWidget);
}

void MainWindow::removeTabWidgetIfEmpty(EditorTabWidget *tabWidget) {
    if(tabWidget->count() == 0) {
        delete tabWidget;
    }
}

void MainWindow::on_action_Open_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
                this,
                tr("Open"),
                settings->value("lastSelectedDir", ".").toString(),
                tr("All files (*)"),
                0, 0);

    this->docEngine->loadDocuments(fileNames,
                                   this->topEditorContainer->currentTabWidget(),
                                   false);
}

int MainWindow::askIfWantToSave(EditorTabWidget *tabWidget, int tab, int reason)
{
    QMessageBox msgBox;
    QString name;

    Editor *editor = (Editor *)tabWidget->widget(tab);

    if (editor->fileName() == "")
    {
        name = tabWidget->tabText(tab).toHtmlEscaped();
    } else {
        name = QFileInfo(editor->fileName()).fileName().toHtmlEscaped();
    }
    msgBox.setWindowTitle(QCoreApplication::applicationName());

    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch(reason)
    {
    case askToSaveChangesReason_generic:
        msgBox.setText("<h3>" + tr("Do you want to save changes to «%1»?").arg(name) + "</h3>");
        msgBox.setButtonText(QMessageBox::Discard, tr("Don't Save"));
        break;
    case askToSaveChangesReason_tabClosing:
        msgBox.setText("<h3>" + tr("Do you want to save changes to «%1» before closing?").arg(name) + "</h3>");
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

int MainWindow::closeTab(EditorTabWidget *tabWidget, int tab)
{
    int result = MainWindow::tabCloseResult_AlreadySaved;
    Editor *editor = (Editor *)tabWidget->widget(tab);

    // Don't remove the tab if it's the last tab, it's empty, in an unmodified state and it's not associated with a file name.
    // Else, continue.
    if (!(tabWidget->count() == 1
         && editor->fileName() == "" && editor->isClean())) {

        if(!editor->isClean()) {
            tabWidget->setCurrentIndex(tab);
            int ret = askIfWantToSave(tabWidget, tab, askToSaveChangesReason_tabClosing);
            if(ret == QMessageBox::Save) {
                int saveResult = save(tabWidget, tab);
                if(saveResult == MainWindow::saveFileResult_Canceled)
                {
                    // The user canceled the "save dialog". Let's ignore the close event.
                    result = MainWindow::tabCloseResult_Canceled;
                } else if(saveResult == MainWindow::saveFileResult_Saved)
                {
                    tabWidget->removeTab(tab);
                    result = MainWindow::tabCloseResult_Saved;
                }
            } else if(ret == QMessageBox::Discard) {
                // Don't save
                tabWidget->removeTab(tab);
                result = MainWindow::tabCloseResult_NotSaved;
            } else if(ret == QMessageBox::Cancel) {
                // Don't save and don't close
                result = MainWindow::tabCloseResult_Canceled;
            }
        } else {
            // The tab is already saved: we can remove it safely.
            tabWidget->removeTab(tab);
            result = MainWindow::tabCloseResult_AlreadySaved;
        }
    }

    if(tabWidget->count() == 0) {
        /* Not so good... 0 tabs opened is a bad idea. So, if there are more
         * than one TabWidgets opened (split-screen) then we completely
         * remove this one. Otherwise, we add a new empty tab.
        */
        if(topEditorContainer->count() > 1) {
            tabWidget->deleteLater();
            topEditorContainer->tabWidget(0)->currentEditor()->setFocus();
        } else {
            ui->action_New->trigger();
        }
    }


    return result;
}

int MainWindow::save(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);

    if(editor->fileName() == "")
    {
        // Call "save as"
        return saveAs(tabWidget, tab, false);
    } else {
        return this->docEngine->saveDocument(tabWidget, tab, editor->fileName());
    }
}

int MainWindow::saveAs(EditorTabWidget *tabWidget, int tab, bool copy)
{
    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(
                0,
                tr("Save as"),
                getSaveDialogDefaultFileName(tabWidget, tab),
                tr("Any file (*)"),
                0, 0);

    if (filename != "") {
        settings->setValue("lastSelectedDir",
                           QFileInfo(filename).absolutePath());
        // Write
        return this->docEngine->saveDocument(tabWidget, tab, filename, copy);
    } else {
        return MainWindow::saveFileResult_Canceled;
    }
}

QString MainWindow::getSaveDialogDefaultFileName(EditorTabWidget *tabWidget, int tab)
{
    QString docFileName = tabWidget->editor(tab)->fileName();

    if(docFileName == "") {
        return settings->value("lastSelectedDir", ".").toString()
                + "/" + tabWidget->tabText(tab);
    } else {
        return docFileName;
    }
}

Editor *MainWindow::currentEditor()
{
    return this->topEditorContainer->currentTabWidget()->currentEditor();
}

void MainWindow::on_tabCloseRequested(EditorTabWidget *tabWidget, int tab)
{
    this->closeTab(tabWidget, tab);
    // FIXME Remove document monitoring
}

void MainWindow::on_actionSave_triggered()
{
    EditorTabWidget *tabW = this->topEditorContainer->currentTabWidget();
    this->save(tabW, tabW->currentIndex());
}

void MainWindow::on_actionSave_as_triggered()
{
    EditorTabWidget *tabW = this->topEditorContainer->currentTabWidget();
    this->saveAs(tabW, tabW->currentIndex(), false);
}

void MainWindow::on_actionSave_a_Copy_As_triggered()
{
    EditorTabWidget *tabW = this->topEditorContainer->currentTabWidget();
    this->saveAs(tabW, tabW->currentIndex(), true);
}

void MainWindow::on_action_Copy_triggered()
{
    QVariant text = currentEditor()->sendMessageWithResult("C_FUN_GET_SELECTIONS_TEXT");

    QApplication::clipboard()->setText(text.toStringList().join("\n"));
}

void MainWindow::on_action_Paste_triggered()
{
    currentEditor()->sendMessage("C_CMD_SET_SELECTIONS_TEXT",
                                 QApplication::clipboard()->text());
}

void MainWindow::on_actionCu_t_triggered()
{
    ui->action_Copy->trigger();
    currentEditor()->sendMessage("C_CMD_SET_SELECTIONS_TEXT", "");
}

void MainWindow::on_currentEditorChanged(EditorTabWidget *tabWidget, int tab)
{
    this->refreshEditorUiInfo(tabWidget->editor(tab));
}

void MainWindow::on_editorAdded(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    connect(editor,
            SIGNAL(cursorActivity()),
            this,
            SLOT(on_cursorActivity()));
}

void MainWindow::on_cursorActivity()
{
    Editor *editor = (Editor *)sender();

    if(currentEditor() == editor) {
        this->refreshEditorUiInfo(editor);
    }
}

void MainWindow::refreshEditorUiInfo(Editor *editor)
{
    // Update status bar
    int len = editor->sendMessageWithResult("C_FUN_GET_TEXT_LENGTH").toInt();
    int lines = editor->sendMessageWithResult("C_FUN_GET_LINE_COUNT").toInt();
    statusBar_lengthInfo->setText(tr("Length : %1     Lines : %2").arg(len).arg(lines));

    QList<QVariant> cursor = editor->sendMessageWithResult("C_FUN_GET_CURSOR").toList();
    statusBar_selectionInfo->setText(tr("Ln : %1     Col : %2     Sel : %3 | %4").
                                     arg(cursor[0].toInt() + 1).
                                     arg(cursor[1].toInt() + 1).
                                     arg(0).arg(0));
}

void MainWindow::on_action_Delete_triggered()
{
    currentEditor()->sendMessage("C_CMD_SET_SELECTIONS_TEXT", "");
}

void MainWindow::on_actionSelect_All_triggered()
{
    currentEditor()->sendMessage("C_CMD_SELECT_ALL");
}

void MainWindow::on_actionAbout_Notepadqq_triggered()
{
    frmAbout *_about;
    _about = new frmAbout(this);
    _about->exec();

    _about->deleteLater();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_action_Undo_triggered()
{
    currentEditor()->sendMessage("C_CMD_UNDO");
}

void MainWindow::on_action_Redo_triggered()
{
    currentEditor()->sendMessage("C_CMD_REDO");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    int tabWidgetsCount = topEditorContainer->count();
    for(int i = 0; i < tabWidgetsCount; i++) {
        EditorTabWidget *tabWidget = topEditorContainer->tabWidget(i);
        int tabCount = tabWidget->count();

        for(int j = 0; j < tabCount; j++) {
            Editor *editor = tabWidget->editor(j);

            if(editor->isClean() == false) {
                tabWidget->setCurrentIndex(j);
                editor->setFocus();

                int ret = askIfWantToSave(tabWidget, j, askToSaveChangesReason_tabClosing);
                if(ret == QMessageBox::Save) {
                    if(save(tabWidget, j) == MainWindow::saveFileResult_Canceled)
                    {
                        // The user canceled the "save dialog". Let's stop the close event.
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

void MainWindow::on_actionE_xit_triggered()
{
    this->close();
}
