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
    private:

    public:

        struct Theme {
            QString name;
            QString path;
        };

        explicit Editor(const Theme &theme, QWidget *parent = 0);
        explicit Editor(QWidget *parent = 0);

        /**
             * @brief Efficiently returns a new Editor object from an internal buffer.
             * @return
             */
        static QSharedPointer<Editor> getNewEditor(QWidget *parent = 0);
        static Editor *getNewEditorUnmanagedPtr(QWidget *parent);

        static void invalidateEditorBuffer();

        struct LanguageGreater {
            inline bool operator()(const QMap<QString, QString> &v1, const QMap<QString, QString> &v2) const {
                return v1.value("name").toLower() < v2.value("name").toLower();
            }
        };

        struct Cursor {
            int line;
            int column;

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

        struct Selection {
            Cursor from;
            Cursor to;
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
        };

        /**
         * @brief Cursor information struct containing cursor and selection information.
         */
        struct CursorInfo {
            int line; /**< The current line of the cursor */
            int column; /**< The current column of the cursor */
            int selectionCharCount; /**< The currently selected text length, in characters */
            int selectionLineCount; /**< The number of lines selected */
            QList<Selection> selections;
        }; 

        // TODO: Maybe combine this into LanguageData?
        struct LanguageInfo {
            QString id;
            QString name;
        };

        struct LanguageData {
            QString id;
            QString name;
            QString mime;
            QString mode;
            QStringList fileNames;
            QStringList fileExtensions;
            QStringList firstNonBlankLine;
        };



        struct IndentationMode {
            bool useTabs;
            int size;
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

        enum selectMode {
            selectMode_cursorBefore,
            selectMode_cursorAfter,
            selectMode_selected
        };

        void insertBanner(QWidget *banner);
        void removeBanner(QWidget *banner);
        void removeBanner(QString objectName);

        // Lower-level message wrappers:
        bool isClean();
        void markClean();
        void markDirty();
        static QVector<LanguageData> languages();

        /**
         * @brief Get the id of the language currently being used by the editor.
         * @return language_id
         */
        QString getLanguage();

        /**
         * @brief Set the language to use for the editor.
         *        It automatically adjusts tab settings from
         *        the default configuration for the specified language.
         * @param language Language id
         */
        void setLanguage(const QString& language);
        void setLanguageFromFileName(const QString& filename);
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
        void clearCustomIndentationMode();
        bool isUsingCustomIndentationMode() const;
        void setSmartIndent(bool enabled);

        qreal zoomFactor() const;
        void setZoomFactor(const qreal &factor);

        
        void setLineWrap(const bool wrap);
        void setEOLVisible(const bool showeol);
        void setWhitespaceVisible(const bool showspace);

        /**
         * @brief Get the current cursor position
         * @return a <line, column> pair.
         */
        QPair<int, int> getCursorPosition();
        void setCursorPosition(const int line, const int column);
        void setCursorPosition(const QPair<int, int> &position);
        void setCursorPosition(const Cursor &cursor);

        /**
         * @brief Get the current scroll position
         * @return a <left, top> pair.
         */
        QPair<int, int> getScrollPosition();
        void setScrollPosition(const int left, const int top);
        void setScrollPosition(const QPair<int, int> &position);


        QString endOfLineSequence() const;
        void setEndOfLineSequence(const QString &endOfLineSequence);

        /**
         * @brief Applies a font family/size to the Editor.
         * @param fontFamily the family to be applied. An empty string or
         *                   nullptr denote no override.
         * @param fontSize the size to be applied. 0 denotes no override.
         */
        void setFont(QString fontFamily, int fontSize, double lineHeight); 

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

        bool bom() const;
        void setBom(bool bom);

        QList<Theme> themes();
        void setTheme(Theme theme);
        static Editor::Theme themeFromName(QString name);
 

        void setSelectionsText(const QStringList &texts, selectMode mode);
        void setSelectionsText(const QStringList &texts);
        void setSelection(int fromLine, int fromCol, int toLine, int toCol); 

        void setOverwrite(bool overwrite);
        void setTabsVisible(bool visible);

        /**
         * @brief Detect the indentation mode used within the current document.
         * @return
         */
        Editor::IndentationMode detectDocumentIndentation(bool *found = nullptr);
        Editor::IndentationMode indentationMode();

        int getLineCount();
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
         * @param callback  Lambda or functor.
         *                  Must accept QList<Editor::Selection> type as parameter.
         */
        const QList<Selection>& getSelections() const;

        /**
         * @brief Get the currently selected texts.
         * @param callback  Lambda or functor.
         *                  Must accept QStringList type as parameter.
         */
        void getSelectedTexts(std::function<void(const QStringList&)> callback);

        /**
         * @brief Get the current editor language.
         * @param callback Lambda or functor.
         *                 Must accept QString type as parameter.
         */
        void getLanguage(std::function<void(const QString&)> callback); 
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
    private:
        static QQueue<Editor*> m_editorBuffer;
        static QVector<LanguageData> m_langCache;
        QEventLoop m_processLoop;
        QVBoxLayout *m_layout;
        CustomQWebView *m_webView;
        JsProxy *m_jsProxy;
        QUrl m_fileName = QUrl();
        bool m_fileOnDiskChanged = false;
        bool m_loaded = false;
        QString m_endOfLineSequence = "\n";
        QTextCodec *m_codec = QTextCodec::codecForName("UTF-8");
        bool m_bom = false;
        bool m_customIndentationMode = false;
        bool m_alreadyLoaded = false;
        
        ContentInfo m_contentInfo;
        CursorInfo m_cursorInfo;

        inline void waitAsyncLoad();
        QString jsStringEscape(QString str) const;

        void fullConstructor(const Theme &theme);

        void setIndentationMode(const bool useTabs, const int size);
        void setIndentationMode(QString language);
        void initContextMenu();
        void initJsProxy();
        void initWebView(const Theme &theme);
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
         * @brief Build data for the languageChange signal.
         * @param data
         * @param cache
         * @return LanguageInfo struct.
         */
        LanguageInfo buildLanguageChangedEventData(const QVariant& data,
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
            QString jsMsg = QString("UiDriver.messageReceived('%1',%2)").arg(msg).arg(jsonData);
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
         * @brief Finds the language with the given ID in m_langCache.
         * @param langId
         * @return QMap<QString, QString>
         */
        QVariant getLanguageData(const QString& langId);

    private slots:
        void on_javaScriptWindowObjectCleared();
        /**
         * @brief A message was received from the proxy.
         * @param msg The message
         * @param data Attached data, if any.
         */
        void on_proxyMessageReceived(QString msg, QVariant data);

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
        void languageChanged(LanguageInfo);

        /**
         * @brief A document was loaded, or reloaded.
         */
        void documentLoaded(bool wasAlreadyLoaded);

        /**
         * @brief The editor finished loading. There should be
         *        no need to use this signal outside this class.
         */
        void editorReady();


    public slots:
        void sendMessage(const QString &msg, const QVariant &data = QVariant());
        QVariant sendMessageWithResult(const QString &msg, const QVariant &data = QVariant());
        void print(QPrinter *printer);
    };
}

#endif // EDITOR_H
