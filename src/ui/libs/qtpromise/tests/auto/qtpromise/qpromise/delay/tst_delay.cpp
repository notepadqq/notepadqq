#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_delay : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfilled();
    void rejected();
};

QTEST_MAIN(tst_qpromise_delay)
#include "tst_delay.moc"

void tst_qpromise_delay::fulfilled()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QPromise<int>::resolve(42).delay(1000).finally([&]() {
        elapsed = timer.elapsed();
    });

    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(p.isFulfilled(), true);

    // Qt::CoarseTimer (default) Coarse timers try to
    // keep accuracy within 5% of the desired interval.
    // Require accuracy within 6% for passing the test.
    QVERIFY(elapsed >= 1000 * 0.94);
    QVERIFY(elapsed <= 1000 * 1.06);
}

void tst_qpromise_delay::rejected()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QPromise<int>::reject(QString("foo")).delay(1000).finally([&]() {
        elapsed = timer.elapsed();
    });

    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
    QVERIFY(elapsed <= 10);
}
