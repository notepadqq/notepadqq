// QtPromise
#include <QtPromise>

// Qt
#include <QtConcurrent>
#include <QtTest>

using namespace QtPromise;

class tst_future : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfilled();
    void fulfilled_void();
    void rejected();
    void rejected_void();
    void unhandled();
    void unhandled_void();
    void canceled();
    void canceled_void();
    void canceledFromThread();
    void then();
    void then_void();
    void fail();
    void fail_void();
    void finally();
    void finallyRejected();

}; // class tst_future

class MyException : public QException
{
public:
    MyException(const QString& error)
        : m_error(error)
    { }

    const QString& error() const { return m_error; }

    void raise() const { throw *this; }
    MyException* clone() const { return new MyException(*this); }

private:
    QString m_error;
};

QTEST_MAIN(tst_future)
#include "tst_future.moc"

void tst_future::fulfilled()
{
    int result = -1;
    auto p = QtPromise::resolve(QtConcurrent::run([]() {
        return 42;
    }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(p.isPending(), true);

    p.then([&](int res) {
        result = res;
    }).wait();

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(result, 42);
}

void tst_future::fulfilled_void()
{
    int result = -1;
    auto p = QtPromise::resolve(QtConcurrent::run([]() { }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(p.isPending(), true);

    p.then([&]() {
        result = 42;
    }).wait();

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(result, 42);
}

void tst_future::rejected()
{
    QString error;
    auto p = QtPromise::resolve(QtConcurrent::run([]() {
        throw MyException("foo");
        return 42;
    }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(p.isPending(), true);

    p.fail([&](const MyException& e) {
        error = e.error();
        return -1;
    }).wait();

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(error, QString("foo"));
}

void tst_future::rejected_void()
{
    QString error;
    auto p = QtPromise::resolve(QtConcurrent::run([]() {
        throw MyException("foo");
    }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));

    QCOMPARE(p.isPending(), true);

    p.fail([&](const MyException& e) {
        error = e.error();
    }).wait();

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(error, QString("foo"));
}

void tst_future::unhandled()
{
    QString error;
    auto p = QtPromise::resolve(QtConcurrent::run([]() {
        throw QString("foo");
        return 42;
    }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

    QCOMPARE(p.isPending(), true);

    p.fail([&](const QString& err) {
        error += err;
        return -1;
    }).fail([&](const QUnhandledException&) {
        error += "bar";
        return -1;
    }).wait();

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(error, QString("bar"));
}

void tst_future::unhandled_void()
{
    QString error;
    auto p = QtPromise::resolve(QtConcurrent::run([]() {
        throw QString("foo");
    }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(p.isPending(), true);

    p.fail([&](const QString& err) {
        error += err;
    }).fail([&](const QUnhandledException&) {
        error += "bar";
    }).wait();

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(error, QString("bar"));
}

void tst_future::canceled()
{
    QString error;
    auto p = QtPromise::resolve(QFuture<int>());  // Constructs an empty, canceled future.

    QCOMPARE(p.isPending(), true);

    p.fail([&](const QPromiseCanceledException&) {
        error = "canceled";
        return -1;
    }).wait();

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(error, QString("canceled"));
}

void tst_future::canceled_void()
{
    QString error;
    auto p = QtPromise::resolve(QFuture<void>());  // Constructs an empty, canceled future.

    QCOMPARE(p.isPending(), true);

    p.fail([&](const QPromiseCanceledException&) {
        error = "canceled";
    }).wait();

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(error, QString("canceled"));
}

void tst_future::canceledFromThread()
{
    QString error;
    auto p = QtPromise::resolve(QtConcurrent::run([]() {
        throw QPromiseCanceledException();
    }));

    QCOMPARE(p.isPending(), true);

    p.fail([&](const QPromiseCanceledException&) {
        error = "bar";
    }).wait();

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(error, QString("bar"));
}

void tst_future::then()
{
    QString result;
    auto input = QtPromise::resolve(42);
    auto output = input.then([](int res) {
        return QtConcurrent::run([=]() {
            return QString("foo%1").arg(res);
        });
    });

    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(output.isPending(), true);

    output.then([&](const QString& res) {
        result = res;
    }).wait();

    QCOMPARE(output.isFulfilled(), true);
    QCOMPARE(result, QString("foo42"));
}

void tst_future::then_void()
{
    QString result;
    auto input = QtPromise::resolve();
    auto output = input.then([&]() {
        return QtConcurrent::run([&]() {
            result = "foo";
        });
    });

    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(output.isPending(), true);

    output.then([&]() {
        result += "bar";
    }).wait();

    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(result, QString("foobar"));
}

void tst_future::fail()
{
    QString result;
    auto input = QPromise<QString>::reject(MyException("bar"));
    auto output = input.fail([](const MyException& e) {
        return QtConcurrent::run([](const QString& error) {
            return QString("foo%1").arg(error);
        }, e.error());
    });

    QCOMPARE(input.isRejected(), true);
    QCOMPARE(output.isPending(), true);

    output.then([&](const QString& res) {
        result = res;
    }).wait();

    QCOMPARE(output.isFulfilled(), true);
    QCOMPARE(result, QString("foobar"));
}

void tst_future::fail_void()
{
    QString result;
    auto input = QPromise<void>::reject(MyException("bar"));
    auto output = input.fail([&](const MyException& e) {
        return QtConcurrent::run([&](const QString& error) {
            result = error;
        }, e.error());
    });

    QCOMPARE(input.isRejected(), true);
    QCOMPARE(output.isPending(), true);

    output.then([&]() {
        result = result.prepend("foo");
    }).wait();

    QCOMPARE(output.isFulfilled(), true);
    QCOMPARE(result, QString("foobar"));
}

void tst_future::finally()
{
    auto input = QtPromise::resolve(42);
    auto output = input.finally([]() {
        return QtConcurrent::run([]() {
            return QString("foo");
        });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(output), QPromise<int>>::value));

    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(output.isPending(), true);

    int value = -1;
    output.then([&](int res) {
        value = res;
    }).wait();

    QCOMPARE(output.isFulfilled(), true);
    QCOMPARE(value, 42);
}

void tst_future::finallyRejected()
{
    auto input = QtPromise::resolve(42);
    auto output = input.finally([]() {
        return QtConcurrent::run([]() {
            throw MyException("foo");
        });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(output), QPromise<int>>::value));

    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(output.isPending(), true);

    QString error;
    output.fail([&](const MyException& e) {
        error = e.error();
        return -1;
    }).wait();

    QCOMPARE(output.isRejected(), true);
    QCOMPARE(error, QString("foo"));
}
