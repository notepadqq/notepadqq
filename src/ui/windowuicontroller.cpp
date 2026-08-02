#include "include/windowuicontroller.h"

#include "include/Extensions/extensionsloader.h"
#include "include/Search/advancedsearchdock.h"
#include "include/iconprovider.h"
#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QLabel>
#include <QMapIterator>
#include <QMenu>
#include <QPalette>
#include <QPushButton>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>

#include <algorithm>

WindowUiController::WindowUiController(
    MainWindow& window, Ui::MainWindow& ui, NqqSettings& settings, AdvancedSearchDock& advancedSearchDock)
    : QObject(&window)
    , m_window(window)
    , m_ui(ui)
    , m_settings(settings)
    , m_advancedSearchDock(advancedSearchDock)
{
}

void WindowUiController::configureStaticUi()
{
    loadIcons();
    configureActionGroups();
    configureToolBar();
    configureStatusBar();
    configureAdvancedSearchDock();
    generateRunMenu();
    setExtensionsMenuVisible(Extensions::ExtensionsLoader::extensionRuntimePresent());
}

void WindowUiController::loadToolBar()
{
    m_window.m_mainToolBar->clear();

    QString toolbarItems = m_settings.MainWindow.getToolBarItems();
    if (toolbarItems.isEmpty())
        toolbarItems = m_window.getDefaultToolBarString();

    const auto actions = m_window.getActions();
    const auto parts = toolbarItems.split('|', Qt::SkipEmptyParts);
    for (const auto& part : parts) {
        if (part == "Separator") {
            m_window.m_mainToolBar->addSeparator();
            continue;
        }

        const auto it = std::find_if(
            actions.cbegin(), actions.cend(), [&part](QAction* action) { return action->objectName() == part; });
        if (it != actions.cend())
            m_window.m_mainToolBar->addAction(*it);
    }
}

void WindowUiController::restoreWindowState()
{
    m_window.restoreGeometry(m_settings.MainWindow.getGeometry());
    m_window.restoreState(m_settings.MainWindow.getWindowState());
    if (!m_window.isMaximized() && MainWindow::m_instances.count() > 1) {
        const QPoint currentPosition = m_window.pos();
        m_window.move(currentPosition.x() + 50, currentPosition.y() + 50);
    }
}

void WindowUiController::generateRunMenu()
{
    QMapIterator<QString, QString> commands(m_settings.Run.getCommands());
    m_ui.menu_Run->clear();

    QAction* action = m_ui.menu_Run->addAction(m_window.tr("Run..."));
    connect(action, &QAction::triggered, &m_window, &MainWindow::runCommand);
    m_ui.menu_Run->addSeparator();
    while (commands.hasNext()) {
        commands.next();
        action = m_ui.menu_Run->addAction(commands.key());
        action->setData(commands.value());
        action->setObjectName("RunCmd" + action->text());
        connect(action, &QAction::triggered, &m_window, &MainWindow::runCommand);
    }
    m_ui.menu_Run->addSeparator();
    action = m_ui.menu_Run->addAction(m_window.tr("Modify Run Commands"));
    connect(action, &QAction::triggered, &m_window, &MainWindow::modifyRunCommands);
}

void WindowUiController::setExtensionsMenuVisible(bool visible)
{ m_ui.menu_Extensions->menuAction()->setVisible(visible); }

void WindowUiController::setFullScreen(bool enabled)
{
    static bool maximized = m_window.isMaximized();
    if (enabled) {
        maximized = m_window.isMaximized();
        m_window.showFullScreen();
    } else if (maximized) {
        m_window.showMaximized();
    } else {
        m_window.showNormal();
    }
}

void WindowUiController::setMenuBarVisible(bool visible)
{
    m_ui.menuBar->setVisible(visible);
    m_settings.MainWindow.setMenuBarVisible(visible);
}

