#include "include/EditorNS/asyncrequesttracker.h"
#include "include/EditorNS/defer.h"
#include "include/EditorNS/editor.h"
#include "include/Extensions/extension.h"
#include "include/Search/filesearcher.h"
#include "include/Search/searchhelpers.h"
#include "include/commandlineopenruntime.h"
#include "include/docengine.h"
#include "include/documentcontroller.h"
#include "include/editoruicontroller.h"
#include "include/localsockethelpers.h"
#include "include/notepadqq.h"
#include "include/statsruntime.h"

#include <QElapsedTimer>
#include <QFile>
#include <QIODevice>
#include <QLocalServer>
#include <QLocalSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QProcess>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

#include <chrono>
#include <future>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <optional>

using namespace std::chrono_literals;

// Supplies a deterministic invalid runtime so Extension can be tested without launching Node.js.
QString Notepadqq::nodejsPath()
{ return QStringLiteral("/definitely-not-a-node-runtime"); }

// Emits controllable editor-readiness events for deferred-send seam tests.
class EditorReadyEmitter : public QObject {
    Q_OBJECT
public:
    // Delivers the synthetic readiness event to any still-connected request.
    void becomeReady() { emit ready(); }

signals:
    // Notifies deferred requests that the synthetic editor is ready to receive messages.
    void ready();
};

// Test-only friend that exercises the private Editor/tracker seam without expanding Editor's public API.
class EditorCorrectnessAccess {
public:
    struct OneWayDispatchResult {
        int queuedMessageCount = 0;
        int queuedAfterReadyNotification = 0;
        int queuedAfterFlush = 0;
        QStringList deliveredMessages;
        QVariantList deliveredPayloads;
    };

    // Uses the same registration helper as Editor's QtPromise overload.
    static QtPromise::QPromise<QVariant> registerPromise(
        EditorNS::AsyncRequestTracker& tracker, unsigned int id, const QString& message)
    { return EditorNS::Editor::registerAsyncPromise(tracker, id, message); }

    // Uses the complete registration/defer/emission seam shared with Editor's QtPromise overload.
    static QtPromise::QPromise<QVariant> sendPromise(EditorNS::AsyncRequestTracker& tracker,
        unsigned int id,
        const QString& message,
        bool ready,
        std::function<void()> emitRequest,
        std::function<QMetaObject::Connection(std::function<void()>)> deferRequest)
    {
        return EditorNS::Editor::registerPromiseAndSend(
            tracker, id, message, ready, std::move(emitRequest), std::move(deferRequest));
    }

    // Uses the complete registration/defer/wait seam shared with Editor's legacy future overload.
    static std::shared_future<QVariant> sendLegacy(EditorNS::AsyncRequestTracker& tracker,
        unsigned int id,
        const QString& message,
        bool ready,
        std::function<void()> emitRequest,
        std::function<QMetaObject::Connection(std::function<void()>)> deferRequest)
    {
        return EditorNS::Editor::trackLegacyAndWait(
            tracker, id, message, nullptr, ready, std::move(emitRequest), std::move(deferRequest));
    }

    // Routes a synthetic wire reply through the same parser and tracker resolution helper as Editor.
    static bool receiveReply(EditorNS::AsyncRequestTracker& tracker, const QString& wireMessage, const QVariant& data)
    { return EditorNS::Editor::resolveAsyncReply(tracker, wireMessage, data).has_value(); }

    // Exercises the one-way bridge queue without constructing a QWebEngine-backed Editor.
    static OneWayDispatchResult queueOneWayMessagesUntilReadyTransitionCompletes()
    {
        std::deque<EditorNS::Editor::QueuedOneWayMessage> pending;
        OneWayDispatchResult result;
        bool deferOneWayMessages = true;
        const auto deliver = [&result](const QString& message, const QVariant& payload) {
            result.deliveredMessages.append(message);
            result.deliveredPayloads.append(payload);
        };

        EditorNS::Editor::sendOrQueueOneWayMessage(false, pending, QStringLiteral("C_CMD_BEFORE_READY"), 1, deliver);
        result.queuedMessageCount = static_cast<int>(pending.size());

        EditorNS::Editor::notifyReadyAndFlushOneWayMessages(
            pending,
            [&] {
                EditorNS::Editor::sendOrQueueOneWayMessage(
                    !deferOneWayMessages, pending, QStringLiteral("C_CMD_DURING_READY"), 2, deliver);
                result.queuedAfterReadyNotification = static_cast<int>(pending.size());
            },
            deliver);
        result.queuedAfterFlush = static_cast<int>(pending.size());

        deferOneWayMessages = false;
        EditorNS::Editor::sendOrQueueOneWayMessage(
            !deferOneWayMessages, pending, QStringLiteral("C_CMD_AFTER_READY"), 3, deliver);
        return result;
    }
};

