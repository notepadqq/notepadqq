#ifndef EDITOR_H
#define EDITOR_H

#include "include/EditorNS/customqwebview.h"
#include "include/EditorNS/jsproxy.h"
#include <QObject>
#include <QVariant>
#include <QJsonValue>
#include <QQueue>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QTextCodec>
#include <QPrinter>
#include <QEventLoop>
#include <QJsonDocument>
#include <functional>
namespace EditorNS
{

    /**
         * @brief Provides a JavaScript CodeMirror instance.
         *
         * Communication works by sending messages to the javascript Editor using
         * the sendMessage() method. On the other side, when a javascript event
         * occurs, the messageReceived() signal will be emitted.
         *
         * In addition to messageReceived(), other signals could be emitted at the
         * same time, for example currentLineChanged(). This is simply for
         * convenience, so that the user of this class doesn't need to manually parse
         * the arguments for pre-defined messages.
         *
         */
    class Editor : public QWidget
    {
        Q_OBJECT
    public:
        explicit Editor(QWidget *parent = 0);

        /**
         * @brief Efficiently returns a new Editor object from an internal buffer.
         * @return
         */
        static QSharedPointer<Editor> getNewEditor(QWidget *parent = 0);
        static Editor *getNewEditorUnmanagedPtr(QWidget *parent);

        static void invalidateEditorBuffer();

        //FIXME: Possibly un-necessary?
        /*
        struct LanguageGreater {
            inline bool operator()(const QMap<QString, QString> &v1, const QMap<QString, QString> &v2) const {
                return v1.value("name").toLower() < v2.value("name").toLower();
            }
        };
        */

        /**
         * @brief Struct containing cursor position information.
         */
        struct Cursor {
            int line; /**< Current line of the cursor */
            int column; /**< Current column of the cursor */

            bool operator == (const Cursor &x) const {
                return line == x.line && column == x.column;
            }

            bool operator < (const Cursor &x) const {
                return std::tie(line, column) < std::tie(x.line, x.column);
            }

            bool operator <= (const Cursor &x) const {
                return *this == x || *this < x;
            }

            bool operator > (const Cursor &x) const {
                return !(*this <= x);
            }

            bool operator >= (const Cursor &x) const {
                return !(*this < x);
            }
        };

        /** 
         * @brief Struct containing selection information.
         */
        struct Selection {
            Cursor from; /**< Cursor struct containing the selection beginning */
            Cursor to; /**< Cursor struct containing the selection ending */
        };

        /**
         * @brief Struct containing indentation information.
         */
        struct IndentationMode {
            bool useTabs = true; /**< Whether to use tabs or not */
            int size = 4; /**< The spacing used for spaces/tabs when tab is pressed */
            bool custom = false; /**< Whether we're using a custom mode or not */
        };

        /**
         * @brief Cursor information struct containing cursor and selection information.
         */
        struct CursorInfo {
            int line; /**< The current line of the cursor */
            int column; /**< The current column of the cursor */
            int selectionCharCount; /**< The currently selected text length, in characters */
            int selectionLineCount; /**< The number of lines selected */
            QList<Selection> selections; /**< List of selection ranges */
        }; 

        /**
         * @brief Struct containing information about specific languages.
         */
        struct LanguageData {
            QString id; /**< The internal language ID */
            QString name; /**< The friendly name of the language */
            QString mime; /**< Internal mime information for the language */
            QString mode; /**< Internal mode information for the language */
            QStringList fileNames; /**< List of file names valid for the language */
            QStringList fileExtensions; /**< list of file extensions valid for the language */
            QStringList firstNonBlankLine; /**< (Possibly deprecated) Language detection sequences */
        };

        /**
         * @brief Content information, such as line count, character count, 
         *        and clean state
         *
         */
        struct ContentInfo {
            int charCount; /**< The number of characters contained in the document */
            int lineCount; /**< The number of lines contained in the document */
            bool clean = true; /**< Whether the content of the editor is clean or not */
            IndentationMode indentMode; /**< Indentation mode for the current document */
            QPair<int, int> scrollPosition; /**< Scroll position for the current document */
            QString newLine = "\n"; /**< Newline sequence for the current document*/
        };


