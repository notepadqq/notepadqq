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
#include <QMutex>
#include <QMutexLocker>
#include <QEventLoop>
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

        /**
         * @brief Document information struct containing line and character counts.
         *
         */
        struct UiChangeInfo {
            int charCount; /**< The number of characters contained in the document */
            int lineCount; /**< The number of lines contained in the document */
        };

        /**
         * @brief Cursor information struct containing cursor and selection information.
         */
        struct UiCursorInfo {
            int line; /**< The current line of the cursor */
            int column; /**< The current column of the cursor */
            int selectionCharCount; /**< The currently selected text length, in characters */
            int selectionLineCount; /**< The number of lines selected */
        }; 

        struct Selection {
            Cursor from;
            Cursor to;
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
        static QList<QMap<QString, QString> > languages();


        /**
         * @brief Get the currently active language used
         *        in the editor.
         * @param val The value to pull from the language data,
         *        or ID by default.
         * @return The value associated with the key "val".
         */

        QString getLanguage(const QString& val = "id");
        /**
         * @brief Set the language to use for the editor.
         *        It automatically adjusts tab settings from
         *        the default configuration for the specified language.
         * @param language Language id
         */
        void setLanguage(const QString &language);
        void setLanguageFromFileName(QString fileName);
        void setLanguageFromFileName();
        
        
        QString value();
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

        /**
         * @brief Returns the currently selected texts.
         * @return
         */
        QStringList getSelectedTexts();
        QList<Selection> getSelections();

        void setOverwrite(bool overwrite);
        void setTabsVisible(bool visible);

        /**
         * @brief Detect the indentation mode used within the current document.
         * @return
         */
        Editor::IndentationMode detectDocumentIndentation(bool *found = nullptr);
        Editor::IndentationMode indentationMode();

        QString getCurrentWord(); 
        int getLineCount();
        int getCharCount();

    private:
        static QQueue<Editor*> m_editorBuffer;
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
        bool m_clean = true;
        
        UiChangeInfo docInfo;
        UiCursorInfo cursorInfo;

        inline void waitAsyncLoad();
        QString jsStringEscape(QString str) const;

        void fullConstructor(const Theme &theme);

        void setIndentationMode(const bool useTabs, const int size);
        void setIndentationMode(QString language);
        void initContextMenu();
        void initJsProxy();
        void initWebView(const Theme &theme);
        /**
         * @brief Generate a document change signal, with data.
         * @param v The data to be parsed.
         */
        void generateChangeActivitySignal(const QVariantMap& v);
        /**
         * @brief Generate a cursor activity signal, with data.
         * @param v The data to be parsed.
         */
        void generateCursorActivitySignal(const QVariantMap& v);
        /**
         * @brief Generate a language change signal, with data.
         * @param v The data to be parsed.
         */
        void generateLanguageChangeSignal(const QVariantMap& v);
    private slots:
        void on_javaScriptWindowObjectCleared();
        /**
         * @brief A message was received from the proxy.
         * @param msg The message
         * @param data Attached data, if any.
         */
        void on_proxyMessageReceived(QString msg, QVariant data);

    signals:
        //FIXME: Is the below signal still relevant?
        /**
         * @brief The editor received a message from Javascript.
         */
        void messageReceived(QString msg, QVariant data);
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
         * @brief The active document was modified.
         */
        void documentChanged(UiChangeInfo info);
        /**
         * @brief Cursor activity was detected.
         */
        void cursorActivity(UiCursorInfo info);
        /**
         * @brief The clean state of the document changed.
         */
        void cleanChanged(bool isClean);
        /**
         * @brief The file name of the document changed.
         */
        void fileNameChanged(const QUrl &oldFileName, const QUrl &newFileName);
        /**
         * @brief A document was loaded, or reloaded.
         */
        void documentLoaded(bool wasAlreadyLoaded);

        /**
         * @brief The editor finished loading. There should be
         *        no need to use this signal outside this class.
         */
        void editorReady();

        /**
         * @brief The editor language was changed.
         */
        void currentLanguageChanged(QString id, QString name);

    public slots:
        void sendMessage(const QString &msg, const QVariant &data = QVariant());
        QVariant sendMessageWithResult(const QString &msg, const QVariant &data = QVariant());
        void on_cursorInfoRequest();
        void print(QPrinter *printer);
    };
}

#endif // EDITOR_H
