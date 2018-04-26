#include "include/mainwindow.h"
#include "ui_mainwindow.h"
#include "include/EditorNS/editor.h"
#include "include/editortabwidget.h"
#include "include/frmabout.h"
#include "include/frmpreferences.h"
#include "include/notepadqq.h"
#include "include/iconprovider.h"
#include "include/EditorNS/bannerfilechanged.h"
#include "include/EditorNS/bannerfileremoved.h"
#include "include/EditorNS/bannerindentationdetected.h"
#include "include/clickablelabel.h"
#include "include/frmencodingchooser.h"
#include "include/frmindentationmode.h"
#include "include/Extensions/extensionsloader.h"
#include "include/frmlinenumberchooser.h"
#include "include/Extensions/Stubs/windowstub.h"
#include "include/Extensions/installextension.h"
#include "include/Sessions/persistentcache.h"
#include "include/Sessions/sessions.h"
#include "include/nqqrun.h"
#include <QFileDialog>
#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QUrl>
#include <QMimeData>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolButton>
#include <QToolBar>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QDesktopServices>
#include <QJsonArray>
#include <QTimer>

QList<MainWindow*> MainWindow::m_instances = QList<MainWindow*>();

MainWindow::MainWindow(const QString &workingDirectory, const QStringList &arguments, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_topEditorContainer(new TopEditorContainer(this)),
    m_settings(NqqSettings::getInstance()),
    m_workingDirectory(workingDirectory),
    m_advSearchDock(new AdvancedSearchDock(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    MainWindow::m_instances.append(this);

    // Gets company name from QCoreApplication::setOrganizationName(). Same for app name.
    setCentralWidget(m_topEditorContainer);

    m_docEngine = new DocEngine(m_topEditorContainer);
    connect(m_docEngine, &DocEngine::fileOnDiskChanged, this, &MainWindow::on_fileOnDiskChanged);
    connect(m_docEngine, &DocEngine::documentSaved, this, &MainWindow::on_documentSaved);
    connect(m_docEngine, &DocEngine::documentReloaded, this, &MainWindow::on_documentReloaded);
    connect(m_docEngine, &DocEngine::documentLoaded, this, &MainWindow::on_documentLoaded);

    loadIcons();

    // Context menu initialization
    m_tabContextMenu = new QMenu(this);
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    QAction *separatorBottom = new QAction(this);
    separatorBottom->setSeparator(true);
    m_tabContextMenuActions.append(ui->actionClose);
    m_tabContextMenuActions.append(ui->actionClose_All_BUT_Current_Document);
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

    // Action group for EOL modes
    QActionGroup *eolActionGroup = new QActionGroup(this);
    eolActionGroup->addAction(ui->actionWindows_Format);
    eolActionGroup->addAction(ui->actionUNIX_Format);
    eolActionGroup->addAction(ui->actionMac_Format);

    // Action group for indentation modes
    QActionGroup *indentationActionGroup = new QActionGroup(this);
    indentationActionGroup->addAction(ui->actionIndentation_Default_Settings);
    indentationActionGroup->addAction(ui->actionIndentation_Custom);

    connect(m_topEditorContainer, &TopEditorContainer::customTabContextMenuRequested,
            this, &MainWindow::on_customTabContextMenuRequested);

    connect(m_topEditorContainer, &TopEditorContainer::tabCloseRequested,
            this, &MainWindow::on_tabCloseRequested);

    connect(m_topEditorContainer, &TopEditorContainer::currentEditorChanged,
            this, &MainWindow::on_currentEditorChanged);

    connect(m_topEditorContainer, &TopEditorContainer::editorAdded,
            this, &MainWindow::on_editorAdded);

    connect(m_topEditorContainer, &TopEditorContainer::editorMouseWheel,
            this, &MainWindow::on_editorMouseWheel);

    connect(m_topEditorContainer, &TopEditorContainer::tabBarDoubleClicked,
            this, &MainWindow::on_tabBarDoubleClicked);

    createStatusBar();

    updateRecentDocsInMenu();

    setAcceptDrops(true);

    generateRunMenu();

    m_mainToolBar = new QToolBar("Toolbar");
    m_mainToolBar->setIconSize(QSize(16,16));
    m_mainToolBar->setObjectName("toolbar");
    addToolBar(m_mainToolBar);
    loadToolBar();

    // Wire up tool- and menubar visibility.
    connect(m_mainToolBar, &QToolBar::visibilityChanged, ui->actionShow_Toolbar, &QAction::setChecked);
    ui->actionShow_Toolbar->setChecked(m_mainToolBar->isVisible());
    ui->menuBar->setVisible( m_settings.MainWindow.getMenuBarVisible() );
    ui->actionShow_Menubar->setChecked(m_settings.MainWindow.getMenuBarVisible());

    // Set popup for actionOpen in toolbar
    QToolButton *btnActionOpen = static_cast<QToolButton *>(m_mainToolBar->widgetForAction(ui->actionOpen));
    if(btnActionOpen) {
        btnActionOpen->setMenu(ui->menuRecent_Files);
        btnActionOpen->setPopupMode(QToolButton::MenuButtonPopup);
    }

    // Initialize UI from settings
    initUI();

    // Inserts at least an editor
    openCommandLineProvidedUrls(workingDirectory, arguments);
    // From now on, there is at least an Editor and at least
    // an EditorTabWidget within m_topEditorContainer.

    // Set zoom from settings
    const qreal zoom = m_settings.General.getZoom();
    for (int i = 0; i < m_topEditorContainer->count(); i++) {
        m_topEditorContainer->tabWidget(i)->setZoomFactor(zoom);
    }

    ui->actionFull_Screen->setChecked(isFullScreen());

    // Initialize the advanced search dock and hook its signals up
    addDockWidget(Qt::BottomDockWidgetArea, m_advSearchDock->getDockWidget() );
    m_advSearchDock->getDockWidget()->hide(); // Hidden by default, user preference is applied via restoreWindowSettings()
    connect(m_advSearchDock, &AdvancedSearchDock::itemInteracted, this, &MainWindow::searchDockItemInteracted);

    restoreWindowSettings();

    // If there was another window already opened, move this window
    // slightly to the bottom-right, so that they won't completely overlap.
    if (!isMaximized() && m_instances.count() > 1) {
        QPoint curPos = pos();
        move(curPos.x() + 50, curPos.y() + 50);
    }

    setupLanguagesMenu();

    showExtensionsMenu(Extensions::ExtensionsLoader::extensionRuntimePresent());

    //Registers all actions so that NqqSettings knows their default and current shortcuts.
    const QList<QAction*> allActions = getActions();

    m_settings.Shortcuts.initShortcuts(allActions);

    //At this point, all actions still have their default shortcuts so we set all actions'
    //shortcuts from settings.
    for (QAction* a : allActions){
        if (a->objectName().isEmpty())
            continue;

        QKeySequence shortcut = m_settings.Shortcuts.getShortcut(a->objectName());

        a->setShortcut(shortcut);
    }

    ui->actionToggle_Smart_Indent->setChecked(m_settings.General.getSmartIndentation());
    on_actionToggle_Smart_Indent_toggled(m_settings.General.getSmartIndentation());

    ui->actionMath_Rendering->setChecked(m_settings.General.getMathRendering());
    on_actionMath_Rendering_toggled(m_settings.General.getMathRendering());

    //Register our meta types for signal/slot calls here.
    emit Notepadqq::getInstance().newWindow(this);
}

MainWindow::MainWindow(const QStringList &arguments, QWidget *parent)
    : MainWindow(QDir::currentPath(), arguments, parent)
{ }

MainWindow::~MainWindow()
{
    MainWindow::m_instances.removeAll(this);

    delete ui;
    delete m_docEngine;
}

QList<MainWindow*> MainWindow::instances()
{
    return MainWindow::m_instances;
}

MainWindow *MainWindow::lastActiveInstance()
{
    if (m_instances.length() > 0) {
        return m_instances.last();
    } else {
        return nullptr;
    }
}

TopEditorContainer *MainWindow::topEditorContainer()
{
    return m_topEditorContainer;
}

void MainWindow::initUI()
{
    bool showAll = m_settings.General.getShowAllSymbols();
    ui->actionWord_wrap->setChecked(m_settings.General.getWordWrap());

    // Simply emitting a signal here initializes actionShow_Tab and
    // actionShow_End_of_Line, due to how action_Show_All_Characters works.
    ui->actionShow_All_Characters->setChecked(showAll);
    emit on_actionShow_All_Characters_toggled(showAll);
}

void MainWindow::restoreWindowSettings()
{
    restoreGeometry(m_settings.MainWindow.getGeometry());
    restoreState(m_settings.MainWindow.getWindowState());
}

void MainWindow::loadIcons()
{
    // To test fallback icons:
    // QIcon::setThemeSearchPaths(QStringList(""));

    // Assign (where possible) system theme icons to our actions.
    // If a system icon doesn't exist, fallback on the already assigned icon.

    // File menu
    ui->actionNew->setIcon(IconProvider::fromTheme("document-new"));
    ui->actionOpen->setIcon(IconProvider::fromTheme("document-open"));
    ui->actionReload_from_Disk->setIcon(IconProvider::fromTheme("view-refresh"));
    ui->actionSave->setIcon(IconProvider::fromTheme("document-save"));
    ui->actionSave_as->setIcon(IconProvider::fromTheme("document-save-as"));
    ui->actionSave_a_Copy_As->setIcon(IconProvider::fromTheme("document-save-as"));
    ui->actionSave_All->setIcon(IconProvider::fromTheme("document-save-all"));
    ui->actionClose->setIcon(IconProvider::fromTheme("document-close"));
    ui->actionClose_All->setIcon(IconProvider::fromTheme("document-close-all"));
    ui->menuRecent_Files->setIcon(IconProvider::fromTheme("document-open-recent"));
    ui->actionExit->setIcon(IconProvider::fromTheme("application-exit"));
    ui->actionPrint->setIcon(IconProvider::fromTheme("document-print"));
    ui->actionPrint_Now->setIcon(IconProvider::fromTheme("document-print")); // currently invisible

    // Edit menu
    ui->actionUndo->setIcon(IconProvider::fromTheme("edit-undo"));
    ui->actionRedo->setIcon(IconProvider::fromTheme("edit-redo"));
    ui->actionCut->setIcon(IconProvider::fromTheme("edit-cut"));
    ui->actionCopy->setIcon(IconProvider::fromTheme("edit-copy"));
    ui->actionPaste->setIcon(IconProvider::fromTheme("edit-paste"));
    ui->actionDelete->setIcon(IconProvider::fromTheme("edit-delete"));
    ui->actionSelect_All->setIcon(IconProvider::fromTheme("edit-select-all"));

    // Search menu
    ui->actionSearch->setIcon(IconProvider::fromTheme("edit-find"));
    ui->actionFind_Next->setIcon(IconProvider::fromTheme("go-next"));
    ui->actionFind_Previous->setIcon(IconProvider::fromTheme("go-previous"));
    ui->actionReplace->setIcon(IconProvider::fromTheme("edit-find-replace"));
    ui->actionGo_to_Line->setIcon(IconProvider::fromTheme("go-jump"));

    // View menu
    ui->actionShow_All_Characters->setIcon(IconProvider::fromTheme("show-special-chars"));
    ui->actionZoom_In->setIcon(IconProvider::fromTheme("zoom-in"));
    ui->actionZoom_Out->setIcon(IconProvider::fromTheme("zoom-out"));
    ui->actionRestore_Default_Zoom->setIcon(IconProvider::fromTheme("zoom-original"));
    ui->actionWord_wrap->setIcon(IconProvider::fromTheme("word-wrap"));
    ui->actionMath_Rendering->setIcon(IconProvider::fromTheme("math-rendering"));
    ui->actionFull_Screen->setIcon(IconProvider::fromTheme("view-fullscreen"));

    // Settings menu
    ui->actionPreferences->setIcon(IconProvider::fromTheme("preferences-other"));

    // Run menu
    ui->actionRun->setIcon(IconProvider::fromTheme("system-run"));

    // Window menu
    ui->actionOpen_a_New_Window->setIcon(IconProvider::fromTheme("window-new"));

    // '?' menu
    ui->actionAbout_Qt->setIcon(IconProvider::fromTheme("help-about"));
    ui->actionAbout_Notepadqq->setIcon(IconProvider::fromTheme("notepadqq"));

    // Macros in toolbar
    ui->action_Start_Recording->setIcon(IconProvider::fromTheme("media-record"));
    ui->action_Stop_Recording->setIcon(IconProvider::fromTheme("media-playback-stop"));
    ui->action_Playback->setIcon(IconProvider::fromTheme("media-playback-start"));
    ui->actionRun_a_Macro_Multiple_Times->setIcon(IconProvider::fromTheme("media-seek-forward"));
    ui->actionSave_Currently_Recorded_Macro->setIcon(IconProvider::fromTheme("document-save-as"));
}

void MainWindow::createStatusBar()
{
    QStatusBar *status = statusBar();
    status->setStyleSheet("QStatusBar::item { border: none; }; ");
    status->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setFrameStyle(QScrollArea::NoFrame);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    scrollArea->setStyleSheet("* { background: transparent; }");

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::NoFrame);
    frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);

    scrollArea->setWidget(frame);
    scrollArea->setWidgetResizable(true);
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar {height:0px;}");
    scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");


    QLabel *label;
    QMargins tmpMargins;

    label = new ClickableLabel("File Format", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(150);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_fileFormat = label;
    connect(dynamic_cast<ClickableLabel*>(label), &ClickableLabel::clicked, [this](){
        ui->menu_Language->exec( QCursor::pos() );
    });


    label = new QLabel("Ln 0, col 0", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(120);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_curPos = label;

    label = new QLabel("Sel 0, 0", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(120);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_selection = label;

    label = new QLabel("0 chars, 0 lines", this);
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_length_lines = label;

    label = new QLabel("EOL", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(118);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_EOLstyle = label;

    label = new ClickableLabel("Encoding", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(118);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_textFormat = label;
    connect(dynamic_cast<ClickableLabel*>(label), &ClickableLabel::clicked, [this](){
        ui->menu_Encoding->exec(QCursor::pos());
    });

    label = new QLabel(tr("INS"), this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(40);
    layout->addWidget(label);
    m_statusBar_overtypeNotify = label;

    if (Notepadqq::oldQt()) {
        ClickableLabel *cklabel = new ClickableLabel(QString("Qt ") + qVersion(), this);
        cklabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        cklabel->setStyleSheet("QLabel { background-color: #FF4136; color: white; }");
        cklabel->setCursor(Qt::PointingHandCursor);
        connect(cklabel, &ClickableLabel::clicked, this, [&]() {
            Notepadqq::showQtVersionWarning(false, this);
        });
        layout->addWidget(cklabel);
    }


    status->addWidget(scrollArea, 1);
    scrollArea->setFixedHeight(frame->height());
}

void MainWindow::loadToolBar()
{
    m_mainToolBar->clear();

    QString toolbarItems = m_settings.MainWindow.getToolBarItems();
    if(toolbarItems.isEmpty())
        toolbarItems = getDefaultToolBarString();

    auto actions = getActions();
    auto parts = toolbarItems.split('|', QString::SkipEmptyParts);

    for (const auto& part : parts) {
        if(part == "Separator") {
            m_mainToolBar->addSeparator();
            continue;
        }

        auto it = std::find_if(actions.begin(), actions.end(), [&part](QAction* ac) {
            return ac->objectName() == part;
        });

        if(it != actions.end())
            m_mainToolBar->addAction( *it );
    }
}

bool MainWindow::saveTabsToCache()
{
    // If saveSession() returns false, something went wrong. Most likely writing to the .xml file.
    while (!Sessions::saveSession(m_docEngine, m_topEditorContainer, PersistentCache::cacheSessionPath(), PersistentCache::cacheDirPath())) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setText(tr("Error while trying to save this session. Please ensure the following directory is accessible:\n\n") +
                       PersistentCache::cacheDirPath() + "\n\n" +
                       tr("By choosing \"ignore\" your session won't be saved."));
        msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
        msgBox.setDefaultButton(QMessageBox::Retry);
        msgBox.setIcon(QMessageBox::Critical);

        int result = msgBox.exec();
        if (result == QMessageBox::Abort) {
            return false;
        } else if (result == QMessageBox::Ignore) {
            // Do as if all went well
            return true;
        }
    }

    return true;
}

bool MainWindow::finalizeAllTabs()
{
    //Close all tabs normally
    int tabWidgetsCount = m_topEditorContainer->count();
    for (int i = 0; i < tabWidgetsCount; i++) {
        EditorTabWidget *tabWidget = m_topEditorContainer->tabWidget(i);
        int tabCount = tabWidget->count();

        for (int j = 0; j < tabCount; j++) {
            int closeResult = closeTab(tabWidget, j, false, false);
            if (closeResult == MainWindow::tabCloseResult_Canceled) {
                return false;
            }
        }
    }
    return true;
}

QList<const QMenu*> MainWindow::getMenus() const {
    return ui->menuBar->findChildren<const QMenu*>(QString(), Qt::FindDirectChildrenOnly);
}

DocEngine* MainWindow::getDocEngine() const
{
    return m_docEngine;
}

//Return a list of all available action items in the menu
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
    Editor *editor = currentEditor();
    if (editor == 0) {
        qDebug() << "currentEditor is null";
        throw;
    }

    QList<QMap<QString, QString>> langs = editor->languages();
    std::sort(langs.begin(), langs.end(), Editor::LanguageGreater());

    //ui->menu_Language->setStyleSheet("* { menu-scrollable: 1 }");
    QMap<QChar, QMenu*> menuInitials;
    for (int i = 0; i < langs.length(); i++) {
        const QMap<QString, QString> &map = langs.at(i);

        QString name = map.value("name", "?");
        if (name.length() == 0) name = "?";
        QChar letter = name.at(0).toUpper();

        QMenu *letterMenu;
        if (menuInitials.contains(letter)) {
            letterMenu = menuInitials.value(letter, 0);
        } else {
            letterMenu = new QMenu(letter, this);
            menuInitials.insert(letter, letterMenu);
            ui->menu_Language->insertMenu(0, letterMenu);
        }

        QString langId = map.value("id", "");
        QAction *action = new QAction(map.value("name"), this);
        connect(action, &QAction::triggered, this, [=](bool /*checked*/ = false) {
            currentEditor()->setLanguage(langId);
        });
        letterMenu->insertAction(0, action);
    }
}

void MainWindow::fixKeyboardShortcuts()
{
    QList<QMenu*> lst;
    lst = ui->menuBar->findChildren<QMenu*>();

    foreach (QMenu* m, lst)
    {
        addAction(m->menuAction());
        addActions(m->actions());
    }
}

QUrl MainWindow::stringToUrl(QString fileName, QString workingDirectory)
{
    if (workingDirectory.isEmpty())
        workingDirectory = m_workingDirectory;

    QUrl f = QUrl(fileName);
    if (f.isRelative()) { // No schema
        QFileInfo fi(fileName);
        if (fi.isRelative()) { // Relative local path
            QString absolute = QDir::cleanPath(workingDirectory + QDir::separator() + fileName);
            return QUrl::fromLocalFile(absolute);
        } else {
            return QUrl::fromLocalFile(fileName);
        }
    } else {
        return f;
    }
}

void MainWindow::openCommandLineProvidedUrls(const QString &workingDirectory, const QStringList &arguments)
{
    const int currentlyOpenTabs = m_topEditorContainer->currentTabWidget()->count();

    if (arguments.count() == 0) {

        if(currentlyOpenTabs==0){
            ui->actionNew->trigger();
        }

        return;
    }

    QSharedPointer<QCommandLineParser> parser = Notepadqq::getCommandLineArgumentsParser(arguments);

    QStringList rawUrls = parser->positionalArguments();

    if (rawUrls.count() == 0 && currentlyOpenTabs == 0)
    {
        // Open a new empty document
        ui->actionNew->trigger();
        return;
    }

    // Open selected files
    QList<QUrl> files;
    for(int i = 0; i < rawUrls.count(); i++)
    {
        files.append(stringToUrl(rawUrls.at(i), workingDirectory));
    }

    m_docEngine->getDocumentLoader()
                .setUrls(files)
                .setTabWidget(m_topEditorContainer->currentTabWidget())
                .execute();

    // Handle --line and --column commandline arguments
    if (!parser->isSet("line") && !parser->isSet("column"))
        return;

    if (rawUrls.size() > 1) {
        qWarning() << tr("The '--line' and '--column' arguments will be ignored since more than one file is opened.");
        return;
    }

    int l = 0;
    if (parser->isSet("line")) {
        bool okay;
        l = parser->value("line").toInt(&okay);

        if(!okay)
            qWarning() << tr("Invalid value for '--line' argument: %1").arg(parser->value("line"));
    }

    int c = 0;
    if (parser->isSet("column")) {
        bool okay;
        c = parser->value("column").toInt(&okay);

        if(!okay)
            qWarning() << tr("Invalid value for '--column' argument: %1").arg(parser->value("column"));
    }

    // This needs to sit inside a timer because CodeMirror apparently chokes on receiving a setCursorPosition()
    // right after construction of the Editor.
    Editor* ed = m_topEditorContainer->currentTabWidget()->currentEditor();
    QTimer* t = new QTimer();
    connect(t, &QTimer::timeout, [t, l, c, ed](){
        ed->setCursorPosition(l-1, c-1);
        t->deleteLater();
    });
    t->start(0);

}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    QMainWindow::dragEnterEvent(e);

    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QMainWindow::dropEvent(e);

    QList<QUrl> fileNames = e->mimeData()->urls();
    if (fileNames.empty())
        return;

    m_docEngine->getDocumentLoader()
            .setUrls(fileNames)
            .setTabWidget(m_topEditorContainer->currentTabWidget())
            .execute();
}

void MainWindow::on_editorUrlsDropped(QList<QUrl> urls)
{
    EditorTabWidget *tabWidget;
    Editor *editor = dynamic_cast<Editor *>(sender());

    if (editor) {
        tabWidget = m_topEditorContainer->tabWidgetFromEditor(editor);
    } else {
        tabWidget = m_topEditorContainer->currentTabWidget();
    }

    if (urls.empty())
        return;

    // If only one URL is dropped and it's a directory, we query the dir's entry list and open that one instead.
    if (urls.size() == 1) {
        const QString path = urls.front().toLocalFile();
        QFileInfo fileInfo(path);
        if (fileInfo.isDir()) {
            urls.clear();
            for (QFileInfo fi : QDir(path).entryInfoList(QDir::Files)) {
                urls.push_back(QUrl::fromLocalFile(fi.filePath()));
            }
        }
    }

    m_docEngine->getDocumentLoader()
            .setUrls(urls)
            .setTabWidget(tabWidget)
            .execute();
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Insert) {
        if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            on_actionPaste_triggered();
        } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
            on_actionCopy_triggered();
        } else {
            toggleOverwrite();
        }
    } else if (ev->key() >= Qt::Key_1 && ev->key() <= Qt::Key_9
               && QApplication::keyboardModifiers().testFlag(Qt::AltModifier)) {
        m_topEditorContainer->currentTabWidget()->setCurrentIndex(ev->key() - Qt::Key_1);
    } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)
               && ev->key() == Qt::Key_PageDown) {
        // switch to the next tab to the right or wrap around if last
        EditorTabWidget *curTabWidget = m_topEditorContainer->currentTabWidget();
        int nextTabIndex = (curTabWidget->currentIndex() + 1) % curTabWidget->count();
        curTabWidget->setCurrentIndex(nextTabIndex);
    } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)
               && ev->key() == Qt::Key_PageUp) {
        // switch to the previous tab or wrap around if first
        EditorTabWidget *curTabWidget = m_topEditorContainer->currentTabWidget();
        int prevTabIndex = (curTabWidget->currentIndex() + curTabWidget->count() - 1)
                           % curTabWidget->count();
        curTabWidget->setCurrentIndex(prevTabIndex);
    } else {
        QMainWindow::keyPressEvent(ev);
    }
}

