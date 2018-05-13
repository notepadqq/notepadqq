// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_helpers : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolve();
    void resolve_void();
    void resolve_promise();
    void resolve_promise_void();

    void allFulfilled();
    void allFulfilled_void();
    void allRejected();
    void allRejected_void();
    void allEmpty();
    void allEmpty_void();

}; // class tst_helpers

QTEST_MAIN(tst_helpers)
#include "tst_helpers.moc"

void tst_helpers::resolve()
{
    int value = -1;
    auto p = QtPromise::qPromise(42);

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int> >::value));

    QCOMPARE(p.isFulfilled(), true);

    p.then([&](int res) {
        value = res;
    }).wait();

    QCOMPARE(value, 42);
}

void tst_helpers::resolve_void()
{
    int value = -1;
    auto p = QtPromise::qPromise();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));

    QCOMPARE(p.isFulfilled(), true);

    p.then([&]() {
        value = 42;
    }).wait();

    QCOMPARE(value, 42);
}

void tst_helpers::resolve_promise()
{
    QString value;
    auto p = QtPromise::qPromise(
        QPromise<QString>([](const QPromiseResolve<QString>& resolve) {
            QtPromisePrivate::qtpromise_defer([=](){
                resolve("foo");
            });
        }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString> >::value));

    QCOMPARE(p.isPending(), true);

    p.then([&](const QString& res) {
        value = res;
    }).wait();

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(value, QString("foo"));
}

void tst_helpers::resolve_promise_void()
{
    QList<int> values;
    auto p = QtPromise::qPromise(
        QPromise<void>([&](const QPromiseResolve<void>& resolve) {
            QtPromisePrivate::qtpromise_defer([=, &values](){
                values << 42;
                resolve();
            });
        }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));

    QCOMPARE(p.isPending(), true);

    p.then([&]() {
        values << 43;
    }).wait();

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(values, QList<int>({42, 43}));
}

void tst_helpers::allFulfilled()
{
    auto p0 = QtPromise::qPromise(42);
    auto p1 = QtPromise::qPromise(44);
    auto p2 = QPromise<int>([](const QPromiseResolve<int>& resolve) {
        QtPromisePrivate::qtpromise_defer([=](){
            resolve(43);
        });
    });

    auto p = qPromiseAll(QVector<QPromise<int> >{p0, p2, p1});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int> > >::value));

    QCOMPARE(p.isPending(), true);
    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(p2.isPending(), true);

    QVector<int> values;
    p.then([&](const QVector<int>& res) {
        values = res;
    }).wait();

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(p2.isFulfilled(), true);
    QCOMPARE(values, QVector<int>({42, 43, 44}));
}

void tst_helpers::allFulfilled_void()
{
    auto p0 = QtPromise::qPromise();
    auto p1 = QtPromise::qPromise();
    auto p2 = QPromise<void>([](const QPromiseResolve<void>& resolve) {
        QtPromisePrivate::qtpromise_defer([=](){
            resolve();
        });
    });

    auto p = qPromiseAll(QVector<QPromise<void> >{p0, p2, p1});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));

    QCOMPARE(p.isPending(), true);
    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(p2.isPending(), true);

    p.wait();

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(p2.isFulfilled(), true);
}

void tst_helpers::allRejected()
{
    auto p0 = QtPromise::qPromise(42);
    auto p1 = QtPromise::qPromise(44);
    auto p2 = QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QtPromisePrivate::qtpromise_defer([=](){
            reject(QString("foo"));
        });
    });

    auto p = qPromiseAll(QVector<QPromise<int> >{p0, p2, p1});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int> > >::value));

    QCOMPARE(p.isPending(), true);
    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(p2.isPending(), true);

    QString error;
    p.fail([&](const QString& err) {
        error = err;
        return QVector<int>();
    }).wait();

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(p2.isRejected(), true);
    QCOMPARE(error, QString("foo"));
}

void tst_helpers::allRejected_void()
{
    auto p0 = QtPromise::qPromise();
    auto p1 = QtPromise::qPromise();
    auto p2 = QPromise<void>([](const QPromiseResolve<void>&, const QPromiseReject<void>& reject) {
        QtPromisePrivate::qtpromise_defer([=](){
            reject(QString("foo"));
        });
    });

    auto p = qPromiseAll(QVector<QPromise<void> >{p0, p2, p1});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));

    QCOMPARE(p.isPending(), true);
    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(p2.isPending(), true);

    QString error;
    p.fail([&](const QString& err) {
        error = err;
    }).wait();

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(p2.isRejected(), true);
    QCOMPARE(error, QString("foo"));
}

void tst_helpers::allEmpty()
{
    auto p = qPromiseAll(QVector<QPromise<int> >());

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int> > >::value));

    QCOMPARE(p.isFulfilled(), true);

    QVector<int> values;
    p.then([&](const QVector<int>& res) {
        values = res;
    }).wait();

    QCOMPARE(values, QVector<int>());
}

void tst_helpers::allEmpty_void()
{
    auto p = qPromiseAll(QVector<QPromise<void> >());

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));

    QCOMPARE(p.isFulfilled(), true);
}
