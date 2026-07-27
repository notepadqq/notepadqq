#include "include/mainwindow.h"

#include "include/EditorNS/bannerindentationdetected.h"
#include "include/EditorNS/editor.h"
#include "include/Extensions/Stubs/windowstub.h"
#include "include/Extensions/extensionsloader.h"
#include "include/Extensions/installextension.h"
#include "include/Sessions/backupservice.h"
#include "include/clickablelabel.h"
#include "include/documentcontroller.h"
#include "include/editortabwidget.h"
#include "include/frmabout.h"
#include "include/frmencodingchooser.h"
#include "include/frmindentationmode.h"
#include "include/frmlinenumberchooser.h"
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

    connect(
        m_topEditorContainer, &TopEditorContainer::currentEditorChanged, this, &MainWindow::on_currentEditorChanged);

    connect(m_topEditorContainer, &TopEditorContainer::editorAdded, this, &MainWindow::on_editorAdded);

    connect(m_topEditorContainer, &TopEditorContainer::editorMouseWheel, this, &MainWindow::on_editorMouseWheel);

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
    // Restore symbol visibility
    bool showAll = m_settings.General.getShowAllSymbols();
    ui->actionWord_wrap->setChecked(m_settings.General.getWordWrap());
    ui->actionShow_All_Characters->setChecked(showAll);
    emit on_actionShow_All_Characters_toggled(showAll);

    // Restore math rendering
    ui->actionMath_Rendering->setChecked(m_settings.General.getMathRendering());

    // Restore full screen
    ui->actionFull_Screen->setChecked(isFullScreen());

    // Restore smart indent
    ui->actionToggle_Smart_Indent->setChecked(m_settings.General.getSmartIndentation());
    on_actionToggle_Smart_Indent_toggled(m_settings.General.getSmartIndentation());

    // Restore zoom
    const qreal zoom = m_settings.General.getZoom();
    for (int i = 0; i < m_topEditorContainer->count(); i++) {
        m_topEditorContainer->tabWidget(i)->setZoomFactor(zoom);
    }

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
{
    std::map<QChar, QMenu*> menuInitials;
    for (const auto& l : LanguageService::getInstance().languages()) {
        QString id = l.id;
        QChar letter = l.name.isEmpty() ? '?' : l.name.at(0).toUpper();
        QMenu* letterMenu;
        if (menuInitials.count(letter) != 0) {
            letterMenu = menuInitials[letter];
        } else {
            letterMenu = new QMenu(letter, this);
            menuInitials.emplace(std::make_pair(letter, letterMenu));
            ui->menu_Language->insertMenu(0, letterMenu);
        }

        QAction* action = new QAction(l.name, this);
        connect(action, &QAction::triggered, this, [id, this](bool = false) { currentEditor()->setLanguage(id); });
        letterMenu->insertAction(0, action);
    }
}

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
{
    m_overwrite = !m_overwrite;

    m_topEditorContainer->forEachEditor(
        [&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget* /*tabWidget*/, Editor* editor) {
            editor->setOverwrite(m_overwrite);
            return true;
        });

    if (m_overwrite) {
        m_sbOvertypeBtn->setText(tr("OVR"));
    } else {
        m_sbOvertypeBtn->setText(tr("INS"));
    }
}

void MainWindow::on_actionNew_triggered()
{
    EditorTabWidget* tabW = m_topEditorContainer->currentTabWidget();

    m_docEngine->addNewDocument(m_docEngine->getNewDocumentName(), true, tabW);
}

void MainWindow::setCurrentEditorLanguage(QString language)
{ currentEditor()->setLanguage(language); }

void MainWindow::on_customTabContextMenuRequested(QPoint point, EditorTabWidget* /*tabWidget*/, int /*tabIndex*/)
{ m_tabContextMenu->exec(point); }

