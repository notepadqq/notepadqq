#ifndef SESSIONS_H
#define SESSIONS_H

#include <QString>

class MainWindow;

namespace Sessions {

/**
 * @brief Saves a session as an XML file
 * @param window The MainWindow whose tabs will be saved.
 * @param sessionPath Path to where the XML file should be created.
 * @param cacheDirPath Path to the directory where modified files will be written to. If
 *        left empty, no files will be cached. All prior files inside the cache directory
 *        will be deleted.
 * @return Whether the save has been successful.
 */
bool saveSession(MainWindow* window, QString sessionPath, QString cacheDirPath=QString());

/**
 * @brief Loads a session XML file and restores all its tabs in the specified window.
 * @param window The MainWindow which will receive all loaded tabs.
 * @param sessionPath Path to where the XML file is located.
 */
void loadSession(MainWindow* window, QString sessionPath);

} // namespace Autosave

#endif // SESSIONS_H
