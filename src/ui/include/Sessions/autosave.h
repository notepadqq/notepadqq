#ifndef AUTOSAVE_H
#define AUTOSAVE_H

#include <QTimer>
#include <QString>
#include <tuple>
#include <vector>

namespace EditorNS{
class Editor;
}
class MainWindow;

class Autosave {
public:

    /**
     * @brief restoreFromAutosave Reads the autosave sessions and recreates
     *        all windows and their tabs.
     */
    static void restoreFromAutosave();

    /**
     * @brief enableAutosave Starts periodic autosaving.
     */
    static void enableAutosave();

    /**
     * @brief disableAutosave Stops periodic autosaving.
     */
    static void disableAutosave();

private:
    // Interval between autosaves, in milliseconds.
    static const int AUTOSAVE_INTERVAL = 5000;

    static QTimer s_autosaveTimer;

    struct WindowData {
        MainWindow* ptr;
        std::vector<std::pair<EditorNS::Editor*, int>> editors;
    };

    static std::vector<WindowData> s_autosaveData;

    static void executeAutosave();
};

#endif // AUTOSAVE_H