bool MainWindow::updateSymbols(bool on)
{
    // Save the currently toggled symbols when deactivating Show_All_Characters using
    // one of the other available symbol actions.
    if (!on && ui->actionShow_All_Characters->isChecked()) {
        m_settings.General.setTabsVisible(ui->actionShow_Tabs->isChecked());
        m_settings.General.setSpacesVisisble(ui->actionShow_Spaces->isChecked());
        m_settings.General.setShowEOL(ui->actionShow_End_of_Line->isChecked());
        ui->actionShow_All_Characters->blockSignals(true);
        ui->actionShow_All_Characters->setChecked(false);
        ui->actionShow_All_Characters->blockSignals(false);
        m_settings.General.setShowAllSymbols(false);
        return true;

    } else if (on && !ui->actionShow_All_Characters->isChecked()) {
        bool showEOL = ui->actionShow_End_of_Line->isChecked();
        bool showTabs = ui->actionShow_Tabs->isChecked();
        bool showSpaces = ui->actionShow_Spaces->isChecked();
        if (showEOL && showTabs && showSpaces) {
            ui->actionShow_All_Characters->setChecked(true);
        }
    }

    return false;
}

void MainWindow::on_actionShow_Tabs_triggered(bool on)
{
    m_topEditorContainer->forEachEditorConcurrent([&](const int /*tabWidgetId*/,
                                                      const int /*editorId*/,
                                                      EditorTabWidget* /*tabWidget*/,
                                                      Editor* editor,
                                                      std::function<void()> done) {
        editor->setTabsVisible(on);
        done();
    });
    if (!updateSymbols(on)) {
        m_settings.General.setTabsVisible(on);
    }
}

void MainWindow::on_actionShow_Spaces_triggered(bool on)
{
    m_topEditorContainer->forEachEditorConcurrent([&](const int /*tabWidgetId*/,
                                                      const int /*editorId*/,
                                                      EditorTabWidget* /*tabWidget*/,
                                                      Editor* editor,
                                                      std::function<void()> done) {
        editor->setWhitespaceVisible(on);
        done();
    });
    if (!updateSymbols(on)) {
        m_settings.General.setSpacesVisisble(on);
    }
}

void MainWindow::on_actionShow_End_of_Line_triggered(bool on)
{
    m_topEditorContainer->forEachEditorConcurrent([&](const int /*tabWidgetId*/,
                                                      const int /*editorId*/,
                                                      EditorTabWidget* /*tabWidget*/,
                                                      Editor* editor,
                                                      std::function<void()> done) {
        editor->setEOLVisible(on);
        done();
    });
    if (!updateSymbols(on)) {
        m_settings.General.setShowEOL(on);
    }
}

void MainWindow::on_actionShow_All_Characters_toggled(bool on)
{
    if (on) {
        ui->actionShow_End_of_Line->setChecked(true);
        ui->actionShow_Tabs->setChecked(true);
        ui->actionShow_Spaces->setChecked(true);

    } else {
        bool showEOL = m_settings.General.getShowEOL();
        bool showTabs = m_settings.General.getTabsVisible();
        bool showSpaces = m_settings.General.getSpacesVisisble();

        if (showEOL && showTabs && showSpaces) {
            showEOL = !showEOL;
            showTabs = !showTabs;
            showSpaces = !showSpaces;
        }

        ui->actionShow_End_of_Line->setChecked(showEOL);
        ui->actionShow_Tabs->setChecked(showTabs);
        ui->actionShow_Spaces->setChecked(showSpaces);
    }

    m_topEditorContainer->forEachEditorConcurrent([&](const int /*tabWidgetId*/,
                                                      const int /*editorId*/,
                                                      EditorTabWidget* /*tabWidget*/,
                                                      Editor* editor,
                                                      std::function<void()> done) {
        editor->setEOLVisible(ui->actionShow_End_of_Line->isChecked());
        editor->setTabsVisible(ui->actionShow_Tabs->isChecked());
        editor->setWhitespaceVisible(on);
        done();
    });

    m_settings.General.setShowAllSymbols(on);
}

void MainWindow::on_actionMath_Rendering_toggled(bool on)
{
    m_topEditorContainer->forEachEditorConcurrent([&](const int /*tabWidgetId*/,
                                                      const int /*editorId*/,
                                                      EditorTabWidget* /*tabWidget*/,
                                                      Editor* editor,
                                                      std::function<void()> done) {
        editor->setMathEnabled(on);
        done();
    });

    m_settings.General.setMathRendering(on);
}

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
{ return m_topEditorContainer->currentTabWidget()->currentEditor(); }

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
{
    currentEditor()->selectedTexts().then([](QStringList sel) { QApplication::clipboard()->setText(sel.join("\n")); });
}

