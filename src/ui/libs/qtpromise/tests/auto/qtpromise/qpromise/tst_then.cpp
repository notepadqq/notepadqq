/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/utils.h"

#include <QtPromise>
#include <QtTest>

#include <functional>

class tst_qpromise_then : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolveSync();
    void resolveAsync();
    void rejectSync();
    void rejectAsync();
    void skipResult();
    void nullHandler();
    void functionPtrHandlers();
    void stdFunctionHandlers();
    void stdBindHandlers();
    void lambdaHandlers();
};

QTEST_MAIN(tst_qpromise_then)
#include "tst_then.moc"

namespace {

const float kRes = 0.42f;
const float kFail = -1.f;

float fnNoArg()
{
    return kRes;
}
float fnArgByVal(float v)
{
    return v;
}
float fnArgByRef(const float& v)
{
    return v;
}

class Klass
{
public: // STATICS
    static float kFnNoArg() { return kRes; }
    static float kFnArgByVal(float v) { return v; }
    static float kFnArgByRef(const float& v) { return v; }

public:
    Klass(float v) : m_v{v} { }

    float fnNoArg() const { return m_v; }
    float fnArgByVal(float v) const { return v + m_v; }
    float fnArgByRef(const float& v) const { return v + m_v; }

private:
    const float m_v;
};

} // namespace

void tst_qpromise_then::resolveSync()
{
    QVariantList values;

    auto input = QtPromise::QPromise<int>::resolve(42);
    auto output = input.then([&](int res) {
        values << res;
        return QString::number(res + 1);
    });

    output
        .then([&](const QString& res) {
            values << res;
        })
        .then([&]() {
            values << 44;
        })
        .wait();

    QCOMPARE(values, (QVariantList{42, QString{"43"}, 44}));
    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(output.isFulfilled(), true);
}

void tst_qpromise_then::resolveAsync()
{
    auto p = QtPromise::QPromise<int>::resolve(42).then([](int res) {
        return QtPromise::QPromise<QString>{
            [=](const QtPromise::QPromiseResolve<QString>& resolve) {
                QtPromisePrivate::qtpromise_defer([=]() {
                    resolve(QString{"foo%1"}.arg(res));
                });
            }};
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QString>>::value));
    QCOMPARE(waitForValue(p, QString{}), QString{"foo42"});
    QCOMPARE(p.isFulfilled(), true);
}

void tst_qpromise_then::rejectSync()
{
    auto input = QtPromise::QPromise<int>::resolve(42);
    auto output = input.then([](int res) {
        throw QString{"foo%1"}.arg(res);
        return 42;
    });

    QString error;
    output
        .then([&](int res) {
            error += "bar" + QString::number(res);
        })
        .fail([&](const QString& err) {
            error += err;
        })
        .wait();

    QCOMPARE(error, QString{"foo42"});
    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(output.isRejected(), true);
}

void tst_qpromise_then::rejectAsync()
{
    auto p = QtPromise::QPromise<int>::resolve(42).then([](int res) {
        return QtPromise::QPromise<void>{[=](const QtPromise::QPromiseResolve<void>&,
                                             const QtPromise::QPromiseReject<void>& reject) {
            QtPromisePrivate::qtpromise_defer([=]() {
                reject(QString{"foo%1"}.arg(res));
            });
        }};
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));
    QCOMPARE(waitForError(p, QString{}), QString{"foo42"});
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_then::skipResult()
{
    auto p = QtPromise::QPromise<int>::resolve(42);

    int value = -1;
    p.then([&]() {
         value = 43;
     }).wait();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<int>>::value));
    QCOMPARE(value, 43);
}

void tst_qpromise_then::nullHandler()
{
    { // resolved
        auto p = QtPromise::QPromise<int>::resolve(42).then(nullptr);

        QCOMPARE(waitForValue(p, -1), 42);
        QCOMPARE(p.isFulfilled(), true);
    }
    { // rejected
        auto p = QtPromise::QPromise<int>::reject(QString{"foo"}).then(nullptr);

        QCOMPARE(waitForError(p, QString{}), QString{"foo"});
        QCOMPARE(p.isRejected(), true);
    }
}