// In-process reply used to deterministically exercise the telemetry cleanup helper.
class FakeReply : public QNetworkReply {
    Q_OBJECT
public:
    explicit FakeReply(QObject* parent, bool finishImmediately)
        : QNetworkReply(parent)
        , m_finishImmediately(finishImmediately)
    {
        open(ReadOnly | Unbuffered);
        setUrl(QUrl(QStringLiteral("http://127.0.0.1/fake")));
        if (m_finishImmediately) {
            QTimer::singleShot(0, this, [this] {
                setFinished(true);
                emit finished();
            });
        }
    }

    void abort() override
    {
        if (isFinished()) {
            return;
        }
        setError(OperationCanceledError, QStringLiteral("aborted"));
        setFinished(true);
        emit finished();
    }

protected:
    qint64 readData(char*, qint64) override { return -1; }

private:
    bool m_finishImmediately = false;
};

// In-process manager that returns either a stalled or an immediately finishing reply.
class FakeManager : public QNetworkAccessManager {
    Q_OBJECT
public:
    explicit FakeManager(bool finishImmediately, QObject* parent = nullptr)
        : QNetworkAccessManager(parent)
        , m_finishImmediately(finishImmediately)
    {
    }

protected:
    QNetworkReply* createRequest(Operation, const QNetworkRequest&, QIODevice*) override
    { return new FakeReply(this, m_finishImmediately); }

private:
    bool m_finishImmediately = false;
};

DocEngine::DecodedText DocEngine::readToString(QFile* file)
{
    DecodedText decoded;
    if (!file->open(QIODevice::ReadOnly)) {
        decoded.error = true;
        return decoded;
    }

    decoded.text = QString::fromUtf8(file->readAll());
    return decoded;
}

class CppCorrectnessTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void asyncRequestTrackerResolvesMatchingReplyOnce();
    void asyncRequestTrackerRejectsMissingReplyAfterTimeout();
    void asyncRequestTrackerRejectsPendingPromisesOnDestruction();
    void asyncRequestTrackerSetsLegacyExceptionOnTimeout();
    void asyncRequestTrackerCallsLegacyCallbackOnlyOnSuccess();
    void editorRegistersAsyncRequestBeforeEmission();
    void editorSyntheticReplyClearsRegisteredRequest();
    void editorTrackerDestructionLeavesNoLiveTimeoutTimers();
    void editorDoesNotEmitTimedOutPromiseWhenReadyLate();
    void editorLegacyWaitExitsOnTimeoutBeforeReady();
    void editorQueuesOneWayMessagesUntilReadyTransitionCompletes();
    void deferredCallbackIsDiscardedWhenContextIsDestroyed();
    void cancellationRequestsInterruptionAndStopsFilesystemSearch();
    void searchPlainText_handlesContentEndingInLf();
    void statsTransmissionDueUsesSeconds();
    void statsPostDeletesFinishedReplies();
    void statsPostAbortsStalledReplies();
    void localSocketProbeDoesNotLeakFailedConnection();
    void localSocketDeletesAcceptedPeerAfterDisconnect();
    void extensionOwnsRuntimeProcessForImmediateDestruction();
    void commandLineOpenContinuesOnlyAfterLoadingCompletes();
    void commandLineOpenDiscardsContinuationWhenOwnerIsDestroyed();
    void documentControllerHasExpectedConstructionContract();
    void editorUiControllerHasExpectedConstructionContract();
    void linePositions_data();
    void linePositions();
};

// Guards the extracted controller's QObject, collaborator-constructor, and non-copyable API shape.
void CppCorrectnessTest::documentControllerHasExpectedConstructionContract()
{
    QVERIFY((std::is_base_of_v<QObject, DocumentController>));
    QVERIFY((std::is_constructible_v<DocumentController, MainWindow&, DocEngine&, TopEditorContainer&, NqqSettings&>));
    QVERIFY((!std::is_copy_constructible_v<DocumentController>));
}