void MainWindow::on_actionPaste_triggered()
{
    // Normalize foreign text format
    QString text = QApplication::clipboard()->text().replace(QRegularExpression("\n|\r\n|\r"), "\n");

    currentEditor()->setSelectionsText(text.split("\n"));
}

void MainWindow::on_actionCut_triggered()
{
    ui->actionCopy->trigger();
    currentEditor()->setSelectionsText(QStringList(""));
}

void MainWindow::on_actionBegin_End_Select_triggered()
{
    if (!beginSelectPositionSet) {
        beginSelectPosition = currentEditor()->cursorPosition();
        beginSelectPositionSet = true;
    } else {
        QPair<int, int> endSelectPosition = currentEditor()->cursorPosition();
        currentEditor()->setSelection(
            beginSelectPosition.first, beginSelectPosition.second, endSelectPosition.first, endSelectPosition.second);
        beginSelectPositionSet = false;
    }
}

void MainWindow::on_currentEditorChanged(EditorTabWidget* tabWidget, int tab)
{
    if (tab != -1) {
        auto editor = tabWidget->editor(tab);
        refreshEditorUiInfo(editor);
        editor->requestDocumentInfo();
        editor->setFocus();
    }
}

void MainWindow::on_editorAdded(EditorTabWidget* tabWidget, int tab)
{
    auto editor = tabWidget->editor(tab);

    // If the tab is not newly opened but only transferred (e.g. with "Move to other View") it may
    // have a banner attached to it. We need to disconnect previous signals to prevent
    // on_bannerRemoved() to be called twice (once for the current connection and once for the connection
    // created a few lines below).
    disconnect(editor, &Editor::bannerRemoved, 0, 0);

    connect(editor, &Editor::cursorActivity, this, &MainWindow::on_cursorActivity);
    connect(editor, &Editor::documentInfoRequested, this, &MainWindow::refreshEditorUiCursorInfo);
    connect(editor, &Editor::currentLanguageChanged, this, [=, this](QString id, QString name) {
        on_currentLanguageChanged(editor, id, name);
    });
    connect(editor, &Editor::bannerRemoved, this, &MainWindow::on_bannerRemoved);
    connect(editor, &Editor::cleanChanged, this, [=, this]() {
        if (currentEditor() == editor)
            refreshEditorUiInfo(editor);
    });
    connect(editor, &Editor::urlsDropped, this, &MainWindow::on_editorUrlsDropped);

    // Initialize editor with UI settings
    editor->setLineWrap(ui->actionWord_wrap->isChecked());
    editor->setTabsVisible(ui->actionShow_Tabs->isChecked());
    editor->setEOLVisible(ui->actionShow_End_of_Line->isChecked());
    editor->setWhitespaceVisible(ui->actionShow_Spaces->isChecked());
    editor->setOverwrite(m_overwrite);
    editor->setFont(m_settings.Appearance.getOverrideFontFamily(),
        m_settings.Appearance.getOverrideFontSize(),
        m_settings.Appearance.getOverrideLineHeight());
    editor->setLineNumbersVisible(m_settings.Appearance.getShowLineNumbers());
    editor->setSmartIndent(m_settings.General.getSmartIndentation());
    editor->setMathEnabled(ui->actionMath_Rendering->isChecked());
}

void MainWindow::on_cursorActivity(QMap<QString, QVariant> data)
{
    Editor* editor = dynamic_cast<Editor*>(sender());
    if (!editor)
        return;

    if (currentEditor() == editor) {
        refreshEditorUiCursorInfo(data);
    }
}

void MainWindow::refreshEditorUiCursorInfo(QMap<QString, QVariant> data)
{
    auto curData = data["cursor"].toList();
    auto selData = data["selections"].toList();
    auto conData = data["content"].toList();
    QString msg = tr("Ln %1, Col %2").arg(curData[0].toInt() + 1).arg(curData[1].toInt() + 1);
    msg += tr("    Sel %1 (%2)").arg(selData[1].toInt()).arg(selData[0].toInt());
    msg += tr("    %1 chars, %2 lines").arg(conData[1].toInt()).arg(conData[0].toInt());
    m_sbDocumentInfoLabel->setText(msg);
}

