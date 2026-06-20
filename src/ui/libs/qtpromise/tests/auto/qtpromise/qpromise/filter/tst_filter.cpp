#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_filter : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptySequence();
    void filterValues();
    void delayedFulfilled();
    void delayedRejected();
    void functorThrows();
    void functorArguments();
    void preserveOrder();
    void sequenceTypes();
};

QTEST_MAIN(tst_qpromise_filter)
#include "tst_filter.moc"

namespace {

template <class Sequence>
struct SequenceTester
{
    static void exec()
    {
        auto p = QtPromise::resolve(Sequence{
            42, 43, 44, 45, 46, 47, 48, 49, 50, 51
        }).filter([](int v, ...) {
            return v > 42 && v < 51;
        }).filter([](int, int i) {
            return QPromise<bool>::resolve(i % 2 == 0);
        }).filter([](int v, ...) {
            return v != 45;
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Sequence>>::value));
        QCOMPARE(waitForValue(p, Sequence()), Sequence({43, 47, 49}));
    }
};

} // anonymous namespace

#include <QtConcurrent/QtConcurrent>

void tst_qpromise_filter::emptySequence()
{
    auto p = QPromise<QVector<int>>::resolve({}).filter([](int v, ...) {
        return v % 2 == 0;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>{});
}

void tst_qpromise_filter::filterValues()
{
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).filter([](int v, ...) {
        return v % 2 == 0;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 44}));
}

void tst_qpromise_filter::delayedFulfilled()
{
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).filter([](int v, ...) {
        return QPromise<bool>([&](const QPromiseResolve<bool>& resolve) {
                QtPromisePrivate::qtpromise_defer([=]() {
                    resolve(v % 2 == 0);
                });
            });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 44}));
}

void tst_qpromise_filter::delayedRejected()
{
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).filter([](int v, ...) {
        return QPromise<bool>([&](
            const QPromiseResolve<bool>& resolve,
            const QPromiseReject<bool>& reject) {
                QtPromisePrivate::qtpromise_defer([=]() {
                    if (v == 43) {
                        reject(QString("foo"));
                    }
                    resolve(true);
                });
            });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_filter::functorThrows()
{
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).filter([](int v, ...) {
        if (v == 43) {
            throw QString("foo");
        }
        return true;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_filter::functorArguments()
{
    QMap<int, int> args;
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).filter([&](int v, int i) {
        args[v] = i;
        return i % 2 == 0;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 44}));
    QMap<int, int> expected{{42, 0}, {43, 1}, {44, 2}};
    QCOMPARE(args, expected);
}

void tst_qpromise_filter::preserveOrder()
{
    auto p = QPromise<QVector<int>>::resolve({250, 50, 100, 400, 300}).filter([](int v, ...) {
        return QPromise<bool>::resolve(v > 200).delay(v);
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({250, 400, 300}));
}

void tst_qpromise_filter::sequenceTypes()
{
    SequenceTester<QList<int>>::exec();
    SequenceTester<QVector<int>>::exec();
    SequenceTester<std::list<int>>::exec();
    SequenceTester<std::vector<int>>::exec();
}
