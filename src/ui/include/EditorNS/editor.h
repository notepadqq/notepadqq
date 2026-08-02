#ifndef EDITOR_H
#define EDITOR_H

#include "include/EditorNS/asyncrequesttracker.h"
#include "include/EditorNS/customqwebview.h"
#include "include/EditorNS/editor_properties.h"
#include "include/EditorNS/languageservice.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QObject>
#include <QPrinter>
#include <QVBoxLayout>
#include <QVariant>
#include <QWheelEvent>
#include <QtCore5Compat/QTextCodec>
#include <QtPromise>

#include <deque>
#include <functional>
#include <future>
#include <utility>

#include <optional>

#ifdef NQQ_CPP_CORRECTNESS_TESTS
class EditorCorrectnessAccess;
#endif

class EditorTabWidget;

namespace EditorNS {

/**
 * @brief An Object injectable into the javascript page, that allows
 *        the javascript code to send messages to an Editor object.
 *        It also allows the js instance to retrieve message data information.
 *
 * Note that this class is only needed for the current Editor
 * implementation, that uses QWebView.
 */
class JsToCppProxy : public QObject {
    Q_OBJECT

private:
    QVariant m_msgData;

public:
    JsToCppProxy(QObject* parent)
        : QObject(parent)
    {
    }

    Q_INVOKABLE void receiveMessage(QString msg, QVariant data) { emit messageReceived(msg, data); }

signals:
    /**
     * @brief A JavaScript message has been received.
     * @param msg Message type
     * @param data Message data
     */
    void messageReceived(QString msg, QVariant data);

    void messageReceivedByJs(QString msg, QVariant data);
};

/**
 * @brief Provides a JavaScript CodeMirror instance.
 *
 * Ownership: an Editor in a tab or layout is owned and destroyed by Qt's parent-child
 * hierarchy. Callers receive non-owning Editor pointers and must use QPointer when a
 * reference crosses an asynchronous boundary. The preload buffer owns only unattached
 * editors until takeNewEditor() transfers them to a Qt parent.
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
class Editor : public QWidget {
    Q_OBJECT
public:
    struct Theme {
        QString name;
        QString path;
        Theme(const QString& name = "default", const QString& path = "")
        {
            this->name = name;
            this->path = path;
        }
    };

    explicit Editor(const Theme& theme, QWidget* parent = nullptr);
    explicit Editor(QWidget* parent = nullptr);

    /**
     * @brief Releases a preloaded editor for immediate adoption by a Qt widget parent.
     *
     * The caller transfers ownership to Qt by adding the widget to a layout or tab widget.
     */
    static Editor* takeNewEditor();

    static void invalidateEditorBuffer();

    /**
     * @brief Just a flag that is used for marking editors that are still loading,
     * meaning for example that the Editor has been created but we still need
     * to load the file contents or setup the syntax highlighting.
     */
    bool isLoading = false;

    /**
     * @brief Adds a new Editor to the internal buffer used by takeNewEditor().
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
    Q_INVOKABLE void setFocus();

    /**
     * @brief Remove the focus from the editor.
     *
     * @param widgetOnly only clear the focus on the actual widget
     */
    Q_INVOKABLE void clearFocus();

    /**
     * @brief Set the file name associated with this editor
     * @param filename full path of the file
     */
    Q_INVOKABLE void setFilePath(const QUrl& filename);

    /**
     * @brief Get the file name associated with this editor
     * @return
     */
    Q_INVOKABLE QUrl filePath() const;

    Q_INVOKABLE bool fileOnDiskChanged() const;
    Q_INVOKABLE void setFileOnDiskChanged(bool fileOnDiskChanged);

    enum class SelectMode { Before, After, Selected };

    void insertBanner(QWidget* banner);
    void removeBanner(QWidget* banner);
    void removeBanner(QString objectName);

    // Lower-level message wrappers:
    QtPromise::QPromise<bool> isCleanP();
    Q_INVOKABLE bool isClean();
    Q_INVOKABLE QtPromise::QPromise<void> markClean();
    Q_INVOKABLE QtPromise::QPromise<void> markDirty();

    /**
     * @brief Returns an integer that denotes the editor's history state. Making changes to
     *        the contents increments the integer while reverting changes decrements it again.
     */
    Q_INVOKABLE QtPromise::QPromise<int> getHistoryGeneration();