void MainWindow::on_currentLanguageChanged(Editor* sender, QString /*id*/, QString /*name*/)
{
    if (currentEditor() == sender) {
        refreshEditorUiInfo(sender);
    }
}

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
{
    // Update current language in statusbar
    QString name = editor->getLanguage()->name;
    m_sbFileFormatBtn->setText(name);

    // Update MainWindow title
    QString newTitle;
    if (editor->filePath().isEmpty()) {
        EditorTabWidget* tabWidget = m_topEditorContainer->tabWidgetFromEditor(editor);
        if (tabWidget != 0) {
            int tab = tabWidget->indexOf(editor);
            if (tab != -1) {
                newTitle = QString("%1 - %2").arg(tabWidget->tabText(tab)).arg(QApplication::applicationName());
            }
        }

    } else {
        QUrl url = editor->filePath();

        QString path =
            url.toDisplayString(QUrl::RemovePassword | QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::RemoveAuthority |
                                QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::PreferLocalFile |
                                QUrl::RemoveFilename | QUrl::NormalizePathSegments | QUrl::StripTrailingSlash);

        newTitle = QString("%1%2 (%3) - %4")
                       .arg(Notepadqq::fileNameFromUrl(editor->filePath()))
                       .arg(editor->isClean() ? "" : "*")
                       .arg(path)
                       .arg(QApplication::applicationName());
    }

    if (newTitle != windowTitle()) {
        setWindowTitle(newTitle.isNull() ? QApplication::applicationName() : newTitle);
    }

    // Enable / disable menus
    QPointer<Editor> guardedEditor = editor;
    editor->isCleanP().then([this, guardedEditor](bool isClean) {
        if (!guardedEditor || currentEditor() != guardedEditor)
            return;
        QUrl fileName = guardedEditor->filePath();
        ui->actionRename->setEnabled(!fileName.isEmpty());
        ui->actionMove_to_New_Window->setEnabled(isClean);
        ui->actionOpen_in_New_Window->setEnabled(isClean);
    });

    bool allowReloading = !editor->filePath().isEmpty();
    ui->actionReload_File_Interpreted_As->setEnabled(allowReloading);
    ui->actionReload_from_Disk->setEnabled(allowReloading);

    // EOL
    QString eol = editor->endOfLineSequence();
    if (eol == "\r\n") {
        ui->actionWindows_Format->setChecked(true);
        m_sbEOLFormatBtn->setText(tr("Windows"));
    } else if (eol == "\n") {
        ui->actionUNIX_Format->setChecked(true);
        m_sbEOLFormatBtn->setText(tr("UNIX / OS X"));
    } else if (eol == "\r") {
        ui->actionMac_Format->setChecked(true);
        m_sbEOLFormatBtn->setText(tr("Old Mac"));
    }

    // Encoding
    QString encoding;
    if (editor->codec()->mibEnum() == MIB_UTF_8 && !editor->bom()) {
        // Is UTF-8 without BOM
        encoding = tr("%1 w/o BOM").arg(QString::fromUtf8(editor->codec()->name()));
    } else {
        encoding = QString::fromUtf8(editor->codec()->name());
    }
    m_sbTextFormatBtn->setText(encoding);

    // Indentation
    if (editor->isUsingCustomIndentationMode()) {
        ui->actionIndentation_Custom->setChecked(true);
    } else {
        ui->actionIndentation_Default_Settings->setChecked(true);
    }
}

void MainWindow::on_actionDelete_triggered()
{ currentEditor()->setSelectionsText(QStringList("")); }

void MainWindow::on_actionSelect_All_triggered()
{ currentEditor()->sendMessage("C_CMD_SELECT_ALL"); }

void MainWindow::on_actionSet_RTL_triggered()
{ currentEditor()->sendMessage("C_CMD_SET_RTL"); }

