#include "include/mainwindow.h"

#include "include/EditorNS/editor.h"
#include "include/Extensions/Stubs/windowstub.h"
#include "include/Extensions/extensionsloader.h"
#include "include/Extensions/installextension.h"
#include "include/Sessions/backupservice.h"
#include "include/clickablelabel.h"
#include "include/documentcontroller.h"
#include "include/editortabwidget.h"
#include "include/editoruicontroller.h"
#include "include/frmabout.h"
#include "include/frmpreferences.h"
#include "include/notepadqq.h"
#include "include/nqqrun.h"
#include "include/windowuicontroller.h"
#include "ui_mainwindow.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPageSetupDialog>
#include <QScrollArea>
#include <QScrollBar>
#include <QTemporaryFile>
#include <QUrl>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QtPromise>

QList<MainWindow*> MainWindow::m_instances = QList<MainWindow*>();

MainWindow::MainWindow(const QString& workingDirectory, const QStringList& arguments, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_topEditorContainer(new TopEditorContainer(this))
    , m_settings(NqqSettings::getInstance())
    , m_workingDirectory(workingDirectory)
    , m_advSearchDock(new AdvancedSearchDock(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    MainWindow::m_instances.append(this);

    // Gets company name from QCoreApplication::setOrganizationName(). Same for app name.
    setCentralWidget(m_topEditorContainer);

    m_docEngine = new DocEngine(m_topEditorContainer);
    m_documentController = new DocumentController(*this, *m_docEngine, *m_topEditorContainer, m_settings);

    m_windowUiController = new WindowUiController(*this, *ui, m_settings, *m_advSearchDock);
    m_windowUiController->configureStaticUi();
    m_editorUiController = new EditorUiController(*this, *ui, *m_docEngine, *m_topEditorContainer, m_settings);

    // Printing a WebEnginePage not supported prior to 5.8
#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
    ui->actionPrint->setEnabled(false);
    ui->actionPrint->setVisible(false);
#endif

    // Context menu initialization
    m_tabContextMenu = new QMenu(this);
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    QAction* separatorBottom = new QAction(this);
    separatorBottom->setSeparator(true);
    m_tabContextMenuActions.append(ui->actionClose);
    m_tabContextMenuActions.append(ui->actionClose_All_BUT_Current_Document);
    m_tabContextMenuActions.append(ui->actionCloseLeft);
    m_tabContextMenuActions.append(ui->actionCloseRight);
    m_tabContextMenuActions.append(ui->actionSave);
    m_tabContextMenuActions.append(ui->actionSave_as);
    m_tabContextMenuActions.append(ui->actionRename);
    m_tabContextMenuActions.append(ui->actionPrint);
    m_tabContextMenuActions.append(separator);
    m_tabContextMenuActions.append(ui->actionCurrent_Full_File_Path_to_Clipboard);
    m_tabContextMenuActions.append(ui->actionCurrent_Filename_to_Clipboard);
    m_tabContextMenuActions.append(ui->actionCurrent_Directory_Path_to_Clipboard);
    m_tabContextMenuActions.append(separatorBottom);
    m_tabContextMenuActions.append(ui->actionMove_to_Other_View);
    m_tabContextMenuActions.append(ui->actionClone_to_Other_View);
    m_tabContextMenuActions.append(ui->actionMove_to_New_Window);
    m_tabContextMenuActions.append(ui->actionOpen_in_New_Window);
    m_tabContextMenu->addActions(m_tabContextMenuActions);

    fixKeyboardShortcuts();

    connect(m_topEditorContainer,
        &TopEditorContainer::customTabContextMenuRequested,
        this,
        &MainWindow::on_customTabContextMenuRequested);

    connect(m_topEditorContainer, &TopEditorContainer::tabCloseRequested, this, &MainWindow::on_tabCloseRequested);

    connect(m_topEditorContainer, &TopEditorContainer::tabBarDoubleClicked, this, &MainWindow::on_tabBarDoubleClicked);

    updateRecentDocsInMenu();

    setAcceptDrops(true);

    // Initialize at least one editor here so things like restoring "zoom"
    // work properly
    openCommandLineProvidedUrls(workingDirectory, arguments);
    configureUserInterface();
    loadToolBar();

    setupLanguagesMenu();

    // Registers all actions so that NqqSettings knows their default and current shortcuts.
    const QList<QAction*> allActions = getActions();

    m_settings.Shortcuts.initShortcuts(allActions);

    // At this point, all actions still have their default shortcuts so we set all actions'
    // shortcuts from settings.
    for (QAction* a : allActions) {
        if (a->objectName().isEmpty())
            continue;

        QKeySequence shortcut = m_settings.Shortcuts.getShortcut(a->objectName());

        a->setShortcut(shortcut);
    }

    // Register our meta types for signal/slot calls here.
    emit Notepadqq::getInstance().newWindow(this);
}

MainWindow::MainWindow(const QStringList& arguments, QWidget* parent)
    : MainWindow(QDir::currentPath(), arguments, parent)
{
}

MainWindow::~MainWindow()
{
    MainWindow::m_instances.removeAll(this);

    delete m_editorUiController;
    m_editorUiController = nullptr;
    delete ui;
    delete m_docEngine;
}

QList<MainWindow*> MainWindow::instances()
{ return MainWindow::m_instances; }

MainWindow* MainWindow::lastActiveInstance()
{
    if (m_instances.length() > 0) {
        return m_instances.last();
    } else {
        return nullptr;
    }
}

TopEditorContainer* MainWindow::topEditorContainer()
{ return m_topEditorContainer; }

void MainWindow::configureUserInterface()
{
    m_editorUiController->configureUiFromSettings();

    // Restore full screen
    ui->actionFull_Screen->setChecked(isFullScreen());

    m_windowUiController->restoreWindowState();
}

void MainWindow::loadToolBar()
{ m_windowUiController->loadToolBar(); }

QList<const QMenu*> MainWindow::getMenus() const
{ return ui->menuBar->findChildren<const QMenu*>(QString(), Qt::FindDirectChildrenOnly); }

DocEngine* MainWindow::getDocEngine() const
{ return m_docEngine; }

// Return a list of all available action items in the menu
QList<QAction*> MainWindow::getActions() const
{
    const QList<const QMenu*> list = ui->menuBar->findChildren<const QMenu*>();
    QList<QAction*> allActions;

    for (auto&& menu : list) {
        if (menu->title() == "&Language")
            continue;

        for (auto&& action : menu->actions()) {
            allActions.append(action);
        }
    }

    return allActions;
}

void MainWindow::setupLanguagesMenu()
{ m_editorUiController->setupLanguagesMenu(); }

void MainWindow::fixKeyboardShortcuts()
{
    QList<QMenu*> lst;
    lst = ui->menuBar->findChildren<QMenu*>();

    foreach (QMenu* m, lst) {
        addAction(m->menuAction());
        addActions(m->actions());
    }
}

QUrl MainWindow::stringToUrl(QString fileName, QString workingDirectory)
{ return m_documentController->stringToUrl(fileName, workingDirectory); }

void MainWindow::openCommandLineProvidedUrls(const QString& workingDirectory, const QStringList& arguments)
{ m_documentController->openCommandLineProvidedUrls(workingDirectory, arguments); }

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
    QMainWindow::dragEnterEvent(e);
    m_documentController->handleDragEnter(e);
}

void MainWindow::dropEvent(QDropEvent* e)
{
    QMainWindow::dropEvent(e);
    m_documentController->handleDrop(e);
}

void MainWindow::on_editorUrlsDropped(QList<QUrl> urls)
{ m_documentController->openDroppedUrls(urls, qobject_cast<Editor*>(sender())); }

void MainWindow::keyPressEvent(QKeyEvent* ev)
{
    if (ev->key() == Qt::Key_Insert) {
        if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            on_actionPaste_triggered();
        } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
            on_actionCopy_triggered();
        } else {
            toggleOverwrite();
        }
    } else if (ev->key() >= Qt::Key_1 && ev->key() <= Qt::Key_9 &&
               QApplication::keyboardModifiers().testFlag(Qt::AltModifier)) {
        m_topEditorContainer->currentTabWidget()->setCurrentIndex(ev->key() - Qt::Key_1);
    } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) && ev->key() == Qt::Key_PageDown) {
        // switch to the next tab to the right or wrap around if last
        EditorTabWidget* curTabWidget = m_topEditorContainer->currentTabWidget();
        int nextTabIndex = (curTabWidget->currentIndex() + 1) % curTabWidget->count();
        curTabWidget->setCurrentIndex(nextTabIndex);
    } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) && ev->key() == Qt::Key_PageUp) {
        // switch to the previous tab or wrap around if first
        EditorTabWidget* curTabWidget = m_topEditorContainer->currentTabWidget();
        int prevTabIndex = (curTabWidget->currentIndex() + curTabWidget->count() - 1) % curTabWidget->count();
        curTabWidget->setCurrentIndex(prevTabIndex);
    } else {
        QMainWindow::keyPressEvent(ev);
    }
}

