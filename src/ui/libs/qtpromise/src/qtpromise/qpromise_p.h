#ifndef QTPROMISE_QPROMISE_P_H
#define QTPROMISE_QPROMISE_P_H

#include "qpromiseglobal.h"

// Qt
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QThread>
#include <QVector>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QSharedData>
#include <QPointer>

namespace QtPromise {

template <typename T>
class QPromise;

template <typename T>
class QPromiseResolve;

template <typename T>
class QPromiseReject;

} // namespace QtPromise

namespace QtPromisePrivate {

// https://stackoverflow.com/a/21653558
template <typename F>
static void qtpromise_defer(F&& f, const QPointer<QThread>& thread)
{
    using FType = typename std::decay<F>::type;

    struct Event : public QEvent
    {
        Event(FType&& f) : QEvent(QEvent::None), m_f(std::move(f)) { }
        Event(const FType& f) : QEvent(QEvent::None), m_f(f) { }
        ~Event() { m_f(); }
        FType m_f;
    };

    if (!thread || thread->isFinished()) {
        // Make sure to not call `f` if the captured thread doesn't exist anymore,
        // which would potentially result in dispatching to the wrong thread (ie.
        // nullptr == current thread). Since the target thread is gone, it should
        // be safe to simply skip that notification.
        return;
    }

    QObject* target = QAbstractEventDispatcher::instance(thread);
    if (!target && QCoreApplication::closingDown()) {
        // When the app is shutting down, the even loop is not anymore available
        // so we don't have any way to dispatch `f`. This case can happen when a
        // promise is resolved after the app is requested to close, in which case
        // we should not trigger any error and skip that notification.
        return;
    }

    Q_ASSERT_X(target, "postMetaCall", "Target thread must have an event loop");
    QCoreApplication::postEvent(target, new Event(std::forward<F>(f)));
}

template <typename F>
static void qtpromise_defer(F&& f)
{
    Q_ASSERT(QThread::currentThread());
    qtpromise_defer(std::forward<F>(f), QThread::currentThread());
}

template <typename T>
class PromiseValue
{
public:
    PromiseValue() { }
    PromiseValue(const T& data) : m_data(new T(data)) { }
    PromiseValue(T&& data) : m_data(new T(std::move(data))) { }
    bool isNull() const { return m_data.isNull(); }
    const T& data() const { return *m_data; }

private:
    QSharedPointer<T> m_data;
};

class PromiseError
{
public:
    template <typename T>
    PromiseError(const T& value)
    {
        try {
            throw value;
        } catch (...) {
            m_data = std::current_exception();
        }
    }

