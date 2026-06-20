#include "../../../shared/data.h"
#include "../../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtConcurrent>
#include <QtTest>

// STL
#include <memory>

using namespace QtPromise;

class tst_deprecations_helpers_qpromise : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void value();
    void noValue();
    void moveRValue();
    void copyLValue();
    void qtSharedPtr();
    void stdSharedPtr();
    void typedPromise();
    void voidPromise();
    void typedFuture();
    void voidFuture();
};

QTEST_MAIN(tst_deprecations_helpers_qpromise)
#include "tst_qpromise.moc"

void tst_deprecations_helpers_qpromise::value()
{
    int v0 = 42;
    const int v1 = 42;

    auto p0 = qPromise(42);
    auto p1 = qPromise(v0);
    auto p2 = qPromise(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QPromise<int>>::value));

    for (const auto& p : {p0, p1, p2}) {
        QCOMPARE(p.isFulfilled(), true);
    }
    for (const auto& p : {p0, p1, p2}) {
        QCOMPARE(waitForValue(p, -1), 42);
    }
}

void tst_deprecations_helpers_qpromise::noValue()
{
    auto p = qPromise();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
}

void tst_deprecations_helpers_qpromise::moveRValue()
{
    Data::logs().reset();

    {
        auto p = qPromise(Data(42)).wait();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Data>>::value));
    }

    QCOMPARE(Data::logs().ctor, 1);
    QCOMPARE(Data::logs().copy, 0);
    QCOMPARE(Data::logs().move, 1);
    QCOMPARE(Data::logs().refs, 0);
}

void tst_deprecations_helpers_qpromise::copyLValue()
{
    Data::logs().reset();

    {
        Data value(42);
        auto p = qPromise(value).wait();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Data>>::value));
    }

    QCOMPARE(Data::logs().ctor, 1);
    QCOMPARE(Data::logs().copy, 1);
    QCOMPARE(Data::logs().move, 0);
    QCOMPARE(Data::logs().refs, 0);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_deprecations_helpers_qpromise::qtSharedPtr()
{
    Data::logs().reset();

    QWeakPointer<Data> wptr;

    {
        QSharedPointer<Data> sptr0(new Data(42));
        const QSharedPointer<Data> sptr1 = sptr0;

        auto p0 = qPromise(QSharedPointer<Data>(new Data(42)));
        auto p1 = qPromise(sptr0);
        auto p2 = qPromise(sptr1);

        Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<QSharedPointer<Data>>>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<QSharedPointer<Data>>>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p2), QPromise<QSharedPointer<Data>>>::value));

        QCOMPARE(waitForValue(p1, QSharedPointer<Data>()), sptr0);
        QCOMPARE(waitForValue(p2, QSharedPointer<Data>()), sptr1);

        wptr = sptr0;

        QCOMPARE(wptr.isNull(), false);
        QCOMPARE(Data::logs().refs, 2);
    }

    QCOMPARE(wptr.isNull(), true);

    QCOMPARE(Data::logs().ctor, 2);
    QCOMPARE(Data::logs().copy, 0);
    QCOMPARE(Data::logs().move, 0);
    QCOMPARE(Data::logs().refs, 0);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_deprecations_helpers_qpromise::stdSharedPtr()
{
    Data::logs().reset();

    std::weak_ptr<Data> wptr;

    {
        std::shared_ptr<Data> sptr0(new Data(42));
        const std::shared_ptr<Data> sptr1 = sptr0;

        auto p0 = qPromise(std::shared_ptr<Data>(new Data(42)));
        auto p1 = qPromise(sptr0);
        auto p2 = qPromise(sptr1);

        Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<std::shared_ptr<Data>>>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<std::shared_ptr<Data>>>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p2), QPromise<std::shared_ptr<Data>>>::value));

        QCOMPARE(waitForValue(p1, std::shared_ptr<Data>()), sptr0);
        QCOMPARE(waitForValue(p2, std::shared_ptr<Data>()), sptr1);

        wptr = sptr0;

        QCOMPARE(wptr.use_count(), 4l);
        QCOMPARE(Data::logs().refs, 2);
    }

    QCOMPARE(wptr.use_count(), 0l);

    QCOMPARE(Data::logs().ctor, 2);
    QCOMPARE(Data::logs().copy, 0);
    QCOMPARE(Data::logs().move, 0);
    QCOMPARE(Data::logs().refs, 0);
}

void tst_deprecations_helpers_qpromise::typedPromise()
{
    auto resolver = [](const QPromiseResolve<int>& resolve) {
        QtPromisePrivate::qtpromise_defer([=](){
            resolve(42);
        });
    };

    QPromise<int> v0(resolver);
    const QPromise<int> v1 = v0;

    auto p0 = qPromise(QPromise<int>(resolver));
    auto p1 = qPromise(v0);
    auto p2 = qPromise(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QPromise<int>>::value));

    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(promise.isPending(), true);
    }
    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(waitForValue(promise, -1), 42);
    }
}

void tst_deprecations_helpers_qpromise::voidPromise()
{
    auto resolver = [](const QPromiseResolve<void>& resolve) {
        QtPromisePrivate::qtpromise_defer([=](){
            resolve();
        });
    };

    QPromise<void> v0(resolver);
    const QPromise<void> v1 = v0;

    auto p0 = qPromise(QPromise<void>(resolver));
    auto p1 = qPromise(v0);
    auto p2 = qPromise(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<void>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<void>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QPromise<void>>::value));

    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(promise.isPending(), true);
    }
    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(waitForValue(promise, -1, 42), 42);
    }
}

void tst_deprecations_helpers_qpromise::typedFuture()
{
    auto fn = [](){ return 42; };
    QFuture<int> v0 = QtConcurrent::run(fn);
    const QFuture<int> v1 = v0;

    auto p0 = qPromise(QtConcurrent::run(fn));
    auto p1 = qPromise(v0);
    auto p2 = qPromise(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QPromise<int>>::value));

    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(promise.isPending(), true);
    }
    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(waitForValue(promise, -1), 42);
    }
}

void tst_deprecations_helpers_qpromise::voidFuture()
{
    auto fn = [](){ };
    QFuture<void> v0 = QtConcurrent::run(fn);
    const QFuture<void> v1 = v0;

    auto p0 = qPromise(QtConcurrent::run(fn));
    auto p1 = qPromise(v0);
    auto p2 = qPromise(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<void>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<void>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QPromise<void>>::value));

    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(promise.isPending(), true);
    }
    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(waitForValue(promise, -1, 42), 42);
    }
}
