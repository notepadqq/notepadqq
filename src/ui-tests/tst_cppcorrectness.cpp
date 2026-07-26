#include "include/EditorNS/asyncrequesttracker.h"
#include "include/EditorNS/editor.h"
#include "include/Search/filesearcher.h"
#include "include/Search/searchhelpers.h"
#include "include/docengine.h"

#include <QFile>
#include <QPointer>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

#include <chrono>
#include <future>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace std::chrono_literals;

class EditorCorrectnessAccess {
public:
    static QtPromise::QPromise<QVariant> registerPromise(
        EditorNS::AsyncRequestTracker& tracker, unsigned int id, const QString& message)
    {
        return EditorNS::Editor::registerAsyncPromise(tracker, id, message);
    }

    static bool receiveReply(EditorNS::AsyncRequestTracker& tracker, const QString& wireMessage, const QVariant& data)
    { return EditorNS::Editor::resolveAsyncReply(tracker, wireMessage, data).has_value(); }
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
    void cancellationRequestsInterruptionAndStopsFilesystemSearch();
    void searchPlainText_handlesContentEndingInLf();
    void linePositions_data();
    void linePositions();
};

void CppCorrectnessTest::editorRegistersAsyncRequestBeforeEmission()
{
    EditorNS::AsyncRequestTracker tracker(this, 100ms);
    auto promise = EditorCorrectnessAccess::registerPromise(tracker, 23, QStringLiteral("C_FUN_ORDER"));
    Q_UNUSED(promise);

    bool emitted = false;
    auto emitRequest = [&] {
        QCOMPARE(tracker.pendingCount(), 1);
        emitted = true;
    };
    emitRequest();

    QVERIFY(emitted);
    QVERIFY(tracker.resolve(23, QVariant()));
}

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

void CppCorrectnessTest::asyncRequestTrackerResolvesMatchingReplyOnce()
{
    EditorNS::AsyncRequestTracker tracker(this, 100ms);
    int resolutionCount = 0;
    QVariant resolvedValue;

    auto promise = QtPromise::QPromise<QVariant>(
        [&](const QtPromise::QPromiseResolve<QVariant>& resolve,
            const QtPromise::QPromiseReject<QVariant>& reject) {
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

void CppCorrectnessTest::asyncRequestTrackerRejectsMissingReplyAfterTimeout()
{
    EditorNS::AsyncRequestTracker tracker(this, 20ms);
    bool rejected = false;

    auto promise = QtPromise::QPromise<QVariant>(
        [&](const QtPromise::QPromiseResolve<QVariant>& resolve,
            const QtPromise::QPromiseReject<QVariant>& reject) {
            tracker.trackPromise(18, QStringLiteral("C_FUN_TIMEOUT"), resolve, reject);
        });
    promise.fail([&](const std::runtime_error&) {
        rejected = true;
        return QVariant();
    });

    QTRY_COMPARE_WITH_TIMEOUT(tracker.pendingCount(), 0, 250);
    QTRY_VERIFY_WITH_TIMEOUT(rejected, 250);
}

void CppCorrectnessTest::asyncRequestTrackerRejectsPendingPromisesOnDestruction()
{
    bool rejected = false;
    auto* tracker = new EditorNS::AsyncRequestTracker(this, 100ms);

    auto promise = QtPromise::QPromise<QVariant>(
        [&](const QtPromise::QPromiseResolve<QVariant>& resolve,
            const QtPromise::QPromiseReject<QVariant>& reject) {
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

void CppCorrectnessTest::asyncRequestTrackerCallsLegacyCallbackOnlyOnSuccess()
{
    EditorNS::AsyncRequestTracker tracker(this, 20ms);
    int callbackCount = 0;
    QVariant callbackValue;

    auto timedOutResult = std::make_shared<std::promise<QVariant>>();
    tracker.trackLegacy(21, QStringLiteral("C_FUN_CALLBACK_TIMEOUT"), timedOutResult,
        [&](const QVariant&) { ++callbackCount; });
    QTRY_COMPARE_WITH_TIMEOUT(tracker.pendingCount(), 0, 250);
    QCOMPARE(callbackCount, 0);

    auto successfulResult = std::make_shared<std::promise<QVariant>>();
    tracker.trackLegacy(22, QStringLiteral("C_FUN_CALLBACK_SUCCESS"), successfulResult,
        [&](const QVariant& value) {
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