    PromiseError() { }
    PromiseError(const std::exception_ptr& exception) : m_data(exception) { }
    void rethrow() const { std::rethrow_exception(m_data); }
    bool isNull() const { return m_data == nullptr; }

private:
    // NOTE(SB) std::exception_ptr is already a shared pointer
    std::exception_ptr m_data;
};

template <typename T>
struct PromiseDeduce
{
    using Type = QtPromise::QPromise<T>;
};

template <typename T>
struct PromiseDeduce<T&>
    : public PromiseDeduce<T>
{ };

template <typename T>
struct PromiseDeduce<const T>
    : public PromiseDeduce<T>
{ };

template <typename T>
struct PromiseDeduce<const volatile T>
    : public PromiseDeduce<T>
{ };

template <typename T>
struct PromiseDeduce<QtPromise::QPromise<T>>
    : public PromiseDeduce<T>
{ };

template <typename Functor, typename... Args>
struct PromiseFunctor
{
    using ResultType = typename std::result_of<Functor(Args...)>::type;
    using PromiseType = typename PromiseDeduce<ResultType>::Type;
};

template <typename T>
struct PromiseFulfill
{
    template <typename V, typename TResolve, typename TReject>
    static void call(V&& value, const TResolve& resolve, const TReject&)
    {
        resolve(std::forward<V>(value));
    }
};

template <typename T>
struct PromiseFulfill<QtPromise::QPromise<T>>
{
    template <typename TResolve, typename TReject>
    static void call(
        const QtPromise::QPromise<T>& promise,
        const TResolve& resolve,
        const TReject& reject)
    {
        if (promise.isFulfilled()) {
            resolve(promise.m_d->value());
        } else if (promise.isRejected()) {
            reject(promise.m_d->error());
        } else {
            promise.then([=]() {
                resolve(promise.m_d->value());
            }, [=]() { // catch all
                reject(promise.m_d->error());
            });
        }
    }
};

template <>
struct PromiseFulfill<QtPromise::QPromise<void>>
{
    template <typename TPromise, typename TResolve, typename TReject>
    static void call(
        const TPromise& promise,
        const TResolve& resolve,
        const TReject& reject)
    {
        if (promise.isFulfilled()) {
            resolve();
        } else if (promise.isRejected()) {
            reject(promise.m_d->error());
        } else {
            promise.then([=]() {
                resolve();
            }, [=]() { // catch all
                reject(promise.m_d->error());
            });
        }
    }
};

template <typename Result>
struct PromiseDispatch
{
    template <typename Resolve, typename Reject, typename Functor, typename... Args>
    static void call(const Resolve& resolve, const Reject& reject, Functor fn, Args&&... args)
    {
        try {
            PromiseFulfill<Unqualified<Result>>::call(
                fn(std::forward<Args>(args)...),
                resolve,
                reject);
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

template <>
struct PromiseDispatch<void>
{
    template <typename Resolve, typename Reject, typename Functor, typename... Args>
    static void call(const Resolve& resolve, const Reject& reject, Functor fn, Args&&... args)
    {
        try {
            fn(std::forward<Args>(args)...);
            resolve();
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

template <typename T, typename THandler, typename TArg = typename ArgsOf<THandler>::first>
struct PromiseHandler
{
    using ResType = typename std::result_of<THandler(T)>::type;
    using Promise = typename PromiseDeduce<ResType>::Type;

    template <typename TResolve, typename TReject>
    static std::function<void(const T&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const T& value) {
            PromiseDispatch<ResType>::call(resolve, reject, handler, value);
        };
    }
};

template <typename T, typename THandler>
struct PromiseHandler<T, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;
    using Promise = typename PromiseDeduce<ResType>::Type;

    template <typename TResolve, typename TReject>
    static std::function<void(const T&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const T&) {
            PromiseDispatch<ResType>::call(resolve, reject, handler);
        };
    }
};

template <typename THandler>
struct PromiseHandler<void, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;
    using Promise = typename PromiseDeduce<ResType>::Type;

    template <typename TResolve, typename TReject>
    static std::function<void()> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=]() {
            PromiseDispatch<ResType>::call(resolve, reject, handler);
        };
    }
};

template <typename T>
struct PromiseHandler<T, std::nullptr_t, void>
{
    using Promise = QtPromise::QPromise<T>;

    template <typename TResolve, typename TReject>
    static std::function<void(const T&)> create(
        std::nullptr_t,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const T& value) {
            // 2.2.7.3. If onFulfilled is not a function and promise1 is fulfilled,
            // promise2 must be fulfilled with the same value as promise1.
            PromiseFulfill<T>::call(std::move(T(value)), resolve, reject);
        };
    }
};

template <>
struct PromiseHandler<void, std::nullptr_t, void>
{
    using Promise = QtPromise::QPromise<void>;

    template <typename TResolve, typename TReject>
    static std::function<void()> create(
        std::nullptr_t,
        const TResolve& resolve,
        const TReject&)
    {
        return [=]() {
            // 2.2.7.3. If onFulfilled is not a function and promise1 is fulfilled,
            // promise2 must be fulfilled with the same value as promise1.
            resolve();
        };
    }
};

template <typename T, typename THandler, typename TArg = typename ArgsOf<THandler>::first>
struct PromiseCatcher
{
    using ResType = typename std::result_of<THandler(TArg)>::type;

    template <typename TResolve, typename TReject>
    static std::function<void(const PromiseError&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const PromiseError& error) {
            try {
                error.rethrow();
            } catch (const TArg& error) {
                PromiseDispatch<ResType>::call(resolve, reject, handler, error);
            } catch (...) {
                reject(std::current_exception());
            }
        };
    }
};

template <typename T, typename THandler>
struct PromiseCatcher<T, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;

    template <typename TResolve, typename TReject>
    static std::function<void(const PromiseError&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const PromiseError& error) {
            try {
                error.rethrow();
            } catch (...) {
                PromiseDispatch<ResType>::call(resolve, reject, handler);
            }
        };
    }
};

template <typename T>
struct PromiseCatcher<T, std::nullptr_t, void>
{
    template <typename TResolve, typename TReject>
    static std::function<void(const PromiseError&)> create(
        std::nullptr_t,
        const TResolve&,
        const TReject& reject)
    {
        return [=](const PromiseError& error) {
            // 2.2.7.4. If onRejected is not a function and promise1 is rejected,
            // promise2 must be rejected with the same reason as promise1
            reject(error);
        };
    }
};

template <typename T, typename F>
struct PromiseMapper
{ };