void MainWindow::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            if (m_instances.length() > 0 && m_instances.last() != this) {
                int pos = m_instances.indexOf(this);
                if (pos > -1) {
                    // Move this instance at the end of the list
                    m_instances.move(pos, m_instances.length() - 1);
                }
            }
        }
    }
}

void MainWindow::toggleOverwrite()
{ m_editorUiController->toggleOverwrite(); }

void MainWindow::on_actionNew_triggered()
{
    EditorTabWidget* tabW = m_topEditorContainer->currentTabWidget();

    m_docEngine->addNewDocument(m_docEngine->getNewDocumentName(), true, tabW);
}

void MainWindow::setCurrentEditorLanguage(QString language)
{ m_editorUiController->setCurrentEditorLanguage(language); }

void MainWindow::on_customTabContextMenuRequested(QPoint point, EditorTabWidget* /*tabWidget*/, int /*tabIndex*/)
{ m_tabContextMenu->exec(point); }

void MainWindow::on_actionShow_Tabs_triggered(bool on)
{ m_editorUiController->setTabsVisible(on); }

void MainWindow::on_actionShow_Spaces_triggered(bool on)
{ m_editorUiController->setSpacesVisible(on); }

void MainWindow::on_actionShow_End_of_Line_triggered(bool on)
{ m_editorUiController->setEndOfLineVisible(on); }

