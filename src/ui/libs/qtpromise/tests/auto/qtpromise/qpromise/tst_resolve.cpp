/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/data.h"
#include "../shared/utils.h"

#include <QtPromise>
#include <QtTest>

#include <memory>

class tst_qpromise_resolve : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void value();
    void noValue();
    void moveRValue();
    void copyLValue();
    void qtSharedPtr();
    void stdSharedPtr();
};

QTEST_MAIN(tst_qpromise_resolve)
#include "tst_resolve.moc"

void tst_qpromise_resolve::value()
{
    int v0 = 42;
    const int v1 = 42;

    auto p0 = QtPromise::QPromise<int>::resolve(42);
    auto p1 = QtPromise::QPromise<int>::resolve(v0);
    auto p2 = QtPromise::QPromise<int>::resolve(v1);

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

void tst_qpromise_resolve::noValue()
{
    auto p = QtPromise::QPromise<void>::resolve();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
}

void tst_qpromise_resolve::moveRValue()
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

void tst_qpromise_resolve::copyLValue()
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
void tst_qpromise_resolve::qtSharedPtr()
{
    using DataSPtr = QSharedPointer<Data>;

    Data::logs().reset();

    QWeakPointer<Data> wptr;

    {
        auto sptr0 = DataSPtr::create(42);
        const DataSPtr sptr1 = sptr0;

        auto p0 = QtPromise::QPromise<DataSPtr>::resolve(DataSPtr::create(42));
        auto p1 = QtPromise::QPromise<DataSPtr>::resolve(sptr0);
        auto p2 = QtPromise::QPromise<DataSPtr>::resolve(sptr1);

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
void tst_qpromise_resolve::stdSharedPtr()
{
    using DataSPtr = std::shared_ptr<Data>;

    Data::logs().reset();

    std::weak_ptr<Data> wptr;

    {
        auto sptr0 = std::make_shared<Data>(42);
        const DataSPtr sptr1 = sptr0;

        auto p0 = QtPromise::QPromise<DataSPtr>::resolve(std::make_shared<Data>(42));
        auto p1 = QtPromise::QPromise<DataSPtr>::resolve(sptr0);
        auto p2 = QtPromise::QPromise<DataSPtr>::resolve(sptr1);

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