void WindowUiController::setToolBarVisible(bool visible)
{ m_window.m_mainToolBar->setVisible(visible); }

void WindowUiController::loadIcons()
{
    // File menu
    m_ui.actionNew->setIcon(IconProvider::fromTheme("document-new"));
    m_ui.actionOpen->setIcon(IconProvider::fromTheme("document-open"));
    m_ui.actionReload_from_Disk->setIcon(IconProvider::fromTheme("view-refresh"));
    m_ui.actionSave->setIcon(IconProvider::fromTheme("document-save"));
    m_ui.actionSave_as->setIcon(IconProvider::fromTheme("document-save-as"));
    m_ui.actionSave_a_Copy_As->setIcon(IconProvider::fromTheme("document-save-as"));
    m_ui.actionSave_All->setIcon(IconProvider::fromTheme("document-save-all"));
    m_ui.actionClose->setIcon(IconProvider::fromTheme("document-close"));
    m_ui.actionClose_All->setIcon(IconProvider::fromTheme("document-close-all"));
    m_ui.menuRecent_Files->setIcon(IconProvider::fromTheme("document-open-recent"));
    m_ui.actionExit->setIcon(IconProvider::fromTheme("application-exit"));
    m_ui.actionPrint->setIcon(IconProvider::fromTheme("document-print"));
    m_ui.actionPrint_Now->setIcon(IconProvider::fromTheme("document-print"));

    // Edit and search menus
    m_ui.actionUndo->setIcon(IconProvider::fromTheme("edit-undo"));
    m_ui.actionRedo->setIcon(IconProvider::fromTheme("edit-redo"));
    m_ui.actionCut->setIcon(IconProvider::fromTheme("edit-cut"));
    m_ui.actionCopy->setIcon(IconProvider::fromTheme("edit-copy"));
    m_ui.actionPaste->setIcon(IconProvider::fromTheme("edit-paste"));
    m_ui.actionDelete->setIcon(IconProvider::fromTheme("edit-delete"));
    m_ui.actionSelect_All->setIcon(IconProvider::fromTheme("edit-select-all"));
    m_ui.actionSearch->setIcon(IconProvider::fromTheme("edit-find"));
    m_ui.actionFind_Next->setIcon(IconProvider::fromTheme("go-next"));
    m_ui.actionFind_Previous->setIcon(IconProvider::fromTheme("go-previous"));
    m_ui.actionReplace->setIcon(IconProvider::fromTheme("edit-find-replace"));
    m_ui.actionGo_to_Line->setIcon(IconProvider::fromTheme("go-jump"));

    // View and application menus
    m_ui.actionShow_All_Characters->setIcon(IconProvider::fromTheme("show-special-chars"));
    m_ui.actionZoom_In->setIcon(IconProvider::fromTheme("zoom-in"));
    m_ui.actionZoom_Out->setIcon(IconProvider::fromTheme("zoom-out"));
    m_ui.actionRestore_Default_Zoom->setIcon(IconProvider::fromTheme("zoom-original"));
    m_ui.actionWord_wrap->setIcon(IconProvider::fromTheme("word-wrap"));
    m_ui.actionMath_Rendering->setIcon(IconProvider::fromTheme("math-rendering"));
    m_ui.actionFull_Screen->setIcon(IconProvider::fromTheme("view-fullscreen"));
    m_ui.actionPreferences->setIcon(IconProvider::fromTheme("preferences-other"));
    m_ui.actionRun->setIcon(IconProvider::fromTheme("system-run"));
    m_ui.actionOpen_a_New_Window->setIcon(IconProvider::fromTheme("window-new"));
    m_ui.actionAbout_Qt->setIcon(IconProvider::fromTheme("help-about"));
    m_ui.actionAbout_Notepadqq->setIcon(IconProvider::fromTheme("notepadqq"));

    // Macro toolbar actions
    m_ui.action_Start_Recording->setIcon(IconProvider::fromTheme("media-record"));
    m_ui.action_Stop_Recording->setIcon(IconProvider::fromTheme("media-playback-stop"));
    m_ui.action_Playback->setIcon(IconProvider::fromTheme("media-playback-start"));
    m_ui.actionRun_a_Macro_Multiple_Times->setIcon(IconProvider::fromTheme("media-seek-forward"));
    m_ui.actionSave_Currently_Recorded_Macro->setIcon(IconProvider::fromTheme("document-save-as"));
}

