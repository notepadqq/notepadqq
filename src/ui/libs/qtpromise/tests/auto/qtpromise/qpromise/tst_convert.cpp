/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/utils.h"

#include <QtPromise>
#include <QtTest>

#include <chrono>

class tst_qpromise_convert : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void fulfillTAsU();
    void fulfillTAsVoid();
    void fulfillTAsQVariant();
    void fulfillQVariantAsU();
    void fulfillQVariantAsVoid();

    void rejectUnconvertibleTypes();
};

QTEST_MAIN(tst_qpromise_convert)
#include "tst_convert.moc"

namespace {
struct Foo
{
    Foo() = default;
    Foo(int foo) : m_foo{foo} { }

    bool operator==(const Foo& rhs) const { return m_foo == rhs.m_foo; }

    int m_foo{-1};
};

struct Bar
{
    Bar() = default;
    Bar(const Foo& other) : m_bar{other.m_foo} { }

    bool operator==(const Bar& rhs) const { return m_bar == rhs.m_bar; }

    int m_bar{-1};
};

enum class Enum1 { Value0, Value1, Value2 };
enum class Enum2 { Value0, Value1, Value2 };
} // namespace

Q_DECLARE_METATYPE(Foo)
Q_DECLARE_METATYPE(Bar)

void tst_qpromise_convert::initTestCase()
{
    // Register converter used by QVariant.
    // https://doc.qt.io/qt-5/qmetatype.html#registerConverter
    QMetaType::registerConverter<Foo, QString>([](const Foo& foo) {
        return QString{"Foo{%1}"}.arg(foo.m_foo);
    });
}

void tst_qpromise_convert::fulfillTAsU()
{
    // Static cast between primitive types.
    {
        auto p = QtPromise::resolve(42.13).convert<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<int>>::value));

        QCOMPARE(waitForValue(p, -1), 42);
        QVERIFY(p.isFulfilled());
    }

    // Convert enum class to int.
    {
        auto p = QtPromise::resolve(Enum1::Value1).convert<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<int>>::value));

        QCOMPARE(waitForValue(p, -1), 1);
        QVERIFY(p.isFulfilled());
    }

    // Convert int to enum class.
    {
        auto p = QtPromise::resolve(1).convert<Enum1>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<Enum1>>::value));

        QCOMPARE(waitForValue(p, Enum1::Value0), Enum1::Value1);
        QVERIFY(p.isFulfilled());
    }

    // Convert between enums
    {
        auto p = QtPromise::resolve(Enum1::Value1).convert<Enum2>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<Enum2>>::value));

        QCOMPARE(waitForValue(p, Enum2::Value0), Enum2::Value1);
        QVERIFY(p.isFulfilled());
    }

    // Converting constructor for Qt types.
    // https://en.cppreference.com/w/cpp/language/converting_constructor
    {
        auto p = QtPromise::resolve(QByteArray{"foo"}).convert<QString>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QString>>::value));

        QCOMPARE(waitForValue(p, QString{}), QString{"foo"});
        QVERIFY(p.isFulfilled());
    }

    // Converting constructor for non-Qt types.
    // https://en.cppreference.com/w/cpp/language/converting_constructor
    {
        auto p = QtPromise::resolve(Foo{42}).convert<Bar>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<Bar>>::value));

        QCOMPARE(waitForValue(p, Bar{}), Bar{42});
        QVERIFY(p.isFulfilled());
    }

    // Conversion of types Qt is aware of via QVariant.
    {
        auto p = QtPromise::resolve(42).convert<QString>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QString>>::value));

        QCOMPARE(waitForValue(p, QString{}), QString{"42"});
        QVERIFY(p.isFulfilled());
    }

    // Conversion of a non-Qt type via QVariant.
    // https://doc.qt.io/qt-5/qmetatype.html#registerConverter
    {
        auto p = QtPromise::resolve(Foo{42}).convert<QString>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QString>>::value));

        QCOMPARE(waitForValue(p, QString{}), QString{"Foo{42}"});
        QVERIFY(p.isFulfilled());
    }
}

void tst_qpromise_convert::fulfillTAsVoid()
{
    auto p = QtPromise::resolve(42).convert<void>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));

    QCOMPARE(waitForValue(p, -1, 42), 42);
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_convert::fulfillTAsQVariant()
{
    // Primitive type to QVariant.
    {
        auto p = QtPromise::resolve(42).convert<QVariant>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVariant>>::value));

        QCOMPARE(waitForValue(p, QVariant{}), QVariant{42});
        QVERIFY(p.isFulfilled());
    }

    // Non-Qt user-defined type to QVariant.
    {
        auto p = QtPromise::resolve(Foo{42}).convert<QVariant>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVariant>>::value));

        QVariant value = waitForValue(p, QVariant{});
        QCOMPARE(value, QVariant::fromValue(Foo{42}));
        QCOMPARE(value.value<Foo>().m_foo, 42);
        QVERIFY(p.isFulfilled());
    }
}

void tst_qpromise_convert::fulfillQVariantAsU()
{
    // Test whether a directly stored value can be extracted.
    {
        auto p = QtPromise::resolve(QVariant{42}).convert<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<int>>::value));

        QCOMPARE(waitForValue(p, -1), 42);
        QVERIFY(p.isFulfilled());
    }

    // Test automatic conversion from string performed by QVariant.
    // https://doc.qt.io/qt-5/qvariant.html#toInt
    {
        auto p = QtPromise::resolve(QVariant{"42"}).convert<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<int>>::value));

        QCOMPARE(waitForValue(p, -1), 42);
        QVERIFY(p.isFulfilled());
    }

    // Non-Qt user-defined type
    {
        auto p = QtPromise::resolve(QVariant::fromValue(Foo{42})).convert<Foo>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<Foo>>::value));

        QCOMPARE(waitForValue(p, Foo{}), Foo{42});
        QVERIFY(p.isFulfilled());
    }
}

void tst_qpromise_convert::fulfillQVariantAsVoid()
{
    auto p = QtPromise::resolve(QVariant{42}).convert<void>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));

    QCOMPARE(waitForValue(p, -1, 42), 42);
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_convert::rejectUnconvertibleTypes()
{
    // A string incompatible with int due to its value.
    {
        auto p = QtPromise::resolve(QString{"42foo"}).convert<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<int>>::value));

        QVERIFY(waitForRejected<QtPromise::QPromiseConversionException>(p));
    }

    // A user-defined type unconvertible to string because there is no converter.
    {
        auto p = QtPromise::resolve(QVariant::fromValue(Bar{42})).convert<QString>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QString>>::value));

        QVERIFY(waitForRejected<QtPromise::QPromiseConversionException>(p));
    }

    // A standard library type unconvertible to a primitive type because there is no converter.
    {
        auto p = QtPromise::resolve(std::vector<int>{42, -42}).convert<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<int>>::value));

        QVERIFY(waitForRejected<QtPromise::QPromiseConversionException>(p));
    }
}