        /**
         * @brief Centralized struct for containing different data
         *        about the editor state, content, language, etc.
         */
        struct EditorInfo {
            CursorInfo cursor;
            ContentInfo content;
            LanguageData language;
        };

        /**
         * @brief Struct containing theme information.
         */
        struct Theme {
            QString name;
            QString path;
        };

        /**
         * @brief Enum containing selection mode information.
         */
        enum class SelectMode {
            CursorBefore,
            CursorAfter,
            Selected
        };

        /**
         * @brief Adds a new Editor to the internal buffer used by getNewEditor().
         *        You might want to call this method e.g. as soon as the application
         *        starts (so that an Editor is ready as soon as it gets required),
         *        or when the application is idle.
         * @param howMany specifies how many Editors to add
         * @return
         */
        static void addEditorToBuffer(const int howMany = 1);

        /**
         * @brief Give focus to the editor, so that the user can start
         *        typing. Note that calling won't automatically switch to
         *        the tab where the editor is. Use EditorTabWidget::setCurrentIndex()
         *        and TopEditorContainer::setFocus() for that.
         */
        void setFocus();

        /**
         * @brief Remove the focus from the editor.
         */
        void clearFocus();

        /**
         * @brief Set the file name associated with this editor
         * @param filename full path of the file
         */
        void setFileName(const QUrl &filename);

        /**
         * @brief Get the file name associated with this editor
         * @return
         */
        QUrl fileName() const;


        bool fileOnDiskChanged() const;
        void setFileOnDiskChanged(bool fileOnDiskChanged);

        /**
         * @brief Inserts a banner into the top of the Editor layout
         * @param banner
         */
        void insertBanner(QWidget *banner);
        /**
         * @brief Removes a banner from the top of the Editor layout
         * @param banner
         */
        void removeBanner(QWidget *banner);
        /**
         * @brief Removes a banner from the top of the Editor layout
         *        by its objectName
         * @param objectName
         */
        void removeBanner(QString objectName);

        /**
         * @brief Get the clean state of the current document.
         * @return bool
         */
        bool isClean();

        /**
         * @brief Marks the current document clean, such as after a file save.
         */
        void markClean();

        /**
         * @brief Marks the current document dirty, such as after EOL change.
         */
        void markDirty();

        /**
         * @brief Static function.  Gets a full list of languages available to
         *        the editor.
         * @return QVector<LanguageData>
         */
        static QVector<LanguageData> languages();

        /**
         * @brief Get the id of the language currently being used by the editor.
         * @return language_id
         */
        Editor::LanguageData getLanguage();

        /**
         * @brief Set the language to use for the editor.
         *        It automatically adjusts tab settings from
         *        the default configuration for the specified language.
         * @param language Language id
         */
        void setLanguage(const QString& language);

        /**
         * @brief This is an overloaded function.  Sets the language
         *        to use for the Editor using a LanguageData struct
         * @param language
         */
        void setLanguage(const Editor::LanguageData& language);

        /**
         * @brief Sets the language based on the filename given.
         * @param filename
         */
        void setLanguageFromFileName(const QString& filename);

        /**
         * @brief This is an overloaded function.  Sets the language
         *        based on the current internal m_fileName member.
         */
        void setLanguageFromFileName();
        
        /**
         * @brief Get the content of the editor.
         * @return QString content text.
         */
        QString value();
        /**
         * @brief Set the content of the editor, overwriting previous content.
         * @param value New content to set.
         */
        void setValue(const QString &value);

        /**
         * @brief Set custom indentation settings which may be different
         *        from the default tab settings associated with the current
         *        language.
         *        If this method is called, further calls to setLanguage()
         *        will NOT modify these tab settings. Use
         *        clearCustomIndentationMode() to reset to default settings.
         * @param useTabs
         * @param size Size of an indentation. If 0, keeps the current one.
         */
        void setCustomIndentationMode(const bool useTabs, const int size);
        void setCustomIndentationMode(const bool useTabs);