    /**
     * @brief Set the language to use for the editor.
     *        It automatically adjusts tab settings from
     *        the default configuration for the specified language.
     * @param language Language id
     */
    Q_INVOKABLE void setLanguage(const Language* language);
    Q_INVOKABLE void setLanguage(const QString& language);
    Q_INVOKABLE void setLanguageFromFilePath(const QString& filePath);
    Q_INVOKABLE void setLanguageFromFilePath();
    Q_INVOKABLE QtPromise::QPromise<void> setValue(const QString& value);
    Q_INVOKABLE QString value();

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

    Q_INVOKABLE void setSmartIndent(bool enabled);
    Q_INVOKABLE qreal zoomFactor() const;
    Q_INVOKABLE void setZoomFactor(const qreal& factor);
    Q_INVOKABLE void setSelectionsText(const QStringList& texts, SelectMode mode);
    Q_INVOKABLE void setSelectionsText(const QStringList& texts);
    const Language* getLanguage() { return m_currentLanguage; }
    Q_INVOKABLE void setLineWrap(const bool wrap);
    Q_INVOKABLE void setEOLVisible(const bool showeol);
    Q_INVOKABLE void setWhitespaceVisible(const bool showspace);
    Q_INVOKABLE void setMathEnabled(const bool enabled);

    /**
     * @brief Get the current cursor position
     * @return a <line, column> pair.
     */
    QPair<int, int> cursorPosition();
    QtPromise::QPromise<QPair<int, int>> cursorPositionP();
    void setCursorPosition(const int line, const int column);
    void setCursorPosition(const QPair<int, int>& position);
    void setCursorPosition(const Cursor& cursor);

    /**
     * @brief Tells the editor that mainwindow needs an update on the contents,
     *        selection, and cursor position of the current document
     */
    void requestDocumentInfo();

    /**
     * @brief Get the current scroll position
     * @return a <left, top> pair.
     */
    QPair<int, int> scrollPosition();
    void setScrollPosition(const int left, const int top);
    void setScrollPosition(const QPair<int, int>& position);
    QString endOfLineSequence() const;
    void setEndOfLineSequence(const QString& endOfLineSequence);

    /**
     * @brief Applies a font family/size to the Editor.
     * @param fontFamily the family to be applied. An empty string or
     *                   nullptr denote no override.
     * @param fontSize the size to be applied. 0 denotes no override.
     */
    void setFont(QString fontFamily, int fontSize, double lineHeight);

    /**
     * @brief Toggles line numbers on/off in the editor
     * @param visible when true, the line numbers will be visible,
     * when false the line numbers will be hidden.
     */
    void setLineNumbersVisible(bool visible);

    QTextCodec* codec() const;

    /**
     * @brief Set the codec for this Editor.
     *        This method does not change the in-memory or on-screen
     *        representation of the document (which is always Unicode).
     *        It serves solely as a way to keep track of the encoding
     *        that needs to be used when the document gets saved.
     * @param codec
     */
    void setCodec(QTextCodec* codec);

    bool bom() const;
    void setBom(bool bom);

    QList<Theme> themes();
    void setTheme(Theme theme);
    static Editor::Theme themeFromName(QString name);

    QList<Selection> selections();

    /**
     * @brief Returns the currently selected texts.
     * @return
     */
    Q_INVOKABLE QtPromise::QPromise<QStringList> selectedTexts();

    void setOverwrite(bool overwrite);
    void setTabsVisible(bool visible);

    /**
     * @brief Detect the indentation mode used within the current document.
     * @return a pair whose first element is the document indentation, that is
     *         significative only if the second element ("found") is true.
     */
    QtPromise::QPromise<std::pair<IndentationMode, bool>> detectDocumentIndentation();
    IndentationMode indentationMode();
    QtPromise::QPromise<IndentationMode> indentationModeP();

    QtPromise::QPromise<QString> getCurrentWord();

    void setSelection(int fromLine, int fromCol, int toLine, int toCol);

    QtPromise::QPromise<int> lineCount();

private:
    friend class ::EditorTabWidget;
#ifdef NQQ_CPP_CORRECTNESS_TESTS
    friend class ::EditorCorrectnessAccess;
#endif

    /** Identifies a parsed reply that successfully settled a tracked request. */
    struct ResolvedAsyncReply {
        unsigned int id;
        QString message;
    };

    /** Stores a one-way bridge command until the JavaScript editor is ready to receive it. */
    struct QueuedOneWayMessage {
        QString message;
        QVariant payload;
    };

    /** Emits one already-formatted one-way bridge command to the JavaScript editor. */
    using EmitOneWayMessage = std::function<void(const QString&, const QVariant&)>;

