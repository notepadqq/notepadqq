// QtPromise
#include <QtPromise>

// Qt
#include <QtConcurrent>
#include <QtTest>

using namespace QtPromise;

class tst_thread : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolve();
    void resolve_void();
    void reject();
    void then();
    void then_void();
    void fail();
    void finally();

}; // class tst_thread

QTEST_MAIN(tst_thread)
#include "tst_thread.moc"

void tst_thread::resolve()
{
    int value = -1;
    size_t target = 0;
    size_t source = 0;

    QPromise<int>([&](const QPromiseResolve<int>& resolve) {
        QtConcurrent::run([=, &source]() {
            source = (size_t)QThread::currentThread();
            resolve(42);
        });
    }).then([&](int res) {
        target = (size_t)QThread::currentThread();
        value = res;
    }).wait();

    QVERIFY(source != 0);
    QVERIFY(source != target);
    QCOMPARE(target, (size_t)QThread::currentThread());
    QCOMPARE(value, 42);
}

void tst_thread::resolve_void()
{
    int value = -1;
    size_t target = 0;
    size_t source = 0;

    QPromise<void>([&](const QPromiseResolve<void>& resolve) {
        QtConcurrent::run([=, &source]() {
            source = (size_t)QThread::currentThread();
            resolve();
        });
    }).then([&]() {
        target = (size_t)QThread::currentThread();
        value = 43;
    }).wait();

    QVERIFY(source != 0);
    QVERIFY(source != target);
    QCOMPARE(target, (size_t)QThread::currentThread());
    QCOMPARE(value, 43);
}

void tst_thread::reject()
{
    QString error;
    size_t target = 0;
    size_t source = 0;

    QPromise<int>([&](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QtConcurrent::run([=, &source]() {
            source = (size_t)QThread::currentThread();
            reject(QString("foo"));
        });
    }).fail([&](const QString& err) {
        target = (size_t)QThread::currentThread();
        error = err;
        return -1;
    }).wait();

    QVERIFY(source != 0);
    QVERIFY(source != target);
    QCOMPARE(target, (size_t)QThread::currentThread());
    QCOMPARE(error, QString("foo"));
}

void tst_thread::then()
{
    size_t source;
    QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
        source = (size_t)QThread::currentThread();
        resolve(42);
    });

    size_t target;
    int value = -1;
    qPromise(QtConcurrent::run([&](const QPromise<int>& p) {
        p.then([&](int res) {
            target = (size_t)QThread::currentThread();
            value = res;
        }).wait();
    }, p)).wait();

    QVERIFY(target != 0);
    QVERIFY(source != target);
    QCOMPARE(source, (size_t)QThread::currentThread());
    QCOMPARE(value, 42);
}

void tst_thread::then_void()
{
    size_t source;
    QPromise<void> p([&](const QPromiseResolve<void>& resolve) {
        source = (size_t)QThread::currentThread();
        resolve();
    });

    size_t target;
    int value = -1;
    qPromise(QtConcurrent::run([&](const QPromise<void>& p) {
        p.then([&]() {
            target = (size_t)QThread::currentThread();
            value = 43;
        }).wait();
    }, p)).wait();

    QVERIFY(target != 0);
    QVERIFY(source != target);
    QCOMPARE(source, (size_t)QThread::currentThread());
    QCOMPARE(value, 43);
}

void tst_thread::fail()
{
    size_t source;
    QPromise<int> p([&](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        source = (size_t)QThread::currentThread();
        reject(QString("foo"));
    });

    size_t target;
    QString error;
    qPromise(QtConcurrent::run([&](const QPromise<int>& p) {
        p.fail([&](const QString& err) {
            target = (size_t)QThread::currentThread();
            error = err;
            return -1;
        }).wait();
    }, p)).wait();

    QVERIFY(target != 0);
    QVERIFY(source != target);
    QCOMPARE(source, (size_t)QThread::currentThread());
    QCOMPARE(error, QString("foo"));
}

void tst_thread::finally()
{
    size_t source;
    QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
        source = (size_t)QThread::currentThread();
        resolve(42);
    });

    size_t target;
    int value = -1;
    qPromise(QtConcurrent::run([&](const QPromise<int>& p) {
        p.finally([&]() {
            target = (size_t)QThread::currentThread();
            value = 43;
        }).wait();
    }, p)).wait();

    QVERIFY(target != 0);
    QVERIFY(source != target);
    QCOMPARE(source, (size_t)QThread::currentThread());
    QCOMPARE(value, 43);
}
