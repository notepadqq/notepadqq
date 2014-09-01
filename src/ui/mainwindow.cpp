#include "include/mainwindow.h"
#include "ui_mainwindow.h"
#include "include/editor.h"
#include "include/editortabwidget.h"
#include "include/frmabout.h"
#include "include/frmsearchlanguage.h"
#include "include/frmpreferences.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QUrl>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_topEditorContainer(new TopEditorContainer(this))
{
    // FIXME Set /usr/share/themes QIcon::setThemeSearchPaths();

    ui->setupUi(this);

    // Gets company name from QCoreApplication::setOrganizationName(). Same for app name.
    m_settings = new QSettings();

    setCentralWidget(m_topEditorContainer);

    m_docEngine = new DocEngine(m_settings, m_topEditorContainer);
    connect(m_docEngine, &DocEngine::fileOnDiskChanged, this, &MainWindow::on_fileOnDiskChanged);

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
    m_tabContextMenu = new QMenu;
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    m_tabContextMenuActions.append(ui->actionClose);
    m_tabContextMenuActions.append(ui->actionClose_All_BUT_Current_Document);
    m_tabContextMenuActions.append(ui->actionSave);
    m_tabContextMenuActions.append(ui->actionSave_as);
    m_tabContextMenuActions.append(ui->actionRename);
    m_tabContextMenuActions.append(ui->actionDelete_from_Disk);
    m_tabContextMenuActions.append(ui->actionPrint);
    m_tabContextMenuActions.append(separator);
    m_tabContextMenuActions.append(ui->actionMove_to_Other_View);
    m_tabContextMenuActions.append(ui->actionClone_to_Other_View);
    m_tabContextMenuActions.append(ui->actionMove_to_New_Instance);
    m_tabContextMenuActions.append(ui->actionOpen_in_New_Instance);
    m_tabContextMenu->addActions(m_tabContextMenuActions);

    connect(m_topEditorContainer,
            &TopEditorContainer::customTabContextMenuRequested,
            this,
            &MainWindow::on_customTabContextMenuRequested);

    connect(m_topEditorContainer,
            &TopEditorContainer::tabCloseRequested,
            this,
            &MainWindow::on_tabCloseRequested);

    connect(m_topEditorContainer,
            &TopEditorContainer::currentEditorChanged,
            this,
            &MainWindow::on_currentEditorChanged);

    connect(m_topEditorContainer,
            &TopEditorContainer::editorAdded,
            this,
            &MainWindow::on_editorAdded);

    createStatusBar();

    setAcceptDrops(true);


    processCommandLineArgs(QApplication::arguments(), false);

    // TODO Add last 15 recent languages to Languages menu (connect to this->setCurrentEditorLanguage() slot)

    // DEBUG: Add a second tabWidget
    //this->topEditorContainer->addTabWidget()->addEditorTab(false, "test");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createStatusBar()
{
    QStatusBar* status = statusBar();
    QLabel* label;

    status->setFixedHeight(22);

    label = new QLabel("File Format", this);
    label->setMinimumWidth(160);
    status->addWidget(label);
    m_statusBar_fileFormat = label;

    label = new QLabel("Length : 0     Lines : 1", this);
    status->addWidget(label);
    m_statusBar_lengthInfo = label;

    label = new QLabel("Ln : 0     Col : 1     Sel : 0 | 0", this);
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    status->addWidget(label);
    m_statusBar_selectionInfo = label;

    label = new QLabel("EOL", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    label->setMinimumWidth(128);
    status->addWidget(label);
    m_statusBar_EOLstyle = label;

    label = new QLabel("Encoding", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    label->setMinimumWidth(128);
    status->addWidget(label);
    m_statusBar_textFormat = label;

    label = new QLabel("INS", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    label->setMinimumWidth(40);
    status->addWidget(label);
    m_statusBar_overtypeNotify = label;
}

void MainWindow::processCommandLineArgs(QStringList arguments, bool fromOtherInstance)
{
    bool activateWnd = false;

    if(arguments.count() <= 1)
    {
        // Open a new empty document
        ui->action_New->trigger();
        activateWnd = true;
    }
    else
    {
        // Open selected files
        QStringList files;
        for(int i = 1; i < arguments.count(); i++)
        {
            files.append(arguments.at(i));
        }

        EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
        // Make sure we have a tabWidget: if not, create it.
        if(tabW == 0) {
            tabW = m_topEditorContainer->addTabWidget();
        }
        m_docEngine->loadDocuments(files, tabW, false);
        activateWnd = true;
    }

    if(fromOtherInstance && activateWnd)
    {
        // Activate the window
        activateWindow();
        raise();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QStringList fileNames;
    foreach (const QUrl &url, e->mimeData()->urls()) {
        fileNames.append(url.toLocalFile());
    }

    if (!fileNames.empty()) {
        m_docEngine->loadDocuments(fileNames,
                                   m_topEditorContainer->currentTabWidget(),
                                   false);
    }
}

void MainWindow::on_action_New_triggered()
{
    static int num = 1; // FIXME maybe find smarter way
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();

    // Make sure we have a tabWidget: if not, create it.
    if(tabW == 0) {
        tabW = m_topEditorContainer->addTabWidget();
    }

    tabW->addEditorTab(true, tr("new %1").arg(num));
    num++;
}

void MainWindow::setCurrentEditorLanguage(QString language)
{
    currentEditor()->setLanguage(language);
}

void MainWindow::on_customTabContextMenuRequested(QPoint point, EditorTabWidget * /*tabWidget*/, int /*tabIndex*/)
{
    m_tabContextMenu->exec(point);
}

void MainWindow::on_actionMove_to_Other_View_triggered()
{
    EditorTabWidget *curTabWidget = m_topEditorContainer->currentTabWidget();
    EditorTabWidget *destTabWidget;

    if(m_topEditorContainer->count() >= 2) {
        int viewId = 1;
        if(m_topEditorContainer->widget(1) == curTabWidget) {
            viewId = 0;
        }

        destTabWidget = (EditorTabWidget *)m_topEditorContainer->widget(viewId);

    } else {
        destTabWidget = m_topEditorContainer->addTabWidget();
    }

    destTabWidget->transferEditorTab(true, curTabWidget, curTabWidget->currentIndex());

    removeTabWidgetIfEmpty(curTabWidget);
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
                m_settings->value("lastSelectedDir", ".").toString(),
                tr("All files (*)"),
                0, 0);

    m_docEngine->loadDocuments(fileNames,
                               m_topEditorContainer->currentTabWidget(),
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

int MainWindow::closeTab(EditorTabWidget *tabWidget, int tab, bool remove, bool force)
{
    int result = MainWindow::tabCloseResult_AlreadySaved;
    Editor *editor = (Editor *)tabWidget->widget(tab);

    // Don't remove the tab if it's the last tab, it's empty, in an unmodified state and it's not associated with a file name.
    // Else, continue.
    if (! (m_topEditorContainer->count() == 1 && tabWidget->count() == 1
         && editor->fileName() == "" && editor->isClean())) {

        if(!force && !editor->isClean()) {
            tabWidget->setCurrentIndex(tab);
            int ret = askIfWantToSave(tabWidget, tab, askToSaveChangesReason_tabClosing);
            if(ret == QMessageBox::Save) {
                // Save
                int saveResult = save(tabWidget, tab);
                if(saveResult == MainWindow::saveFileResult_Canceled)
                {
                    // The user canceled the "save dialog". Let's ignore the close event.
                    result = MainWindow::tabCloseResult_Canceled;
                } else if(saveResult == MainWindow::saveFileResult_Saved)
                {
                    if (remove) m_docEngine->closeDocument(tabWidget, tab);
                    result = MainWindow::tabCloseResult_Saved;
                }
            } else if(ret == QMessageBox::Discard) {
                // Don't save and close
                if (remove) m_docEngine->closeDocument(tabWidget, tab);
                result = MainWindow::tabCloseResult_NotSaved;
            } else if(ret == QMessageBox::Cancel) {
                // Don't save and cancel closing
                result = MainWindow::tabCloseResult_Canceled;
            }
        } else {
            // The tab is already saved: we can remove it safely.
            if (remove) m_docEngine->closeDocument(tabWidget, tab);
            result = MainWindow::tabCloseResult_AlreadySaved;
        }

        // Ensure the focus is still on this tabWidget
        if (tabWidget->count() > 0) {
            tabWidget->currentEditor()->setFocus();
        }
    }

    if(tabWidget->count() == 0) {
        /* Not so good... 0 tabs opened is a bad idea. So, if there are more
         * than one TabWidgets opened (split-screen) then we completely
         * remove this one. Otherwise, we add a new empty tab.
        */
        if(m_topEditorContainer->count() > 1) {
            tabWidget->deleteLater();
            m_topEditorContainer->tabWidget(0)->currentEditor()->setFocus();
        } else {
            ui->action_New->trigger();
        }
    }


    return result;
}

int MainWindow::closeTab(EditorTabWidget *tabWidget, int tab)
{
    return closeTab(tabWidget, tab, true, false);
}

int MainWindow::save(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);

    if(editor->fileName() == "")
    {
        // Call "save as"
        return saveAs(tabWidget, tab, false);

    } else {
        // If the file has changed outside the editor, ask
        // the user if he want to save it.
        QFile file(editor->fileName());
        if (editor->fileOnDiskChanged() && file.exists()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(QCoreApplication::applicationName());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("<h3>" +
                           tr("The file on disk has changed since the last "
                              "read.\nDo you want to save it anyway?") +
                           "</h3>");
            msgBox.setInformativeText(tr("Saving the file might cause "
                                         "loss of external data."));
            msgBox.setStandardButtons(QMessageBox::Save |
                                      QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Cancel)
                return MainWindow::saveFileResult_Canceled;
        }

        return m_docEngine->saveDocument(tabWidget, tab, editor->fileName());
    }
}

int MainWindow::saveAs(EditorTabWidget *tabWidget, int tab, bool copy)
{
    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(
                this,
                tr("Save as"),
                getSaveDialogDefaultFileName(tabWidget, tab),
                tr("Any file (*)"),
                0, 0);

    if (filename != "") {
        m_settings->setValue("lastSelectedDir",
                           QFileInfo(filename).absolutePath());
        // Write
        return m_docEngine->saveDocument(tabWidget, tab, filename, copy);
    } else {
        return MainWindow::saveFileResult_Canceled;
    }
}

QString MainWindow::getSaveDialogDefaultFileName(EditorTabWidget *tabWidget, int tab)
{
    QString docFileName = tabWidget->editor(tab)->fileName();

    if(docFileName == "") {
        return m_settings->value("lastSelectedDir", ".").toString()
                + "/" + tabWidget->tabText(tab);
    } else {
        return docFileName;
    }
}

Editor *MainWindow::currentEditor()
{
    return m_topEditorContainer->currentTabWidget()->currentEditor();
}

void MainWindow::on_tabCloseRequested(EditorTabWidget *tabWidget, int tab)
{
    closeTab(tabWidget, tab);
}

void MainWindow::on_actionSave_triggered()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    save(tabW, tabW->currentIndex());
}

void MainWindow::on_actionSave_as_triggered()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    saveAs(tabW, tabW->currentIndex(), false);
}

void MainWindow::on_actionSave_a_Copy_As_triggered()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    saveAs(tabW, tabW->currentIndex(), true);
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
    refreshEditorUiInfo(tabWidget->editor(tab));
}

void MainWindow::on_editorAdded(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    connect(editor, &Editor::cursorActivity,
            this, &MainWindow::on_cursorActivity);
}

void MainWindow::on_cursorActivity()
{
    Editor *editor = (Editor *)sender();

    if(currentEditor() == editor) {
        refreshEditorUiInfo(editor);
    }
}

void MainWindow::refreshEditorUiInfo(Editor *editor)
{
    if (editor != 0) {
        // Update status bar
        int len = editor->sendMessageWithResult("C_FUN_GET_TEXT_LENGTH").toInt();
        int lines = editor->sendMessageWithResult("C_FUN_GET_LINE_COUNT").toInt();
        m_statusBar_lengthInfo->setText(tr("Length : %1     Lines : %2").arg(len).arg(lines));

        QList<QVariant> cursor = editor->sendMessageWithResult("C_FUN_GET_CURSOR").toList();
        m_statusBar_selectionInfo->setText(tr("Ln : %1     Col : %2     Sel : %3 | %4").
                                         arg(cursor[0].toInt() + 1).
                                         arg(cursor[1].toInt() + 1).
                                         arg(0).arg(0));
    }
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
    int tabWidgetsCount = m_topEditorContainer->count();
    for(int i = 0; i < tabWidgetsCount; i++) {
        EditorTabWidget *tabWidget = m_topEditorContainer->tabWidget(i);
        int tabCount = tabWidget->count();

        for(int j = 0; j < tabCount; j++) {
            int closeResult = closeTab(tabWidget, j, false, false);
            if (closeResult == MainWindow::tabCloseResult_Canceled) {
                // Cancel all
                event->ignore();
                return;
            }
        }
    }
}

void MainWindow::on_actionE_xit_triggered()
{
    close();
}

void MainWindow::on_actionSearch_triggered()
{
    if(!m_frmSearchReplace) {
        m_frmSearchReplace = new frmSearchReplace(
                            m_topEditorContainer,
                            frmSearchReplace::TabSearch,
                            this);
    }
    m_frmSearchReplace->show();
}

void MainWindow::on_actionSearchLanguage_triggered()
{
    frmSearchLanguage *_frm;
    _frm = new frmSearchLanguage(currentEditor(), this);
    int result = _frm->exec();
    if (result == QDialog::Accepted) {
        setCurrentEditorLanguage(_frm->selectedMimeType());
    }

    _frm->deleteLater();
}

void MainWindow::on_actionCurrent_Full_File_path_to_Clipboard_triggered()
{
    Editor *editor = currentEditor();
    if(currentEditor()->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(editor->fileName()).absoluteFilePath());
    } else {
        EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
        QApplication::clipboard()->setText(tabWidget->tabText(tabWidget->indexOf(editor)));
    }
}

void MainWindow::on_actionCurrent_Filename_to_Clipboard_triggered()
{
    Editor *editor = currentEditor();
    if(currentEditor()->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(editor->fileName()).fileName());
    } else {
        EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
        QApplication::clipboard()->setText(tabWidget->tabText(tabWidget->indexOf(editor)));
    }
}

