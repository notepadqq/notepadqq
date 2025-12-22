#ifndef QTPROMISE_QPROMISERESOLVER_H
#define QTPROMISE_QPROMISERESOLVER_H

#include "qpromiseexceptions.h"

// Qt
#include <QExplicitlySharedDataPointer>

namespace QtPromise {

template <typename T> class QPromise;

} // namespace QtPromise

namespace QtPromisePrivate {

template <typename T>
class PromiseResolver
{
public:
    PromiseResolver(QtPromise::QPromise<T> promise)
        : m_d(new Data())
    {
        m_d->promise = new QtPromise::QPromise<T>(std::move(promise));
    }

    template <typename E>
    void reject(E&& error)
    {
        auto promise = m_d->promise;
        if (promise) {
            Q_ASSERT(promise->isPending());
            promise->m_d->reject(std::forward<E>(error));
            promise->m_d->dispatch();
            release();
        }
    }

    void reject()
    {
        auto promise = m_d->promise;
        if (promise) {
            Q_ASSERT(promise->isPending());
            promise->m_d->reject(QtPromise::QPromiseUndefinedException());
            promise->m_d->dispatch();
            release();
        }
    }

    template <typename V>
    void resolve(V&& value)
    {
        auto promise = m_d->promise;
        if (promise) {
            Q_ASSERT(promise->isPending());
            promise->m_d->resolve(std::forward<V>(value));
            promise->m_d->dispatch();
            release();
        }
    }

    void resolve()
    {
        auto promise = m_d->promise;
        if (promise) {
            Q_ASSERT(promise->isPending());
            promise->m_d->resolve();
            promise->m_d->dispatch();
            release();
        }
    }

private:
    struct Data : public QSharedData
    {
        QtPromise::QPromise<T>* promise = nullptr;
    };

    QExplicitlySharedDataPointer<Data> m_d;

    void release()
    {
        Q_ASSERT(m_d->promise);
        Q_ASSERT(!m_d->promise->isPending());
        delete m_d->promise;
        m_d->promise = nullptr;
    }
};

} // QtPromisePrivate

namespace QtPromise {

template <class T>
class QPromiseResolve
{
public:
    QPromiseResolve(QtPromisePrivate::PromiseResolver<T> resolver)
        : m_resolver(std::move(resolver))
    { }

    template <typename V>
    void operator()(V&& value) const
    {
        m_resolver.resolve(std::forward<V>(value));
    }

    void operator()() const
    {
        m_resolver.resolve();
    }

private:
    mutable QtPromisePrivate::PromiseResolver<T> m_resolver;
};

template <class T>
class QPromiseReject
{
public:
    QPromiseReject(QtPromisePrivate::PromiseResolver<T> resolver)
        : m_resolver(std::move(resolver))
    { }

    template <typename E>
    void operator()(E&& error) const
    {
        m_resolver.reject(std::forward<E>(error));
    }

    void operator()() const
    {
        m_resolver.reject();
    }

private:
    mutable QtPromisePrivate::PromiseResolver<T> m_resolver;
};

} // namespace QtPromise

#endif // QTPROMISE_QPROMISERESOLVER_H