// Guards the editor UI controller's QObject, collaborator-constructor, and non-copyable API shape.
void CppCorrectnessTest::editorUiControllerHasExpectedConstructionContract()
{
    QVERIFY((std::is_base_of_v<QObject, EditorUiController>));
    QVERIFY((std::is_constructible_v<EditorUiController,
        MainWindow&,
        Ui::MainWindow&,
        DocEngine&,
        TopEditorContainer&,
        NqqSettings&>));
    QVERIFY((!std::is_copy_constructible_v<EditorUiController>));
}

// Guards the bridge dispatch against invoking an Editor callback after the Editor is destroyed.
void CppCorrectnessTest::deferredCallbackIsDiscardedWhenContextIsDestroyed()
{
    bool called = false;
    auto* context = new QObject;

    EditorNS::deferToObject(context, [&] { called = true; });
    delete context;

    QCoreApplication::processEvents();
    QVERIFY(!called);
}

// Ensures a failed single-instance probe does not leave a socket child behind.
void CppCorrectnessTest::localSocketProbeDoesNotLeakFailedConnection()
{
    QObject owner;
    const int before = owner.findChildren<QLocalSocket*>().size();
    QVERIFY(LocalSocketHelpers::probe(
                &owner, QStringLiteral("nqq-nonexistent-socket"), [](QLocalSocket*) { return false; }) == nullptr);
    QCOMPARE(owner.findChildren<QLocalSocket*>().size(), before);
}

// Ensures accepted server sockets are released automatically when their peer disconnects.
void CppCorrectnessTest::localSocketDeletesAcceptedPeerAfterDisconnect()
{
    auto* accepted = new QLocalSocket;
    QPointer<QLocalSocket> observed = accepted;
    LocalSocketHelpers::deleteOnDisconnect(accepted);
    QVERIFY(QMetaObject::invokeMethod(accepted, "disconnected", Qt::DirectConnection));
    QTRY_VERIFY(observed.isNull());
}

// Ensures an extension owns its runtime process even when shutdown stops the event loop.
void CppCorrectnessTest::extensionOwnsRuntimeProcessForImmediateDestruction()
{
    QTemporaryDir extensionDir;
    QVERIFY(extensionDir.isValid());
    QFile manifest(extensionDir.filePath(QStringLiteral("nqq-manifest.json")));
    QVERIFY(manifest.open(QIODevice::WriteOnly));
    manifest.write(R"({"name":"test extension","runtime":"nodejs","main":"index.js"})");
    manifest.close();

    auto* extension = new Extensions::Extension(extensionDir.path(), QStringLiteral("test-socket"));
    const QList<QProcess*> processes = extension->findChildren<QProcess*>();
    QCOMPARE(processes.size(), 1);
    QPointer<QProcess> process = processes.constFirst();

    delete extension;
    QVERIFY(process.isNull());
}

// Ensures command-line follow-up work is deferred until document loading has completed.
void CppCorrectnessTest::commandLineOpenContinuesOnlyAfterLoadingCompletes()
{
    std::optional<QtPromise::QPromiseResolve<void>> resolve;
    auto loading =
        QtPromise::QPromise<void>([&resolve](const QtPromise::QPromiseResolve<void>& resolver) { resolve = resolver; });
    QObject owner;
    bool continued = false;

    CommandLineOpenRuntime::continueAfterLoading(loading, &owner, [&] { continued = true; });
    QVERIFY(!continued);

    (*resolve)();
    QTRY_VERIFY(continued);
}

// Ensures a closing window discards command-line follow-up work before document loading completes.
void CppCorrectnessTest::commandLineOpenDiscardsContinuationWhenOwnerIsDestroyed()
{
    std::optional<QtPromise::QPromiseResolve<void>> resolve;
    auto loading =
        QtPromise::QPromise<void>([&resolve](const QtPromise::QPromiseResolve<void>& resolver) { resolve = resolver; });
    auto* owner = new QObject;
    bool continued = false;

    CommandLineOpenRuntime::continueAfterLoading(loading, owner, [&] { continued = true; });
    delete owner;
    (*resolve)();

    QCoreApplication::processEvents();
    QVERIFY(!continued);
}

// Guards against emitting a bridge request before its completion is registered.
void CppCorrectnessTest::editorRegistersAsyncRequestBeforeEmission()
{
    EditorNS::AsyncRequestTracker tracker(this, 100ms);
    bool emitted = false;
    auto promise = EditorCorrectnessAccess::sendPromise(tracker,
        23,
        QStringLiteral("C_FUN_ORDER"),
        true,
        [&] {
            QCOMPARE(tracker.pendingCount(), 1);
            emitted = true;
        },
        {});
    Q_UNUSED(promise);

    QVERIFY(emitted);
    QVERIFY(tracker.resolve(23, QVariant()));
}

