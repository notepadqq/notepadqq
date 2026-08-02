#ifndef EDITORUICONTROLLER_H
#define EDITORUICONTROLLER_H

#include <QMap>
#include <QObject>
#include <QPair>
#include <QStringList>
#include <QVariant>
#include <QtPromise>

#include <functional>

class DocEngine;
class EditorTabWidget;
class MainWindow;
class NqqSettings;
class QTextCodec;
class TopEditorContainer;
class QWheelEvent;
class QWidget;

namespace EditorNS {
class Editor;
}

namespace Ui {
class MainWindow;
}

/**
 * Coordinates editor signals, settings-backed actions, and active-editor UI state.
 *
 * MainWindow owns this QObject through Qt parent ownership. The document engine,
 * editor container, settings, generated UI, actions, and editors are referenced
 * non-owningly.
 */
class EditorUiController : public QObject {
public:
    /** Creates a controller over the existing window, UI, engine, container, and settings. */
    EditorUiController(MainWindow& window,
        Ui::MainWindow& ui,
        DocEngine& docEngine,
        TopEditorContainer& editorContainer,
        NqqSettings& settings);

    /// Restores editor-related action state and settings for editors already present.
    void configureUiFromSettings();

    /// Populates the Language menu and binds each language action to the active editor.
    void setupLanguagesMenu();

    /// Returns the active editor in the active editor pane.
    EditorNS::Editor* currentEditor() const;

    /// Connects one newly added editor and applies the current editor preferences to it.
    void connectEditor(EditorTabWidget* tabWidget, int tab);

    /// Refreshes and focuses the editor selected in @p tabWidget at @p tab.
    void currentEditorChanged(EditorTabWidget* tabWidget, int tab);

    /// Refreshes title, actions, language, EOL, encoding, and indentation for @p editor.
    void refreshCurrentEditor(EditorNS::Editor* editor);

    /// Updates the cursor, selection, character, and line summary in the status bar.
    void refreshCursorInfo(const QMap<QString, QVariant>& data);

    /// Refreshes cursor information when @p editor is still the active editor.
    void cursorActivity(EditorNS::Editor* editor, const QMap<QString, QVariant>& data);

    /// Refreshes language information when @p editor is still the active editor.
    void currentLanguageChanged(EditorNS::Editor* editor);

    /// Applies Ctrl-wheel zoom from the editor pane that received @p event.
    void handleMouseWheel(EditorTabWidget* tabWidget, int tab, QWheelEvent* event);

    /// Toggles insert/overwrite mode for all editors and updates its status button.
    void toggleOverwrite();

    /// Sets the active editor language to @p language.
    void setCurrentEditorLanguage(const QString& language);

    /// Shows or hides tab markers in every editor and persists the action state.
    void setTabsVisible(bool visible);

    /// Shows or hides space markers in every editor and persists the action state.
    void setSpacesVisible(bool visible);

    /// Shows or hides end-of-line markers in every editor and persists the action state.
    void setEndOfLineVisible(bool visible);

    /// Applies the Show All Characters state and restores individual symbol preferences when disabled.
    void setSymbols(bool visible);

    /// Enables or disables math rendering in every editor and persists the setting.
    void setMathRendering(bool enabled);

    /// Enables or disables line wrapping in every editor and persists the setting.
    void setWordWrap(bool enabled);

    /// Enables or disables smart indentation in every editor and persists the setting.
    void setSmartIndent(bool enabled);

    /// Applies and persists @p zoom for the active editor pane.
    void setZoom(qreal zoom);

    /// Restores the saved default zoom for the active editor pane.
    void restoreDefaultZoom();

    /// Applies the persisted zoom to every editor pane after construction or session loading.
    void restoreSavedZoom();

    /// Adjusts active-editor zoom by @p delta and persists the result.
    void adjustZoom(qreal delta);

    /// Copies active selections to the clipboard separated by line breaks.
    void copySelections();

    /// Pastes normalized clipboard text into every active selection.
    void pasteSelections();

    /// Copies and removes the active selections.
    void cutSelections();

    /// Alternates between recording a selection anchor and selecting to the current cursor.
    void beginEndSelect();

    /// Removes all active selections.
    void deleteSelections();

    /// Selects the full active document.
    void selectAll();

    /// Applies right-to-left text direction to the active editor.
    void setRightToLeft();

    /// Applies left-to-right text direction to the active editor.
    void setLeftToRight();

    /// Undoes the last active-editor change.
    void undo();

    /// Redoes the last undone active-editor change.
    void redo();

    /// Sets the active editor language to plain text.
    void setPlainText();

    /// Replaces each active selection using @p transform while retaining the selection.
    void transformSelectedText(const std::function<QString(const QString&)>& transform);

    /// Deletes a detached editor banner after its removal signal.
    void removeBanner(QWidget* banner);

    /// Detects indentation drift for @p editor and presents the existing correction banner.
    void checkIndentationMode(EditorNS::Editor* editor);

    /// Sets the active editor EOL sequence and marks it dirty.
    void setEndOfLineSequence(const QString& sequence);

    /// Converts the active editor to @p codec with optional BOM and marks it dirty.
    void convertCurrentEditorEncoding(QTextCodec* codec, bool bom);

    /// Reinterprets the active editor as @p codec with optional BOM and refreshes its UI.
    void reinterpretCurrentEditorEncoding(QTextCodec* codec, bool bom);

    /// Shows the encoding chooser and converts the active editor to its selected codec.
    void chooseEncodingForConversion();

    /// Shows the encoding chooser and reloads the active file using its selected codec.
    void chooseEncodingForReload();

    /// Clears the active editor's custom indentation mode.
    void useDefaultIndentation();

    /// Shows the indentation chooser and applies its result to the active editor.
    void chooseCustomIndentation();

    /// Shows the encoding chooser and reinterprets the active editor using its selected codec.
    void chooseEncodingForInterpretation();

    /// Returns selected terms, or the current word when the first selection is empty.
    QtPromise::QPromise<QStringList> currentWordOrSelections();

    /// Returns the first selected term, or the current word when no text is selected.
    QtPromise::QPromise<QString> currentWordOrSelection();

    /// Sends an existing fire-and-forget editor command to the active editor.
    void sendEditorCommand(const QString& command);

    /// Shows the line-number chooser and moves the active cursor to the selected line.
    void goToLine();

private:
    /// Synchronizes individual symbol preferences with the Show All Characters action.
    bool updateSymbols(bool visible);

    /// Converts @p editor to @p codec with optional BOM and refreshes active-editor UI.
    void convertEditorEncoding(EditorNS::Editor* editor, QTextCodec* codec, bool bom);

    MainWindow& m_window;
    Ui::MainWindow& m_ui;
    DocEngine& m_docEngine;
    TopEditorContainer& m_editorContainer;
    NqqSettings& m_settings;
    bool m_overwrite = false;
    QPair<int, int> m_beginSelectPosition;
    bool m_beginSelectPositionSet = false;
};

#endif // EDITORUICONTROLLER_H