void MainWindow::changeEvent(QEvent *e)
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

    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setOverwrite(m_overwrite);
        return true;
    });

    if (m_overwrite) {
        m_statusBar_overtypeNotify->setText(tr("OVR"));
    } else {
        m_statusBar_overtypeNotify->setText(tr("INS"));
    }
}

void MainWindow::on_actionNew_triggered()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();

    m_docEngine->addNewDocument(m_docEngine->getNewDocumentName(), true, tabW);
}

void MainWindow::setCurrentEditorLanguage(QString language)
{
    currentEditor()->setLanguage(language);
}

void MainWindow::on_customTabContextMenuRequested(QPoint point, EditorTabWidget * /*tabWidget*/, int /*tabIndex*/)
{
    m_tabContextMenu->exec(point);
}

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
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setTabsVisible(on);
        return true;
    });
    if (!updateSymbols(on)) {
        m_settings.General.setTabsVisible(on);
    }
}

void MainWindow::on_actionShow_Spaces_triggered(bool on)
{
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setWhitespaceVisible(on);
        return true;
    });
    if (!updateSymbols(on)) {
        m_settings.General.setSpacesVisisble(on);
    }
}

void MainWindow::on_actionShow_End_of_Line_triggered(bool on)
{
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setEOLVisible(on);
        return true;
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

    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setEOLVisible(ui->actionShow_End_of_Line->isChecked());
        editor->setTabsVisible(ui->actionShow_Tabs->isChecked());
        editor->setWhitespaceVisible(on);
        return true;
    });

    m_settings.General.setShowAllSymbols(on);
}