        /**
         * @brief Clears the custom indentation mode if one was set.
         */
        void clearCustomIndentationMode();

        /**
         * @brief Whether the editor is currently using a custom indentation mode
         *        or not.
         * @return bool
         */
        bool isUsingCustomIndentationMode() const;

        /**
         * @brief Enable/disable smart indentation mode for the editor.
         * @param enabled
         */
        void setSmartIndent(bool enabled);
        
        /**
         * @brief Get the current zoom factor for the m_webView member.
         * @return qreal
         */
        qreal zoomFactor() const;

        /**
         * @brief Set the current zoom factor for the m_webView member.
         * @param factor
         */
        void setZoomFactor(const qreal &factor);

        /**
         * @brief Enable/disable line wrap mode for the editor.
         * @param wrap
         */
        void setLineWrap(const bool wrap);

        /**
         * @brief Enable/disable visibility of new line sequences
         * @param showeol
         */
        void setEOLVisible(const bool showeol);

        /**
         * @brief Enable/disable visibility of white space
         * @param showspace
         */
        void setWhitespaceVisible(const bool showspace);

        /**
         * @brief Get the current cursor position
         * @return a <line, column> pair.
         */
        QPair<int, int> getCursorPosition();

        /**
         * @brief Set the current cursor position
         * @param line
         * @param column
         */
        void setCursorPosition(const int line, const int column);

        /**
         * @brief This is an overloaded function. Set the current cursor
         *        position using a QPair of ints as input.
         * @param position
         */
        void setCursorPosition(const QPair<int, int> &position);
        /**
         * @brief This is an overloaded function. Set the current cursor
         *        position using a Cursor struct as input.
         * @param cursor
         */
        void setCursorPosition(const Cursor &cursor);

        /**
         * @brief Get the current scroll position
         * @return a <left, top> pair.
         */
        QPair<int, int> getScrollPosition();
        /**
         * @brief Set the current scroll position, relative to the top/left
         *        corner of the CodeMirror instance.
         * @param left
         * @param top
         */
        void setScrollPosition(const int left, const int top);
        /**
         * @brief This is an overloaded function.  Set the current scroll
         *        position using a QPair of ints as input.
         * @param position
         */
        void setScrollPosition(const QPair<int, int> &position);

        /**
         * @brief Get the current new line sequence for the editor.
         * @return QString
         */
        const QString& endOfLineSequence() const;

        /**
         * @brief Set the current new line sequence for the editor.
         * @param endOfLineSequence
         */
        void setEndOfLineSequence(const QString &endOfLineSequence);

        /**
         * @brief Applies a font family/size to the Editor.
         * @param fontFamily the family to be applied. An empty string or
         *                   nullptr denote no override.
         * @param fontSize the size to be applied. 0 denotes no override.
         */
        void setFont(QString fontFamily, int fontSize, double lineHeight); 

        /**
         * @brief Get the current codec for the editor.  This is mainly
         *        used when saving the document.
         * @return QTextCodec pointer
         */
        QTextCodec *codec() const;

        /**
         * @brief Set the codec for this Editor.
         *        This method does not change the in-memory or on-screen
         *        representation of the document (which is always Unicode).
         *        It serves solely as a way to keep track of the encoding
         *        that needs to be used when the document gets saved.
         * @param codec
         */
        void setCodec(QTextCodec *codec);

        /**
         * @brief Whether or not the editor is using a byte order mark
         * @return bool
         */
        bool bom() const;
        /**
         * @brief Enable/disable byte order mark for the editor.
         * @param bom
         */
        void setBom(bool bom);

        /**
         * @brief Returns a list of themes available to the editor.
         * @return QList<Theme>
         */
        QList<Theme> themes();
        /**
         * @brief Sets the theme for the editor.
         * @param theme
         */
        void setTheme(Theme theme);
        /**
         * @brief Returns a Theme struct for a given name.  An emptry struct
         *        is returned if no theme is found.
         * @param name
         * @return Theme struct
         */
        static Editor::Theme themeFromName(QString name);
 
