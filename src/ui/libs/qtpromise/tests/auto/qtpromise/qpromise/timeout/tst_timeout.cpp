#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_timeout : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfilled();
    void rejected();
    void timeout();
};

QTEST_MAIN(tst_qpromise_timeout)
#include "tst_timeout.moc"

void tst_qpromise_timeout::fulfilled()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QPromise<int>([](const QPromiseResolve<int>& resolve) {
        QTimer::singleShot(1000, [=]() {
            resolve(42);
        });
    }).timeout(2000).finally([&]() {
        elapsed = timer.elapsed();
    });

    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(p.isFulfilled(), true);
    QVERIFY(elapsed < 2000);
}

void tst_qpromise_timeout::rejected()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QTimer::singleShot(1000, [=]() {
            reject(QString("foo"));
        });
    }).timeout(2000).finally([&]() {
        elapsed = timer.elapsed();
    });


    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
    QVERIFY(elapsed < 2000);
}

void tst_qpromise_timeout::timeout()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;
    bool failed = false;

    timer.start();

    auto p = QPromise<int>([](const QPromiseResolve<int>& resolve) {
        QTimer::singleShot(4000, [=]() {
            resolve(42);
        });
    }).timeout(2000).finally([&]() {
        elapsed = timer.elapsed();
    });

    p.fail([&](const QPromiseTimeoutException&) {
        failed = true;
        return -1;
    }).wait();

    QCOMPARE(waitForValue(p, -1), -1);
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(failed, true);

    // Qt::CoarseTimer (default) Coarse timers try to
    // keep accuracy within 5% of the desired interval.
    // Require accuracy within 6% for passing the test.
    QVERIFY(elapsed >= 2000 * 0.94);
    QVERIFY(elapsed <= 2000 * 1.06);
}