void MainWindow::on_actionMath_Rendering_toggled(bool on)
{
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setMathEnabled(on);
        return true;
    });

    m_settings.General.setMathRendering(on);
}

bool MainWindow::reloadWithWarning(EditorTabWidget *tabWidget, int tab, QTextCodec *codec, bool bom)
{
    // Don't do anything if there is no file to reload from.
    if (tabWidget->editor(tab)->filePath().isEmpty())
        return false;

    if (!tabWidget->editor(tab)->isClean()) {
        QMessageBox msgBox(this);
        QString name = tabWidget->tabText(tab).toHtmlEscaped();

        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

        msgBox.setText("<h3>" + tr("Your changes to «%1» will be discarded.").arg(name) + "</h3>");
        msgBox.setButtonText(QMessageBox::Ok, tr("Reload"));

        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setEscapeButton(QMessageBox::Cancel);

        msgBox.exec();

        if (msgBox.standardButton(msgBox.clickedButton()) == QMessageBox::Cancel)
            return false;
    }

    Editor *editor = tabWidget->editor(tab);

    m_docEngine->getDocumentLoader()
            .setUrl(editor->filePath())
            .setTabWidget(tabWidget)
            .setTextCodec(codec)
            .setBOM(bom)
            .setIsReload(true)
            .execute();

    return true;
}

