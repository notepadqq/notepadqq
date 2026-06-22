/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include <QtPromise>
#include <QtTest>

using namespace QtPromisePrivate;

class tst_internals_argsof : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void nullFunction();
    void basicFunction();
    void callOperator();
    void stdFunction();
    void lambda();

}; // class tst_argsof

QTEST_MAIN(tst_internals_argsof)
#include "tst_argsof.moc"

#define TEST_ARGS_FOR_TYPE(T, E)                                                                   \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<T>::first, E>::value));                          \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<T&>::first, E>::value));                         \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<T&&>::first, E>::value));                        \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<const T>::first, E>::value));                    \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<const T&>::first, E>::value));                   \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<const T&&>::first, E>::value));                  \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<volatile T>::first, E>::value));                 \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<volatile T&>::first, E>::value));                \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<volatile T&&>::first, E>::value));               \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<const volatile T>::first, E>::value));           \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<const volatile T&>::first, E>::value));          \
    Q_STATIC_ASSERT((std::is_same<typename ArgsOf<const volatile T&&>::first, E>::value));

namespace {

const float kRes = 0.42f;

float fnNoArg()
{
    return kRes;
}
float fnOneArg(float v)
{
    return v;
}
float fnManyArgs(const float& v0, int, char*)
{
    return v0;
}

struct OpNoArg
{
    float operator()() { return kRes; }
};
struct OpOneArg
{
    float operator()(float v) { return v; }
};
struct OpManyArgs
{
    float operator()(const float& v, int, char*) { return v; }
};

struct OpCNoArg
{
    float operator()() const { return kRes; }
};
struct OpCOneArg
{
    float operator()(float v) const { return v; }
};
struct OpCManyArgs
{
    float operator()(const float& v, int, char*) const { return v; }
};

struct OpVNoArg
{
    float operator()() volatile { return kRes; }
};
struct OpVOneArg
{
    float operator()(float v) volatile { return v; }
};
struct OpVManyArgs
{
    float operator()(const float& v, int, char*) volatile { return v; }
};

struct OpCVNoArg
{
    float operator()() const volatile { return kRes; }
};
struct OpCVOneArg
{
    float operator()(float v) const volatile { return v; }
};
struct OpCVManyArgs
{
    float operator()(const float& v, int, char*) const volatile { return v; }
};

} // namespace

void tst_internals_argsof::initTestCase()
{
    Q_UNUSED(fnNoArg())
    Q_UNUSED(fnOneArg(42))
    Q_UNUSED(fnManyArgs(42, 42, nullptr))
}

void tst_internals_argsof::nullFunction()
{
    Q_STATIC_ASSERT((std::is_same<ArgsOf<std::nullptr_t>::first, void>::value));
}

void tst_internals_argsof::basicFunction()
{
    // Function type.
    Q_STATIC_ASSERT((std::is_same<ArgsOf<decltype(fnNoArg)>::first, void>::value));
    Q_STATIC_ASSERT((std::is_same<ArgsOf<decltype(fnOneArg)>::first, float>::value));
    Q_STATIC_ASSERT((std::is_same<ArgsOf<decltype(fnManyArgs)>::first, const float&>::value));

    // Function pointer type.
    Q_STATIC_ASSERT((std::is_same<ArgsOf<decltype(&fnNoArg)>::first, void>::value));
    Q_STATIC_ASSERT((std::is_same<ArgsOf<decltype(&fnOneArg)>::first, float>::value));
    Q_STATIC_ASSERT((std::is_same<ArgsOf<decltype(&fnManyArgs)>::first, const float&>::value));
}

void tst_internals_argsof::callOperator()
{
    // non-const
    TEST_ARGS_FOR_TYPE(OpNoArg, void);
    TEST_ARGS_FOR_TYPE(OpOneArg, float);
    TEST_ARGS_FOR_TYPE(OpManyArgs, const float&);

    // const
    TEST_ARGS_FOR_TYPE(OpCNoArg, void);
    TEST_ARGS_FOR_TYPE(OpCOneArg, float);
    TEST_ARGS_FOR_TYPE(OpCManyArgs, const float&);

    // volatile
    TEST_ARGS_FOR_TYPE(OpVNoArg, void);
    TEST_ARGS_FOR_TYPE(OpVOneArg, float);
    TEST_ARGS_FOR_TYPE(OpVManyArgs, const float&);

    // const volatile
    TEST_ARGS_FOR_TYPE(OpCVNoArg, void);
    TEST_ARGS_FOR_TYPE(OpCVOneArg, float);
    TEST_ARGS_FOR_TYPE(OpCVManyArgs, const float&);
}

void tst_internals_argsof::stdFunction()
{
    TEST_ARGS_FOR_TYPE(std::function<void()>, void);
    TEST_ARGS_FOR_TYPE(std::function<void(float)>, float);
    TEST_ARGS_FOR_TYPE(std::function<void(const float&, int, char**)>, const float&);
}

void tst_internals_argsof::lambda()
{
    auto lNoArg = []() {};
    auto lOneArg = [](float) {};
    auto lManyArgs = [](const float&, int, char**) {};
    auto lMutable = [](const float&, int, char**) mutable {};

    TEST_ARGS_FOR_TYPE(decltype(lNoArg), void);
    TEST_ARGS_FOR_TYPE(decltype(lOneArg), float);
    TEST_ARGS_FOR_TYPE(decltype(lManyArgs), const float&);
    TEST_ARGS_FOR_TYPE(decltype(lMutable), const float&);
}
