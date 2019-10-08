#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_each : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptySequence();
    void preserveValues();
    void ignoreResult();
    void delayedFulfilled();
    void delayedRejected();
    void functorThrows();
    void functorArguments();
    void sequenceTypes();
};

QTEST_MAIN(tst_qpromise_each)
#include "tst_each.moc"

namespace {

template <class Sequence>
struct SequenceTester
{
    static void exec()
    {
        QVector<int> values;
        auto p = QtPromise::resolve(Sequence{42, 43, 44}).each([&](int v, int i) {
            values << i << v;
        }).each([&](int v, ...) {
            values << v;
            return QString("foo");
        }).each([&](int v, ...) {
            values << v + 1;
            return QtPromise::resolve(QString("foo")).then([&](){
                values << -1;
            });
        }).each([&](int v, ...) {
            values << v + 2;
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Sequence>>::value));
        QCOMPARE(waitForValue(p, Sequence()), Sequence({42, 43, 44}));

        QVector<int> expected{
            0, 42, 1, 43, 2, 44,
            42, 43, 44,
            43, 44, 45,
            -1, -1, -1,
            44, 45, 46
        };
        QCOMPARE(values, expected);
    }
};

} // anonymous namespace

void tst_qpromise_each::emptySequence()
{
    QVector<int> values;
    auto p = QPromise<QVector<int>>::resolve({}).each([&](int v, ...) {
        values << v;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>());
    QCOMPARE(values, QVector<int>({}));
}

void tst_qpromise_each::preserveValues()
{
    QVector<int> values;
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).each([&](int v, ...) {
        values << v + 1;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
    QCOMPARE(values, QVector<int>({43, 44, 45}));
}

void tst_qpromise_each::ignoreResult()
{
    QVector<int> values;
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).each([&](int v, ...) {
        values << v + 1;
        return "Foo";
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
    QCOMPARE(values, QVector<int>({43, 44, 45}));
}

void tst_qpromise_each::delayedFulfilled()
{
    QMap<int, int> values;
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).each([&](int v, int index) {
        return QPromise<void>::resolve().delay(250).then([=, &values]() {
            values[v] = index;
            return 42;
        });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
    QMap<int, int> expected{{42, 0}, {43, 1}, {44, 2}};
    QCOMPARE(values, expected);
}

void tst_qpromise_each::delayedRejected()
{
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).each([](int v, ...) {
        return QPromise<int>([&](
            const QPromiseResolve<int>& resolve,
            const QPromiseReject<int>& reject) {
                QtPromisePrivate::qtpromise_defer([=]() {
                    if (v == 44) {
                        reject(QString("foo"));
                    }
                    resolve(v);
                });
            });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_each::functorThrows()
{
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).each([](int v, ...) {
        if (v == 44) {
            throw QString("foo");
        }
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_each::functorArguments()
{
    QVector<int> values;
    auto p = QPromise<QVector<int>>::resolve({42, 43, 44}).each([&](int v, int i) {
        values << i << v;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
    QCOMPARE(values, QVector<int>({0, 42, 1, 43, 2, 44}));
}

void tst_qpromise_each::sequenceTypes()
{
    SequenceTester<QList<int>>::exec();
    SequenceTester<QVector<int>>::exec();
    SequenceTester<std::list<int>>::exec();
    SequenceTester<std::vector<int>>::exec();
}
