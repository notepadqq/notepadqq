#ifndef SESSIONS_H
#define SESSIONS_H

#include <QString>

class DocEngine;
class TopEditorContainer;

namespace Sessions {

/**
 * @brief Saves a session as an XML file
 * @param docEngine The DocEngine that will be used to save all tabs to disk.
 * @param editorContainer The TopEditorContainer whose views and tabs will be saved.
 * @param sessionPath Path to where the XML file should be created.
 * @param cacheDirPath Path to the directory where modified files will be written to. If
 *        left empty, no files will be cached. All prior files inside the cache directory
 *        will be deleted.
 * @return Whether the save has been successful.
 */
bool saveSession(DocEngine* docEngine, TopEditorContainer* editorContainer, QString sessionPath, QString cacheDirPath=QString());

/**
 * @brief Loads a session XML file and restores all its tabs in the specified window.
 * @param docEngine The DocEngine used to load all files.
 * @param editorContainer The TopEditorContainer which will receive all newly crated Tabs.
 * @param sessionPath Path to where the XML file is located.
 */
void loadSession(DocEngine* docEngine, TopEditorContainer* editorContainer, QString sessionPath);

} // namespace Autosave

#endif // SESSIONS_H