void tst_qpromise_then::functionPtrHandlers()
{
    { // Global functions.
        auto p0 = QtPromise::resolve().then(&fnNoArg);
        auto p1 = QtPromise::resolve(kRes).then(&fnArgByVal);
        auto p2 = QtPromise::resolve(kRes).then(&fnArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // Static member functions.
        auto p0 = QtPromise::resolve().then(&Klass::kFnNoArg);
        auto p1 = QtPromise::resolve(kRes).then(&Klass::kFnArgByVal);
        auto p2 = QtPromise::resolve(kRes).then(&Klass::kFnArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
}

// https://github.com/simonbrunel/qtpromise/issues/29
void tst_qpromise_then::stdFunctionHandlers()
{
    { // lvalue.
        std::function<float()> stdFnNoArg = fnNoArg;
        std::function<float(float)> stdFnArgByVal = fnArgByVal;
        std::function<float(const float&)> stdFnArgByRef = fnArgByRef;

        auto p0 = QtPromise::resolve().then(stdFnNoArg);
        auto p1 = QtPromise::resolve(kRes).then(stdFnArgByVal);
        auto p2 = QtPromise::resolve(kRes).then(stdFnArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // const lvalue.
        const std::function<float()> stdFnNoArg = fnNoArg;
        const std::function<float(float)> stdFnArgByVal = fnArgByVal;
        const std::function<float(const float&)> stdFnArgByRef = fnArgByRef;

        auto p0 = QtPromise::resolve().then(stdFnNoArg);
        auto p1 = QtPromise::resolve(kRes).then(stdFnArgByVal);
        auto p2 = QtPromise::resolve(kRes).then(stdFnArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // rvalue.
        auto p0 = QtPromise::resolve().then(std::function<float()>{fnNoArg});
        auto p1 = QtPromise::resolve(kRes).then(std::function<float(float)>{fnArgByVal});
        auto p2 = QtPromise::resolve(kRes).then(std::function<float(const float&)>{fnArgByRef});

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
}

//// https://github.com/simonbrunel/qtpromise/issues/29
void tst_qpromise_then::stdBindHandlers()
{
    using namespace std::placeholders;

    const float val{42.f};
    const Klass obj{val};

    const std::function<float()> bindNoArg = std::bind(&Klass::fnNoArg, &obj);
    const std::function<float(float)> bindArgByVal = std::bind(&Klass::fnArgByVal, &obj, _1);
    const std::function<float(const float&)> bindArgByRef = std::bind(&Klass::fnArgByRef, &obj, _1);

    auto p0 = QtPromise::resolve().then(bindNoArg);
    auto p1 = QtPromise::resolve(kRes).then(bindArgByVal);
    auto p2 = QtPromise::resolve(kRes).then(bindArgByRef);

    QCOMPARE(waitForValue(p0, kFail), val);
    QCOMPARE(waitForValue(p1, kFail), val + kRes);
    QCOMPARE(waitForValue(p2, kFail), val + kRes);
}

void tst_qpromise_then::lambdaHandlers()
{
    { // lvalue.
        auto lambdaNoArg = []() {
            return kRes;
        };
        auto lambdaArgByVal = [](float v) {
            return v;
        };
        auto lambdaArgByRef = [](const float& v) {
            return v;
        };

        auto p0 = QtPromise::resolve().then(lambdaNoArg);
        auto p1 = QtPromise::resolve(kRes).then(lambdaArgByVal);
        auto p2 = QtPromise::resolve(kRes).then(lambdaArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // const lvalue.
        const auto lambdaNoArg = []() {
            return kRes;
        };
        const auto lambdaArgByVal = [](float v) {
            return v;
        };
        const auto lambdaArgByRef = [](const float& v) {
            return v;
        };

        auto p0 = QtPromise::resolve().then(lambdaNoArg);
        auto p1 = QtPromise::resolve(kRes).then(lambdaArgByVal);
        auto p2 = QtPromise::resolve(kRes).then(lambdaArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // rvalue.
        auto p0 = QtPromise::resolve().then([]() {
            return kRes;
        });
        auto p1 = QtPromise::resolve(kRes).then([](float v) {
            return v;
        });
        auto p2 = QtPromise::resolve(kRes).then([](const float& v) {
            return v;
        });

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
}