void MainWindow::on_actionShow_All_Characters_toggled(bool on)
{ m_editorUiController->setSymbols(on); }

void MainWindow::on_actionMath_Rendering_toggled(bool on)
{ m_editorUiController->setMathRendering(on); }

void MainWindow::on_actionMove_to_Other_View_triggered()
{
    EditorTabWidget* curTabWidget = m_topEditorContainer->currentTabWidget();
    EditorTabWidget* destTabWidget = m_topEditorContainer->inactiveTabWidget(true);

    destTabWidget->transferEditorTab(true, curTabWidget, curTabWidget->currentIndex());

    removeTabWidgetIfEmpty(curTabWidget);
}

void MainWindow::removeTabWidgetIfEmpty(EditorTabWidget* tabWidget)
{
    if (tabWidget->count() == 0) {
        delete tabWidget;
    }
}

void MainWindow::on_actionOpen_triggered()
{ m_documentController->openFiles(); }

void MainWindow::on_actionOpen_Folder_triggered()
{ m_documentController->openFolder(); }

int MainWindow::closeTab(EditorTabWidget* tabWidget, int tab, bool remove, bool force)
{ return m_documentController->closeTab(tabWidget, tab, remove, force); }

int MainWindow::closeTab(EditorTabWidget* tabWidget, int tab)
{ return m_documentController->closeTab(tabWidget, tab); }

int MainWindow::save(EditorTabWidget* tabWidget, int tab)
{ return m_documentController->save(tabWidget, tab); }

int MainWindow::saveAs(EditorTabWidget* tabWidget, int tab, bool copy)
{ return m_documentController->saveAs(tabWidget, tab, copy); }

Editor* MainWindow::currentEditor()
{ return m_editorUiController->currentEditor(); }

QAction* MainWindow::addExtensionMenuItem(QString extensionId, QString text)
{
    QMap<QString, QSharedPointer<Extensions::Extension>> extensions = Extensions::ExtensionsLoader::loadedExtensions();

    if (extensions.contains(extensionId)) {
        QSharedPointer<Extensions::Extension> extension = extensions.value(extensionId);

        // Create the menu for the extension if it doesn't exist yet.
        if (!m_extensionMenus.contains(extension)) {
            QMenu* menu = new QMenu(extension->name(), this);
            ui->menu_Extensions->addMenu(menu);
            m_extensionMenus.insert(extension, menu);
        }

        // Create the menu item
        QAction* action = new QAction(text, this);
        m_extensionMenus[extension]->addAction(action);

        return action;
    } else {
        // Invalid extension id
        return NULL;
    }
}

void MainWindow::on_tabCloseRequested(EditorTabWidget* tabWidget, int tab)
{ closeTab(tabWidget, tab); }

void MainWindow::on_actionSave_triggered()
{
    EditorTabWidget* tabW = m_topEditorContainer->currentTabWidget();
    save(tabW, tabW->currentIndex());
}

void MainWindow::on_actionSave_as_triggered()
{
    EditorTabWidget* tabW = m_topEditorContainer->currentTabWidget();
    saveAs(tabW, tabW->currentIndex(), false);
}

void MainWindow::on_actionSave_a_Copy_As_triggered()
{
    EditorTabWidget* tabW = m_topEditorContainer->currentTabWidget();
    saveAs(tabW, tabW->currentIndex(), true);
}

void MainWindow::on_actionCopy_triggered()
{ m_editorUiController->copySelections(); }

void MainWindow::on_actionPaste_triggered()
{ m_editorUiController->pasteSelections(); }

void MainWindow::on_actionCut_triggered()
{ m_editorUiController->cutSelections(); }

void MainWindow::on_actionBegin_End_Select_triggered()
{ m_editorUiController->beginEndSelect(); }

void MainWindow::on_currentEditorChanged(EditorTabWidget* tabWidget, int tab)
{ m_editorUiController->currentEditorChanged(tabWidget, tab); }

void MainWindow::on_editorAdded(EditorTabWidget* tabWidget, int tab)
{ m_editorUiController->connectEditor(tabWidget, tab); }

void MainWindow::on_cursorActivity(QMap<QString, QVariant> data)
{ m_editorUiController->cursorActivity(qobject_cast<Editor*>(sender()), data); }