        /**
         * @brief Sets the selected text within the editor.
         * @param texts
         * @param selectMode
         */
        void setSelectionsText(const QStringList &texts, SelectMode mode);
        void setSelectionsText(const QStringList &texts);

        /**
         * @brief Sets the selection range for the editor.
         * @param fromLine
         * @param fromCol
         * @param toLine
         * @param toCol
         */
        void setSelection(int fromLine, int fromCol, int toLine, int toCol); 

        /**
         * @brief Enable/disable overwrite mode within the editor.
         * @param overwrite.
         */
        void setOverwrite(bool overwrite);

        /**
         * @brief Set the visibility of tab characters within the editor.
         * @param visible
         */
        void setTabsVisible(bool visible);

        /**
         * @brief Get the indentation mode for the current document.
         * @return Editor::IndentationMode struct
         */
        Editor::IndentationMode indentationMode();

        /**
         * @brief Set the indentation mode for the current document.
         * If transport is set to false, the data isn't sent to javascript.
         * @param useTabs
         * @param size
         * @param custom
         * @param transport
         */
        void setIndentationMode(const bool useTabs, const int size, 
                const bool custom, const bool transport = true);

        /**
         * @brief This is an overloaded function.  Sets the indentation mode
         *        based on the language data.
         * @param language
         * @param transport
         */
        void setIndentationMode(QString language, const bool transport = true);

        /**
         * @param Get the current line count for the document.
         * @return int
         */
        int getLineCount();

        /**
         * @param Get the current character count for the document.
         * @return int
         */
        int getCharCount();

        /**
         * @brief Requests content information from the editor.
         */
        void requestContentInfo();

        /**
         * @brief Requests cursor information from the editor.
         */
        void requestCursorInfo();

        /**
         * @brief Get the current selection boundaries.
         * @return const reference of cached selections.
         */
        const QList<Selection>& getSelections() const;

        /**
         * @brief Get the currently selected texts.
         * @param callback  Lambda or functor.
         *                  Must accept QStringList type as parameter.
         */
        void getSelectedTexts(std::function<void(const QStringList&)> callback);

        /**
         * @brief Get the currently selected text, or the word under the cursor
         *        if no text is selected.
         * @param callback Lambda or functor.
         *                 Must accept QStringList type as parameter.
         */

        void getCurrentWordOrSelections(std::function<void(const QStringList&)> callback); 

        /**
         * @brief Get the current content of the editor.
         * @param callback Lambda or functor.
         *        Must accept QString as type parameter.
         */
        void getValue(std::function<void(const QString&)> callback);

        /**
         * @brief Initialize the language cache for fast access.
         */
        static void initLanguageCache();

        bool isLoadedDocument();
    private:
        static QQueue<Editor*> m_editorBuffer;
        static QVector<LanguageData> m_langCache;
        CustomQWebView *m_webView = nullptr;
        EditorInfo m_info;
        JsProxy *m_jsProxy;
        QEventLoop m_processLoop;
        QTextCodec *m_codec = QTextCodec::codecForName("UTF-8");
        QUrl m_fileName = QUrl();
        bool m_fileOnDiskChanged = false;
        bool m_loaded = false;
        bool m_bom = false;
        bool m_alreadyLoaded = false;
        
        /**
         * @brief Waits for the editor to become ready for input.
         */
        inline void waitAsyncLoad();

        /**
         * @brief Returns an escaped version of the input string.
         * @param str
         * @return QString
         */
        QString jsStringEscape(QString str) const;


        /**
         * @brief Initializers for Editor construction
         */
        void initContextMenu();
        void initJsProxy();
        void initWebView();
        void fullConstructor();

        /**
         * @brief Build data for the contentChange signal.
         * @param data 
         * @param cache
         * @return ContentInfo struct.
         */
        ContentInfo buildContentChangedEventData(const QVariant& data,
                bool cache = true);
        /**
         * @brief Build data for the cursorActivity signal.
         * @param data 
         * @param cache
         * @return CursorInfo struct.
         */
        CursorInfo buildCursorEventData(const QVariant& data, 
                bool cache = true);

