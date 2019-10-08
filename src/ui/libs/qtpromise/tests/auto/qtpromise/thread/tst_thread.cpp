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
    QThread* target = nullptr;
    QThread* source = nullptr;

    QPromise<int>([&](const QPromiseResolve<int>& resolve) {
        QtConcurrent::run([=, &source]() {
            source = QThread::currentThread();
            resolve(42);
        });
    }).then([&](int res) {
        target = QThread::currentThread();
        value = res;
    }).wait();

    QVERIFY(source != nullptr);
    QVERIFY(source != target);
    QCOMPARE(target, QThread::currentThread());
    QCOMPARE(value, 42);
}

void tst_thread::resolve_void()
{
    int value = -1;
    QThread* target = nullptr;
    QThread* source = nullptr;

    QPromise<void>([&](const QPromiseResolve<void>& resolve) {
        QtConcurrent::run([=, &source]() {
            source = QThread::currentThread();
            resolve();
        });
    }).then([&]() {
        target = QThread::currentThread();
        value = 43;
    }).wait();

    QVERIFY(source != nullptr);
    QVERIFY(source != target);
    QCOMPARE(target, QThread::currentThread());
    QCOMPARE(value, 43);
}

void tst_thread::reject()
{
    QString error;
    QThread* target = nullptr;
    QThread* source = nullptr;

    QPromise<int>([&](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QtConcurrent::run([=, &source]() {
            source = QThread::currentThread();
            reject(QString("foo"));
        });
    }).fail([&](const QString& err) {
        target = QThread::currentThread();
        error = err;
        return -1;
    }).wait();

    QVERIFY(source != nullptr);
    QVERIFY(source != target);
    QCOMPARE(target, QThread::currentThread());
    QCOMPARE(error, QString("foo"));
}

void tst_thread::then()
{
    QThread* source = nullptr;
    QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
        source = QThread::currentThread();
        resolve(42);
    });

    int value = -1;
    QThread* target = nullptr;
    QtPromise::resolve(QtConcurrent::run([&](const QPromise<int>& p) {
        p.then([&](int res) {
            target = QThread::currentThread();
            value = res;
        }).wait();
    }, p)).wait();

    QVERIFY(target != nullptr);
    QVERIFY(source != target);
    QCOMPARE(source, QThread::currentThread());
    QCOMPARE(value, 42);
}

void tst_thread::then_void()
{
    QThread* source = nullptr;
    QPromise<void> p([&](const QPromiseResolve<void>& resolve) {
        source = QThread::currentThread();
        resolve();
    });

    int value = -1;
    QThread* target = nullptr;
    QtPromise::resolve(QtConcurrent::run([&](const QPromise<void>& p) {
        p.then([&]() {
            target = QThread::currentThread();
            value = 43;
        }).wait();
    }, p)).wait();

    QVERIFY(target != nullptr);
    QVERIFY(source != target);
    QCOMPARE(source, QThread::currentThread());
    QCOMPARE(value, 43);
}

void tst_thread::fail()
{
    QThread* source = nullptr;
    QPromise<int> p([&](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        source = QThread::currentThread();
        reject(QString("foo"));
    });

    QString error;
    QThread* target = nullptr;
    QtPromise::resolve(QtConcurrent::run([&](const QPromise<int>& p) {
        p.fail([&](const QString& err) {
            target = QThread::currentThread();
            error = err;
            return -1;
        }).wait();
    }, p)).wait();

    QVERIFY(target != nullptr);
    QVERIFY(source != target);
    QCOMPARE(source, QThread::currentThread());
    QCOMPARE(error, QString("foo"));
}

void tst_thread::finally()
{
    QThread* source = nullptr;
    QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
        source = QThread::currentThread();
        resolve(42);
    });

    int value = -1;
    QThread* target = nullptr;
    QtPromise::resolve(QtConcurrent::run([&](const QPromise<int>& p) {
        p.finally([&]() {
            target = QThread::currentThread();
            value = 43;
        }).wait();
    }, p)).wait();

    QVERIFY(target != nullptr);
    QVERIFY(source != target);
    QCOMPARE(source, QThread::currentThread());
    QCOMPARE(value, 43);
}
