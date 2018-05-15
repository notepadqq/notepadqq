#ifndef QTPROMISE_QPROMISE_P_H
#define QTPROMISE_QPROMISE_P_H

// QtPromise
#include "qpromiseerror.h"
#include "qpromiseglobal.h"

// Qt
#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QPointer>
#include <QReadWriteLock>
#include <QSharedData>
#include <QSharedPointer>
#include <QThread>
#include <QVector>

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
    struct Event : public QEvent
    {
        using FType = typename std::decay<F>::type;
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
struct PromiseDeduce
{
    using Type = QtPromise::QPromise<Unqualified<T> >;
};

template <typename T>
struct PromiseDeduce<QtPromise::QPromise<T> >
    : public PromiseDeduce<T>
{ };

template <typename T>
struct PromiseFulfill
{
    static void call(
        T&& value,
        const QtPromise::QPromiseResolve<T>& resolve,
        const QtPromise::QPromiseReject<T>&)
    {
        resolve(std::move(value));
    }
};

template <typename T>
struct PromiseFulfill<QtPromise::QPromise<T> >
{
    static void call(
        const QtPromise::QPromise<T>& promise,
        const QtPromise::QPromiseResolve<T>& resolve,
        const QtPromise::QPromiseReject<T>& reject)
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
struct PromiseFulfill<QtPromise::QPromise<void> >
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

template <typename T, typename TRes>
struct PromiseDispatch
{
    using Promise = typename PromiseDeduce<TRes>::Type;
    using ResType = Unqualified<TRes>;

    template <typename THandler, typename TResolve, typename TReject>
    static void call(const T& value, THandler handler, const TResolve& resolve, const TReject& reject)
    {
        try {
            PromiseFulfill<ResType>::call(handler(value), resolve, reject);
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

template <typename T>
struct PromiseDispatch<T, void>
{
    using Promise = QtPromise::QPromise<void>;

    template <typename THandler, typename TResolve, typename TReject>
    static void call(const T& value, THandler handler, const TResolve& resolve, const TReject& reject)
    {
        try {
            handler(value);
            resolve();
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

template <typename TRes>
struct PromiseDispatch<void, TRes>
{
    using Promise = typename PromiseDeduce<TRes>::Type;
    using ResType = Unqualified<TRes>;

    template <typename THandler, typename TResolve, typename TReject>
    static void call(THandler handler, const TResolve& resolve, const TReject& reject)
    {
        try {
            PromiseFulfill<ResType>::call(handler(), resolve, reject);
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

template <>
struct PromiseDispatch<void, void>
{
    using Promise = QtPromise::QPromise<void>;

    template <typename THandler, typename TResolve, typename TReject>
    static void call(THandler handler, const TResolve& resolve, const TReject& reject)
    {
        try {
            handler();
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
    using Promise = typename PromiseDispatch<T, ResType>::Promise;

    template <typename TResolve, typename TReject>
    static std::function<void(const T&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const T& value) {
            PromiseDispatch<T, ResType>::call(value, std::move(handler), resolve, reject);
        };
    }
};

template <typename T, typename THandler>
struct PromiseHandler<T, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;
    using Promise = typename PromiseDispatch<T, ResType>::Promise;

    template <typename TResolve, typename TReject>
    static std::function<void(const T&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const T&) {
            PromiseDispatch<void, ResType>::call(handler, resolve, reject);
        };
    }
};

template <typename THandler>
struct PromiseHandler<void, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;
    using Promise = typename PromiseDispatch<void, ResType>::Promise;

    template <typename TResolve, typename TReject>
    static std::function<void()> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=]() {
            PromiseDispatch<void, ResType>::call(handler, resolve, reject);
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
    static std::function<void(const QtPromise::QPromiseError&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const QtPromise::QPromiseError& error) {
            try {
                error.rethrow();
            } catch (const TArg& error) {
                PromiseDispatch<TArg, ResType>::call(error, handler, resolve, reject);
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
    static std::function<void(const QtPromise::QPromiseError&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const QtPromise::QPromiseError& error) {
            try {
                error.rethrow();
            } catch (...) {
                PromiseDispatch<void, ResType>::call(handler, resolve, reject);
            }
        };
    }
};

template <typename T>
struct PromiseCatcher<T, std::nullptr_t, void>
{
    template <typename TResolve, typename TReject>
    static std::function<void(const QtPromise::QPromiseError&)> create(
        std::nullptr_t,
        const TResolve&,
        const TReject& reject)
    {
        return [=](const QtPromise::QPromiseError& error) {
            // 2.2.7.4. If onRejected is not a function and promise1 is rejected,
            // promise2 must be rejected with the same reason as promise1
            reject(error);
        };
    }
};

template <typename T> class PromiseData;

template <typename T, typename F>
class PromiseDataBase : public QSharedData
{
public:
    using Error = QtPromise::QPromiseError;
    using Handler = std::pair<QPointer<QThread>, std::function<F> >;
    using Catcher = std::pair<QPointer<QThread>, std::function<void(const Error&)> >;

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

    void addCatcher(std::function<void(const Error&)> catcher)
    {
        QWriteLocker lock(&m_lock);
        m_catchers.append({QThread::currentThread(), std::move(catcher)});
    }

    void reject(Error error)
    {
        Q_ASSERT(isPending());
        Q_ASSERT(m_error.isNull());
        m_error.reset(new Error(std::move(error)));
        setSettled();
    }

    void reject(const QSharedPointer<Error>& error)
    {
        Q_ASSERT(isPending());
        Q_ASSERT(m_error.isNull());
        m_error = error;
        this->setSettled();
    }

    const QSharedPointer<Error>& error() const
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

        QSharedPointer<Error> error = m_error;
        Q_ASSERT(!error.isNull());

        for (const auto& catcher: catchers) {
            const auto& fn = catcher.second;
            qtpromise_defer([=]() {
                fn(*error);
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
    QSharedPointer<Error> m_error;
};

template <typename T>
class PromiseData : public PromiseDataBase<T, void(const T&)>
{
    using Handler = typename PromiseDataBase<T, void(const T&)>::Handler;

public:
    void resolve(T&& value)
    {
        Q_ASSERT(this->isPending());
        Q_ASSERT(m_value.isNull());
        m_value.reset(new T(std::move(value)));
        this->setSettled();
    }

    void resolve(const T& value)
    {
        Q_ASSERT(this->isPending());
        Q_ASSERT(m_value.isNull());
        m_value.reset(new T(value));
        this->setSettled();
    }

    void resolve(const QSharedPointer<T>& value)
    {
        Q_ASSERT(this->isPending());
        Q_ASSERT(m_value.isNull());
        m_value = value;
        this->setSettled();
    }

    const QSharedPointer<T>& value() const
    {
        Q_ASSERT(this->isFulfilled());
        return m_value;
    }

    void notify(const QVector<Handler>& handlers) Q_DECL_OVERRIDE
    {
        QSharedPointer<T> value(m_value);
        Q_ASSERT(!value.isNull());

        for (const auto& handler: handlers) {
            const auto& fn = handler.second;
            qtpromise_defer([=]() {
                fn(*value);
            }, handler.first);
        }
    }

private:
    QSharedPointer<T> m_value;
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

} // namespace QtPromise

#endif // ifndef QTPROMISE_QPROMISE_H
