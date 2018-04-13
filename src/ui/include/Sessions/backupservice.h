#ifndef AUTOSAVE_H
#define AUTOSAVE_H

#include <QTimer>
#include <QString>
#include <tuple>
#include <set>

namespace EditorNS{
class Editor;
}
class MainWindow;

/**
 * @brief The BackupService class handles automatic saving of currently open windows, tabs, and documents.
 *        When this system is used, clearBackupData() should be called at program shutdown.
 *        If, on program start, there is backup data to be found, the system will assume an improper shutdown.
 */
class BackupService {
public:

    /**
     * @brief restoreFromAutosave Reads the autosave sessions and recreates
     *        all windows and their tabs.
     */
    static bool restoreFromBackup();

    /**
     * @brief detectImproperShutdown
     * @return True if the system suspects that Nqq has crashed. This means backup data was found.
     */
    static bool detectImproperShutdown();

    /**
     * @brief enableAutosave Starts periodic autosaving.
     */
    static void enableAutosave(int intervalInSeconds);

    /**
     * @brief disableAutosave Stops periodic autosaving.
     */
    static void disableAutosave();

    static void clearBackupData();

private:
    static QTimer s_autosaveTimer;
    static bool s_autosaveEnabled;

    /**
     * @brief The WindowData struct contains a list Editor*'s and the history generation
     *        at the time of the last autosave. With every new autosave this data will be
     *        compared to the MainWindow's current list of Editor*'s and their history gens
     *        to determine whether the MainWindow has changed since the last save.
     */
    struct WindowData {
        MainWindow* ptr;
        std::vector<std::pair<EditorNS::Editor*, int>> editors;

        // Note this is only a shallow comparison. Do deep compares using isFullyEqual().
        bool operator==(const WindowData& other) const { return ptr == other.ptr; }
        bool operator<(const WindowData& other) const { return ptr < other.ptr; }

        bool isFullyEqual(const WindowData& other) const { return ptr == other.ptr && editors == other.editors; }
    };

    /**
     * @brief s_backupWindowData contains the WindowData at the time of the last backup.
     */
    static std::set<WindowData> s_backupWindowData;

    /**
     * @brief executeAutosave Updates the data inside s_autosaveData and executes saveSession
     *        for every open MainWindow that needs it.
     */
    static void executeBackup();

    /**
     * @brief writeBackup Writes a backup of the given MainWindow into a unique location inside the backupCache
     * @return True if the backup was created successfully
     */
    static bool writeBackup(MainWindow* wnd);
};

#endif // AUTOSAVE_H