        /**
         * @brief Build data for documentLoaded signal.
         * @param data
         * @param cache
         * @return IndentationMode struct.
         */
        IndentationMode buildDocumentLoadedEventData(const QVariant& data,
                bool cache = true);

        /**
         * @brief Sends a javascript message, including data, to the editor.
         *        Invoking the specified "callback" when execution has 
         *        completed.
         * @param msg Initial message, which maps to a javascript function.
         * @param data QVariant filled with data to send to javascript.
         * @param callback Functor or lambda.
         */
        template<typename T>
        void sendMessageWithCallback(const QString& msg, const QVariant &data, T callback) {
            QString jsonData = QJsonDocument::fromVariant(data).toJson();
            if (jsonData.isEmpty()) 
                jsonData.append("0");
            QString jsMsg = QString("App.proxy.messageReceived('%1',%2)").arg(msg).arg(jsonData);
            m_webView->page()->runJavaScript(jsMsg, callback);
        }

        /**
         * @brief This is an overloaded function.
         *        Sends a javascript message to the editor, invoking the 
         *        specified "callback" when execution has completed.
         * @param msg Initial message, which maps to a javascript function.
         * @param data QVariant filled with data to send to javascript.
         * @param callback Functor or lambda.
         */
        template<typename T>
        void sendMessageWithCallback(const QString& msg, T callback) {
            sendMessageWithCallback(msg, QVariant(0), callback);
        }

        /**
         * @brief Finds the language with the given ID in m_langCache, and
         *        returns a QVariant representation of it for javascript.
         * @param langId
         * @return QVariant
         */
        QVariant getLanguageVariantData(const QString& langId);
    private slots:
        void on_loadFinished();
        /**
         * @brief Registers JsProxy to the WebChannel for use.
         */
        void on_javaScriptWindowObjectCleared();

        /**
         * @brief A message was received from the proxy.
         * @param msg The message
         * @param data Attached data, if any.
         */
        void on_proxyMessageReceived(QString msg, QVariant data);
        /**
         * @brief Updates the page background to match CodeMirror.  Keeps
         *        white flashes and ugly resizing at bay.
         */
        void updateBackground();
    signals:
        /**
         * @brief The editor got focus.
         */
        void gotFocus();

        /**
         * @brief A mouse wheel event was detected.
         */
        void mouseWheel(QWheelEvent *ev);

        /**
         * @brief An URL drop event was detected.
         */
        void urlsDropped(QList<QUrl> urls);

        /**
         * @brief The current banner was removed.
         */
        void bannerRemoved(QWidget *banner);

        /**
         * @brief The clean state of the document changed.
         */
        void cleanChanged(bool isClean);

        /**
         * @brief The active document was modified.
         */
        void contentChanged(ContentInfo);

        /**
         * @brief Cursor activity was detected.
         */
        void cursorActivity(CursorInfo);

        /**
         * @brief The file name of the document changed.
         */
        void fileNameChanged(const QUrl &oldFileName, const QUrl &newFileName);

        /**
         * @brief The editor language was changed.
         */
        void languageChanged(LanguageData);

        /**
         * @brief A document was loaded, or reloaded.
         */
        void documentLoaded(bool wasAlreadyLoaded, Editor::IndentationMode);

        /**
         * @brief The editor finished loading. There should be
         *        no need to use this signal outside this class.
         */
        void editorReady();

    public slots:
        /**
         * @brief Sends a message to JsProxy for the editor to interpret.
         * @param msg Command or function name.
         * @param data QVariant data object.
         */
        void sendMessage(const QString &msg, const QVariant &data = QVariant());
        /**
         * @brief This is a blocking call, use with caution.  Sends a message to
         *        JsProxy and waits for the result signal to return editor data.
         * @param msg Command or function name.
         * @param data QVariant data object
         * @return QVariant containing retrieved data.
         */
        QVariant sendMessageWithResult(const QString &msg, const QVariant &data = QVariant());
        /**
         * @brief Prints the current document.
         * @param printer
         */
        void print(QPrinter *printer);
    };
}

#endif // EDITOR_H