void MainWindow::on_actionMove_to_Other_View_triggered()
{
    EditorTabWidget *curTabWidget = m_topEditorContainer->currentTabWidget();
    EditorTabWidget *destTabWidget = m_topEditorContainer->inactiveTabWidget(true);

    destTabWidget->transferEditorTab(true, curTabWidget, curTabWidget->currentIndex());

    removeTabWidgetIfEmpty(curTabWidget);
}

void MainWindow::removeTabWidgetIfEmpty(EditorTabWidget *tabWidget) {
    if(tabWidget->count() == 0) {
        delete tabWidget;
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QUrl defaultUrl = currentEditor()->filePath();
    if (defaultUrl.isEmpty())
        defaultUrl = QUrl::fromLocalFile(m_settings.General.getLastSelectedDir());

    QList<QUrl> fileNames = QFileDialog::getOpenFileUrls(
                                this,
                                tr("Open"),
                                defaultUrl,
                                tr("All files (*)"),
                                0, 0);

    if (fileNames.empty())
        return;

    m_docEngine->getDocumentLoader()
            .setUrls(fileNames)
            .setTabWidget(m_topEditorContainer->currentTabWidget())
            .execute();
}

void MainWindow::on_actionOpen_Folder_triggered()
{
    QUrl defaultUrl = currentEditor()->filePath();
    if (defaultUrl.isEmpty())
        defaultUrl = QUrl::fromLocalFile(m_settings.General.getLastSelectedDir());

    // Select directory
    QString folder = QFileDialog::getExistingDirectory(this, tr("Open Folder"), defaultUrl.toLocalFile(), 0);
    if (folder.isEmpty())
        return;

    // Get files within directory
    QDir dir(folder);
    QStringList files = dir.entryList(QStringList(), QDir::Files);

    // Convert file names to urls
    QList<QUrl> fileNames;
    for (QString file : files) {
        // Exclude hidden and backup files
        if (!file.startsWith(".") && !file.endsWith("~")) {
            fileNames.append(stringToUrl(file, folder));
        }
    }

    if (fileNames.isEmpty())
        return;

    m_docEngine->getDocumentLoader()
            .setUrls(fileNames)
            .setTabWidget(m_topEditorContainer->currentTabWidget())
            .execute();
}

int MainWindow::askIfWantToSave(EditorTabWidget *tabWidget, int tab, int reason)
{
    QMessageBox msgBox(this);
    QString name = tabWidget->tabText(tab).toHtmlEscaped();

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

    QPixmap img = IconProvider::fromTheme("document-save").pixmap(64,64).scaled(64,64,Qt::KeepAspectRatio, Qt::SmoothTransformation);
    msgBox.setIconPixmap(img);

    msgBox.exec();

    return msgBox.standardButton(msgBox.clickedButton());
}

int MainWindow::closeTab(EditorTabWidget *tabWidget, int tab, bool remove, bool force)
{
    int result = MainWindow::tabCloseResult_AlreadySaved;
    Editor *editor = tabWidget->editor(tab);

    // If the tab is the only existing one, is not associated with a file, and has no contents,
    // we'll not close it.
    if ( m_topEditorContainer->count()==1 && tabWidget->count()==1 &&
         editor->filePath().isEmpty() && editor->value().isEmpty()) {

        // If user tried to close last open (clean) tab, check if Nqq should just quit.
        if(m_settings.General.getExitOnLastTabClose())
            close();

        goto cleanup;
    }

    if (force || editor->isClean() || (editor->filePath().isEmpty() && editor->value().isEmpty())) {
        if (remove) m_docEngine->closeDocument(tabWidget, tab);
        goto cleanup;
    }

    // Ask the user to choose what to do with the modified contents.
    tabWidget->setCurrentIndex(tab);
    switch(askIfWantToSave(tabWidget, tab, askToSaveChangesReason_tabClosing)) {
    case QMessageBox::Save: {
        switch(save(tabWidget, tab)) {
        case DocEngine::saveFileResult_Canceled:
            result = MainWindow::tabCloseResult_Canceled;
            break;
        case DocEngine::saveFileResult_Saved:
            if (remove) m_docEngine->closeDocument(tabWidget, tab);
            result = MainWindow::tabCloseResult_Saved;
            break;
        }
        break;
    }
    case QMessageBox::Discard: {
        if (remove) m_docEngine->closeDocument(tabWidget, tab);
        result = MainWindow::tabCloseResult_NotSaved;
        break;
    }
    case QMessageBox::Cancel: {
        // Don't save and cancel closing
        result = MainWindow::tabCloseResult_Canceled;
    }
    }

    // Ensure the focus is still on this tabWidget
    if (tabWidget->count() > 0) {
        tabWidget->currentEditor()->setFocus();
    }

cleanup:
    if(tabWidget->count() > 0)
        return result;

    // If we just closed the last tab we'll either
    // * close the tabWidget and switch to a different one,
    // * close the editor if ExitOnLastTabClose() is enabled, or
    // * open a new tab.
    if(m_topEditorContainer->count() > 1) {
        delete tabWidget;
        m_topEditorContainer->tabWidget(0)->currentEditor()->setFocus();
    } else {
        if(m_settings.General.getExitOnLastTabClose())
            close();
        else
            ui->actionNew->trigger();
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

    if (editor->filePath().isEmpty())
    {
        // Call "save as"
        return saveAs(tabWidget, tab, false);

    } else {
        // If the file has changed outside the editor, ask
        // the user if he want to save it.
        bool fileOverwrite = false;
        if (editor->filePath().isLocalFile())
            fileOverwrite = QFile(editor->filePath().toLocalFile()).exists();

        if (editor->fileOnDiskChanged() && fileOverwrite) {
            QMessageBox msgBox(this);
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
                return DocEngine::saveFileResult_Canceled;
        }

        return m_docEngine->saveDocument(tabWidget, tab, editor->filePath());
    }
}

int MainWindow::saveAs(EditorTabWidget *tabWidget, int tab, bool copy)
{
    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(
                           this,
                           tr("Save as"),
                           getSaveDialogDefaultFileName(tabWidget, tab).toLocalFile(),
                           tr("Any file (*)"),
                           0, 0);

    if (filename != "") {
        m_settings.General.setLastSelectedDir(QFileInfo(filename).absolutePath());
        // Write
        return m_docEngine->saveDocument(tabWidget, tab, QUrl::fromLocalFile(filename), copy);
    } else {
        return DocEngine::saveFileResult_Canceled;
    }
}

QUrl MainWindow::getSaveDialogDefaultFileName(EditorTabWidget *tabWidget, int tab)
{
    QUrl docFileName = tabWidget->editor(tab)->filePath();

    if (docFileName.isEmpty()) {
        return QUrl::fromLocalFile(m_settings.General.getLastSelectedDir()
                                   + "/" + tabWidget->tabText(tab));
    } else {
        return docFileName;
    }
}

Editor *MainWindow::currentEditor()
{
    return m_topEditorContainer->currentTabWidget()->currentEditor();
}

QSharedPointer<Editor> MainWindow::currentEditorSharedPtr()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    return tabW->editorSharedPtr(tabW->currentIndex());
}

QAction * MainWindow::addExtensionMenuItem(QString extensionId, QString text)
{
    QMap<QString, QSharedPointer<Extensions::Extension>> extensions = Extensions::ExtensionsLoader::loadedExtensions();

    if (extensions.contains(extensionId)) {
        QSharedPointer<Extensions::Extension> extension = extensions.value(extensionId);

        // Create the menu for the extension if it doesn't exist yet.
        if (!m_extensionMenus.contains(extension)) {
            QMenu *menu = new QMenu(extension->name(), this);
            ui->menu_Extensions->addMenu(menu);
            m_extensionMenus.insert(extension, menu);
        }

        // Create the menu item
        QAction *action = new QAction(text, this);
        m_extensionMenus[extension]->addAction(action);

        return action;
    } else {
        // Invalid extension id
        return NULL;
    }
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

void MainWindow::on_actionCopy_triggered()
{
    QStringList sel = currentEditor()->selectedTexts();
    QApplication::clipboard()->setText(sel.join("\n"));
}

void MainWindow::on_actionPaste_triggered()
{
    // Normalize foreign text format
    QString text = QApplication::clipboard()->text()
                   .replace(QRegularExpression("\n|\r\n|\r"), "\n");

    currentEditor()->setSelectionsText(text.split("\n"));
}

void MainWindow::on_actionCut_triggered()
{
    ui->actionCopy->trigger();
    currentEditor()->setSelectionsText(QStringList(""));
}

void MainWindow::on_currentEditorChanged(EditorTabWidget *tabWidget, int tab)
{
    if (tab != -1) {
        Editor *editor = tabWidget->editor(tab);
        refreshEditorUiInfo(editor);
        refreshEditorUiCursorInfo(editor);
    }
}

void MainWindow::on_editorAdded(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);

    // If the tab is not newly opened but only transferred (e.g. with "Move to other View") it may
    // have a banner attached to it. We need to disconnect previous signals to prevent
    // on_bannerRemoved() to be called twice (once for the current connection and once for the connection
    // created a few lines below).
    disconnect(editor, &Editor::bannerRemoved, 0, 0);

    connect(editor, &Editor::cursorActivity, this, &MainWindow::on_cursorActivity);
    connect(editor, &Editor::currentLanguageChanged, this, &MainWindow::on_currentLanguageChanged);
    connect(editor, &Editor::bannerRemoved, this, &MainWindow::on_bannerRemoved);
    connect(editor, &Editor::cleanChanged, this, [=]() {
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
    editor->setSmartIndent(m_settings.General.getSmartIndentation());
    editor->setMathEnabled(ui->actionMath_Rendering->isChecked());
}

void MainWindow::on_cursorActivity()
{
    Editor *editor = dynamic_cast<Editor *>(sender());
    if (!editor)
        return;

    if (currentEditor() == editor) {
        refreshEditorUiCursorInfo(editor);
    }
}

void MainWindow::on_currentLanguageChanged(QString /*id*/, QString /*name*/)
{
    Editor *editor = dynamic_cast<Editor *>(sender());
    if (!editor)
        return;

    if (currentEditor() == editor) {
        refreshEditorUiInfo(editor);
    }
}

void MainWindow::refreshEditorUiCursorInfo(Editor *editor)
{
    if (editor != 0) {
        // Update status bar
        editor->asyncSendMessageWithResult("C_FUN_GET_TEXT_LENGTH", [=](QVariant len){
            int lines = editor->lineCount();

            m_statusBar_length_lines->setText(tr("%1 chars, %2 lines").arg(len.toInt()).arg(lines));

            QPair<int, int> cursor = editor->cursorPosition();
            int selectedChars = 0;
            int selectedPieces = 0;
            QStringList selections = editor->selectedTexts();
            for (QString sel : selections) {
                selectedChars += sel.length();
                selectedPieces += sel.split("\n").count();
            }

            m_statusBar_curPos->setText(tr("Ln %1, col %2")
                                        .arg(cursor.first + 1)
                                        .arg(cursor.second + 1));

            m_statusBar_selection->setText(tr("Sel %1 (%2)").arg(selectedChars).arg(selectedPieces));
        });
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
        // Make sure the editor is still open by searching for it first.
        Editor* found = doc.editor;
        EditorTabWidget* parentWidget = m_topEditorContainer->tabWidgetFromEditor(found);
        if (!parentWidget) return;

        parentWidget->setCurrentWidget(found);
        if (result) {
            found->setSelection(result->lineNumber-1, result->positionInLine, //selection start
                                result->lineNumber-1, result->positionInLine + result->matchLength); //selection end
        }
        found->setFocus();

    } else if (doc.docType == DocResult::TypeFile) {
        // Check the file's existence before trying to open it through the DocEngine. that is needed because
        // DocEngine will even open nonexistent documents and just show them as empty.
        if (!QFile(doc.fileName).exists()) return;

        QUrl url = stringToUrl(doc.fileName);

        m_docEngine->getDocumentLoader()
                .setUrl(url)
                .setTabWidget(m_topEditorContainer->currentTabWidget())
                .execute();

        QPair<int, int> pos = m_docEngine->findOpenEditorByUrl(url);

        if (pos.first == -1 || pos.second == -1)
            return;

        Editor *editor = m_topEditorContainer->tabWidget(pos.first)->editor(pos.second);

        if (result) {
            editor->setSelection(result->lineNumber-1, result->positionInLine, //selection start
                                result->lineNumber-1, result->positionInLine + result->matchLength); //selection end
        }
        editor->setFocus();
    }
}

void MainWindow::refreshEditorUiInfo(Editor *editor)
{
    // Update current language in statusbar
    QVariantMap data = editor->asyncSendMessageWithResult("C_FUN_GET_CURRENT_LANGUAGE").get().toMap();
    QString name = data.value("lang").toMap().value("name").toString();
    m_statusBar_fileFormat->setText(name);


    // Update MainWindow title
    QString newTitle;
    if (editor->filePath().isEmpty()) {

        EditorTabWidget *tabWidget = m_topEditorContainer->tabWidgetFromEditor(editor);
        if (tabWidget != 0) {
            int tab = tabWidget->indexOf(editor);
            if (tab != -1) {
                newTitle = QString("%1 - %2")
                           .arg(tabWidget->tabText(tab))
                           .arg(QApplication::applicationName());
            }
        }

    } else {
        QUrl url = editor->filePath();

        QString path = url.toDisplayString(QUrl::RemovePassword |
                                           QUrl::RemoveUserInfo |
                                           QUrl::RemovePort |
                                           QUrl::RemoveAuthority |
                                           QUrl::RemoveQuery |
                                           QUrl::RemoveFragment |
                                           QUrl::PreferLocalFile |
                                           QUrl::RemoveFilename |
                                           QUrl::NormalizePathSegments |
                                           QUrl::StripTrailingSlash
                                           );

        newTitle = QString("%1 (%2) - %3")
                   .arg(Notepadqq::fileNameFromUrl(editor->filePath()))
                   .arg(path)
                   .arg(QApplication::applicationName());

    }

    if (newTitle != windowTitle()) {
        setWindowTitle(newTitle.isNull() ? QApplication::applicationName() : newTitle);
    }


    // Enable / disable menus
    bool isClean = editor->isClean();
    QUrl fileName = editor->filePath();
    ui->actionRename->setEnabled(!fileName.isEmpty());
    ui->actionMove_to_New_Window->setEnabled(isClean);
    ui->actionOpen_in_New_Window->setEnabled(isClean);

    bool allowReloading = !editor->filePath().isEmpty();
    ui->actionReload_File_Interpreted_As->setEnabled(allowReloading);
    ui->actionReload_from_Disk->setEnabled(allowReloading);

    // EOL
    QString eol = editor->endOfLineSequence();
    if (eol == "\r\n") {
        ui->actionWindows_Format->setChecked(true);
        m_statusBar_EOLstyle->setText(tr("Windows"));
    } else if (eol == "\n") {
        ui->actionUNIX_Format->setChecked(true);
        m_statusBar_EOLstyle->setText(tr("UNIX / OS X"));
    } else if (eol == "\r") {
        ui->actionMac_Format->setChecked(true);
        m_statusBar_EOLstyle->setText(tr("Old Mac"));
    }

    // Encoding
    QString encoding;
    if (editor->codec()->mibEnum() == MIB_UTF_8 && !editor->bom()) {
        // Is UTF-8 without BOM
        encoding = tr("%1 w/o BOM").arg(QString::fromUtf8(editor->codec()->name()));
    } else {
        encoding = QString::fromUtf8(editor->codec()->name());
    }
    m_statusBar_textFormat->setText(encoding);

    // Indentation
    if (editor->isUsingCustomIndentationMode()) {
        ui->actionIndentation_Custom->setChecked(true);
    } else {
        ui->actionIndentation_Default_Settings->setChecked(true);
    }
}

void MainWindow::on_actionDelete_triggered()
{
    currentEditor()->setSelectionsText(QStringList(""));
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

void MainWindow::on_actionUndo_triggered()
{
    currentEditor()->sendMessage("C_CMD_UNDO");
}

void MainWindow::on_actionRedo_triggered()
{
    currentEditor()->sendMessage("C_CMD_REDO");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);

    // Only save tabs to cache if the closing window is the last one in the process.
    bool followThrough = m_instances.size()==1 && m_settings.General.getRememberTabsOnExit() ?
                             saveTabsToCache() :
                             finalizeAllTabs();

    if (!followThrough) {
        event->ignore();
        return;
    }

    m_settings.MainWindow.setGeometry(saveGeometry());
    m_settings.MainWindow.setWindowState(saveState());

    // Disconnect signals to avoid handling events while
    // the UI is being destroyed.
    disconnect(m_topEditorContainer, 0, this, 0);
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::instantiateFrmSearchReplace()
{
    if (!m_frmSearchReplace) {
        m_frmSearchReplace = new frmSearchReplace(
                                 m_topEditorContainer,
                                 this);

        connect(m_frmSearchReplace, &frmSearchReplace::toggleAdvancedSearch, [this](){
            QWidget* dockWidget = m_advSearchDock->getDockWidget();
            dockWidget->setVisible( !dockWidget->isVisible() );
        });
    }
}


void MainWindow::on_actionSearch_triggered()
{
    if (!m_frmSearchReplace) {
        instantiateFrmSearchReplace();
    }

    QStringList sel = currentEditor()->selectedTexts();
    if (sel.length() > 0 && sel[0].length() > 0) {
        m_frmSearchReplace->setSearchText(sel[0]);
    }

    m_frmSearchReplace->show(frmSearchReplace::TabSearch);
    m_frmSearchReplace->activateWindow();
}

void MainWindow::on_actionCurrent_Full_File_Path_to_Clipboard_triggered()
{
    Editor *editor = currentEditor();
    if (currentEditor()->filePath().isEmpty())
    {
        EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
        QApplication::clipboard()->setText(tabWidget->tabText(tabWidget->indexOf(editor)));
    } else {
        QApplication::clipboard()->setText(
                    editor->filePath().toDisplayString(QUrl::PreferLocalFile |
                                                       QUrl::RemovePassword));
    }
}

void MainWindow::on_actionCurrent_Filename_to_Clipboard_triggered()
{
    Editor *editor = currentEditor();
    if (currentEditor()->filePath().isEmpty())
    {
        EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
        QApplication::clipboard()->setText(tabWidget->tabText(tabWidget->indexOf(editor)));
    } else {
        QApplication::clipboard()->setText(Notepadqq::fileNameFromUrl(editor->filePath()));
    }
}

void MainWindow::on_actionCurrent_Directory_Path_to_Clipboard_triggered()
{
    Editor *editor = currentEditor();
    if(currentEditor()->filePath().isEmpty())
    {
        QApplication::clipboard()->setText("");
    } else {
        QApplication::clipboard()->setText(
                    editor->filePath().toDisplayString(QUrl::RemovePassword |
                                                       QUrl::RemoveUserInfo |
                                                       QUrl::RemovePort |
                                                       QUrl::RemoveAuthority |
                                                       QUrl::RemoveQuery |
                                                       QUrl::RemoveFragment |
                                                       QUrl::PreferLocalFile |
                                                       QUrl::RemoveFilename |
                                                       QUrl::NormalizePathSegments
                                                       ));
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    frmPreferences *_pref;
    _pref = new frmPreferences(m_topEditorContainer, this);
    _pref->exec();
    _pref->deleteLater();
}

void MainWindow::on_actionClose_triggered()
{
    closeTab(m_topEditorContainer->currentTabWidget(),
             m_topEditorContainer->currentTabWidget()->currentIndex());
}

void MainWindow::on_actionClose_All_triggered()
{
    bool canceled = false;

    // Save what needs to be saved, check if user wants to cancel the closing
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor */*editor*/) {
        int closeResult = closeTab(tabWidget, editorId, false, false);
        if (closeResult == MainWindow::tabCloseResult_Canceled) {
            canceled = true;
            return false; // Cancel all
        } else {
            return true;
        }
    });

    if (!canceled) {
        m_topEditorContainer->forEachEditor(true, [&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor */*editor*/) {
            closeTab(tabWidget, editorId, true, true);
            return true;
        });
    }
}

void MainWindow::on_fileOnDiskChanged(EditorTabWidget *tabWidget, int tab, bool removed)
{
    Editor *editor = tabWidget->editor(tab);

    if (removed) {
        BannerFileRemoved *banner = new BannerFileRemoved(this);
        banner->setObjectName("fileremoved");
        editor->insertBanner(banner);

        connect(banner, &BannerFileRemoved::ignore, this, [=]() {
            editor->removeBanner(banner);
            editor->setFocus();
        });

        connect(banner, &BannerFileRemoved::save, this, [=]() {
            save(tabWidget, tab);
        });

    } else {
        BannerFileChanged *banner = new BannerFileChanged(this);
        banner->setObjectName("filechanged");
        editor->insertBanner(banner);

        connect(banner, &BannerFileChanged::ignore, this, [=]() {
            editor->removeBanner(banner);
            editor->setFocus();
            // FIXME Set editor as clean
        });

        connect(banner, &BannerFileChanged::reload, this, [=]() {
            editor->removeBanner(banner);
            editor->setFocus();

            m_docEngine->getDocumentLoader()
                    .setUrl(editor->filePath())
                    .setTabWidget(tabWidget)
                    .setIsReload(true)
                    .execute();
        });
    }
}

void MainWindow::on_actionReplace_triggered()
{
    if (!m_frmSearchReplace) {
        instantiateFrmSearchReplace();
    }

    QStringList sel = currentEditor()->selectedTexts();
    if (sel.length() > 0 && sel[0].length() > 0) {
        m_frmSearchReplace->setSearchText(sel[0]);
    }

    m_frmSearchReplace->show(frmSearchReplace::TabReplace);
    m_frmSearchReplace->activateWindow();
}

void MainWindow::on_actionPlain_text_triggered()
{
    currentEditor()->setLanguage("plaintext");
}

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

void MainWindow::on_editorMouseWheel(EditorTabWidget *tabWidget, int tab, QWheelEvent *ev)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        qreal curZoom = tabWidget->editor(tab)->zoomFactor();
        qreal diff = ev->delta() / 120;
        diff /= 10;

        // Increment/Decrement zoom factor by 0.1 at each step.
        qreal newZoom = curZoom + diff;
        tabWidget->setZoomFactor(newZoom);
        m_settings.General.setZoom(newZoom);
    }
}

void MainWindow::transformSelectedText(std::function<QString (const QString &)> func)
{
    Editor *editor = currentEditor();
    QStringList sel = editor->selectedTexts();

    for (int i = 0; i < sel.length(); i++) {
        sel.replace(i, func(sel.at(i)));
    }

    editor->setSelectionsText(sel, Editor::selectMode_selected);
}

void MainWindow::on_actionUPPERCASE_triggered()
{
    transformSelectedText([](const QString &str) {
        return str.toUpper();
    });
}

void MainWindow::on_actionLowercase_triggered()
{
    transformSelectedText([](const QString &str) {
        return str.toLower();
    });
}

void MainWindow::on_actionClose_All_BUT_Current_Document_triggered()
{
    Editor *keepOpen = currentEditor();
    bool canceled = false;

    // Save what needs to be saved, check if user wants to cancel the closing
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor *editor) {
        if (keepOpen == editor)
            return true;

        int closeResult = closeTab(tabWidget, editorId, false, false);
        if (closeResult == MainWindow::tabCloseResult_Canceled) {
            canceled = true;
            return false; // Cancel all
        } else {
            return true;
        }
    });

    if (!canceled) {
        m_topEditorContainer->forEachEditor(true, [&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor *editor) {
            if (keepOpen == editor)
                return true;

            closeTab(tabWidget, editorId, true, true);
            return true;
        });
    }

}