void MainWindow::on_actionCurrent_Directory_Path_to_Clipboard_triggered()
{
    Editor *editor = currentEditor();
    if(currentEditor()->fileName() != "")
    {
        QApplication::clipboard()->setText(QFileInfo(editor->fileName()).absolutePath());
    } else {
        QApplication::clipboard()->setText("");
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    frmPreferences *_pref;
    _pref = new frmPreferences(this);
    _pref->exec();

    _pref->deleteLater();
}

void MainWindow::on_actionClose_triggered()
{
    closeTab(m_topEditorContainer->currentTabWidget(),
             m_topEditorContainer->currentTabWidget()->currentIndex());
}

void MainWindow::on_actionC_lose_All_triggered()
{
    // Save what needs to be saved, check if user wants to cancel the closing
    int tabWidgetsCount = m_topEditorContainer->count();
    for(int i = 0; i < tabWidgetsCount; i++) {
        EditorTabWidget *tabWidget = m_topEditorContainer->tabWidget(i);
        int tabCount = tabWidget->count();

        for(int j = 0; j < tabCount; j++) {
            int closeResult = closeTab(tabWidget, j, false, false);
            if (closeResult == MainWindow::tabCloseResult_Canceled)
                return; // Cancel all

        }
    }


    // Actually remove the tabs
    do {
        EditorTabWidget *tabWidget = m_topEditorContainer->tabWidget(0);

        do {
            int oldCount = tabWidget->count();

            closeTab(tabWidget, 0, true, true);

            if (oldCount == 1 && tabWidget->count() == 1) {
                // We removed the last tab, and a new one has been created.
                // So we're done.
                return;
            }

        } while (tabWidget->count() > 0);

    } while (1);


}

void MainWindow::on_fileOnDiskChanged(EditorTabWidget *tabWidget, int tab, bool removed)
{
    Editor *editor = tabWidget->editor(tab);

    QMessageBox msgBox;
    msgBox.setWindowTitle(QCoreApplication::applicationName());
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes |
                              QMessageBox::No |
                              QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Yes);

    if (removed) {
        // TODO Better looking msgbox
        msgBox.setText(tr("The file \"%1\" has been removed from the "
                          "file system. Would you like to save it now?")
                       .arg(editor->fileName()));

        int ret = msgBox.exec();

        if (ret == QMessageBox::Close) {
            closeTab(tabWidget, tab);
        } else if (ret == QMessageBox::Yes) {
            save(tabWidget, tab);
        }

    } else {
        // TODO Better looking msgbox
        msgBox.setText(tr("The file \"%1\" has been changed outside of "
                          "the editor. Would you like to reload it?")
                       .arg(editor->fileName()));

        int ret = msgBox.exec();

        if (ret == QMessageBox::Close) {
            closeTab(tabWidget, tab);
        } else if (ret == QMessageBox::Yes) {
            m_docEngine->loadDocuments(QStringList(editor->fileName()),
                                       tabWidget,
                                       true);
        }
    }
}
