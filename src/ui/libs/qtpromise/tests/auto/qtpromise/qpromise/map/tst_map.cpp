#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_map : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptySequence();
    void modifyValues();
    void convertValues();
    void delayedFulfilled();
    void delayedRejected();
    void functorThrows();
    void functorArguments();
    void preserveOrder();
    void sequenceTypes();
};

QTEST_MAIN(tst_qpromise_map)
#include "tst_map.moc"

namespace {

template <class Sequence>
struct SequenceTester
{
    static void exec()
    {
        auto p = QtPromise::resolve(Sequence{42, 43, 44}).map([](int v, ...) {
            return QString::number(v + 1);
        }).map([](const QString& v, int i) {
            return QtPromise::resolve(QString("%1:%2").arg(i).arg(v));
        }).map([](const QString& v, ...) {
            return QtPromise::resolve((v + "!").toUtf8());
        }).map([](const QByteArray& v, ...) {
            return QString::fromUtf8(v);
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<QString>>>::value));
        QCOMPARE(waitForValue(p, QVector<QString>()), QVector<QString>({"0:43!", "1:44!", "2:45!"}));
    }
};

} // anonymous namespace

void tst_qpromise_map::emptySequence()
{
    auto p = QtPromise::resolve(QVector<int>{}).map([](int v, ...) {
        return v + 1;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({}));
}

void tst_qpromise_map::modifyValues()
{
    auto p = QtPromise::resolve(QVector<int>{42, 43, 44}).map([](int v, ...) {
        return v + 1;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44, 45}));
}

void tst_qpromise_map::convertValues()
{
    auto p = QtPromise::resolve(QVector<int>{42, 43, 44}).map([](int v, ...) {
        return QString::number(v + 1);
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<QString>>>::value));
    QCOMPARE(waitForValue(p, QVector<QString>()), QVector<QString>({"43", "44", "45"}));
}

void tst_qpromise_map::delayedFulfilled()
{
    auto p = QtPromise::resolve(QVector<int>{42, 43, 44}).map([](int v, ...) {
        return QPromise<int>([&](const QPromiseResolve<int>& resolve) {
                QtPromisePrivate::qtpromise_defer([=]() {
                    resolve(v + 1);
                });
            });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44, 45}));
}

void tst_qpromise_map::delayedRejected()
{
    auto p = QtPromise::resolve(QVector<int>{42, 43, 44}).map([](int v, ...) {
        return QPromise<int>([&](
            const QPromiseResolve<int>& resolve,
            const QPromiseReject<int>& reject) {
                QtPromisePrivate::qtpromise_defer([=]() {
                    if (v == 43) {
                        reject(QString("foo"));
                    }
                    resolve(v);
                });
            });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_map::functorThrows()
{
    auto p = QtPromise::resolve(QVector<int>{42, 43, 44}).map([](int v, ...) {
        if (v == 43) {
            throw QString("foo");
        }
        return v + 1;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_map::functorArguments()
{
    auto p1 = QtPromise::resolve(QVector<int>{42, 42, 42}).map([](int v, int i) {
        return v * i;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p1, QVector<int>()), QVector<int>({0, 42, 84}));
}

void tst_qpromise_map::preserveOrder()
{
    auto p = QtPromise::resolve(QVector<int>{250, 500, 100}).map([](int v, ...) {
        return QtPromise::resolve(v + 1).delay(v);
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({251, 501, 101}));
}

void tst_qpromise_map::sequenceTypes()
{
    SequenceTester<QList<int>>::exec();
    SequenceTester<QVector<int>>::exec();
    SequenceTester<std::list<int>>::exec();
    SequenceTester<std::vector<int>>::exec();
}