void MainWindow::on_actionSave_All_triggered()
{
    // No tab must get closed (or added) while we're iterating!!
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor *editor) {
        if (editor->isClean()) {
            return true;
        } else {
            tabWidget->setCurrentIndex(editorId);
            int result = save(tabWidget, editorId);
            return (result != DocEngine::saveFileResult_Canceled);
        }
    });
}

void MainWindow::on_bannerRemoved(QWidget *banner)
{
    delete banner;
}

void MainWindow::on_documentSaved(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    editor->removeBanner("filechanged");
    editor->removeBanner("fileremoved");

    if (editor == currentEditor()) {
        ui->actionRename->setEnabled(true);
    }
}

void MainWindow::on_documentReloaded(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    editor->removeBanner("filechanged");
    editor->removeBanner("fileremoved");

    if (currentEditor() == editor) {
        refreshEditorUiInfo(editor);
        refreshEditorUiCursorInfo(editor);
    }
}

void MainWindow::on_documentLoaded(EditorTabWidget *tabWidget, int tab, bool wasAlreadyOpened, bool updateRecentDocs)
{
    Editor *editor = tabWidget->editor(tab);

    const int MAX_RECENT_ENTRIES = 10;

    if(updateRecentDocs){
        QUrl newUrl = editor->filePath();
        QList<QVariant> recentDocs = m_settings.General.getRecentDocuments();
        recentDocs.insert(0, QVariant(newUrl));

        // Remove duplicates
        for (int i = recentDocs.count() - 1; i >= 1; i--) {
            if (newUrl == recentDocs[i].toUrl())
                recentDocs.removeAt(i);
        }

        while (recentDocs.count() > MAX_RECENT_ENTRIES)
            recentDocs.removeLast();

        m_settings.General.setRecentDocuments(recentDocs);

        updateRecentDocsInMenu();
    }

    if (!wasAlreadyOpened) {
        if (m_settings.General.getWarnForDifferentIndentation()) {
            checkIndentationMode(editor);
        }
    }
}