void MainWindow::refreshEditorUiCursorInfo(QMap<QString, QVariant> data)
{ m_editorUiController->refreshCursorInfo(data); }

void MainWindow::on_currentLanguageChanged(Editor* sender, QString /*id*/, QString /*name*/)
{ m_editorUiController->currentLanguageChanged(sender); }

void MainWindow::searchDockItemInteracted(const DocResult& doc, const MatchResult* result, SearchUserInteraction type)
{
    if (type == SearchUserInteraction::OpenContainingFolder) {
        QUrl fileUrl;

        if (doc.docType == DocResult::TypeDocument)
            fileUrl = doc.editor->filePath();
        else
            fileUrl = stringToUrl(doc.fileName);

        if (fileUrl.isEmpty())
            return;

        QFileInfo fInfo(fileUrl.toLocalFile());
        QString dirName = fInfo.dir().path();
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirName));
        return;
    }

    // Else: type == OpenDocument
    if (doc.docType == DocResult::TypeDocument) {
        if (!doc.editor)
            return;
        // Make sure the editor is still open by searching for it first.
        Editor* found = doc.editor;
        EditorTabWidget* parentWidget = m_topEditorContainer->tabWidgetFromEditor(found);
        if (!parentWidget)
            return;

        parentWidget->setCurrentWidget(found);
        if (result) {
            found->setSelection(result->lineNumber - 1,
                result->positionInLine, // selection start
                result->lineNumber - 1,
                result->positionInLine + result->matchLength); // selection end
        }
        found->setFocus();

    } else if (doc.docType == DocResult::TypeFile) {
        // Check the file's existence before trying to open it through the DocEngine. that is needed because
        // DocEngine will even open nonexistent documents and just show them as empty.
        if (!QFile(doc.fileName).exists())
            return;

        QUrl url = stringToUrl(doc.fileName);

        m_docEngine->getDocumentLoader()
            .setUrl(url)
            .setTabWidget(m_topEditorContainer->currentTabWidget())
            .execute()
            .wait(); // FIXME Transform to async

        QPair<int, int> pos = m_docEngine->findOpenEditorByUrl(url);

        if (pos.first == -1 || pos.second == -1)
            return;

        auto editor = m_topEditorContainer->tabWidget(pos.first)->editor(pos.second);

        if (result) {
            editor->setSelection(result->lineNumber - 1,
                result->positionInLine, // selection start
                result->lineNumber - 1,
                result->positionInLine + result->matchLength); // selection end
        }
        editor->setFocus();
    }
}

void MainWindow::refreshEditorUiInfo(Editor* editor)
{ m_editorUiController->refreshCurrentEditor(editor); }

void MainWindow::on_actionDelete_triggered()
{ m_editorUiController->deleteSelections(); }

void MainWindow::on_actionSelect_All_triggered()
{ m_editorUiController->selectAll(); }

void MainWindow::on_actionSet_RTL_triggered()
{ m_editorUiController->setRightToLeft(); }

void MainWindow::on_actionSet_LTR_triggered()
{ m_editorUiController->setLeftToRight(); }

void MainWindow::on_actionAbout_Notepadqq_triggered()
{
    frmAbout* _about;
    _about = new frmAbout(this);
    _about->exec();

    _about->deleteLater();
}

void MainWindow::on_actionAbout_Qt_triggered()
{ QApplication::aboutQt(); }

void MainWindow::on_actionUndo_triggered()
{ m_editorUiController->undo(); }

void MainWindow::on_actionRedo_triggered()
{ m_editorUiController->redo(); }

void MainWindow::closeEvent(QCloseEvent* event)
{
    QMainWindow::closeEvent(event);

    // Only save tabs to cache if the closing window is the last one in the process.
    if (!m_documentController->prepareToClose()) {
        event->ignore();
        return;
    }

    m_settings.MainWindow.setGeometry(saveGeometry());
    m_settings.MainWindow.setWindowState(saveState());

    // Disconnect signals to avoid handling events while
    // the UI is being destroyed.
    m_topEditorContainer->disconnectAllTabWidgets(); // Fixes segfault on exit
    disconnect(m_topEditorContainer, 0, this, 0);
    for (QObject* signalSource : findChildren<QObject*>())
        QObject::disconnect(signalSource, nullptr, m_editorUiController, nullptr);
}

void MainWindow::on_actionExit_triggered()
{ close(); }

void MainWindow::instantiateFrmSearchReplace()
{
    if (!m_frmSearchReplace) {
        m_frmSearchReplace = new frmSearchReplace(m_topEditorContainer, this);

        connect(m_frmSearchReplace, &frmSearchReplace::toggleAdvancedSearch, this, [this]() {
            m_advSearchDock->show(!m_advSearchDock->isVisible(), true);
        });
    }
}

