#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_finally : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfilledSync();
    void fulfilledSync_void();
    void fulfilledThrows();
    void fulfilledThrows_void();
    void fulfilledAsyncResolve();
    void fulfilledAsyncReject();
    void rejectedSync();
    void rejectedSync_void();
    void rejectedThrows();
    void rejectedThrows_void();
    void rejectedAsyncResolve();
    void rejectedAsyncReject();
};

QTEST_MAIN(tst_qpromise_finally)
#include "tst_finally.moc"

void tst_qpromise_finally::fulfilledSync()
{
    int value = -1;
    auto p = QPromise<int>::resolve(42).finally([&]() {
        value = 8;
        return 16; // ignored!
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(value, 8);
}

void tst_qpromise_finally::fulfilledSync_void()
{
    int value = -1;
    auto p = QPromise<void>::resolve().finally([&]() {
        value = 8;
        return 16; // ignored!
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(value, 8);
}

void tst_qpromise_finally::fulfilledThrows()
{
    auto p = QPromise<int>::resolve(42).finally([&]() {
        throw QString("bar");
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_finally::fulfilledThrows_void()
{
    auto p = QPromise<void>::resolve().finally([&]() {
        throw QString("bar");
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_finally::fulfilledAsyncResolve()
{
    QVector<int> values;
    auto p = QPromise<int>::resolve(42).finally([&]() {
        QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
            QtPromisePrivate::qtpromise_defer([=, &values]() {
                values << 64;
                resolve(16); // ignored!
            });
        });

        values << 8;
        return p;
    });

    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(values, QVector<int>({8, 64}));
}

void tst_qpromise_finally::fulfilledAsyncReject()
{
    auto p = QPromise<int>::resolve(42).finally([]() {
        return QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
            QtPromisePrivate::qtpromise_defer([=]() {
                reject(QString("bar"));
            });
        });
    });

    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_finally::rejectedSync()
{
    int value = -1;
    auto p = QPromise<int>::reject(QString("foo")).finally([&]() {
        value = 8;
        return 16; // ignored!
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(value, 8);
}

void tst_qpromise_finally::rejectedSync_void()
{
    int value = -1;
    auto p = QPromise<void>::reject(QString("foo")).finally([&]() {
        value = 8;
        return 16; // ignored!
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(value, 8);
}

void tst_qpromise_finally::rejectedThrows()
{
    auto p = QPromise<int>::reject(QString("foo")).finally([&]() {
        throw QString("bar");
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_finally::rejectedThrows_void()
{
    auto p = QPromise<void>::reject(QString("foo")).finally([&]() {
        throw QString("bar");
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_finally::rejectedAsyncResolve()
{
    QVector<int> values;
    auto p = QPromise<int>::reject(QString("foo")).finally([&]() {
        QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
            QtPromisePrivate::qtpromise_defer([=, &values]() {
                values << 64;
                resolve(16); // ignored!
            });
        });

        values << 8;
        return p;
    });

    p.then([&](int r) {
        values << r;
    }).wait();

    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(values, QVector<int>({8, 64}));
}

void tst_qpromise_finally::rejectedAsyncReject()
{
    auto p = QPromise<int>::reject(QString("foo")).finally([]() {
        return QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
            QtPromisePrivate::qtpromise_defer([=]() {
                reject(QString("bar"));
            });
        });
    });

    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(p.isRejected(), true);
}