    /**
     * Sends a one-way command immediately when the editor is ready, otherwise retains it in FIFO order.
     * This keeps callers responsive during WebEngine startup without losing commands sent before J_EVT_READY.
     */
    static void sendOrQueueOneWayMessage(bool ready,
        std::deque<QueuedOneWayMessage>& pendingMessages,
        const QString& message,
        const QVariant& payload,
        const EmitOneWayMessage& emitMessage)
    {
        if (ready) {
            emitMessage(message, payload);
            return;
        }

        pendingMessages.push_back({message, payload});
    }

    /** Drains queued one-way commands in FIFO order after the JavaScript editor reports readiness. */
    static void flushQueuedOneWayMessages(
        std::deque<QueuedOneWayMessage>& pendingMessages, const EmitOneWayMessage& emitMessage)
    {
        while (!pendingMessages.empty()) {
            QueuedOneWayMessage message = std::move(pendingMessages.front());
            pendingMessages.pop_front();
            emitMessage(message.message, message.payload);
        }
    }

    /** Notifies request/reply listeners before draining one-way commands queued during editor startup. */
    static void notifyReadyAndFlushOneWayMessages(std::deque<QueuedOneWayMessage>& pendingMessages,
        const std::function<void()>& notifyReady,
        const EmitOneWayMessage& emitMessage)
    {
        notifyReady();
        flushQueuedOneWayMessages(pendingMessages, emitMessage);
    }

    /** Registers the QtPromise with the tracker before the caller emits its bridge request. */
    static QtPromise::QPromise<QVariant> registerAsyncPromise(
        AsyncRequestTracker& tracker, unsigned int id, const QString& message)
    {
        return QtPromise::QPromise<QVariant>(
            [&tracker, id, message](const QtPromise::QPromiseResolve<QVariant>& resolve,
                const QtPromise::QPromiseReject<QVariant>& reject) {
                tracker.trackPromise(id, message, resolve, reject);
            });
    }

    /** Callable that emits one fully formatted bridge request. */
    using EmitAsyncRequest = std::function<void()>;

    /** Callable that connects a bridge request to editor readiness and returns that connection. */
    using DeferAsyncRequest = std::function<QMetaObject::Connection(EmitAsyncRequest)>;

    /** Registers before emitting, and cancels deferred emission if the promise times out first. */
    static QtPromise::QPromise<QVariant> registerPromiseAndSend(AsyncRequestTracker& tracker,
        unsigned int id,
        const QString& message,
        bool ready,
        EmitAsyncRequest emitRequest,
        DeferAsyncRequest deferRequest)
    {
        auto promise = registerAsyncPromise(tracker, id, message);
        if (ready) {
            emitRequest();
            return promise;
        }

        auto connection = std::make_shared<QMetaObject::Connection>();
        *connection = deferRequest([connection, &tracker, id, emitRequest = std::move(emitRequest)]() mutable {
            QObject::disconnect(*connection);
            if (tracker.isPending(id))
                emitRequest();
        });
        promise.fail([connection](const std::runtime_error&) {
            QObject::disconnect(*connection);
            return QVariant();
        });
        return promise;
    }