void MainWindow::on_actionSearch_triggered()
{
    if (!m_frmSearchReplace) {
        instantiateFrmSearchReplace();
    }

    currentEditor()->selectedTexts().then([=, this](QStringList sel) {
        if (sel.length() > 0 && sel[0].length() > 0) {
            m_frmSearchReplace->setSearchText(sel[0]);
        }

        m_frmSearchReplace->show(frmSearchReplace::TabSearch);
        m_frmSearchReplace->activateWindow();
    });
}

void MainWindow::on_actionCurrent_Full_File_Path_to_Clipboard_triggered()
{
    auto editor = currentEditor();
    if (editor->filePath().isEmpty()) {
        EditorTabWidget* tabWidget = m_topEditorContainer->currentTabWidget();
        QApplication::clipboard()->setText(tabWidget->tabText(tabWidget->indexOf(editor)));
    } else {
        QApplication::clipboard()->setText(
            editor->filePath().toDisplayString(QUrl::PreferLocalFile | QUrl::RemovePassword));
    }
}

void MainWindow::on_actionCurrent_Filename_to_Clipboard_triggered()
{
    auto editor = currentEditor();
    if (editor->filePath().isEmpty()) {
        EditorTabWidget* tabWidget = m_topEditorContainer->currentTabWidget();
        QApplication::clipboard()->setText(tabWidget->tabText(tabWidget->indexOf(editor)));
    } else {
        QApplication::clipboard()->setText(Notepadqq::fileNameFromUrl(editor->filePath()));
    }
}