void MainWindow::checkIndentationMode(Editor *editor)
{
    bool found = false;
    Editor::IndentationMode detected = editor->detectDocumentIndentation(&found);
    if (found) {
        Editor::IndentationMode curr = editor->indentationMode();
        bool differentTabSpaces = detected.useTabs != curr.useTabs;
        bool differentSpaceSize = detected.useTabs == false && curr.useTabs == false && detected.size != curr.size;

        if (differentTabSpaces || differentSpaceSize) {
            // Show msg
            BannerIndentationDetected *banner = new BannerIndentationDetected(
                                                    differentSpaceSize,
                                                    detected,
                                                    curr,
                                                    this);
            banner->setObjectName("indentationdetected");

            editor->insertBanner(banner);

            connect(banner, &BannerIndentationDetected::useApplicationSettings, this, [=]() {
                editor->removeBanner(banner);
                editor->setFocus();
            });

            connect(banner, &BannerIndentationDetected::useDocumentSettings, this, [=]() {
                editor->removeBanner(banner);
                if (detected.useTabs) {
                    editor->setCustomIndentationMode(true);
                } else {
                    editor->setCustomIndentationMode(detected.useTabs, detected.size);
                }
                ui->actionIndentation_Custom->setChecked(true);
                editor->setFocus();
            });
        }
    }
}