// Guards the Editor reply parser-to-tracker resolution seam.
void CppCorrectnessTest::editorSyntheticReplyClearsRegisteredRequest()
{
    EditorNS::AsyncRequestTracker tracker(this, 100ms);
    QVariant resolvedValue;
    auto promise = EditorCorrectnessAccess::registerPromise(tracker, 24, QStringLiteral("C_FUN_REPLY"));
    promise.then([&](const QVariant& value) { resolvedValue = value; });

    QVERIFY(EditorCorrectnessAccess::receiveReply(
        tracker, QStringLiteral("[ASYNC_REPLY]C_FUN_REPLY[ID=24]"), QStringLiteral("synthetic")));

    QCOMPARE(tracker.pendingCount(), 0);
    QTRY_COMPARE(resolvedValue, QVariant(QStringLiteral("synthetic")));
}

// Guards tracker timer ownership when the Editor-equivalent parent is destroyed.
void CppCorrectnessTest::editorTrackerDestructionLeavesNoLiveTimeoutTimers()
{
    auto* owner = new QObject;
    auto* tracker = new EditorNS::AsyncRequestTracker(owner, 100ms);
    auto promise = EditorCorrectnessAccess::registerPromise(*tracker, 25, QStringLiteral("C_FUN_DESTROY_TIMER"));
    promise.fail([](const std::runtime_error&) { return QVariant(); });

    const QList<QTimer*> timers = tracker->findChildren<QTimer*>();
    QCOMPARE(timers.size(), 1);
    QPointer<QTimer> timer = timers.constFirst();

    delete owner;

    QVERIFY(timer.isNull());
}

// Guards against executing a deferred command after its promise has timed out.
void CppCorrectnessTest::editorDoesNotEmitTimedOutPromiseWhenReadyLate()
{
    EditorNS::AsyncRequestTracker tracker(this, 20ms);
    EditorReadyEmitter readyEmitter;
    bool emitted = false;

    auto promise = EditorCorrectnessAccess::sendPromise(
        tracker,
        26,
        QStringLiteral("C_CMD_LATE_PROMISE"),
        false,
        [&] { emitted = true; },
        [&](std::function<void()> request) {
            return connect(&readyEmitter, &EditorReadyEmitter::ready, this, std::move(request));
        });
    promise.fail([](const std::runtime_error&) { return QVariant(); });

    QTRY_COMPARE_WITH_TIMEOUT(tracker.pendingCount(), 0, 250);
    readyEmitter.becomeReady();
    QCoreApplication::processEvents();
    QVERIFY(!emitted);
}

// Guards the legacy compatibility wait when the editor never becomes ready.
void CppCorrectnessTest::editorLegacyWaitExitsOnTimeoutBeforeReady()
{
    EditorNS::AsyncRequestTracker tracker(this, 20ms);
    EditorReadyEmitter readyEmitter;
    bool emitted = false;
    QTimer::singleShot(200, &readyEmitter, &EditorReadyEmitter::becomeReady);
    QElapsedTimer elapsed;
    elapsed.start();

    auto future = EditorCorrectnessAccess::sendLegacy(
        tracker,
        27,
        QStringLiteral("C_FUN_LATE_LEGACY"),
        false,
        [&] { emitted = true; },
        [&](std::function<void()> request) {
            return connect(&readyEmitter, &EditorReadyEmitter::ready, this, std::move(request));
        });

    QVERIFY(elapsed.elapsed() < 150);
    QCOMPARE(tracker.pendingCount(), 0);
    QVERIFY(future.wait_for(0ms) == std::future_status::ready);
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, future.get());
    readyEmitter.becomeReady();
    QCoreApplication::processEvents();
    QVERIFY(!emitted);
}

// Guards legacy one-way commands against blocking on WebEngine startup while retaining their payload.
void CppCorrectnessTest::editorQueuesOneWayMessagesUntilReadyTransitionCompletes()
{
    const auto result = EditorCorrectnessAccess::queueOneWayMessagesUntilReadyTransitionCompletes();

    QCOMPARE(result.queuedMessageCount, 1);
    QCOMPARE(result.queuedAfterReadyNotification, 2);
    QCOMPARE(result.queuedAfterFlush, 0);
    const QStringList expectedMessages{QStringLiteral("C_CMD_BEFORE_READY"),
        QStringLiteral("C_CMD_DURING_READY"),
        QStringLiteral("C_CMD_AFTER_READY")};
    const QVariantList expectedPayloads{QVariant(1), QVariant(2), QVariant(3)};
    QCOMPARE(result.deliveredMessages, expectedMessages);
    QCOMPARE(result.deliveredPayloads, expectedPayloads);
}