void MainWindow::on_actionCurrent_Directory_Path_to_Clipboard_triggered()
{
    auto editor = currentEditor();
    if (editor->filePath().isEmpty()) {
        QApplication::clipboard()->setText("");
    } else {
        QApplication::clipboard()->setText(editor->filePath().toDisplayString(
            QUrl::RemovePassword | QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::RemoveAuthority | QUrl::RemoveQuery |
            QUrl::RemoveFragment | QUrl::PreferLocalFile | QUrl::RemoveFilename | QUrl::NormalizePathSegments));
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    frmPreferences* _pref;
    _pref = new frmPreferences(m_topEditorContainer, this);
    _pref->exec();
    _pref->deleteLater();
}

void MainWindow::on_actionClose_triggered()
{ closeTab(m_topEditorContainer->currentTabWidget(), m_topEditorContainer->currentTabWidget()->currentIndex()); }

void MainWindow::on_actionClose_All_triggered()
{ m_documentController->closeAll(); }

void MainWindow::on_actionReplace_triggered()
{
    if (!m_frmSearchReplace) {
        instantiateFrmSearchReplace();
    }

    currentEditor()->selectedTexts().then([=, this](QStringList sel) {
        if (sel.length() > 0 && sel[0].length() > 0) {
            m_frmSearchReplace->setSearchText(sel[0]);
        }

        m_frmSearchReplace->show(frmSearchReplace::TabReplace);
        m_frmSearchReplace->activateWindow();
    });
}

void MainWindow::on_actionPlain_text_triggered()
{ m_editorUiController->setPlainText(); }

void MainWindow::on_actionRestore_Default_Zoom_triggered()
{ m_editorUiController->restoreDefaultZoom(); }

void MainWindow::on_actionZoom_In_triggered()
{ m_editorUiController->adjustZoom(0.25); }

void MainWindow::on_actionZoom_Out_triggered()
{ m_editorUiController->adjustZoom(-0.25); }

void MainWindow::on_editorMouseWheel(EditorTabWidget* tabWidget, int tab, QWheelEvent* ev)
{ m_editorUiController->handleMouseWheel(tabWidget, tab, ev); }

void MainWindow::on_actionUPPERCASE_triggered()
{
    m_editorUiController->transformSelectedText([](const QString& str) { return str.toUpper(); });
}

void MainWindow::on_actionLowercase_triggered()
{
    m_editorUiController->transformSelectedText([](const QString& str) { return str.toLower(); });
}

void MainWindow::on_actionClose_All_BUT_Current_Document_triggered()
{ m_documentController->closeAllExceptCurrent(); }

void MainWindow::on_actionCloseLeft_triggered()
{ m_documentController->closeLeft(); }

void MainWindow::on_actionCloseRight_triggered()
{ m_documentController->closeRight(); }

void MainWindow::on_actionSave_All_triggered()
{ m_documentController->saveAll(); }

void MainWindow::on_bannerRemoved(QWidget* banner)
{ m_editorUiController->removeBanner(banner); }

void MainWindow::checkIndentationMode(Editor* editor)
{ m_editorUiController->checkIndentationMode(editor); }

void MainWindow::updateRecentDocsInMenu()
{
    QList<QVariant> recentDocs = m_settings.General.getRecentDocuments();

    ui->menuRecent_Files->clear();

    QList<QAction*> actions;
    for (QVariant recentDoc : recentDocs) {
        QUrl url = recentDoc.toUrl();
        QAction* action = new QAction(Notepadqq::fileNameFromUrl(url), this);
        connect(action, &QAction::triggered, this, [this, url]() { openRecentFileEntry(url); });

        actions.append(action);
    }

    // If there are no recent files, show a placeholder
    bool anyRecentDoc = (actions.count() != 0);
    if (!anyRecentDoc) {
        QAction* action = new QAction(tr("No recent files"), this);
        action->setEnabled(false);
        actions.append(action);
    }

    ui->menuRecent_Files->addActions(actions);

    if (anyRecentDoc) {
        ui->menuRecent_Files->addSeparator();
        ui->menuRecent_Files->addActions({ui->actionOpen_All_Recent_Files, ui->actionEmpty_Recent_Files_List});
    }
}

void MainWindow::on_actionReload_from_Disk_triggered()
{ m_documentController->reloadCurrentDocument(); }

void MainWindow::on_actionFind_Next_triggered()
{
    if (m_frmSearchReplace)
        m_frmSearchReplace->findFromUI(true);
}

void MainWindow::on_actionFind_Previous_triggered()
{
    if (m_frmSearchReplace)
        m_frmSearchReplace->findFromUI(false);
}

void MainWindow::on_actionRename_triggered()
{ m_documentController->renameCurrentDocument(); }

void MainWindow::on_actionWord_wrap_toggled(bool on)
{ m_editorUiController->setWordWrap(on); }

void MainWindow::on_actionEmpty_Recent_Files_List_triggered()
{ m_documentController->clearRecentFiles(); }

void MainWindow::on_actionOpen_All_Recent_Files_triggered()
{ m_documentController->openAllRecentFiles(); }

void MainWindow::on_actionUNIX_Format_triggered()
{ m_editorUiController->setEndOfLineSequence("\n"); }

void MainWindow::on_actionWindows_Format_triggered()
{ m_editorUiController->setEndOfLineSequence("\r\n"); }

void MainWindow::on_actionMac_Format_triggered()
{ m_editorUiController->setEndOfLineSequence("\r"); }

void MainWindow::on_actionUTF_8_triggered()
{ m_editorUiController->convertCurrentEditorEncoding(QTextCodec::codecForName("UTF-8"), true); }

void MainWindow::on_actionUTF_8_without_BOM_triggered()
{ m_editorUiController->convertCurrentEditorEncoding(QTextCodec::codecForName("UTF-8"), false); }

void MainWindow::on_actionUTF_16BE_triggered()
{ m_editorUiController->convertCurrentEditorEncoding(QTextCodec::codecForName("UTF-16BE"), true); }

void MainWindow::on_actionUTF_16LE_triggered()
{ m_editorUiController->convertCurrentEditorEncoding(QTextCodec::codecForName("UTF-16LE"), true); }

void MainWindow::on_actionInterpret_as_UTF_8_triggered()
{ m_editorUiController->reinterpretCurrentEditorEncoding(QTextCodec::codecForName("UTF-8"), true); }

void MainWindow::on_actionInterpret_as_UTF_8_without_BOM_triggered()
{ m_editorUiController->reinterpretCurrentEditorEncoding(QTextCodec::codecForName("UTF-8"), false); }

void MainWindow::on_actionInterpret_as_UTF_16BE_UCS_2_Big_Endian_triggered()
{ m_editorUiController->reinterpretCurrentEditorEncoding(QTextCodec::codecForName("UTF-16BE"), true); }

void MainWindow::on_actionInterpret_as_UTF_16LE_UCS_2_Little_Endian_triggered()
{ m_editorUiController->reinterpretCurrentEditorEncoding(QTextCodec::codecForName("UTF-16LE"), true); }

void MainWindow::on_actionConvert_to_triggered()
{ m_editorUiController->chooseEncodingForConversion(); }

void MainWindow::on_actionReload_File_Interpreted_As_triggered()
{ m_editorUiController->chooseEncodingForReload(); }

void MainWindow::on_actionIndentation_Default_Settings_triggered()
{ m_editorUiController->useDefaultIndentation(); }

void MainWindow::on_actionIndentation_Custom_triggered()
{ m_editorUiController->chooseCustomIndentation(); }

void MainWindow::on_actionInterpret_As_triggered()
{ m_editorUiController->chooseEncodingForInterpretation(); }

void MainWindow::generateRunMenu()
{ m_windowUiController->generateRunMenu(); }

/**
 * @brief Configure any user interface after loading session
 */
void MainWindow::configurePostSessionUserInterface()
{ m_editorUiController->restoreSavedZoom(); }

void MainWindow::modifyRunCommands()
{
    NqqRun::RunPreferences p;
    if (p.exec() == 1) {
        generateRunMenu();
    }
}

void MainWindow::runCommand()
{
    QAction* a = qobject_cast<QAction*>(sender());
    QString command;

    if (a->data().toString().size()) {
        command = a->data().toString();
    } else {
        NqqRun::RunDialog rd;
        int ok = rd.exec();

        if (rd.saved()) {
            generateRunMenu();
        }

        if (!ok) {
            return;
        }

        command = rd.getCommandInput();
    }

    auto editor = currentEditor();

    QUrl url = currentEditor()->filePath();
    editor->selectedTexts().then([=, this](QStringList selection) {
        QString cmd = command;
        if (!url.isEmpty()) {
            cmd.replace("\%url\%", url.toString(QUrl::None));
            cmd.replace("\%path\%", url.path(QUrl::FullyDecoded));
            cmd.replace("\%filename\%", url.fileName(QUrl::FullyDecoded));
            cmd.replace("\%directory\%", QFileInfo(url.toLocalFile()).absolutePath());
        }
        if (!selection.first().isEmpty()) {
            cmd.replace("\%selection\%", selection.first());
        }
        QStringList args = NqqRun::RunDialog::parseCommandString(cmd);
        if (!args.isEmpty()) {
            cmd = args.takeFirst();
            if (!QProcess::startDetached(cmd, args)) {}
        }
    });
}

void MainWindow::on_actionPrint_triggered()
{
    // TODO If ghostscript is available on the system, we could
    //        - show a QPrintDialog to the user
    //        - generate the pdf file
    //        - print the pdf via ghostscript
    //      https://stackoverflow.com/questions/2599925/how-to-print-pdf-on-default-network-printer-using-ghostscript-gswin32c-exe-she

    QPageSetupDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        currentEditor()->printToPdf(dlg.printer()->pageLayout()).then([this](QByteArray data) {
            QFile file(QDir::tempPath() + "/notepadqq.print." +
                       QString::number(QDateTime::currentMSecsSinceEpoch(), 16) + ".pdf");

            if (file.open(QIODevice::WriteOnly)) { // FIXME: Delete the file when we're done
                file.write(data);
                file.close();

                bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(file.fileName()));
                if (!ok) {
                    QMessageBox::warning(this,
                        QCoreApplication::applicationName(),
                        tr("%1 wasn't able to open the produced pdf file:\n%2")
                            .arg(QCoreApplication::applicationName(), file.fileName()),
                        QMessageBox::Ok,
                        QMessageBox::Ok);
                }
            }
        });
    }
}