void MainWindow::updateRecentDocsInMenu()
{
    QList<QVariant> recentDocs = m_settings.General.getRecentDocuments();

    ui->menuRecent_Files->clear();

    QList<QAction *> actions;
    for (QVariant recentDoc : recentDocs) {
        QUrl url = recentDoc.toUrl();
        QAction *action = new QAction(Notepadqq::fileNameFromUrl(url), this);
        connect(action, &QAction::triggered, this, [this, url]() {
            openRecentFileEntry(url);
        });

        actions.append(action);
    }

    // If there are no recent files, show a placeholder
    bool anyRecentDoc = (actions.count() != 0);
    if (!anyRecentDoc) {
        QAction *action = new QAction(tr("No recent files"), this);
        action->setEnabled(false);
        actions.append(action);
    }

    ui->menuRecent_Files->addActions(actions);

    if (anyRecentDoc) {
        ui->menuRecent_Files->addSeparator();
        ui->menuRecent_Files->addActions({ui->actionOpen_All_Recent_Files,
                                          ui->actionEmpty_Recent_Files_List});
    }
}

void MainWindow::on_actionReload_from_Disk_triggered()
{
    EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
    Editor *editor = tabWidget->currentEditor();

    reloadWithWarning(tabWidget,
                      tabWidget->currentIndex(),
                      editor->codec(),
                      editor->bom());
}

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
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    QUrl oldFilename = tabW->currentEditor()->filePath();
    int result = saveAs(tabW, tabW->currentIndex(), false);

    if (result == DocEngine::saveFileResult_Saved && !oldFilename.isEmpty()) {

        if (QFileInfo(oldFilename.toLocalFile()) != QFileInfo(tabW->currentEditor()->filePath().toLocalFile())) {

            // Remove the old file
            QString filename = oldFilename.toLocalFile();
            if (QFile::exists(filename)) {
                if(!QFile::remove(filename)) {
                    QMessageBox::warning(this, QApplication::applicationName(),
                                         QString("Error: unable to remove file %1")
                                         .arg(filename));
                }
            }
        }

    }
}

void MainWindow::on_actionWord_wrap_toggled(bool on)
{
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setLineWrap(on);
        return true;
    });
    m_settings.General.setWordWrap(on);
}

void MainWindow::on_actionEmpty_Recent_Files_List_triggered()
{
    m_settings.General.resetRecentDocuments();
    updateRecentDocsInMenu();
}

void MainWindow::on_actionOpen_All_Recent_Files_triggered()
{
    QList<QVariant> allRecentUrlVariants = m_settings.General.getRecentDocuments();
    QList<QUrl> urlsToOpen;
    QList<QUrl> urlsOfMissingFiles;

    for (const auto& doc : allRecentUrlVariants) {
        const QUrl url = doc.toUrl();

        if(QFileInfo::exists(url.toLocalFile()))
            urlsToOpen.push_back(url);
        else
            urlsOfMissingFiles.push_back(url);
    }

    if (!urlsOfMissingFiles.empty()) {
        QString text = tr("The following files do not exist anymore. Do you want to open them anyway?\n");

        for(const auto& url : urlsOfMissingFiles)
            text += '\n' + url.toLocalFile();

        QMessageBox msg;
        msg.setIcon(QMessageBox::Question);
        msg.setText(text);
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        if (msg.exec() == QMessageBox::Yes) {
            // Clear the list and re-add all to preserve their order.
            urlsToOpen.clear();
            for (const auto& url : allRecentUrlVariants)
                urlsToOpen.push_back(url.toUrl());
        } else { // QMessageBox::No
            // Remove all missing files from the recent list.
            for (const auto& url : urlsOfMissingFiles)
                allRecentUrlVariants.removeOne(QVariant::fromValue(url));

            m_settings.General.setRecentDocuments(allRecentUrlVariants);
            updateRecentDocsInMenu();
        }
    }

    m_docEngine->getDocumentLoader()
            .setUrls(urlsToOpen)
            .setTabWidget(m_topEditorContainer->currentTabWidget())
            .execute();
}

void MainWindow::on_actionUNIX_Format_triggered()
{
    Editor *editor = currentEditor();
    editor->setEndOfLineSequence("\n");
    editor->markDirty();
}

void MainWindow::on_actionWindows_Format_triggered()
{
    Editor *editor = currentEditor();
    editor->setEndOfLineSequence("\r\n");
    editor->markDirty();
}

void MainWindow::on_actionMac_Format_triggered()
{
    Editor *editor = currentEditor();
    editor->setEndOfLineSequence("\r");
    editor->markDirty();
}

void MainWindow::convertEditorEncoding(Editor *editor, QTextCodec *codec, bool bom)
{
    editor->setCodec(codec);
    editor->setBom(bom);
    editor->markDirty();

    if (editor == currentEditor())
        refreshEditorUiInfo(editor);
}

void MainWindow::on_actionUTF_8_triggered()
{
    convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), true);
}

void MainWindow::on_actionUTF_8_without_BOM_triggered()
{
    convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), false);
}

void MainWindow::on_actionUTF_16BE_triggered()
{
    convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-16BE"), true);
}

void MainWindow::on_actionUTF_16LE_triggered()
{
    convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-16LE"), true);
}

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
    Editor *editor = currentEditor();
    frmEncodingChooser *dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Convert to:"));

    if (dialog->exec() == QDialog::Accepted) {
        convertEditorEncoding(editor, dialog->selectedCodec(), false);
    }

    dialog->deleteLater();
}

void MainWindow::on_actionReload_File_Interpreted_As_triggered()
{
    Editor *editor = currentEditor();
    frmEncodingChooser *dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Reload as:"));

    if (dialog->exec() == QDialog::Accepted) {
        EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
        reloadWithWarning(tabWidget, tabWidget->currentIndex(), dialog->selectedCodec(), false);
    }

    dialog->deleteLater();
}

void MainWindow::on_actionIndentation_Default_Settings_triggered()
{
    currentEditor()->clearCustomIndentationMode();
}

void MainWindow::on_actionIndentation_Custom_triggered()
{
    Editor *editor = currentEditor();

    frmIndentationMode *dialog = new frmIndentationMode(this);
    dialog->populateWidgets(editor->indentationMode());

    if (dialog->exec() == QDialog::Accepted) {
        Editor::IndentationMode indent = dialog->indentationMode();
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
    Editor *editor = currentEditor();
    frmEncodingChooser *dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Interpret as:"));

    if (dialog->exec() == QDialog::Accepted) {
        m_docEngine->reinterpretEncoding(editor, dialog->selectedCodec(), false);
    }

    dialog->deleteLater();
}

void MainWindow::generateRunMenu()
{
    QMap <QString, QString> runners = m_settings.Run.getCommands();
    QMapIterator<QString, QString> i(runners);
    ui->menu_Run->clear();

    QAction *a = ui->menu_Run->addAction(tr("Run..."));
    connect(a, &QAction::triggered, this, &MainWindow::runCommand);
    ui->menu_Run->addSeparator();

    while (i.hasNext()) {
        i.next();
        a = ui->menu_Run->addAction(i.key());
        a->setData(i.value());
        a->setObjectName("RunCmd"+a->text());
        connect(a, &QAction::triggered, this, &MainWindow::runCommand);
    }
    ui->menu_Run->addSeparator();
    a = ui->menu_Run->addAction(tr("Modify Run Commands"));
    connect(a, &QAction::triggered, this, &MainWindow::modifyRunCommands);
}

void MainWindow::modifyRunCommands()
{
    NqqRun::RunPreferences p;
    if(p.exec() == 1) {
        generateRunMenu();
    }
}

void MainWindow::runCommand()
{
    QAction *a = qobject_cast<QAction*>(sender());
    QString cmd;

    if (a->data().toString().size()) {
        cmd = a->data().toString();
    } else {
        NqqRun::RunDialog rd;
        int ok = rd.exec();

        if (rd.saved()) {
            generateRunMenu();
        }

        if (!ok) {
            return;
        }

        cmd = rd.getCommandInput();
    }

    Editor *editor = currentEditor();

    QUrl url = currentEditor()->filePath();
    QStringList selection = editor->selectedTexts();
    if (!url.isEmpty()) {
        cmd.replace("\%url\%", url.toString(QUrl::None));
        cmd.replace("\%path\%", url.path(QUrl::FullyEncoded));
        cmd.replace("\%filename\%", url.fileName(QUrl::FullyEncoded));
        cmd.replace("\%directory\%", QFileInfo(url.toLocalFile()).absolutePath());
    }
    if (!selection.first().isEmpty()) {
        cmd.replace("\%selection\%",selection.first());
    }
    QStringList args = NqqRun::RunDialog::parseCommandString(cmd);
    if (!args.isEmpty()) {
        cmd = args.takeFirst();
        if(!QProcess::startDetached(cmd, args)) {

        }
    }
}

void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer);
    if (dialog.exec() == QDialog::Accepted)
        currentEditor()->print(&printer);
}

