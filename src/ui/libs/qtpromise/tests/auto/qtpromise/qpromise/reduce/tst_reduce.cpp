#include "../../shared/data.h"
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_reduce : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptySequence();
    void regularValues();
    void promiseValues();
    void convertResultType();
    void delayedInitialValue();
    void delayedFulfilled();
    void delayedRejected();
    void functorThrows();
    void sequenceTypes();
};

QTEST_MAIN(tst_qpromise_reduce)
#include "tst_reduce.moc"

namespace {

template <class Sequence>
struct SequenceTester
{
    static void exec()
    {
        Sequence inputs{
            QtPromise::resolve(4).delay(400),
            QtPromise::resolve(6).delay(300),
            QtPromise::resolve(8).delay(200)
        };
        QVector<int> v0;
        QVector<int> v1;

        auto p0 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
            v0 << acc << cur << idx;
            return acc + cur + idx;
        });
        auto p1 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
            v1 << acc << cur << idx;
            return acc + cur + idx;
        }, QtPromise::resolve(2).delay(100));

        Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));

        QCOMPARE(p0.isPending(), true);
        QCOMPARE(p1.isPending(), true);
        QCOMPARE(waitForValue(p0, -1), 21);
        QCOMPARE(waitForValue(p1, -1), 23);
        QCOMPARE(v0, QVector<int>({4, 6, 1, 11, 8, 2}));
        QCOMPARE(v1, QVector<int>({2, 4, 0, 6, 6, 1, 13, 8, 2}));
    }
};

} // anonymous namespace

void tst_qpromise_reduce::emptySequence()
{
    bool called = false;

    auto p = QtPromise::resolve(QVector<int>{}).reduce([&](...) {
        called = true;
        return 43;
    }, 42);

    // NOTE(SB): reduce() on an empty sequence without an initial value is an error!

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(called, false);
}

void tst_qpromise_reduce::regularValues()
{
    QVector<int> inputs{4, 6, 8};
    QVector<int> v0;
    QVector<int> v1;

    auto p0 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v0 << acc << cur << idx;
        return acc + cur + idx;
    });
    auto p1 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v1 << acc << cur << idx;
        return acc + cur + idx;
    }, 2);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);
    QCOMPARE(waitForValue(p0, -1), 21);
    QCOMPARE(waitForValue(p1, -1), 23);
    QCOMPARE(v0, QVector<int>({4, 6, 1, 11, 8, 2}));
    QCOMPARE(v1, QVector<int>({2, 4, 0, 6, 6, 1, 13, 8, 2}));
}

void tst_qpromise_reduce::promiseValues()
{
    QVector<QPromise<int>> inputs{
        QtPromise::resolve(4).delay(400),
        QtPromise::resolve(6).delay(300),
        QtPromise::resolve(8).delay(200)
    };
    QVector<int> v0;
    QVector<int> v1;

    auto p0 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v0 << acc << cur << idx;
        return acc + cur + idx;
    });
    auto p1 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v1 << acc << cur << idx;
        return acc + cur + idx;
    }, 2);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);
    QCOMPARE(waitForValue(p0, -1), 21);
    QCOMPARE(waitForValue(p1, -1), 23);
    QCOMPARE(v0, QVector<int>({4, 6, 1, 11, 8, 2}));
    QCOMPARE(v1, QVector<int>({2, 4, 0, 6, 6, 1, 13, 8, 2}));
}

void tst_qpromise_reduce::convertResultType()
{
    QVector<int> inputs{4, 6, 8};

    auto p = QtPromise::resolve(inputs).reduce([&](const QString& acc, int cur, int idx) {
        return QString("%1:%2:%3").arg(acc).arg(cur).arg(idx);
    }, QString("foo"));

    // NOTE(SB): when no initial value is given, the result type is the sequence type.

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, QString()), QString("foo:4:0:6:1:8:2"));
}

void tst_qpromise_reduce::delayedInitialValue()
{
    QVector<int> values;

    auto p = QtPromise::resolve(QVector<int>{4, 6, 8}).reduce([&](int acc, int cur, int idx) {
        values << acc << cur << idx;
        return acc + cur + idx;
    }, QtPromise::resolve(2).delay(100));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1), 23);
    QCOMPARE(values, QVector<int>({2, 4, 0, 6, 6, 1, 13, 8, 2}));
}

void tst_qpromise_reduce::delayedFulfilled()
{
    QVector<int> inputs{4, 6, 8};
    QVector<int> v0;
    QVector<int> v1;

    auto p0 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v0 << acc << cur << idx;
        return QtPromise::resolve(acc + cur + idx).delay(100);
    });
    auto p1 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v1 << acc << cur << idx;
        return QtPromise::resolve(acc + cur + idx).delay(100);
    }, 2);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);
    QCOMPARE(waitForValue(p0, -1), 21);
    QCOMPARE(waitForValue(p1, -1), 23);
    QCOMPARE(v0, QVector<int>({4, 6, 1, 11, 8, 2}));
    QCOMPARE(v1, QVector<int>({2, 4, 0, 6, 6, 1, 13, 8, 2}));
}

void tst_qpromise_reduce::delayedRejected()
{
    QVector<int> inputs{4, 6, 8};
    QVector<int> v0;
    QVector<int> v1;

    auto p0 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v0 << acc << cur << idx;
        if (cur == 6) {
            return QPromise<int>::reject(QString("foo"));
        }
        return QtPromise::resolve(acc + cur + idx);
    });
    auto p1 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v1 << acc << cur << idx;
        if (cur == 6) {
            return QPromise<int>::reject(QString("bar"));
        }
        return QtPromise::resolve(acc + cur + idx);
    }, 2);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);
    QCOMPARE(waitForError(p0, QString()), QString("foo"));
    QCOMPARE(waitForError(p1, QString()), QString("bar"));
    QCOMPARE(v0, QVector<int>({4, 6, 1}));
    QCOMPARE(v1, QVector<int>({2, 4, 0, 6, 6, 1}));
}

void tst_qpromise_reduce::functorThrows()
{
    QVector<int> inputs{4, 6, 8};
    QVector<int> v0;
    QVector<int> v1;

    auto p0 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v0 << acc << cur << idx;
        if (cur == 6) {
            throw QString("foo");
        }
        return acc + cur + idx;
    });
    auto p1 = QtPromise::resolve(inputs).reduce([&](int acc, int cur, int idx) {
        v1 << acc << cur << idx;
        if (cur == 6) {
            throw QString("bar");
        }
        return acc + cur + idx;
    }, 2);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);
    QCOMPARE(waitForError(p0, QString()), QString("foo"));
    QCOMPARE(waitForError(p1, QString()), QString("bar"));
    QCOMPARE(v0, QVector<int>({4, 6, 1}));
    QCOMPARE(v1, QVector<int>({2, 4, 0, 6, 6, 1}));
}

void tst_qpromise_reduce::sequenceTypes()
{
    SequenceTester<QLinkedList<QPromise<int>>>::exec();
    SequenceTester<QList<QPromise<int>>>::exec();
    SequenceTester<QVector<QPromise<int>>>::exec();
    SequenceTester<std::list<QPromise<int>>>::exec();
    SequenceTester<std::vector<QPromise<int>>>::exec();
}