void MainWindow::on_actionPrint_Now_triggered()
{ qWarning() << "Not implemented."; }
/*
void MainWindow::on_actionLaunch_in_Chrome_triggered()
{
    QUrl fileName = currentEditor()->fileName();
    if (!fileName.isEmpty()) {
        QStringList args;
        args << fileName.toString(QUrl::None);
        QProcess::startDetached("google-chrome", args);
    }
}
*/
QtPromise::QPromise<QStringList> MainWindow::currentWordOrSelections()
{ return m_editorUiController->currentWordOrSelections(); }

QtPromise::QPromise<QString> MainWindow::currentWordOrSelection()
{ return m_editorUiController->currentWordOrSelection(); }

void MainWindow::currentWordOnlineSearch(const QString& searchUrl)
{
    currentWordOrSelection().then([=, this](QString term) {
        if (!term.isNull() && !term.isEmpty()) {
            QUrl phpHelp = QUrl(searchUrl.arg(QString(QUrl::toPercentEncoding(term))));
            QDesktopServices::openUrl(phpHelp);
        }
    });
}

void MainWindow::openRecentFileEntry(QUrl url)
{ m_documentController->openRecentFileEntry(url); }

void MainWindow::on_actionOpen_a_New_Window_triggered()
{
    MainWindow* b = new MainWindow(QStringList(), 0);
    b->show();
}

void MainWindow::on_actionOpen_in_New_Window_triggered()
{
    QStringList args;
    args.append(QApplication::arguments().first());
    if (!currentEditor()->filePath().isEmpty()) {
        args.append(currentEditor()->filePath().toString(QUrl::None));
    }

    MainWindow* b = new MainWindow(args, 0);
    b->show();
}

void MainWindow::on_actionMove_to_New_Window_triggered()
{
    QStringList args;
    args.append(QApplication::arguments().first());
    if (!currentEditor()->filePath().isEmpty()) {
        args.append(currentEditor()->filePath().toString(QUrl::None));
    }

    EditorTabWidget* tabWidget = m_topEditorContainer->currentTabWidget();
    int tab = tabWidget->currentIndex();
    if (closeTab(tabWidget, tab) != tabCloseResult_Canceled) {
        MainWindow* b = new MainWindow(args, 0);
        b->show();
    }
}

void MainWindow::on_actionOpen_file_triggered()
{
    currentWordOrSelections().then([=, this](QStringList terms) {
        if (terms.isEmpty())
            return;

        QList<QUrl> urls;
        for (QString term : terms) {
            urls.append(QUrl::fromLocalFile(term));
        }

        m_docEngine->getDocumentLoader().setUrls(urls).setTabWidget(m_topEditorContainer->currentTabWidget()).execute();
    });
}