    /** Tracks a legacy request, processes events until settlement, and cancels any stale deferred send. */
    static std::shared_future<QVariant> trackLegacyAndWait(AsyncRequestTracker& tracker,
        unsigned int id,
        const QString& message,
        std::function<void(QVariant)> callback,
        bool ready,
        EmitAsyncRequest emitRequest,
        DeferAsyncRequest deferRequest)
    {
        auto resultPromise = std::make_shared<std::promise<QVariant>>();
        std::shared_future<QVariant> future = resultPromise->get_future().share();
        tracker.trackLegacy(id, message, resultPromise, std::move(callback));

        std::shared_ptr<QMetaObject::Connection> connection;
        if (ready) {
            emitRequest();
        } else {
            connection = std::make_shared<QMetaObject::Connection>();
            *connection = deferRequest([connection, &tracker, id, emitRequest = std::move(emitRequest)]() mutable {
                QObject::disconnect(*connection);
                if (tracker.isPending(id))
                    emitRequest();
            });
        }

        while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        }
        if (connection)
            QObject::disconnect(*connection);
        return future;
    }

    /** Parses a wire reply and settles it, ignoring malformed, unknown, or late IDs. */
    static std::optional<ResolvedAsyncReply> resolveAsyncReply(
        AsyncRequestTracker& tracker, const QString& wireMessage, const QVariant& data)
    {
        const QString prefix = QStringLiteral("[ASYNC_REPLY]");
        const QString idPrefix = QStringLiteral("[ID=");
        if (!wireMessage.startsWith(prefix) || !wireMessage.endsWith(QLatin1Char(']')))
            return std::nullopt;

        const qsizetype idStart = wireMessage.lastIndexOf(idPrefix);
        if (idStart < prefix.size())
            return std::nullopt;

        bool validId = false;
        const unsigned int id =
            wireMessage.mid(idStart + idPrefix.size(), wireMessage.size() - idStart - idPrefix.size() - 1)
                .toUInt(&validId);
        if (!validId || !tracker.resolve(id, data))
            return std::nullopt;

        return ResolvedAsyncReply{id, wireMessage.mid(prefix.size(), idStart - prefix.size())};
    }

    // These functions should only be used by EditorTabWidget to manage the tab's title. This works around
    // KDE's habit to automatically modify QTabWidget's tab titles to insert shortcut sequences (like &1).
    QString tabName() const;
    void setTabName(const QString& name);

    static std::deque<std::unique_ptr<Editor>> m_editorBuffer;
    std::deque<QueuedOneWayMessage> m_pendingOneWayMessages;
    QVBoxLayout* m_layout;
    CustomQWebView* m_webView;
    JsToCppProxy* m_jsToCppProxy;
    /** Child-owned tracker that bounds every promise- and future-based bridge request. */
    AsyncRequestTracker* m_asyncRequestTracker = nullptr;
    QUrl m_filePath = QUrl();
    QString m_tabName;
    bool m_fileOnDiskChanged = false;
    bool m_loaded = false;
    bool m_deferringOneWayMessages = true;
    QString m_endOfLineSequence = "\n";
    QTextCodec* m_codec = QTextCodec::codecForName("UTF-8");
    bool m_bom = false;
    bool m_customIndentationMode = false;
    const Language* m_currentLanguage = nullptr;

    static bool useMonaco();

    void fullConstructor(const Theme& theme);

    QtPromise::QPromise<void> setIndentationMode(const bool useTabs, const int size);
    QtPromise::QPromise<void> setIndentationMode(const Language*);

private slots:
    void on_proxyMessageReceived(QString msg, QVariant data);

signals:
    void messageReceived(QString msg, QVariant data);
    void asyncReplyReceived(unsigned int id, QString msg, QVariant data);
    void gotFocus();
    void mouseWheel(QWheelEvent* ev);
    void urlsDropped(QList<QUrl> urls);
    void bannerRemoved(QWidget* banner);

    // Pre-interpreted messages:
    void contentChanged();
    void cursorActivity(QMap<QString, QVariant> data);
    void documentInfoRequested(QMap<QString, QVariant> data);
    void cleanChanged(bool isClean);
    void fileNameChanged(const QUrl& oldFileName, const QUrl& newFileName);

    /**
     * @brief The editor finished loading. There should be
     *        no need to use this signal outside this class.
     */
    void editorReady();

    void currentLanguageChanged(QString id, QString name);

public slots:

    // [[deprecated]]
    void sendMessage(const QString msg, const QVariant data);
    // [[deprecated]]
    void sendMessage(const QString msg);

    QtPromise::QPromise<QVariant> asyncSendMessageWithResultP(const QString msg, const QVariant data);
    QtPromise::QPromise<QVariant> asyncSendMessageWithResultP(const QString msg);

    /**
     * @brief asyncSendMessageWithResult
     * @param msg
     * @param data
     * @param callback When set, the result is returned asynchronously via the provided function.
     *                 If set, you should NOT use the return value of this method.
     * @return
     */
    // [[deprecated]]
    std::shared_future<QVariant> asyncSendMessageWithResult(
        const QString msg, const QVariant data, std::function<void(QVariant)> callback = nullptr);
    // [[deprecated]]
    std::shared_future<QVariant> asyncSendMessageWithResult(
        const QString msg, std::function<void(QVariant)> callback = nullptr);

    /**
     * @brief Print the editor. As of Qt 5.11, it produces low-quality, non-vector graphics with big dimension.
     * @param printer
     */
    void print(std::shared_ptr<QPrinter> printer);

    /**
     * @brief Returns the content of the editor layed out in a pdf file that can be directly saved to disk.
     *        This method produces light, vector graphics.
     * @param pageLayout
     * @return
     */
    QtPromise::QPromise<QByteArray> printToPdf(
        const QPageLayout& pageLayout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF()));
};

} // namespace EditorNS

#endif // EDITOR_H