// Guards single settlement and late-duplicate rejection.
void CppCorrectnessTest::asyncRequestTrackerResolvesMatchingReplyOnce()
{
    EditorNS::AsyncRequestTracker tracker(this, 100ms);
    int resolutionCount = 0;
    QVariant resolvedValue;

    auto promise = QtPromise::QPromise<QVariant>(
        [&](const QtPromise::QPromiseResolve<QVariant>& resolve, const QtPromise::QPromiseReject<QVariant>& reject) {
            tracker.trackPromise(17, QStringLiteral("C_FUN_TEST"), resolve, reject);
        });
    promise.then([&](const QVariant& value) {
        ++resolutionCount;
        resolvedValue = value;
    });

    QVERIFY(tracker.resolve(17, QStringLiteral("reply")));
    QTRY_COMPARE(resolutionCount, 1);
    QCOMPARE(resolvedValue, QVariant(QStringLiteral("reply")));
    QCOMPARE(tracker.pendingCount(), 0);
    QVERIFY(!tracker.resolve(17, QStringLiteral("duplicate")));
    QCoreApplication::processEvents();
    QCOMPARE(resolutionCount, 1);
}

// Guards bounded QtPromise rejection and record removal when no reply arrives.
void CppCorrectnessTest::asyncRequestTrackerRejectsMissingReplyAfterTimeout()
{
    EditorNS::AsyncRequestTracker tracker(this, 20ms);
    bool rejected = false;

    auto promise = QtPromise::QPromise<QVariant>(
        [&](const QtPromise::QPromiseResolve<QVariant>& resolve, const QtPromise::QPromiseReject<QVariant>& reject) {
            tracker.trackPromise(18, QStringLiteral("C_FUN_TIMEOUT"), resolve, reject);
        });
    promise.fail([&](const std::runtime_error&) {
        rejected = true;
        return QVariant();
    });

    QTRY_COMPARE_WITH_TIMEOUT(tracker.pendingCount(), 0, 250);
    QTRY_VERIFY_WITH_TIMEOUT(rejected, 250);
}

// Guards teardown rejection of pending QtPromise requests.
void CppCorrectnessTest::asyncRequestTrackerRejectsPendingPromisesOnDestruction()
{
    bool rejected = false;
    auto* tracker = new EditorNS::AsyncRequestTracker(this, 100ms);

    auto promise = QtPromise::QPromise<QVariant>(
        [&](const QtPromise::QPromiseResolve<QVariant>& resolve, const QtPromise::QPromiseReject<QVariant>& reject) {
            tracker->trackPromise(19, QStringLiteral("C_FUN_DESTROY"), resolve, reject);
        });
    promise.fail([&](const std::runtime_error&) {
        rejected = true;
        return QVariant();
    });

    QCOMPARE(tracker->pendingCount(), 1);
    delete tracker;
    QTRY_VERIFY_WITH_TIMEOUT(rejected, 250);
}

// Guards timeout propagation through the compatibility std::future API.
void CppCorrectnessTest::asyncRequestTrackerSetsLegacyExceptionOnTimeout()
{
    EditorNS::AsyncRequestTracker tracker(this, 20ms);
    auto result = std::make_shared<std::promise<QVariant>>();
    auto future = result->get_future();

    tracker.trackLegacy(20, QStringLiteral("C_FUN_LEGACY_TIMEOUT"), result, nullptr);

    QTRY_COMPARE_WITH_TIMEOUT(tracker.pendingCount(), 0, 250);
    QVERIFY(future.wait_for(0ms) == std::future_status::ready);
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, future.get());
}

// Guards the legacy callback against timeout invocation while preserving successful delivery.
void CppCorrectnessTest::asyncRequestTrackerCallsLegacyCallbackOnlyOnSuccess()
{
    EditorNS::AsyncRequestTracker tracker(this, 20ms);
    int callbackCount = 0;
    QVariant callbackValue;

    auto timedOutResult = std::make_shared<std::promise<QVariant>>();
    tracker.trackLegacy(
        21, QStringLiteral("C_FUN_CALLBACK_TIMEOUT"), timedOutResult, [&](const QVariant&) { ++callbackCount; });
    QTRY_COMPARE_WITH_TIMEOUT(tracker.pendingCount(), 0, 250);
    QCOMPARE(callbackCount, 0);

    auto successfulResult = std::make_shared<std::promise<QVariant>>();
    tracker.trackLegacy(22, QStringLiteral("C_FUN_CALLBACK_SUCCESS"), successfulResult, [&](const QVariant& value) {
        ++callbackCount;
        callbackValue = value;
    });
    QVERIFY(tracker.resolve(22, 42));

    QTRY_COMPARE(callbackCount, 1);
    QCOMPARE(callbackValue, QVariant(42));
    QCOMPARE(tracker.pendingCount(), 0);
}