void MainWindow::on_actionOpen_in_another_window_triggered()
{
    currentWordOrSelections().then([=, this](QStringList terms) {
        if (!terms.isEmpty()) {
            terms.prepend(QApplication::arguments().first());

            MainWindow* b = new MainWindow(terms, 0);
            b->show();
        }
    });
}

void MainWindow::on_tabBarDoubleClicked(EditorTabWidget* tabWidget, int tab)
{
    if (tab == -1) {
        m_docEngine->addNewDocument(m_docEngine->getNewDocumentName(), true, tabWidget);
    }
}

void MainWindow::on_actionFind_in_Files_triggered()
{ m_advSearchDock->show(!m_advSearchDock->isVisible(), true); }

void MainWindow::on_actionDelete_Line_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_DELETE_LINE"); }

void MainWindow::on_actionDuplicate_Line_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_DUPLICATE_LINE"); }

void MainWindow::on_actionMove_Line_Up_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_MOVE_LINE_UP"); }

void MainWindow::on_actionMove_Line_Down_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_MOVE_LINE_DOWN"); }

void MainWindow::on_actionTranspose_Line_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_TRANSPOSE_LINE"); }

void MainWindow::on_actionTrim_Trailing_Space_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_TRIM_TRAILING_SPACE"); }

void MainWindow::on_actionTrim_Leading_Space_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_TRIM_LEADING_SPACE"); }

void MainWindow::on_actionTrim_Leading_and_Trailing_Space_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_TRIM_LEADING_TRAILING_SPACE"); }

void MainWindow::on_actionEOL_to_Space_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_EOL_TO_SPACE"); }

void MainWindow::on_actionTAB_to_Space_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_TAB_TO_SPACE"); }

void MainWindow::on_actionSpace_to_TAB_All_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_SPACE_TO_TAB_ALL"); }

void MainWindow::on_actionSpace_to_TAB_Leading_triggered()
{ m_editorUiController->sendEditorCommand("C_CMD_SPACE_TO_TAB_LEADING"); }

void MainWindow::on_actionGo_to_Line_triggered()
{ m_editorUiController->goToLine(); }

void MainWindow::on_actionInstall_Extension_triggered()
{
    // See https://github.com/notepadqq/notepadqq/issues/654
    BackupServicePauser bsp;
    bsp.pause();

    QString file = QFileDialog::getOpenFileName(this, tr("Extension"), QString(), "Notepadqq extensions (*.nqqext)");
    if (!file.isNull()) {
        Extensions::InstallExtension* installExt = new Extensions::InstallExtension(file, this);
        installExt->exec();
        installExt->deleteLater();
    }
}

void MainWindow::showExtensionsMenu(bool show)
{ m_windowUiController->setExtensionsMenuVisible(show); }

QString MainWindow::getDefaultToolBarString() const
{
    QStringList list;

    list << ui->actionNew->objectName();
    list << ui->actionOpen->objectName();
    list << ui->actionSave->objectName();
    list << ui->actionSave_All->objectName();
    list << ui->actionClose->objectName();
    list << ui->actionClose_All->objectName();
    list << "Separator";
    list << ui->actionCut->objectName();
    list << ui->actionCopy->objectName();
    list << ui->actionPaste->objectName();
    list << "Separator";
    list << ui->actionUndo->objectName();
    list << ui->actionRedo->objectName();
    list << "Separator";
    list << ui->actionZoom_In->objectName();
    list << ui->actionZoom_Out->objectName();
    list << "Separator";
    list << ui->actionWord_wrap->objectName();
    list << ui->actionShow_All_Characters->objectName();

    return list.join('|');
}

QToolBar* MainWindow::getToolBar() const
{ return m_mainToolBar; }

void MainWindow::on_actionFull_Screen_toggled(bool on)
{ m_windowUiController->setFullScreen(on); }

void MainWindow::on_actionToggle_Smart_Indent_toggled(bool on)
{ m_editorUiController->setSmartIndent(on); }

void MainWindow::on_actionLoad_Session_triggered()
{ m_documentController->loadSession(); }

void MainWindow::on_actionSave_Session_triggered()
{ m_documentController->saveSession(); }

void MainWindow::on_actionShow_Menubar_toggled(bool arg1)
{ m_windowUiController->setMenuBarVisible(arg1); }

void MainWindow::on_actionShow_Toolbar_toggled(bool arg1)
{ m_windowUiController->setToolBarVisible(arg1); }

void MainWindow::on_actionToggle_To_Former_Tab_triggered()
{
    EditorTabWidget* curTabWidget = m_topEditorContainer->currentTabWidget();
    curTabWidget->setCurrentIndex(curTabWidget->formerTabIndex());
}
