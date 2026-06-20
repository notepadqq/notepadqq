#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_then : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolveSync();
    void resolveAsync();
    void rejectSync();
    void rejectAsync();
    void skipResult();
    void noHandler();
};

QTEST_MAIN(tst_qpromise_then)
#include "tst_then.moc"

void tst_qpromise_then::resolveSync()
{
    QVariantList values;

    auto input = QPromise<int>::resolve(42);
    auto output = input.then([&](int res) {
        values << res;
        return QString::number(res+1);
    });

    output.then([&](const QString& res) {
        values << res;
    }).then([&]() {
        values << 44;
    }).wait();

    QCOMPARE(values, QVariantList({42, QString("43"), 44}));
    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(output.isFulfilled(), true);
}

void tst_qpromise_then::resolveAsync()
{
    auto p = QPromise<int>::resolve(42).then([](int res) {
        return QPromise<QString>([=](const QPromiseResolve<QString>& resolve) {
            QtPromisePrivate::qtpromise_defer([=]() {
                resolve(QString("foo%1").arg(res));
            });
        });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(waitForValue(p, QString()), QString("foo42"));
    QCOMPARE(p.isFulfilled(), true);
}

void tst_qpromise_then::rejectSync()
{
    auto input = QPromise<int>::resolve(42);
    auto output = input.then([](int res) {
        throw QString("foo%1").arg(res);
        return 42;
    });

    QString error;
    output.then([&](int res) {
        error += "bar" + QString::number(res);
    }).fail([&](const QString& err) {
        error += err;
    }).wait();

    QCOMPARE(error, QString("foo42"));
    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(output.isRejected(), true);
}

void tst_qpromise_then::rejectAsync()
{
    auto p = QPromise<int>::resolve(42).then([](int res) {
        return QPromise<void>([=](const QPromiseResolve<void>&, const QPromiseReject<void>& reject) {
            QtPromisePrivate::qtpromise_defer([=]() {
                reject(QString("foo%1").arg(res));
            });
        });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo42"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_then::skipResult()
{
    auto p = QPromise<int>::resolve(42);

    int value = -1;
    p.then([&]() {
        value = 43;
    }).wait();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(value, 43);
}

void tst_qpromise_then::noHandler()
{
    {   // resolved
        auto p = QPromise<int>::resolve(42).then(nullptr);

        QCOMPARE(waitForValue(p, -1), 42);
        QCOMPARE(p.isFulfilled(), true);
    }
    {   // rejected
        auto p = QPromise<int>::reject(QString("foo")).then(nullptr);

        QCOMPARE(waitForError(p, QString()), QString("foo"));
        QCOMPARE(p.isRejected(), true);
    }
}