void WindowUiController::configureActionGroups()
{
    auto* eolActionGroup = new QActionGroup(&m_window);
    eolActionGroup->addAction(m_ui.actionWindows_Format);
    eolActionGroup->addAction(m_ui.actionUNIX_Format);
    eolActionGroup->addAction(m_ui.actionMac_Format);

    auto* indentationActionGroup = new QActionGroup(&m_window);
    indentationActionGroup->addAction(m_ui.actionIndentation_Default_Settings);
    indentationActionGroup->addAction(m_ui.actionIndentation_Custom);
}

void WindowUiController::configureToolBar()
{
    m_window.m_mainToolBar = new QToolBar("Toolbar", &m_window);
    m_window.m_mainToolBar->setIconSize(QSize(16, 16));
    m_window.m_mainToolBar->setObjectName("toolbar");
    m_window.addToolBar(m_window.m_mainToolBar);
    connect(m_window.m_mainToolBar, &QToolBar::visibilityChanged, m_ui.actionShow_Toolbar, &QAction::setChecked);
    m_ui.actionShow_Toolbar->setChecked(m_window.m_mainToolBar->isVisible());
    m_ui.menuBar->setVisible(m_settings.MainWindow.getMenuBarVisible());
    m_ui.actionShow_Menubar->setChecked(m_settings.MainWindow.getMenuBarVisible());

    QToolButton* openButton = static_cast<QToolButton*>(m_window.m_mainToolBar->widgetForAction(m_ui.actionOpen));
    if (openButton) {
        openButton->setMenu(m_ui.menuRecent_Files);
        openButton->setPopupMode(QToolButton::MenuButtonPopup);
    }
}

void WindowUiController::configureStatusBar()
{
    m_window.m_sbDocumentInfoLabel = new QLabel;
    m_window.m_sbDocumentInfoLabel->setMinimumWidth(1);
    m_window.statusBar()->addWidget(m_window.m_sbDocumentInfoLabel);
    const auto createStatusButton = [this](const QString& text, QMenu* menu = nullptr) {
        auto* button = new QPushButton(text);
        button->setFlat(true);
        button->setMenu(menu);
        button->setFocusPolicy(Qt::NoFocus);
#ifdef Q_OS_MACOS
        button->setStyleSheet(QString("QPushButton { background: %1; }").arg(QPalette().shadow().color().name()));
#endif
        m_window.statusBar()->addPermanentWidget(button);
        return button;
    };
    m_window.m_sbFileFormatBtn = createStatusButton("File Format", m_ui.menu_Language);
    m_window.m_sbEOLFormatBtn = createStatusButton("EOL", m_ui.menuEOL_Conversion);
    m_window.m_sbTextFormatBtn = createStatusButton("Encoding", m_ui.menu_Encoding);
    m_window.m_sbOvertypeBtn = createStatusButton("INS");
    connect(m_window.m_sbOvertypeBtn, &QPushButton::clicked, &m_window, &MainWindow::toggleOverwrite);
}

void WindowUiController::configureAdvancedSearchDock()
{
    m_window.addDockWidget(Qt::BottomDockWidgetArea, m_advancedSearchDock.getDockWidget());
    m_advancedSearchDock.getDockWidget()->hide(); // Saved preference is applied by restoreWindowState().
    connect(
        &m_advancedSearchDock, &AdvancedSearchDock::itemInteracted, &m_window, &MainWindow::searchDockItemInteracted);
}