void MainWindow::on_actionSet_LTR_triggered()
{ currentEditor()->sendMessage("C_CMD_SET_LTR"); }

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
{ currentEditor()->sendMessage("C_CMD_UNDO"); }

void MainWindow::on_actionRedo_triggered()
{ currentEditor()->sendMessage("C_CMD_REDO"); }

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
{ currentEditor()->setLanguage("plaintext"); }

void MainWindow::on_actionRestore_Default_Zoom_triggered()
{
    const qreal newZoom = m_settings.General.resetZoom();
    m_topEditorContainer->currentTabWidget()->setZoomFactor(newZoom);
}

void MainWindow::on_actionZoom_In_triggered()
{
    qreal curZoom = currentEditor()->zoomFactor();
    qreal newZoom = curZoom + 0.25;
    m_topEditorContainer->currentTabWidget()->setZoomFactor(newZoom);
    m_settings.General.setZoom(newZoom);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    qreal curZoom = currentEditor()->zoomFactor();
    qreal newZoom = curZoom - 0.25;
    m_topEditorContainer->currentTabWidget()->setZoomFactor(newZoom);
    m_settings.General.setZoom(newZoom);
}

void MainWindow::on_editorMouseWheel(EditorTabWidget* tabWidget, int tab, QWheelEvent* ev)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        qreal curZoom = tabWidget->editor(tab)->zoomFactor();
        qreal diff = ev->angleDelta().y() / 120;
        diff /= 10;

        // Increment/Decrement zoom factor by 0.1 at each step.
        qreal newZoom = curZoom + diff;
        tabWidget->setZoomFactor(newZoom);
        m_settings.General.setZoom(newZoom);
    }
}

void MainWindow::transformSelectedText(std::function<QString(const QString&)> func)
{
    auto editor = currentEditor();
    editor->selectedTexts().then([=, this](QStringList sel) {
        for (int i = 0; i < sel.length(); i++) {
            sel.replace(i, func(sel.at(i)));
        }

        editor->setSelectionsText(sel, Editor::SelectMode::Selected);
    });
}

void MainWindow::on_actionUPPERCASE_triggered()
{
    transformSelectedText([](const QString& str) { return str.toUpper(); });
}

void MainWindow::on_actionLowercase_triggered()
{
    transformSelectedText([](const QString& str) { return str.toLower(); });
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
{ delete banner; }

void MainWindow::checkIndentationMode(Editor* editor)
{
    QPointer<Editor> guardedEditor = editor;
    editor->detectDocumentIndentation().then([this, guardedEditor](const std::pair<IndentationMode, bool> result) {
        if (!guardedEditor)
            return;
        IndentationMode detected = result.first;
        bool found = result.second;

        if (found) {
            guardedEditor->indentationModeP().then([this, guardedEditor, detected](IndentationMode curr) {
                if (!guardedEditor)
                    return;
                bool differentTabSpaces = detected.useTabs != curr.useTabs;
                bool differentSpaceSize =
                    detected.useTabs == false && curr.useTabs == false && detected.size != curr.size;

                if (differentTabSpaces || differentSpaceSize) {
                    // Show msg
                    BannerIndentationDetected* banner =
                        new BannerIndentationDetected(differentSpaceSize, detected, curr, this);
                    banner->setObjectName("indentationdetected");

                    guardedEditor->insertBanner(banner);

                    connect(banner,
                        &BannerIndentationDetected::useApplicationSettings,
                        this,
                        [this, guardedEditor, banner] {
                            if (!guardedEditor)
                                return;
                            guardedEditor->removeBanner(banner);
                            guardedEditor->setFocus();
                        });

                    connect(banner,
                        &BannerIndentationDetected::useDocumentSettings,
                        this,
                        [this, guardedEditor, banner, detected] {
                            if (!guardedEditor)
                                return;
                            guardedEditor->removeBanner(banner);
                            if (detected.useTabs) {
                                guardedEditor->setCustomIndentationMode(true);
                            } else {
                                guardedEditor->setCustomIndentationMode(detected.useTabs, detected.size);
                            }
                            ui->actionIndentation_Custom->setChecked(true);
                            guardedEditor->setFocus();
                        });
                }
            });
        }
    });
}

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
{
    m_topEditorContainer->forEachEditor(
        [&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget* /*tabWidget*/, Editor* editor) {
            editor->setLineWrap(on);
            return true;
        });
    m_settings.General.setWordWrap(on);
}

void MainWindow::on_actionEmpty_Recent_Files_List_triggered()
{ m_documentController->clearRecentFiles(); }

void MainWindow::on_actionOpen_All_Recent_Files_triggered()
{ m_documentController->openAllRecentFiles(); }

void MainWindow::on_actionUNIX_Format_triggered()
{
    auto editor = currentEditor();
    editor->setEndOfLineSequence("\n");
    editor->markDirty();
}

void MainWindow::on_actionWindows_Format_triggered()
{
    auto editor = currentEditor();
    editor->setEndOfLineSequence("\r\n");
    editor->markDirty();
}

void MainWindow::on_actionMac_Format_triggered()
{
    auto editor = currentEditor();
    editor->setEndOfLineSequence("\r");
    editor->markDirty();
}

void MainWindow::convertEditorEncoding(Editor* editor, QTextCodec* codec, bool bom)
{
    editor->setCodec(codec);
    editor->setBom(bom);
    editor->markDirty();

    if (editor == currentEditor())
        refreshEditorUiInfo(editor);
}

void MainWindow::on_actionUTF_8_triggered()
{ convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), true); }

