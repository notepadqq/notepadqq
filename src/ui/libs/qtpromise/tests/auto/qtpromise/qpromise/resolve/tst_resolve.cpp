// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_resolve : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void value();
    void empty_void();
};

QTEST_MAIN(tst_qpromise_resolve)
#include "tst_resolve.moc"

void tst_qpromise_resolve::value()
{
    const int value = 42;
    auto p0 = QPromise<int>::resolve(value);
    auto p1 = QPromise<int>::resolve(43);

    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(waitForValue(p0, -1), 42);
    QCOMPARE(waitForValue(p1, -1), 43);
}

void tst_qpromise_resolve::empty_void()
{
    auto p = QPromise<void>::resolve();

    QCOMPARE(p.isFulfilled(), true);
}
