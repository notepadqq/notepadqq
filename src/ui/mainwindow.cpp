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
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QDesktopServices>
#include <QJsonArray>

QList<MainWindow*> MainWindow::m_instances = QList<MainWindow*>();

MainWindow::MainWindow(const QString &workingDirectory, const QStringList &arguments, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(NqqSettings::getInstance()),
    m_fileSearchResultsWidget(new FileSearchResultsWidget()),
    m_workingDirectory(workingDirectory)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    MainWindow::m_instances.append(this);

    m_docEngine = new DocEngine(nullptr);
    connect(m_docEngine, &DocEngine::fileOnDiskChanged, this, &MainWindow::on_fileOnDiskChanged);
    connect(m_docEngine, &DocEngine::documentSaved, this, &MainWindow::on_documentSaved); //TODO: not emitted currently
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
    m_tabContextMenuActions.append(ui->actionCurrent_Full_File_path_to_Clipboard);
    m_tabContextMenuActions.append(ui->actionCurrent_Filename_to_Clipboard);
    m_tabContextMenuActions.append(ui->actionCurrent_Directory_Path_to_Clipboard);
    m_tabContextMenuActions.append(separatorBottom);
    m_tabContextMenuActions.append(ui->actionMove_to_Other_View);
    m_tabContextMenuActions.append(ui->actionClone_to_Other_View);
    m_tabContextMenuActions.append(ui->actionMove_to_New_Window);
    m_tabContextMenuActions.append(ui->actionOpen_in_New_Window);
    m_tabContextMenu->addActions(m_tabContextMenuActions);

    fixKeyboardShortcuts();
    // Set popup for action_Open in toolbar
    QToolButton *btnActionOpen = static_cast<QToolButton *>(ui->mainToolBar->widgetForAction(ui->action_Open));
    btnActionOpen->setMenu(ui->menuRecent_Files);
    btnActionOpen->setPopupMode(QToolButton::MenuButtonPopup);

    // Action group for EOL modes
    QActionGroup *eolActionGroup = new QActionGroup(this);
    eolActionGroup->addAction(ui->actionWindows_Format);
    eolActionGroup->addAction(ui->actionUNIX_Format);
    eolActionGroup->addAction(ui->actionMac_Format);

    // Action group for indentation modes
    QActionGroup *indentationActionGroup = new QActionGroup(this);
    indentationActionGroup->addAction(ui->actionIndentation_Default_settings);
    indentationActionGroup->addAction(ui->actionIndentation_Custom);

    createStatusBar();

    updateRecentDocsInMenu();

    setAcceptDrops(true);

    ui->dockFileSearchResults->setWidget(m_fileSearchResultsWidget);
    connect(m_fileSearchResultsWidget, &FileSearchResultsWidget::resultMatchClicked,
            this, &MainWindow::on_resultMatchClicked);


    // Initiate the new and wonderful tab widget!
    m_nqqSplitPane = new NqqSplitPane();
    m_nqqSplitPane->createNewTabWidget();
    setCentralWidget(m_nqqSplitPane->m_splitter);

    connect(m_nqqSplitPane, &NqqSplitPane::currentTabChanged, this, &MainWindow::on_currenTabChanged);
    connect(m_nqqSplitPane, &NqqSplitPane::tabCloseRequested, this, &MainWindow::on_tabCloseRequested);
    connect(m_nqqSplitPane, &NqqSplitPane::newTabAdded, this, &MainWindow::on_tabAdded);
    connect(m_nqqSplitPane, &NqqSplitPane::customContextMenuRequested, this, &MainWindow::on_customTabContextMenuRequested);
    connect(m_nqqSplitPane, &NqqSplitPane::urlsDropped, this, &MainWindow::on_editorUrlsDropped);

    connect(m_nqqSplitPane, &NqqSplitPane::currentTabCursorActivity, this, &MainWindow::on_cursorActivity);
    connect(m_nqqSplitPane, &NqqSplitPane::currentTabLanguageChanged, this, &MainWindow::on_currentLanguageChanged);
    connect(m_nqqSplitPane, &NqqSplitPane::currentTabCleanStatusChanged, this, [this](NqqTab* tab) {
        refreshTabUiInfo(tab);
    });

    // We wanna add a test document so we're good to go until session restore is available again.
    Editor* ed = m_docEngine->loadDocumentProper(QUrl::fromLocalFile("/home/s3rius/Downloads/Important Stuff"));
    getCurrentTabWidget()->createTab(ed);


    // Initialize UI from settings
    initUI();


    // We want to restore tabs only if...
    if (    m_instances.size()==1 && // this window is the first one to be opened,
            m_settings.General.getRememberTabsOnExit() // and the Remember-tabs option is enabled
    ) {
        /*Sessions::loadSession(m_docEngine, m_topEditorContainer, PersistentCache::cacheSessionPath());
        if (m_topEditorContainer->count() > 0 && m_topEditorContainer->currentTabWidget()->count() > 0) {
            refreshEditorUiInfo(m_topEditorContainer->currentTabWidget()->currentEditor());
        }*/


    }

    openCommandLineProvidedUrls(workingDirectory, arguments);

    // TODO: Since this isn't handled in NqqTabWidget yet, we'll manually create a tab if it's still empty
    if (getCurrentTabWidget()->isEmpty())
        getCurrentTabWidget()->createEmptyTab();

    // Set zoom from settings
    const qreal zoom = m_settings.General.getZoom();
    for(NqqTab* tab : getCurrentTabWidget()->getAllTabs())
        tab->setZoomFactor(zoom);

    restoreWindowSettings();

    ui->actionFull_Screen->setChecked(isFullScreen());

    ui->dockFileSearchResults->hide();

    // If there was another window already opened, move this window
    // slightly to the bottom-right, so that they won't completely overlap.
    if (!isMaximized() && m_instances.count() > 1) {
        QPoint curPos = pos();
        move(curPos.x() + 50, curPos.y() + 50);
    }

    setupLanguagesMenu();
    generateRunMenu();

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

    //Register our meta types for signal/slot calls here.
    qRegisterMetaType<FileSearchResult::SearchResult>("FileSearchResult::SearchResult");
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

NqqTabWidget*MainWindow::getCurrentTabWidget()
{
    return m_nqqSplitPane->getCurrentTabWidget();
}

NqqTab*MainWindow::getCurrentTab()
{
    return getCurrentTabWidget()->getCurrentTab();
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
    ui->action_New->setIcon(IconProvider::fromTheme("document-new"));
    ui->action_Open->setIcon(IconProvider::fromTheme("document-open"));
    ui->actionSave->setIcon(IconProvider::fromTheme("document-save"));
    ui->actionSave_as->setIcon(IconProvider::fromTheme("document-save-as"));
    ui->actionSave_All->setIcon(IconProvider::fromTheme("document-save-all"));
    ui->actionClose->setIcon(IconProvider::fromTheme("document-close"));
    ui->actionC_lose_All->setIcon(IconProvider::fromTheme("document-close-all"));
    ui->actionPrint_Now->setIcon(IconProvider::fromTheme("document-print"));
    ui->actionCu_t->setIcon(IconProvider::fromTheme("edit-cut"));
    ui->action_Copy->setIcon(IconProvider::fromTheme("edit-copy"));
    ui->action_Paste->setIcon(IconProvider::fromTheme("edit-paste"));
    ui->action_Undo->setIcon(IconProvider::fromTheme("edit-undo"));
    ui->action_Redo->setIcon(IconProvider::fromTheme("edit-redo"));
    ui->actionZoom_In->setIcon(IconProvider::fromTheme("zoom-in"));
    ui->actionZoom_Out->setIcon(IconProvider::fromTheme("zoom-out"));
    ui->actionRestore_Default_Zoom->setIcon(IconProvider::fromTheme("zoom-original"));
    ui->action_Start_Recording->setIcon(IconProvider::fromTheme("media-record"));
    ui->action_Stop_Recording->setIcon(IconProvider::fromTheme("media-playback-stop"));
    ui->action_Playback->setIcon(IconProvider::fromTheme("media-playback-start"));
    ui->actionRun_a_Macro_Multiple_Times->setIcon(IconProvider::fromTheme("media-seek-forward"));
    ui->actionSave_Currently_Recorded_Macro->setIcon(IconProvider::fromTheme("document-save-as"));
    ui->actionPreferences->setIcon(IconProvider::fromTheme("preferences-other"));
    ui->actionSearch->setIcon(IconProvider::fromTheme("edit-find"));
    ui->actionReplace->setIcon(IconProvider::fromTheme("edit-find-replace"));
    ui->actionShow_All_Characters->setIcon(IconProvider::fromTheme("show-special-chars"));
    ui->actionWord_wrap->setIcon(IconProvider::fromTheme("word-wrap"));
    ui->actionFind_Next->setIcon(IconProvider::fromTheme("go-next"));
    ui->actionFind_Previous->setIcon(IconProvider::fromTheme("go-previous"));
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

bool MainWindow::saveTabsToCache()
{
    // If saveSession() returns false, something went wrong. Most likely writing to the .xml file.
    /*while (!Sessions::saveSession(m_docEngine, m_topEditorContainer, PersistentCache::cacheSessionPath(), PersistentCache::cacheDirPath())) {
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
*/
    return true;
}

bool MainWindow::finalizeAllTabs()
{

    //Close all tabs normally
    /*int tabWidgetsCount = m_topEditorContainer->count();
    for (int i = 0; i < tabWidgetsCount; i++) {
        EditorTabWidget *tabWidget = m_topEditorContainer->tabWidget(i);
        int tabCount = tabWidget->count();

        for (int j = 0; j < tabCount; j++) {
            int closeResult = closeTab(tabWidget, j, false, false);
            if (closeResult == MainWindow::tabCloseResult_Canceled) {
                return false;
            }
        }
    }*/
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
    Editor *editor = getCurrentTabWidget()->getCurrentTab()->m_editor;

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
        connect(action, &QAction::triggered, this, [=](bool) {
            getCurrentTabWidget()->getCurrentTab()->m_editor->setLanguage(langId);
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
    QSharedPointer<QCommandLineParser> parser = Notepadqq::getCommandLineArgumentsParser(arguments);

    QStringList rawUrls = parser->positionalArguments();

    for (const QString& item : rawUrls) {
        const QUrl url = stringToUrl(item, workingDirectory);
        Editor* editor = m_docEngine->loadDocumentProper(url);

        getCurrentTabWidget()->createTab(editor);
    }
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
    qDebug() << "MainWindow::dropEvent";

    QMainWindow::dropEvent(e);

    const QList<QUrl>& fileUrls = e->mimeData()->urls();

    if (fileUrls.empty())
        return;

    if (fileUrls.count() > 6) {
        bool wantContinue = QMessageBox::question(this,
                                                  "Do you want to continue?",
                                                  QString("You are about to open %1 files. Do you want to continue?")
                                                  .arg(fileUrls.count()),
                                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

        if(!wantContinue) return;
    }

    for (const QUrl& url : fileUrls) {
        Editor* editor = m_docEngine->loadDocumentProper(url);
        getCurrentTabWidget()->createTab(editor, true);
    }
}

void MainWindow::on_editorUrlsDropped(QList<QUrl> fileUrls)
{
    qDebug() << "MainWindow::on_editorUrlsDropepd";

    /*EditorTabWidget *tabWidget; //TODO
    Editor *editor = dynamic_cast<Editor *>(sender());

    if (editor) {
        tabWidget = m_topEditorContainer->tabWidgetFromEditor(editor);
    } else {
        tabWidget = m_topEditorContainer->currentTabWidget();
    }*/

    if (fileUrls.count() > 6) {
        bool wantContinue = QMessageBox::question(this,
                                                  "Do you want to continue?",
                                                  QString("You are about to open %1 files. Do you want to continue?")
                                                  .arg(fileUrls.count()),
                                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

        if(!wantContinue) return;
    }

    for (const QUrl& url : fileUrls) {
        Editor* editor = m_docEngine->loadDocumentProper(url);
        if(editor)
            getCurrentTabWidget()->createTab(editor, true);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    qDebug() << "MainWindow::keyPressEvent";

    if (ev->key() == Qt::Key_Insert) {  // TODO: These key bindings cannot be changed
        if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            on_action_Paste_triggered();
        } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
            on_action_Copy_triggered();
        } else {
            toggleOverwrite();
        }
    } else if (ev->key() >= Qt::Key_1 && ev->key() <= Qt::Key_9
               && QApplication::keyboardModifiers().testFlag(Qt::AltModifier)) {
        getCurrentTabWidget()->makeCurrent(ev->key() - Qt::Key_1);
    } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)
               && ev->key() == Qt::Key_PageDown) {
        const int tabCount = getCurrentTabWidget()->getAllTabs().size();
        const int currentIndex = getCurrentTabWidget()->getCurrentIndex();
        const int nextIndex = (currentIndex + 1) % tabCount;
        getCurrentTabWidget()->makeCurrent(nextIndex);
    } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)
               && ev->key() == Qt::Key_PageUp) {
        const int tabCount = getCurrentTabWidget()->getAllTabs().size();
        const int currentIndex = getCurrentTabWidget()->getCurrentIndex();
        const int prevIndex = (currentIndex + tabCount - 1) % tabCount;
        getCurrentTabWidget()->makeCurrent(prevIndex);
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

    for (NqqTab* tab : getCurrentTabWidget()->getAllTabs())
        tab->m_editor->setOverwrite(m_overwrite);

    if (m_overwrite) {
        m_statusBar_overtypeNotify->setText(tr("OVR"));
    } else {
        m_statusBar_overtypeNotify->setText(tr("INS"));
    }
}

void MainWindow::on_action_New_triggered()
{
    /*EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();

    m_docEngine->addNewDocument(m_docEngine->getNewDocumentName(), true, tabW);*/
    getCurrentTabWidget()->createEmptyTab();
}

void MainWindow::setCurrentEditorLanguage(QString language)
{
    getCurrentTabWidget()->getCurrentTab()->m_editor->setLanguage(language);
}

void MainWindow::on_customTabContextMenuRequested(const QPoint& point)
{
    qDebug() << "MainWindow::on_customContextMenuRequested";

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
    for(NqqTab* tab : getCurrentTabWidget()->getAllTabs())
        tab->m_editor->setTabsVisible(on);

    if (!updateSymbols(on)) {
        m_settings.General.setTabsVisible(on);
    }
}

void MainWindow::on_actionShow_Spaces_triggered(bool on)
{
    for(NqqTab* tab : getCurrentTabWidget()->getAllTabs())
        tab->m_editor->setWhitespaceVisible(on);

    if (!updateSymbols(on)) {
        m_settings.General.setSpacesVisisble(on);
    }
}

void MainWindow::on_actionShow_End_of_Line_triggered(bool on)
{
    for(NqqTab* tab : getCurrentTabWidget()->getAllTabs())
        tab->m_editor->setEOLVisible(on);

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

    for(NqqTab* tab : getCurrentTabWidget()->getAllTabs()) {
        tab->m_editor->setTabsVisible(on);
        tab->m_editor->setWhitespaceVisible(on);
        tab->m_editor->setEOLVisible(on);
    }

    m_settings.General.setShowAllSymbols(on);
}

bool MainWindow::reloadWithWarning(EditorTabWidget *tabWidget, int tab, QTextCodec *codec, bool bom)
{
    // Don't do anything if there is no file to reload from.
    if (tabWidget->editor(tab)->fileName().isEmpty())
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

    return m_docEngine->reloadDocument(tabWidget, tab, codec, bom);
}

void MainWindow::on_actionMove_to_Other_View_triggered()
{
    NqqTabWidget* currTabWidget = m_nqqSplitPane->getCurrentTabWidget();
    NqqTabWidget* nextTabWidget = m_nqqSplitPane->getNextTabWidget();

    NqqTab* currTab = currTabWidget->getCurrentTab();

    if( !currTabWidget->detachTab(currTab) )
        return;


    if(currTabWidget==nextTabWidget)
        nextTabWidget = m_nqqSplitPane->createNewTabWidget(currTab);
    else
        nextTabWidget->attachTab(currTab);

    currTab->m_parentTabWidget->makeCurrent(currTab); // TODO: Shitty workaround
}

void MainWindow::removeTabWidgetIfEmpty(EditorTabWidget *tabWidget) {
    if(tabWidget->count() == 0) {
        delete tabWidget;
    }
}

void MainWindow::on_action_Open_triggered()
{
    QUrl defaultUrl = getCurrentTabWidget()->getCurrentTab()->getFileUrl();;
    if (defaultUrl.isEmpty())
        defaultUrl = QUrl::fromLocalFile(m_settings.General.getLastSelectedDir());

    QList<QUrl> fileUrls = QFileDialog::getOpenFileUrls(
                                this,
                                tr("Open"),
                                defaultUrl,
                                tr("All files (*)"),
                                0, 0);

    for (QUrl url : fileUrls) {
        Editor* ed = m_docEngine->loadDocumentProper(url);
        getCurrentTabWidget()->createTab(ed, true);
    }

    if (!fileUrls.empty()) {
        m_settings.General.setLastSelectedDir(QFileInfo(fileUrls.front().toLocalFile()).absolutePath());
    }
}

void MainWindow::on_actionOpen_Folder_triggered()
{
    QUrl defaultUrl = getCurrentTabWidget()->getCurrentTab()->getFileUrl();
    if (defaultUrl.isEmpty())
        defaultUrl = QUrl::fromLocalFile(m_settings.General.getLastSelectedDir());

    // Select directory
    QString folder = QFileDialog::getExistingDirectory(this, tr("Open Folder"), defaultUrl.toLocalFile(), 0);

    if (folder.isEmpty())
        return;

    // Get files within directory
    QDir dir(folder);
    QFileInfoList files = dir.entryInfoList(QStringList(), QDir::Files);

    std::remove_if(files.begin(), files.end(), [](const QFileInfo& file) {
        QString name = file.fileName();
        return name.startsWith(".") || name.endsWith("~");
    });

    if (files.isEmpty())
        return;

    if (files.count() > 6) {
        bool wantContinue = QMessageBox::question(this,
                                                  "Do you want to continue?",
                                                  QString("You are about to open %1 files. Do you want to continue?")
                                                  .arg(files.count()),
                                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

        if(!wantContinue) return;
    }

    for (const QFileInfo& fileInfo : files) {
        Editor* editor = m_docEngine->loadDocumentProper(QUrl::fromLocalFile(fileInfo.filePath()));
        getCurrentTabWidget()->createTab(editor, true);
    }

    m_settings.General.setLastSelectedDir(folder);
}

int MainWindow::save(NqqTab* tab)
{
    Editor* editor = tab->m_editor;

    if (editor->fileName().isEmpty())
    {
        // Call "save as"
        return saveAs(tab, false);

    } else {
        // If the file has changed outside the editor, ask
        // the user if he want to save it.
        bool fileOverwrite = false;
        if (editor->fileName().isLocalFile())
            fileOverwrite = QFile(editor->fileName().toLocalFile()).exists();

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

        return m_docEngine->saveDocumentProper(editor, editor->fileName());
    }
}

int MainWindow::saveAs(NqqTab* tab, bool copy)
{
    Editor* editor = tab->m_editor;

    QUrl docFileName = editor->fileName();

    if (docFileName.isEmpty()) {
        docFileName = QUrl::fromLocalFile(m_settings.General.getLastSelectedDir()
                                   + "/" + tab->getTabTitle());
    }

    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(
                           this,
                           tr("Save as"),
                           docFileName.toLocalFile(),
                           tr("Any file (*)"),
                           0, 0);

    if (filename == "")
        return DocEngine::saveFileResult_Canceled;

    m_settings.General.setLastSelectedDir(QFileInfo(filename).absolutePath());
    return m_docEngine->saveDocumentProper(editor, QUrl::fromLocalFile(filename), copy);
}

QUrl MainWindow::getSaveDialogDefaultFileName(EditorTabWidget *tabWidget, int tab)
{
    QUrl docFileName = tabWidget->editor(tab)->fileName();

    if (docFileName.isEmpty()) {
        return QUrl::fromLocalFile(m_settings.General.getLastSelectedDir()
                                   + "/" + tabWidget->tabText(tab));
    } else {
        return docFileName;
    }
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

int MainWindow::askIfWantToSave(NqqTab* tab, int reason) {

    const QString name = tab->getTabTitle().toHtmlEscaped();

    QMessageBox msgBox(this);
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

MainWindow::TabCloseResult MainWindow::processTabClose(NqqTab* tab) {
    Editor* ed = tab->m_editor;

    qDebug() << "processTabClose for " << tab->getTabTitle();

    if(ed->isClean()) {
        return TabWantToClose;
    }

    int ret = askIfWantToSave(tab, askToSaveChangesReason_tabClosing);
    if(ret == QMessageBox::Save) {
        int saveResult = DocEngine::saveFileResult_Canceled; //save(tabWidget, tab); //TODO
        if(saveResult == DocEngine::saveFileResult_Canceled)
        {
            return TabDontClose;
        } else if(saveResult == DocEngine::saveFileResult_Saved)
        {
            return TabWantToClose;
        }
    } else if(ret == QMessageBox::Discard) {
        return TabWantToClose; // Don't save and close
    } else if(ret == QMessageBox::Cancel) {
        return TabDontClose;
    }

    return TabDontClose;
}

void MainWindow::on_tabCloseRequested(NqqTab* tab)
{
    if (processTabClose(tab) == TabWantToClose)
        tab->forceCloseTab();
}

void MainWindow::on_actionSave_triggered()
{
    save(getCurrentTabWidget()->getCurrentTab());
}

void MainWindow::on_actionSave_as_triggered()
{
    saveAs(getCurrentTabWidget()->getCurrentTab());
}

void MainWindow::on_actionSave_a_Copy_As_triggered()
{
    saveAs(getCurrentTabWidget()->getCurrentTab(), true);
}

void MainWindow::on_action_Copy_triggered()
{
    QStringList sel = getCurrentTabWidget()->getCurrentTab()->m_editor->selectedTexts();
    QApplication::clipboard()->setText(sel.join("\n"));
}

void MainWindow::on_action_Paste_triggered()
{
    // Normalize foreign text format
    QString text = QApplication::clipboard()->text()
                   .replace(QRegularExpression("\n|\r\n|\r"), "\n");

    getCurrentTabWidget()->getCurrentTab()->m_editor->setSelectionsText(text.split("\n"));
}

void MainWindow::on_actionCu_t_triggered()
{
    ui->action_Copy->trigger();
    getCurrentTabWidget()->getCurrentTab()->m_editor->setSelectionsText(QStringList(""));
}

void MainWindow::on_currenTabChanged(NqqTab* tab)
{
    if(!tab) {
        qDebug() << "null tab";
        return; //TODO: Can/should this be null?

    }
    qDebug() << "MainWindow::on_currentTabChanged to " << tab->getTabTitle();

    getCurrentTabWidget()->setFocus(tab); //TODO: Do we need to set focus every time we change tabs? Here?

    Editor *editor = tab->m_editor;
    refreshTabUiInfo(tab);
    refreshEditorUiCursorInfo(editor);

}

void MainWindow::on_tabAdded(NqqTab* tab)
{
    qDebug() << "MainWindow::on_tabAdded";

    Editor *editor = tab->m_editor;
    
    // If the tab is not newly opened but only transferred (e.g. with "Move to other View") it may
    // have a banner attached to it. We need to disconnect previous signals to prevent
    // on_bannerRemoved() to be called twice (once for the current connection and once for the connection
    // created a few lines below).

    /*disconnect(editor, &Editor::bannerRemoved, 0, 0);
    

    connect(editor, &Editor::bannerRemoved, this, &MainWindow::on_bannerRemoved);
    */

    // Initialize editor with UI settings
    editor->setLineWrap(ui->actionWord_wrap->isChecked());
    editor->setTabsVisible(ui->actionShow_Tabs->isChecked());
    editor->setEOLVisible(ui->actionShow_End_of_Line->isChecked());
    editor->setWhitespaceVisible(ui->actionShow_Spaces->isChecked());
    editor->setOverwrite(m_overwrite);
    editor->setFont(m_settings.Appearance.getOverrideFontFamily(),
                    m_settings.Appearance.getOverrideFontSize(),
                    m_settings.Appearance.getOverrideLineHeight());
    editor->setSmartIndent(ui->actionToggle_Smart_Indent->isChecked());
}

void MainWindow::on_cursorActivity(NqqTab* tab)
{
    refreshEditorUiCursorInfo(tab->m_editor);
}

void MainWindow::on_currentLanguageChanged(NqqTab* tab)
{
    refreshTabUiInfo(tab);
}

void MainWindow::refreshEditorUiCursorInfo(Editor *editor)
{
    if (editor != 0) {
        // Update status bar
        int len = editor->sendMessageWithResult("C_FUN_GET_TEXT_LENGTH").toInt();
        int lines = editor->lineCount();
        m_statusBar_length_lines->setText(tr("%1 chars, %2 lines").arg(len).arg(lines));

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
    }
}

void MainWindow::refreshTabUiInfo(NqqTab* tab)
{
    Editor* editor = tab->m_editor;

    // Update current language in statusbar
    QVariantMap data = editor->sendMessageWithResult("C_FUN_GET_CURRENT_LANGUAGE").toMap();
    QString name = data.value("lang").toMap().value("name").toString();
    m_statusBar_fileFormat->setText(name);


    // Update MainWindow title
    QString newTitle;
    if (editor->fileName().isEmpty()) {
        newTitle = QString("%1 - %2").arg(tab->getTabTitle()).arg(QApplication::applicationName());
    } else {
        QUrl url = editor->fileName();

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
                   .arg(Notepadqq::fileNameFromUrl(editor->fileName()))
                   .arg(path)
                   .arg(QApplication::applicationName());

    }

    if (newTitle != windowTitle()) {
        setWindowTitle(newTitle.isNull() ? QApplication::applicationName() : newTitle);
    }


    // Enable / disable menus
    bool isClean = editor->isClean();
    QUrl fileName = editor->fileName();
    ui->actionRename->setEnabled(!fileName.isEmpty());
    ui->actionMove_to_New_Window->setEnabled(isClean);
    ui->actionOpen_in_New_Window->setEnabled(isClean);

    bool allowReloading = !editor->fileName().isEmpty();
    ui->actionReload_file_interpreted_as->setEnabled(allowReloading);
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
        ui->actionIndentation_Default_settings->setChecked(true);
    }
}

void MainWindow::on_action_Delete_triggered()
{
    getCurrentTabWidget()->getCurrentTab()->m_editor->setSelectionsText(QStringList(""));
}

void MainWindow::on_actionSelect_All_triggered()
{
    getCurrentTabWidget()->getCurrentTab()->m_editor->sendMessage("C_CMD_SELECT_ALL");
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
    getCurrentTabWidget()->getCurrentTab()->m_editor->sendMessage("C_CMD_UNDO");
}

void MainWindow::on_action_Redo_triggered()
{
    getCurrentTabWidget()->getCurrentTab()->m_editor->sendMessage("C_CMD_REDO");
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
    //disconnect(m_topEditorContainer, 0, this, 0);
}

void MainWindow::on_actionE_xit_triggered()
{
    close();
}

void MainWindow::instantiateFrmSearchReplace()
{
    if (!m_frmSearchReplace) {
       /* m_frmSearchReplace = new frmSearchReplace(
                                 m_topEditorContainer,
                                 this);

        connect(m_frmSearchReplace, &frmSearchReplace::fileSearchResultFinished,
                this, &MainWindow::on_fileSearchResultFinished);*/
    }
}

void MainWindow::on_fileSearchResultFinished(FileSearchResult::SearchResult result)
{
    m_fileSearchResultsWidget->addSearchResult(result);
    ui->dockFileSearchResults->show();
}

void MainWindow::on_actionSearch_triggered()
{
    if (!m_frmSearchReplace) {
        instantiateFrmSearchReplace();
    }

   /* QStringList sel = currentEditor()->selectedTexts();
    if (sel.length() > 0 && sel[0].length() > 0) {
        m_frmSearchReplace->setSearchText(sel[0]);
    }

    m_frmSearchReplace->show(frmSearchReplace::TabSearch);
    m_frmSearchReplace->activateWindow();*/
}

void MainWindow::on_actionCurrent_Full_File_path_to_Clipboard_triggered()
{
    NqqTab* tab = getCurrentTabWidget()->getCurrentTab();
    Editor *editor = tab->m_editor;

    if (editor->fileName().isEmpty())
    {
        QApplication::clipboard()->setText(tab->getTabTitle());
    } else {
        QApplication::clipboard()->setText(
                    editor->fileName().toDisplayString(QUrl::PreferLocalFile |
                                                       QUrl::RemovePassword));
    }
}

void MainWindow::on_actionCurrent_Filename_to_Clipboard_triggered()
{
    NqqTab* tab = getCurrentTabWidget()->getCurrentTab();
    Editor *editor = tab->m_editor;

    if (editor->fileName().isEmpty())
    {
        QApplication::clipboard()->setText(tab->getTabTitle());
    } else {
        QApplication::clipboard()->setText(Notepadqq::fileNameFromUrl(editor->fileName()));
    }
}

void MainWindow::on_actionCurrent_Directory_Path_to_Clipboard_triggered()
{
    NqqTab* tab = getCurrentTabWidget()->getCurrentTab();
    Editor *editor = tab->m_editor;

    if(editor->fileName().isEmpty())
    {
        QApplication::clipboard()->setText("");
    } else {
        QApplication::clipboard()->setText(
                    editor->fileName().toDisplayString(QUrl::RemovePassword |
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
    /*frmPreferences *_pref;
    _pref = new frmPreferences(m_topEditorContainer, this);
    _pref->exec();
    _pref->deleteLater();*/
}

void MainWindow::on_actionClose_triggered()
{
    on_tabCloseRequested( getCurrentTabWidget()->getCurrentTab() );
}

void MainWindow::on_actionC_lose_All_triggered()
{
    qDebug() << "MainWindow::on_actionCloseAll";

    bool canceled = false;

    for (NqqTab* tab : getCurrentTabWidget()->getAllTabs()) {
        if(processTabClose(tab) == TabDontClose) {
            canceled = true;
            break;
        }
    }

    if(canceled) return;

    for (NqqTab* tab : getCurrentTabWidget()->getAllTabs()) {
        tab->forceCloseTab();
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
            //TODO save(tabWidget, tab);
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

            m_docEngine->reloadDocument(tabWidget, tab);
        });
    }
}

void MainWindow::on_actionReplace_triggered()
{
    if (!m_frmSearchReplace) {
        instantiateFrmSearchReplace();
    }

    /*QStringList sel = currentEditor()->selectedTexts();
    if (sel.length() > 0 && sel[0].length() > 0) {
        m_frmSearchReplace->setSearchText(sel[0]);
    }

    m_frmSearchReplace->show(frmSearchReplace::TabReplace);
    m_frmSearchReplace->activateWindow();*/
}

void MainWindow::on_actionPlain_text_triggered()
{
   // currentEditor()->setLanguage("plaintext");
}

void MainWindow::on_actionRestore_Default_Zoom_triggered()
{
    const qreal newZoom = m_settings.General.resetZoom();
   // m_topEditorContainer->currentTabWidget()->setZoomFactor(newZoom);
}

void MainWindow::on_actionZoom_In_triggered()
{
    /*qreal curZoom = currentEditor()->zoomFactor();
    qreal newZoom = curZoom + 0.25;*/
   // m_topEditorContainer->currentTabWidget()->setZoomFactor(newZoom);
    //m_settings.General.setZoom(newZoom);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    /*qreal curZoom = currentEditor()->zoomFactor();
    qreal newZoom = curZoom - 0.25;*/
    //m_topEditorContainer->currentTabWidget()->setZoomFactor(newZoom);
    //m_settings.General.setZoom(newZoom);
}

void MainWindow::on_editorMouseWheel(EditorTabWidget *tabWidget, int tab, QWheelEvent *ev)
{
    // Currently Unused
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        qreal curZoom = tabWidget->editor(tab)->zoomFactor();
        qreal diff = ev->delta() / 120;
        diff /= 10;

        // Increment/Decrement zoom factor by 0.1 at each step.
        qreal newZoom = curZoom + diff;
        tabWidget->setZoomFactor(newZoom);
        //m_settings.General.setZoom(newZoom); //TODO: We don't save the zoom factor after changing it at the moment
    }
}

void MainWindow::transformSelectedText(std::function<QString (const QString &)> func)
{
    Editor* editor = getCurrentTabWidget()->getCurrentTab()->m_editor;
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
    qDebug() << "MainWindow::on_actionCloseAllButCurrent";

    NqqTab* currentTab = getCurrentTabWidget()->getCurrentTab();
    bool canceled = false;

    for (NqqTab* tab : getCurrentTabWidget()->getAllTabs()) {
        if(tab == currentTab) continue;
        if(processTabClose(tab) == TabDontClose) {
            canceled = true;
            break;
        }
    }

    if(canceled) return;

    for (NqqTab* tab : getCurrentTabWidget()->getAllTabs()) {
        if(tab == currentTab) continue;
        tab->forceCloseTab();
    }
}

void MainWindow::on_actionSave_All_triggered()
{
    for (NqqTab* tab : getCurrentTabWidget()->getAllTabs()) {
        Editor* ed = tab->m_editor;

        if(ed->isClean()) continue;

        if (save(tab) == DocEngine::saveFileResult_Canceled)
            break;
    }
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

    /*if (editor == currentEditor()) {
        ui->actionRename->setEnabled(true);
    }*/
}

void MainWindow::on_documentReloaded(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    editor->removeBanner("filechanged");
    editor->removeBanner("fileremoved");



   /* if (currentEditor() == editor) {
        refreshEditorUiInfo(editor);
        refreshEditorUiCursorInfo(editor);
    }*/
}

void MainWindow::on_documentLoaded(EditorTabWidget *tabWidget, int tab, bool wasAlreadyOpened, bool updateRecentDocs)
{
    Editor *editor = tabWidget->editor(tab);

    const int MAX_RECENT_ENTRIES = 10;

    if(updateRecentDocs){
        QUrl newUrl = editor->fileName();
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
        connect(action, &QAction::triggered, this, [=]() {
            Editor* ed = m_docEngine->loadDocumentProper(url);
            getCurrentTabWidget()->createTab(ed);
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
//    EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
//    Editor *editor = tabWidget->currentEditor();

//    reloadWithWarning(tabWidget,
//                      tabWidget->currentIndex(),
//                      editor->codec(),
//                      editor->bom());
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
//    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
//    QUrl oldFilename = tabW->currentEditor()->fileName();
//    int result = saveAs(tabW, tabW->currentIndex(), false);

//    if (result == DocEngine::saveFileResult_Saved && !oldFilename.isEmpty()) {

//        if (QFileInfo(oldFilename.toLocalFile()) != QFileInfo(tabW->currentEditor()->fileName().toLocalFile())) {

//            // Remove the old file
//            QString filename = oldFilename.toLocalFile();
//            if (QFile::exists(filename)) {
//                if(!QFile::remove(filename)) {
//                    QMessageBox::warning(this, QApplication::applicationName(),
//                                         QString("Error: unable to remove file %1")
//                                         .arg(filename));
//                }
//            }
//        }

//    }
}

void MainWindow::on_actionWord_wrap_toggled(bool on)
{
    for (NqqTab* tab : getCurrentTabWidget()->getAllTabs()) {
        tab->m_editor->setLineWrap(on);
    }

    m_settings.General.setWordWrap(on);
}

void MainWindow::on_actionEmpty_Recent_Files_List_triggered()
{
    m_settings.General.resetRecentDocuments();
    updateRecentDocsInMenu();
}

void MainWindow::on_actionOpen_All_Recent_Files_triggered()
{
    QList<QVariant> recentDocs = m_settings.General.getRecentDocuments();

    for (const QVariant& doc : recentDocs) {
        Editor* ed = m_docEngine->loadDocumentProper(doc.toUrl());
        getCurrentTabWidget()->createTab(ed);
    }
}

void MainWindow::on_actionUNIX_Format_triggered()
{
    /*Editor *editor = currentEditor();
    editor->setEndOfLineSequence("\n");
    editor->markDirty();*/
}

void MainWindow::on_actionWindows_Format_triggered()
{
   /* Editor *editor = currentEditor();
    editor->setEndOfLineSequence("\r\n");
    editor->markDirty();*/
}

void MainWindow::on_actionMac_Format_triggered()
{
   /* Editor *editor = currentEditor();
    editor->setEndOfLineSequence("\r");
    editor->markDirty();*/
}

void MainWindow::convertEditorEncoding(Editor *editor, QTextCodec *codec, bool bom)
{
   /* editor->setCodec(codec);
    editor->setBom(bom);
    editor->markDirty();

    if (editor == currentEditor())
        refreshEditorUiInfo(editor);*/
}

void MainWindow::on_actionUTF_8_triggered()
{
    //convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), true);
}

void MainWindow::on_actionUTF_8_without_BOM_triggered()
{
    //convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), false);
}

void MainWindow::on_actionUTF_16BE_triggered()
{
    //convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-16BE"), true);
}

void MainWindow::on_actionUTF_16LE_triggered()
{
    //convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-16LE"), true);
}

void MainWindow::on_actionInterpret_as_UTF_8_triggered()
{
   /* m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), true);
    refreshEditorUiInfo(currentEditor());*/
}

void MainWindow::on_actionInterpret_as_UTF_8_without_BOM_triggered()
{
    /*m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), false);
    refreshEditorUiInfo(currentEditor());*/
}

void MainWindow::on_actionInterpret_as_UTF_16BE_UCS_2_Big_Endian_triggered()
{
   /* m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-16BE"), true);
    refreshEditorUiInfo(currentEditor());*/
}

void MainWindow::on_actionInterpret_as_UTF_16LE_UCS_2_Little_Endian_triggered()
{
    /*m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-16LE"), true);
    refreshEditorUiInfo(currentEditor());*/
}

void MainWindow::on_actionConvert_to_triggered()
{
   /* Editor *editor = currentEditor();
    frmEncodingChooser *dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Convert to:"));

    if (dialog->exec() == QDialog::Accepted) {
        convertEditorEncoding(editor, dialog->selectedCodec(), false);
    }

    dialog->deleteLater();*/
}

void MainWindow::on_actionReload_file_interpreted_as_triggered()
{
   /* Editor *editor = currentEditor();
    frmEncodingChooser *dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Reload as:"));*/

   /* if (dialog->exec() == QDialog::Accepted) {
        EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
        reloadWithWarning(tabWidget, tabWidget->currentIndex(), dialog->selectedCodec(), false);
    }*/

    //dialog->deleteLater();
}

void MainWindow::on_actionIndentation_Default_settings_triggered()
{
    //currentEditor()->clearCustomIndentationMode();
}

void MainWindow::on_actionIndentation_Custom_triggered()
{
   /* Editor *editor = currentEditor();

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
        ui->actionIndentation_Default_settings->setChecked(true);
    }

    dialog->deleteLater();*/
}

void MainWindow::on_actionInterpret_as_triggered()
{
  /*  Editor *editor = currentEditor();
    frmEncodingChooser *dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Interpret as:"));

    if (dialog->exec() == QDialog::Accepted) {
        m_docEngine->reinterpretEncoding(editor, dialog->selectedCodec(), false);
    }

    dialog->deleteLater();*/
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
 /*   QAction *a = qobject_cast<QAction*>(sender());
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

    QUrl url = currentEditor()->fileName();
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
    }*/
}

void MainWindow::on_actionPrint_triggered()
{
   /* QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer);
    if (dialog.exec() == QDialog::Accepted)
        currentEditor()->print(&printer);*/
}

void MainWindow::on_actionPrint_Now_triggered()
{
  //  QPrinter printer(QPrinter::HighResolution);
  //  currentEditor()->print(&printer);
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
    Editor *editor = getCurrentTabWidget()->getCurrentTab()->m_editor;
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

void MainWindow::on_actionOpen_a_New_Window_triggered()
{
    MainWindow *b = new MainWindow(QStringList(), 0);
    b->show();
}

void MainWindow::on_actionOpen_in_New_Window_triggered()
{
    Editor* editor = getCurrentTabWidget()->getCurrentTab()->m_editor;
    QStringList args;
    args.append(QApplication::arguments().first());
    if (!editor->fileName().isEmpty()) {
        args.append(editor->fileName().toString(QUrl::None));
    }

    MainWindow *b = new MainWindow(args, 0);
    b->show();
}

void MainWindow::on_actionMove_to_New_Window_triggered()
{
  /*  QStringList args;
    args.append(QApplication::arguments().first());
    if (!currentEditor()->fileName().isEmpty()) {
        args.append(currentEditor()->fileName().toString(QUrl::None));
    }*/

  /*  EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
    int tab = tabWidget->currentIndex();
    if (closeTab(tabWidget, tab) != tabCloseResult_Canceled) {
        MainWindow *b = new MainWindow(args, 0);
        b->show();
    }*/
}

void MainWindow::on_actionOpen_file_triggered()
{
    QStringList terms = currentWordOrSelections();
    if (!terms.isEmpty()) {
        for (QString term : terms) {   
            Editor* ed = m_docEngine->loadDocumentProper(QUrl::fromLocalFile(term));
            getCurrentTabWidget()->createTab(ed);
        }
    }
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

void MainWindow::on_actionFind_in_Files_triggered()
{
    //if (!m_frmSearchReplace) {
    //    instantiateFrmSearchReplace();
    //}

/*    QStringList sel = currentEditor()->selectedTexts();
    if (sel.length() > 0 && sel[0].length() > 0) {
        m_frmSearchReplace->setSearchText(sel[0]);
    }*/

    //m_frmSearchReplace->show(frmSearchReplace::TabSearchInFiles);
    //m_frmSearchReplace->activateWindow();
}

void MainWindow::on_actionDelete_Line_triggered()
{
    getCurrentTabWidget()->getCurrentTab()->m_editor->sendMessage("C_CMD_DELETE_LINE");
}

void MainWindow::on_actionDuplicate_Line_triggered()
{
    getCurrentTabWidget()->getCurrentTab()->m_editor->sendMessage("C_CMD_DUPLICATE_LINE");
}

void MainWindow::on_actionMove_Line_Up_triggered()
{
    getCurrentTabWidget()->getCurrentTab()->m_editor->sendMessage("C_CMD_MOVE_LINE_UP");
}

void MainWindow::on_actionMove_Line_Down_triggered()
{
    getCurrentTabWidget()->getCurrentTab()->m_editor->sendMessage("C_CMD_MOVE_LINE_DOWN");
}

void MainWindow::on_resultMatchClicked(const QString &fileName, int startLine, int startCol, int endLine, int endCol)
{
    QUrl url = stringToUrl(fileName);
   /* m_docEngine->loadDocument(url,
                              m_topEditorContainer->currentTabWidget());*/

    QPair<int, int> pos = m_docEngine->findOpenEditorByUrl(url);

    if (pos.first == -1 || pos.second == -1)
        return;

    /*EditorTabWidget *tabW = m_topEditorContainer->tabWidget(pos.first);
    Editor *editor = tabW->editor(pos.second);

    editor->setSelection(startLine, startCol, endLine, endCol);

    editor->setFocus();*/
}

void MainWindow::on_actionTrim_Trailing_Space_triggered()
{
  //  currentEditor()->sendMessage("C_CMD_TRIM_TRAILING_SPACE");
}

void MainWindow::on_actionTrim_Leading_Space_triggered()
{
   // currentEditor()->sendMessage("C_CMD_TRIM_LEADING_SPACE");
}

void MainWindow::on_actionTrim_Leading_and_Trailing_Space_triggered()
{
  //  currentEditor()->sendMessage("C_CMD_TRIM_LEADING_TRAILING_SPACE");
}

void MainWindow::on_actionEOL_to_Space_triggered()
{
   // currentEditor()->sendMessage("C_CMD_EOL_TO_SPACE");
}

void MainWindow::on_actionTAB_to_Space_triggered()
{
   // currentEditor()->sendMessage("C_CMD_TAB_TO_SPACE");
}

void MainWindow::on_actionSpace_to_TAB_All_triggered()
{
  //  currentEditor()->sendMessage("C_CMD_SPACE_TO_TAB_ALL");
}

void MainWindow::on_actionSpace_to_TAB_Leading_triggered()
{
  //  currentEditor()->sendMessage("C_CMD_SPACE_TO_TAB_LEADING");
}

void MainWindow::on_actionGo_to_line_triggered()
{
   /* Editor *editor = currentEditor();
    int currentLine = editor->cursorPosition().first;
    int lines = editor->lineCount();
    frmLineNumberChooser *frm = new frmLineNumberChooser(1, lines, currentLine + 1, this);
    if (frm->exec() == QDialog::Accepted) {
        int line = frm->value();
        editor->setSelection(line - 1, 0, line - 1, 0);
    }*/
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
   /* m_topEditorContainer->forEachEditor([&](const int, const int, EditorTabWidget *, Editor *editor) {
        editor->setSmartIndent(on);
        return true;
    });*/
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

   // Sessions::loadSession(m_docEngine, m_topEditorContainer, filePath);
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

   /* if (Sessions::saveSession(m_docEngine, m_topEditorContainer, filePath)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setText(tr("Error while trying to save this session. Please try a different file name."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
    }*/
}