void MainWindow::on_actionUTF_8_without_BOM_triggered()
{ convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), false); }

void MainWindow::on_actionUTF_16BE_triggered()
{ convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-16BE"), true); }

void MainWindow::on_actionUTF_16LE_triggered()
{ convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-16LE"), true); }

void MainWindow::on_actionInterpret_as_UTF_8_triggered()
{
    m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), true);
    refreshEditorUiInfo(currentEditor());
}

void MainWindow::on_actionInterpret_as_UTF_8_without_BOM_triggered()
{
    m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), false);
    refreshEditorUiInfo(currentEditor());
}

void MainWindow::on_actionInterpret_as_UTF_16BE_UCS_2_Big_Endian_triggered()
{
    m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-16BE"), true);
    refreshEditorUiInfo(currentEditor());
}

void MainWindow::on_actionInterpret_as_UTF_16LE_UCS_2_Little_Endian_triggered()
{
    m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-16LE"), true);
    refreshEditorUiInfo(currentEditor());
}

void MainWindow::on_actionConvert_to_triggered()
{
    auto editor = currentEditor();
    frmEncodingChooser* dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Convert to:"));

    if (dialog->exec() == QDialog::Accepted) {
        convertEditorEncoding(editor, dialog->selectedCodec(), false);
    }

    dialog->deleteLater();
}

void MainWindow::on_actionReload_File_Interpreted_As_triggered()
{
    auto editor = currentEditor();

    if (editor->filePath().isEmpty())
        return;

    frmEncodingChooser* dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Reload as:"));

    if (dialog->exec() == QDialog::Accepted) {
        EditorTabWidget* tabWidget = m_topEditorContainer->currentTabWidget();

        m_docEngine->getDocumentLoader()
            .setUrl(editor->filePath())
            .setTabWidget(tabWidget)
            .setTextCodec(dialog->selectedCodec())
            .execute();
    }

    dialog->deleteLater();
}

void MainWindow::on_actionIndentation_Default_Settings_triggered()
{ currentEditor()->clearCustomIndentationMode(); }

void MainWindow::on_actionIndentation_Custom_triggered()
{
    auto editor = currentEditor();

    frmIndentationMode* dialog = new frmIndentationMode(this);
    dialog->populateWidgets(editor->indentationMode());

    if (dialog->exec() == QDialog::Accepted) {
        IndentationMode indent = dialog->indentationMode();
        editor->setCustomIndentationMode(indent.useTabs, indent.size);
    }

    // Make sure the UI is consistent even if the user canceled the dialog.
    if (editor->isUsingCustomIndentationMode()) {
        ui->actionIndentation_Custom->setChecked(true);
    } else {
        ui->actionIndentation_Default_Settings->setChecked(true);
    }

    dialog->deleteLater();
}

