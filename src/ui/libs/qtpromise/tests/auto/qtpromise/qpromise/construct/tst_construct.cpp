#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

// STL
#include <memory>

using namespace QtPromise;

class tst_qpromise_construct : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolveSyncOneArg();
    void resolveSyncOneArg_void();
    void resolveSyncTwoArgs();
    void resolveSyncTwoArgs_void();
    void resolveAsyncOneArg();
    void resolveAsyncOneArg_void();
    void resolveAsyncTwoArgs();
    void resolveAsyncTwoArgs_void();
    void rejectThrowOneArg();
    void rejectThrowOneArg_void();
    void rejectThrowTwoArgs();
    void rejectThrowTwoArgs_void();
    void rejectSync();
    void rejectSync_void();
    void rejectAsync();
    void rejectAsync_void();
    void rejectUndefined();
    void rejectUndefined_void();
    void connectAndResolve();
    void connectAndReject();
};

QTEST_MAIN(tst_qpromise_construct)
#include "tst_construct.moc"

void tst_qpromise_construct::resolveSyncOneArg()
{
    QPromise<int> p([](const QPromiseResolve<int>& resolve) {
        resolve(42);
    });

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForError(p, QString()), QString());
    QCOMPARE(waitForValue(p, -1), 42);
}

void tst_qpromise_construct::resolveSyncOneArg_void()
{
    QPromise<void> p([](const QPromiseResolve<void>& resolve) {
        resolve();
    });

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForError(p, QString()), QString());
    QCOMPARE(waitForValue(p, -1, 42), 42);
}

void tst_qpromise_construct::resolveSyncTwoArgs()
{
    QPromise<int> p([](const QPromiseResolve<int>& resolve, const QPromiseReject<int>&) {
        resolve(42);
    });

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForError(p, QString()), QString());
    QCOMPARE(waitForValue(p, -1), 42);
}

void tst_qpromise_construct::resolveSyncTwoArgs_void()
{
    QPromise<void> p([](const QPromiseResolve<void>& resolve, const QPromiseReject<void>&) {
        resolve();
    });

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForError(p, QString()), QString());
    QCOMPARE(waitForValue(p, -1, 42), 42);
}

void tst_qpromise_construct::resolveAsyncOneArg()
{
    QPromise<int> p([](const QPromiseResolve<int>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve(42);
        });
    });

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, QString()), QString());
    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(p.isFulfilled(), true);
}

void tst_qpromise_construct::resolveAsyncOneArg_void()
{
    QPromise<void> p([](const QPromiseResolve<void>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve();
        });
    });

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, QString()), QString());
    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(p.isFulfilled(), true);
}

void tst_qpromise_construct::resolveAsyncTwoArgs()
{
    QPromise<int> p([](const QPromiseResolve<int>& resolve, const QPromiseReject<int>&) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve(42);
        });
    });

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, QString()), QString());
    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(p.isFulfilled(), true);
}

void tst_qpromise_construct::resolveAsyncTwoArgs_void()
{
    QPromise<void> p([](const QPromiseResolve<void>& resolve, const QPromiseReject<void>&) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve();
        });
    });

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, QString()), QString());
    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(p.isFulfilled(), true);
}

void tst_qpromise_construct::rejectSync()
{
    QPromise<int> p([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        reject(QString("foo"));
    });

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForValue(p, -1), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_construct::rejectSync_void()
{
    QPromise<void> p([](const QPromiseResolve<void>&, const QPromiseReject<void>& reject) {
        reject(QString("foo"));
    });

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForValue(p, -1, 42), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_construct::rejectAsync()
{
    QPromise<int> p([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QtPromisePrivate::qtpromise_defer([=]() {
            reject(QString("foo"));
        });
    });

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_construct::rejectAsync_void()
{
    QPromise<void> p([](const QPromiseResolve<void>&, const QPromiseReject<void>& reject) {
        QtPromisePrivate::qtpromise_defer([=]() {
            reject(QString("foo"));
        });
    });

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1, 42), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_construct::rejectThrowOneArg()
{
    QPromise<int> p([](const QPromiseResolve<int>&) {
        throw QString("foo");
    });

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForValue(p, -1), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_construct::rejectThrowOneArg_void()
{
    QPromise<void> p([](const QPromiseResolve<void>&) {
        throw QString("foo");
    });

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForValue(p, -1, 42), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_construct::rejectThrowTwoArgs()
{
    QPromise<int> p([](const QPromiseResolve<int>&, const QPromiseReject<int>&) {
        throw QString("foo");
    });

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForValue(p, -1), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_construct::rejectThrowTwoArgs_void()
{
    QPromise<void> p([](const QPromiseResolve<void>&, const QPromiseReject<void>&) {
        throw QString("foo");
    });

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForValue(p, -1, 42), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise_construct::rejectUndefined()
{
    QPromise<int> p([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QtPromisePrivate::qtpromise_defer([=]() {
            reject();
        });
    });

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForRejected<QPromiseUndefinedException>(p), true);
}

void tst_qpromise_construct::rejectUndefined_void()
{
    QPromise<void> p([](const QPromiseResolve<void>&, const QPromiseReject<void>& reject) {
        QtPromisePrivate::qtpromise_defer([=]() {
            reject();
        });
    });

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForRejected<QPromiseUndefinedException>(p), true);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_qpromise_construct::connectAndResolve()
{
    QScopedPointer<QObject> object(new QObject());

    std::weak_ptr<int> wptr;

    {
        auto p = QPromise<std::shared_ptr<int>>([&](
            const QPromiseResolve<std::shared_ptr<int>>& resolve,
            const QPromiseReject<std::shared_ptr<int>>& reject) {

            connect(object.data(), &QObject::objectNameChanged,
                [=, &wptr](const QString& name) {
                    std::shared_ptr<int> sptr(new int(42));

                    wptr = sptr;

                    if (name == "foobar") {
                        resolve(sptr);
                    } else {
                        reject(42);
                    }
                });
        });

        QCOMPARE(p.isPending(), true);

        object->setObjectName("foobar");

        QCOMPARE(waitForValue(p, std::shared_ptr<int>()), wptr.lock());
        QCOMPARE(wptr.use_count(), 1l); // "p" still holds a reference
    }

    QCOMPARE(wptr.use_count(), 0l);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_qpromise_construct::connectAndReject()
{
    QScopedPointer<QObject> object(new QObject());

    std::weak_ptr<int> wptr;

    {
        auto p = QPromise<int>([&](
            const QPromiseResolve<int>& resolve,
            const QPromiseReject<int>& reject) {

            connect(object.data(), &QObject::objectNameChanged,
                [=, &wptr](const QString& name) {
                    std::shared_ptr<int> sptr(new int(42));

                    wptr = sptr;

                    if (name == "foobar") {
                        reject(sptr);
                    } else {
                        resolve(42);
                    }
                });
        });

        QCOMPARE(p.isPending(), true);

        object->setObjectName("foobar");

        QCOMPARE(waitForError(p, std::shared_ptr<int>()), wptr.lock());
        QCOMPARE(wptr.use_count(), 1l); // "p" still holds a reference
    }

    QCOMPARE(wptr.use_count(), 0l);
}