template <typename T, typename F, template <typename, typename...> class Sequence, typename ...Args>
struct PromiseMapper<Sequence<T, Args...>, F>
{
    using ReturnType = typename std::result_of<F(T, int)>::type;
    using ResultType = QVector<typename PromiseDeduce<ReturnType>::Type::Type>;
    using PromiseType = QtPromise::QPromise<ResultType>;
};

template <typename T> class PromiseData;

template <typename T, typename F>
class PromiseDataBase : public QSharedData
{
public:
    using Handler = std::pair<QPointer<QThread>, std::function<F>>;
    using Catcher = std::pair<QPointer<QThread>, std::function<void(const PromiseError&)>>;

    virtual ~PromiseDataBase() {}

    bool isFulfilled() const
    {
        return !isPending() && m_error.isNull();
    }

    bool isRejected() const
    {
        return !isPending() && !m_error.isNull();
    }

    bool isPending() const
    {
        QReadLocker lock(&m_lock);
        return !m_settled;
    }

    void addHandler(std::function<F> handler)
    {
        QWriteLocker lock(&m_lock);
        m_handlers.append({QThread::currentThread(), std::move(handler)});
    }

    void addCatcher(std::function<void(const PromiseError&)> catcher)
    {
        QWriteLocker lock(&m_lock);
        m_catchers.append({QThread::currentThread(), std::move(catcher)});
    }

    template <typename E>
    void reject(E&& error)
    {
        Q_ASSERT(isPending());
        Q_ASSERT(m_error.isNull());
        m_error = PromiseError(std::forward<E>(error));
        setSettled();
    }

    const PromiseError& error() const
    {
        Q_ASSERT(isRejected());
        return m_error;
    }

    void dispatch()
    {
        if (isPending()) {
            return;
        }

        // A promise can't be resolved multiple times so once settled, its state can't
        // change. When fulfilled, handlers must be called (a single time) and catchers
        // ignored indefinitely (or vice-versa when the promise is rejected), so make
        // sure to clear both handlers AND catchers when dispatching. This also prevents
        // shared pointer circular reference memory leaks when the owning promise is
        // captured in the handler and/or catcher lambdas.

        m_lock.lockForWrite();
        QVector<Handler> handlers(std::move(m_handlers));
        QVector<Catcher> catchers(std::move(m_catchers));
        m_lock.unlock();

        if (m_error.isNull()) {
            notify(handlers);
            return;
        }

        PromiseError error(m_error);
        Q_ASSERT(!error.isNull());

        for (const auto& catcher: catchers) {
            const auto& fn = catcher.second;
            qtpromise_defer([=]() {
                fn(error);
            }, catcher.first);
        }
    }

protected:
    mutable QReadWriteLock m_lock;

    void setSettled()
    {
        QWriteLocker lock(&m_lock);
        Q_ASSERT(!m_settled);
        m_settled = true;
    }

    virtual void notify(const QVector<Handler>&) = 0;

private:
    bool m_settled = false;
    QVector<Handler> m_handlers;
    QVector<Catcher> m_catchers;
    PromiseError m_error;
};

template <typename T>
class PromiseData : public PromiseDataBase<T, void(const T&)>
{
    using Handler = typename PromiseDataBase<T, void(const T&)>::Handler;

public:
    template <typename V>
    void resolve(V&& value)
    {
        Q_ASSERT(this->isPending());
        Q_ASSERT(m_value.isNull());
        m_value = PromiseValue<T>(std::forward<V>(value));
        this->setSettled();
    }

    const PromiseValue<T>& value() const
    {
        Q_ASSERT(this->isFulfilled());
        return m_value;
    }

    void notify(const QVector<Handler>& handlers) Q_DECL_OVERRIDE
    {
        PromiseValue<T> value(m_value);
        Q_ASSERT(!value.isNull());

        for (const auto& handler: handlers) {
            const auto& fn = handler.second;
            qtpromise_defer([=]() {
                fn(value.data());
            }, handler.first);
        }
    }

private:
    PromiseValue<T> m_value;
};

template <>
class PromiseData<void> : public PromiseDataBase<void, void()>
{
    using Handler = PromiseDataBase<void, void()>::Handler;

public:
    void resolve()
    {
        setSettled();
    }

protected:
    void notify(const QVector<Handler>& handlers) Q_DECL_OVERRIDE
    {
        for (const auto& handler: handlers) {
            qtpromise_defer(handler.second, handler.first);
        }
    }
};

struct PromiseInspect
{
    template <typename T>
    static inline PromiseData<T>* get(const QtPromise::QPromise<T>& p)
    {
        return p.m_d.data();
    }
};

} // namespace QtPromisePrivate

#endif // QTPROMISE_QPROMISE_H