void MainWindow::on_actionInterpret_As_triggered()
{
    auto editor = currentEditor();
    frmEncodingChooser* dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Interpret as:"));

    if (dialog->exec() == QDialog::Accepted) {
        m_docEngine->reinterpretEncoding(editor, dialog->selectedCodec(), false);
    }

    dialog->deleteLater();
}

void MainWindow::generateRunMenu()
{ m_windowUiController->generateRunMenu(); }

/**
 * @brief Configure any user interface after loading session
 */
void MainWindow::configurePostSessionUserInterface()
{
    // Restore zoom after load session
    const qreal zoom = m_settings.General.getZoom();
    for (int i = 0; i < m_topEditorContainer->count(); i++) {
        m_topEditorContainer->tabWidget(i)->setZoomFactor(zoom);
    }
}

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
{
    QPointer<Editor> editor = currentEditor();
    return editor->selectedTexts().then([editor](QStringList selection) {
        if (!editor)
            return QtPromise::QPromise<QStringList>::resolve({});
        if (selection.isEmpty() || selection.first().isEmpty()) {
            return editor->getCurrentWord().then([](QString word) { return QStringList(word); });
        } else {
            return QtPromise::QPromise<QStringList>::resolve(selection);
        }
    });
}

QtPromise::QPromise<QString> MainWindow::currentWordOrSelection()
{
    return currentWordOrSelections().then([=, this](QStringList terms) {
        if (terms.isEmpty()) {
            return QString();
        } else {
            return terms.first();
        }
    });
}

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
{ currentEditor()->sendMessage("C_CMD_DELETE_LINE"); }

void MainWindow::on_actionDuplicate_Line_triggered()
{ currentEditor()->sendMessage("C_CMD_DUPLICATE_LINE"); }

void MainWindow::on_actionMove_Line_Up_triggered()
{ currentEditor()->sendMessage("C_CMD_MOVE_LINE_UP"); }

void MainWindow::on_actionMove_Line_Down_triggered()
{ currentEditor()->sendMessage("C_CMD_MOVE_LINE_DOWN"); }

void MainWindow::on_actionTranspose_Line_triggered()
{ currentEditor()->sendMessage("C_CMD_TRANSPOSE_LINE"); }

void MainWindow::on_actionTrim_Trailing_Space_triggered()
{ currentEditor()->sendMessage("C_CMD_TRIM_TRAILING_SPACE"); }

void MainWindow::on_actionTrim_Leading_Space_triggered()
{ currentEditor()->sendMessage("C_CMD_TRIM_LEADING_SPACE"); }

void MainWindow::on_actionTrim_Leading_and_Trailing_Space_triggered()
{ currentEditor()->sendMessage("C_CMD_TRIM_LEADING_TRAILING_SPACE"); }

void MainWindow::on_actionEOL_to_Space_triggered()
{ currentEditor()->sendMessage("C_CMD_EOL_TO_SPACE"); }

void MainWindow::on_actionTAB_to_Space_triggered()
{ currentEditor()->sendMessage("C_CMD_TAB_TO_SPACE"); }

void MainWindow::on_actionSpace_to_TAB_All_triggered()
{ currentEditor()->sendMessage("C_CMD_SPACE_TO_TAB_ALL"); }

void MainWindow::on_actionSpace_to_TAB_Leading_triggered()
{ currentEditor()->sendMessage("C_CMD_SPACE_TO_TAB_LEADING"); }

void MainWindow::on_actionGo_to_Line_triggered()
{
    QPointer<Editor> editor = currentEditor();
    int currentLine = editor->cursorPosition().first;
    editor->lineCount().then([this, editor, currentLine](int lines) {
        if (!editor)
            return;
        frmLineNumberChooser* frm = new frmLineNumberChooser(1, lines, currentLine + 1, this);
        if (frm->exec() == QDialog::Accepted) {
            int line = frm->value();
            editor->setSelection(line - 1, 0, line - 1, 0);
        }
    });
}

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
{
    m_topEditorContainer->forEachEditor([&](const int, const int, EditorTabWidget*, Editor* editor) {
        editor->setSmartIndent(on);
        return true;
    });
    m_settings.General.setSmartIndentation(on);
}

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
