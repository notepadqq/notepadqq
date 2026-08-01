#ifndef DOCUMENTCONTROLLER_H
#define DOCUMENTCONTROLLER_H

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

class DocEngine;
class EditorTabWidget;
class MainWindow;
class NqqSettings;
class QDragEnterEvent;
class QDropEvent;
class TopEditorContainer;

namespace EditorNS {
class Editor;
}

/**
 * Coordinates the document lifecycle for one MainWindow.
 *
 * MainWindow owns this QObject through Qt parent ownership. The engine, editor
 * container, settings, generated UI, and actions remain owned elsewhere and
 * are referenced non-owningly.
 */
class DocumentController : public QObject {
public:
    /**
     * Creates a controller for @p window using non-owning references to its
     * document engine, editor container, and settings.
     */
    DocumentController(
        MainWindow& window, DocEngine& docEngine, TopEditorContainer& editorContainer, NqqSettings& settings);

    /// Opens paths supplied on the command line and applies an optional cursor position after loading.
    void openCommandLineProvidedUrls(const QString& workingDirectory, const QStringList& arguments);

    /// Accepts a drag operation when it carries URLs.
    void handleDragEnter(QDragEnterEvent* event);

    /// Opens URLs dropped on the main window.
    void handleDrop(QDropEvent* event);

    /// Opens URLs dropped on an editor, or on the active view when @p sourceEditor is null.
    void openDroppedUrls(QList<QUrl> urls, EditorNS::Editor* sourceEditor = nullptr);

    /// Shows the file picker and opens the selected documents.
    void openFiles();

    /// Shows the folder picker and opens its non-hidden, non-backup files.
    void openFolder();

    /// Saves cached session data or checks all documents before the window closes.
    bool prepareToClose();

    /// Closes one tab using the requested removal and prompting policy.
    int closeTab(EditorTabWidget* tabWidget, int tab, bool remove, bool force);

    /// Closes one tab, prompting for unsaved changes when needed.
    int closeTab(EditorTabWidget* tabWidget, int tab);

    /// Saves a document, prompting for a path when it has none.
    int save(EditorTabWidget* tabWidget, int tab);

    /// Saves a document to a selected path, optionally leaving the original association unchanged.
    int saveAs(EditorTabWidget* tabWidget, int tab, bool copy);

    /// Closes every document after confirming all unsaved changes.
    void closeAll();

    /// Closes every document except the active one.
    void closeAllExceptCurrent();

    /// Closes every document to the left of the active tab.
    void closeLeft();

    /// Closes every document to the right of the active tab.
    void closeRight();

    /// Saves all dirty documents until one save is cancelled.
    void saveAll();

    /// Handles a document becoming changed or removed on disk.
    void fileOnDiskChanged(EditorTabWidget* tabWidget, int tab, bool removed);

    /// Clears disk-change UI after a successful save.
    void documentSaved(EditorTabWidget* tabWidget, int tab);

    /// Clears disk-change UI and refreshes the active editor after a reload.
    void documentReloaded(EditorTabWidget* tabWidget, int tab);

    /// Updates recent history and post-load document checks.
    void documentLoaded(EditorTabWidget* tabWidget, int tab, bool wasAlreadyOpened, bool updateRecentDocs);

    /// Reloads the active document from disk with its current encoding.
    void reloadCurrentDocument();

    /// Saves the active document under a new name and removes the old local file.
    void renameCurrentDocument();

    /// Clears the recent-document history.
    void clearRecentFiles();

    /// Opens every recent document, after resolving missing-file entries with the user.
    void openAllRecentFiles();

    /// Opens one recent-file entry or removes it when the user declines recreation.
    void openRecentFileEntry(const QUrl& url);

    /// Shows a session picker and loads the selected session.
    void loadSession();

    /// Shows a session picker and saves the current session.
    void saveSession();

    /// Resolves a command-line or search result path relative to the window working directory.
    QUrl stringToUrl(const QString& fileName, const QString& workingDirectory = QString()) const;

private:
    /// Persists the current tabs to the recovery cache, allowing retry or ignore on failure.
    bool saveTabsToCache();

    /// Confirms every dirty tab can be finalized without removing it.
    bool finalizeAllTabs();

    /// Prompts whether a dirty tab should be saved, discarded, or kept open.
    int askIfWantToSave(EditorTabWidget* tabWidget, int tab, int reason);

    /// Chooses the existing path or a language-aware default for the save dialog.
    QUrl saveDialogDefaultFileName(EditorTabWidget* tabWidget, int tab) const;

    MainWindow& m_window;
    DocEngine& m_docEngine;
    TopEditorContainer& m_editorContainer;
    NqqSettings& m_settings;
};

#endif // DOCUMENTCONTROLLER_H
