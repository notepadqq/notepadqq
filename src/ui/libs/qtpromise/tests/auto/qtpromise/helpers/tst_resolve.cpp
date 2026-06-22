/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/data.h"
#include "../shared/utils.h"

#include <QtConcurrent>
#include <QtPromise>
#include <QtTest>

#include <memory>

class tst_helpers_resolve : public QObject
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

QTEST_MAIN(tst_helpers_resolve)
#include "tst_resolve.moc"

void tst_helpers_resolve::value()
{
    int v0 = 42;
    const int v1 = 42;

    auto p0 = QtPromise::resolve(42);
    auto p1 = QtPromise::resolve(v0);
    auto p2 = QtPromise::resolve(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QtPromise::QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QtPromise::QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QtPromise::QPromise<int>>::value));

    for (const auto& p : {p0, p1, p2}) {
        QCOMPARE(p.isFulfilled(), true);
    }
    for (const auto& p : {p0, p1, p2}) {
        QCOMPARE(waitForValue(p, -1), 42);
    }
}

void tst_helpers_resolve::noValue()
{
    auto p = QtPromise::resolve();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
}

void tst_helpers_resolve::moveRValue()
{
    Data::logs().reset();

    {
        auto p = QtPromise::resolve(Data{42}).wait();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<Data>>::value));
    }

    QCOMPARE(Data::logs().ctor, 1);
    QCOMPARE(Data::logs().copy, 0);
    QCOMPARE(Data::logs().move, 1);
    QCOMPARE(Data::logs().refs, 0);
}

void tst_helpers_resolve::copyLValue()
{
    Data::logs().reset();

    {
        Data value{42};
        auto p = QtPromise::resolve(value).wait();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<Data>>::value));
    }

    QCOMPARE(Data::logs().ctor, 1);
    QCOMPARE(Data::logs().copy, 1);
    QCOMPARE(Data::logs().move, 0);
    QCOMPARE(Data::logs().refs, 0);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_helpers_resolve::qtSharedPtr()
{
    using DataSPtr = QSharedPointer<Data>;

    Data::logs().reset();

    QWeakPointer<Data> wptr;

    {
        auto sptr0 = DataSPtr::create(42);
        const DataSPtr sptr1 = sptr0;

        auto p0 = QtPromise::resolve(DataSPtr::create(42));
        auto p1 = QtPromise::resolve(sptr0);
        auto p2 = QtPromise::resolve(sptr1);

        Q_STATIC_ASSERT((std::is_same<decltype(p0), QtPromise::QPromise<DataSPtr>>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p1), QtPromise::QPromise<DataSPtr>>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p2), QtPromise::QPromise<DataSPtr>>::value));

        QCOMPARE(waitForValue(p1, DataSPtr{}), sptr0);
        QCOMPARE(waitForValue(p2, DataSPtr{}), sptr1);

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
void tst_helpers_resolve::stdSharedPtr()
{
    using DataSPtr = std::shared_ptr<Data>;

    Data::logs().reset();

    std::weak_ptr<Data> wptr;

    {
        auto sptr0 = std::make_shared<Data>(42);
        const DataSPtr sptr1 = sptr0;

        auto p0 = QtPromise::resolve(std::make_shared<Data>(42));
        auto p1 = QtPromise::resolve(sptr0);
        auto p2 = QtPromise::resolve(sptr1);

        Q_STATIC_ASSERT((std::is_same<decltype(p0), QtPromise::QPromise<DataSPtr>>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p1), QtPromise::QPromise<DataSPtr>>::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p2), QtPromise::QPromise<DataSPtr>>::value));

        QCOMPARE(waitForValue(p1, DataSPtr{}), sptr0);
        QCOMPARE(waitForValue(p2, DataSPtr{}), sptr1);

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

void tst_helpers_resolve::typedPromise()
{
    auto resolver = [](const QtPromise::QPromiseResolve<int>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve(42);
        });
    };

    QtPromise::QPromise<int> v0{resolver};
    const QtPromise::QPromise<int> v1 = v0;

    auto p0 = QtPromise::resolve(QtPromise::QPromise<int>{resolver});
    auto p1 = QtPromise::resolve(v0);
    auto p2 = QtPromise::resolve(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QtPromise::QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QtPromise::QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QtPromise::QPromise<int>>::value));

    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(promise.isPending(), true);
    }
    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(waitForValue(promise, -1), 42);
    }
}

void tst_helpers_resolve::voidPromise()
{
    auto resolver = [](const QtPromise::QPromiseResolve<void>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve();
        });
    };

    QtPromise::QPromise<void> v0{resolver};
    const QtPromise::QPromise<void> v1 = v0;

    auto p0 = QtPromise::resolve(QtPromise::QPromise<void>{resolver});
    auto p1 = QtPromise::resolve(v0);
    auto p2 = QtPromise::resolve(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QtPromise::QPromise<void>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QtPromise::QPromise<void>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QtPromise::QPromise<void>>::value));

    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(promise.isPending(), true);
    }
    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(waitForValue(promise, -1, 42), 42);
    }
}

void tst_helpers_resolve::typedFuture()
{
    auto fn = []() {
        return 42;
    };
    QFuture<int> v0 = QtConcurrent::run(fn);
    const QFuture<int> v1 = v0;

    auto p0 = QtPromise::resolve(QtConcurrent::run(fn));
    auto p1 = QtPromise::resolve(v0);
    auto p2 = QtPromise::resolve(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QtPromise::QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QtPromise::QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QtPromise::QPromise<int>>::value));

    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(promise.isPending(), true);
    }
    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(waitForValue(promise, -1), 42);
    }
}

void tst_helpers_resolve::voidFuture()
{
    auto fn = []() {};
    QFuture<void> v0 = QtConcurrent::run(fn);
    const QFuture<void> v1 = v0;

    auto p0 = QtPromise::resolve(QtConcurrent::run(fn));
    auto p1 = QtPromise::resolve(v0);
    auto p2 = QtPromise::resolve(v1);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QtPromise::QPromise<void>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QtPromise::QPromise<void>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p2), QtPromise::QPromise<void>>::value));

    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(promise.isPending(), true);
    }
    for (const auto& promise : {p0, p1, p2}) {
        QCOMPARE(waitForValue(promise, -1, 42), 42);
    }
}
