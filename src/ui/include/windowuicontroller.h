#ifndef WINDOWUICONTROLLER_H
#define WINDOWUICONTROLLER_H

#include <QObject>

class AdvancedSearchDock;
class MainWindow;
class NqqSettings;

namespace Ui {
class MainWindow;
}

/**
 * Configures MainWindow widgets that do not depend on document state.
 */
class WindowUiController : public QObject {
public:
    /// Creates a controller for the window's static widgets using non-owning references.
    WindowUiController(
        MainWindow& window, Ui::MainWindow& ui, NqqSettings& settings, AdvancedSearchDock& advancedSearchDock);

    /// Configures the window's static presentation once during construction.
    void configureStaticUi();

    /// Rebuilds the toolbar from the saved toolbar action list.
    void loadToolBar();

    /// Restores geometry and dock visibility saved for the window.
    void restoreWindowState();

    /// Recreates the commands shown in the Run menu.
    void generateRunMenu();

    /// Shows or hides the Extensions menu entry.
    void setExtensionsMenuVisible(bool visible);

    /// Applies the full-screen action state to the window.
    void setFullScreen(bool enabled);

    /// Applies and persists the menubar visibility state.
    void setMenuBarVisible(bool visible);

    /// Applies the toolbar visibility state.
    void setToolBarVisible(bool visible);

private:
    /// Assigns theme icons to static menus and actions.
    void loadIcons();

    /// Creates exclusive action groups used by static menu actions.
    void configureActionGroups();

    /// Creates the main toolbar and initializes its visibility controls.
    void configureToolBar();

    /// Creates the static status-bar widgets and their connections.
    void configureStatusBar();

    /// Adds the advanced-search dock and connects its static interaction signal.
    void configureAdvancedSearchDock();

    MainWindow& m_window;
    Ui::MainWindow& m_ui;
    NqqSettings& m_settings;
    AdvancedSearchDock& m_advancedSearchDock;
};

#endif // WINDOWUICONTROLLER_H