void CppCorrectnessTest::cancellationRequestsInterruptionAndStopsFilesystemSearch()
{
    SearchConfig config;
    config.searchScope = SearchConfig::ScopeFileSystem;

    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QByteArray fileContents = QByteArrayLiteral("searchable text\n") + QByteArray(64 * 1024, 'x');
    for (int fileIndex = 0; fileIndex < 200; ++fileIndex) {
        QFile file(temporaryDirectory.filePath(QStringLiteral("file-%1.txt").arg(fileIndex)));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(fileContents), qint64(fileContents.size()));
    }

    config.directory = temporaryDirectory.path();
    config.filePattern = QStringLiteral("*.txt");
    config.searchString = QStringLiteral("searchable");

    FileSearcher* searcher = FileSearcher::prepareAsyncSearch(config);
    searcher->start();
    QTRY_VERIFY_WITH_TIMEOUT(searcher->isRunning(), 1000);
    searcher->cancel();
    QVERIFY(searcher->isInterruptionRequested());
    QVERIFY(searcher->wait(2000));
    QVERIFY(searcher->getResult().results.size() < 200);
    delete searcher;
}

void CppCorrectnessTest::searchPlainText_handlesContentEndingInLf()
{
    SearchConfig config;
    config.searchString = "needle";

    const DocResult result = FileSearcher::searchPlainText(config, "needle\n");

    QCOMPARE(result.results.size(), 1);
    QCOMPARE(result.results.constFirst().lineNumber, 1);
}

// Guards the weekly statistics interval against the old milliseconds-vs-seconds mismatch.
void CppCorrectnessTest::statsTransmissionDueUsesSeconds()
{
    QVERIFY(!StatsRuntime::isTransmissionDue(100, 100 + (7 * 24 * 60 * 60) - 1));
    QVERIFY(StatsRuntime::isTransmissionDue(100, 100 + (7 * 24 * 60 * 60)));
}

// Guards the telemetry helper against leaking a normal reply after completion.
void CppCorrectnessTest::statsPostDeletesFinishedReplies()
{
    FakeManager manager(true);
    QNetworkRequest request(QUrl(QStringLiteral("http://127.0.0.1/fake")));
    QPointer<QNetworkReply> reply = StatsRuntime::post(&manager, request, QByteArrayLiteral("{}"), 200ms);
    QVERIFY(reply);
    QTRY_VERIFY_WITH_TIMEOUT(reply.isNull(), 2000);
}

// Guards the telemetry helper against leaving a stalled reply alive indefinitely.
void CppCorrectnessTest::statsPostAbortsStalledReplies()
{
    FakeManager manager(false);
    QNetworkRequest request(QUrl(QStringLiteral("http://127.0.0.1/fake")));
    QPointer<QNetworkReply> reply = StatsRuntime::post(&manager, request, QByteArrayLiteral("{}"), 20ms);
    QVERIFY(reply);
    QTRY_VERIFY_WITH_TIMEOUT(reply.isNull(), 2000);
}

void CppCorrectnessTest::linePositions_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<std::vector<int>>("expected");

    QTest::newRow("empty") << "" << std::vector<int>({0, 0});
    QTest::newRow("no-line-break") << "abc" << std::vector<int>({0, 3});
    QTest::newRow("terminal-lf") << "abc\n" << std::vector<int>({0, 4, 4});
    QTest::newRow("terminal-cr") << "abc\r" << std::vector<int>({0, 4, 4});
    QTest::newRow("crlf") << "a\r\nb" << std::vector<int>({0, 3, 4});
}

void CppCorrectnessTest::linePositions()
{
    QFETCH(QString, data);
    QFETCH(std::vector<int>, expected);

    QCOMPARE(SearchHelpers::linePositions(data), expected);
}

QTEST_GUILESS_MAIN(CppCorrectnessTest)

#include "tst_cppcorrectness.moc"