void MainWindow::on_actionPrint_Now_triggered()
{
    QPrinter printer(QPrinter::HighResolution);
    currentEditor()->print(&printer);
}
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
QStringList MainWindow::currentWordOrSelections()
{
    Editor *editor = currentEditor();
    QStringList selection = editor->selectedTexts();

    if (selection.isEmpty() || selection.first().isEmpty()) {
        return QStringList(editor->getCurrentWord());
    } else {
        return selection;
    }
}

QString MainWindow::currentWordOrSelection()
{
    QStringList terms = currentWordOrSelections();
    if (terms.isEmpty()) {
        return QString();
    } else {
        return terms.first();
    }
}

void MainWindow::currentWordOnlineSearch(const QString &searchUrl)
{
    QString term = currentWordOrSelection();

    if (!term.isNull() && !term.isEmpty()) {
        QUrl phpHelp = QUrl(searchUrl.arg(QString(QUrl::toPercentEncoding(term))));
        QDesktopServices::openUrl(phpHelp);
    }
}

void MainWindow::openRecentFileEntry(QUrl url)
{
    const QString filePath = url.toLocalFile();

    if (!QFileInfo::exists(filePath)) {
        QMessageBox msg;
        msg.setIcon(QMessageBox::Question);
        msg.setText(tr("The file \"%1\" does not exist. Do you want to re-create it?").arg(filePath));
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        if (msg.exec() == QMessageBox::No) {
            // Remove this entry from the history if the user does not want to recreate the file.
            QList<QVariant> recentDocs = m_settings.General.getRecentDocuments();
            recentDocs.removeOne( QVariant::fromValue(url) );
            m_settings.General.setRecentDocuments(recentDocs);
            updateRecentDocsInMenu();
            return;
        }
    }

    m_docEngine->getDocumentLoader()
            .setUrl(url)
            .setTabWidget(m_topEditorContainer->currentTabWidget())
            .execute();
}

void MainWindow::on_actionOpen_a_New_Window_triggered()
{
    MainWindow *b = new MainWindow(QStringList(), 0);
    b->show();
}

void MainWindow::on_actionOpen_in_New_Window_triggered()
{
    QStringList args;
    args.append(QApplication::arguments().first());
    if (!currentEditor()->filePath().isEmpty()) {
        args.append(currentEditor()->filePath().toString(QUrl::None));
    }

    MainWindow *b = new MainWindow(args, 0);
    b->show();
}

void MainWindow::on_actionMove_to_New_Window_triggered()
{
    QStringList args;
    args.append(QApplication::arguments().first());
    if (!currentEditor()->filePath().isEmpty()) {
        args.append(currentEditor()->filePath().toString(QUrl::None));
    }

    EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
    int tab = tabWidget->currentIndex();
    if (closeTab(tabWidget, tab) != tabCloseResult_Canceled) {
        MainWindow *b = new MainWindow(args, 0);
        b->show();
    }
}

void MainWindow::on_actionOpen_file_triggered()
{
    QStringList terms = currentWordOrSelections();
    if (terms.isEmpty())
        return;

    QList<QUrl> urls;
    for (QString term : terms) {
        urls.append(QUrl::fromLocalFile(term));
    }

    m_docEngine->getDocumentLoader()
            .setUrls(urls)
            .setTabWidget(m_topEditorContainer->currentTabWidget())
            .execute();
}

void MainWindow::on_actionOpen_in_another_window_triggered()
{
    QStringList terms = currentWordOrSelections();
    if (!terms.isEmpty()) {
        terms.prepend(QApplication::arguments().first());

        MainWindow *b = new MainWindow(terms, 0);
        b->show();
    }
}

void MainWindow::on_tabBarDoubleClicked(EditorTabWidget *tabWidget, int tab)
{
    if (tab == -1) {
        m_docEngine->addNewDocument(m_docEngine->getNewDocumentName(), true, tabWidget);
    }
}

void MainWindow::on_actionFind_in_Files_triggered()
{
    QWidget* dockWidget = m_advSearchDock->getDockWidget();
    dockWidget->setVisible( !dockWidget->isVisible() );
}

void MainWindow::on_actionDelete_Line_triggered()
{
    currentEditor()->sendMessage("C_CMD_DELETE_LINE");
}

void MainWindow::on_actionDuplicate_Line_triggered()
{
    currentEditor()->sendMessage("C_CMD_DUPLICATE_LINE");
}

void MainWindow::on_actionMove_Line_Up_triggered()
{
    currentEditor()->sendMessage("C_CMD_MOVE_LINE_UP");
}

void MainWindow::on_actionMove_Line_Down_triggered()
{
    currentEditor()->sendMessage("C_CMD_MOVE_LINE_DOWN");
}

void MainWindow::on_actionTrim_Trailing_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_TRIM_TRAILING_SPACE");
}

void MainWindow::on_actionTrim_Leading_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_TRIM_LEADING_SPACE");
}

void MainWindow::on_actionTrim_Leading_and_Trailing_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_TRIM_LEADING_TRAILING_SPACE");
}

void MainWindow::on_actionEOL_to_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_EOL_TO_SPACE");
}

void MainWindow::on_actionTAB_to_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_TAB_TO_SPACE");
}

void MainWindow::on_actionSpace_to_TAB_All_triggered()
{
    currentEditor()->sendMessage("C_CMD_SPACE_TO_TAB_ALL");
}

void MainWindow::on_actionSpace_to_TAB_Leading_triggered()
{
    currentEditor()->sendMessage("C_CMD_SPACE_TO_TAB_LEADING");
}

void MainWindow::on_actionGo_to_Line_triggered()
{
    Editor *editor = currentEditor();
    int currentLine = editor->cursorPosition().first;
    int lines = editor->lineCount();
    frmLineNumberChooser *frm = new frmLineNumberChooser(1, lines, currentLine + 1, this);
    if (frm->exec() == QDialog::Accepted) {
        int line = frm->value();
        editor->setSelection(line - 1, 0, line - 1, 0);
    }
}

void MainWindow::on_actionInstall_Extension_triggered()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Extension"), QString(), "Notepadqq extensions (*.nqqext)");
    if (!file.isNull()) {
        Extensions::InstallExtension *installExt = new Extensions::InstallExtension(file, this);
        installExt->exec();
        installExt->deleteLater();
    }
}

void MainWindow::showExtensionsMenu(bool show)
{
    ui->menu_Extensions->menuAction()->setVisible(show);
}

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

QToolBar*MainWindow::getToolBar() const
{
    return m_mainToolBar;
}

void MainWindow::on_actionFull_Screen_toggled(bool on)
{
    static bool maximized = isMaximized();

    if (on) {
        maximized = isMaximized();
        showFullScreen();
    } else {
        if (maximized) {
            showMaximized();
        } else {
            showNormal();
        }
    }
}

void MainWindow::on_actionToggle_Smart_Indent_toggled(bool on)
{
    m_topEditorContainer->forEachEditor([&](const int, const int, EditorTabWidget *, Editor *editor) {
        editor->setSmartIndent(on);
        return true;
    });
    m_settings.General.setSmartIndentation(on);
}

void MainWindow::on_actionLoad_Session_triggered()
{
    QString recentFolder = QUrl::fromLocalFile(
                               m_settings.General.getLastSelectedSessionDir())
                               .toLocalFile();

    QString filePath = QFileDialog::getOpenFileName(
                           this,
                           tr("Open Session..."),
                           recentFolder,
                           tr("Session file (*.xml);;Any file (*)"),
                           0, 0);

    if (filePath.isEmpty())
        return;

    m_settings.General.setLastSelectedSessionDir(QFileInfo(filePath).dir().absolutePath());

    Sessions::loadSession(m_docEngine, m_topEditorContainer, filePath);
}

void MainWindow::on_actionSave_Session_triggered()
{
    QString recentFolder = QUrl::fromLocalFile(
                               m_settings.General.getLastSelectedSessionDir())
                               .toLocalFile();

    QFileDialog dialog(this,
                       tr("Save Session as..."),
                       recentFolder,
                       tr("Session file (*.xml);;Any file (*)"));

    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix("xml");
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if (!dialog.exec())
        return;

    QStringList fileNames = dialog.selectedFiles();

    if (fileNames.empty())
        return;

    QString filePath = fileNames[0];

    if (filePath.isEmpty())
        return;

    m_settings.General.setLastSelectedSessionDir(QFileInfo(filePath).dir().absolutePath());

    if (Sessions::saveSession(m_docEngine, m_topEditorContainer, filePath)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setText(tr("Error while trying to save this session. Please try a different file name."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
    }
}

void MainWindow::on_actionShow_Menubar_toggled(bool arg1)
{
    ui->menuBar->setVisible(arg1);
    m_settings.MainWindow.setMenuBarVisible(arg1);
}

void MainWindow::on_actionShow_Toolbar_toggled(bool arg1)
{
    m_mainToolBar->setVisible(arg1);
}
