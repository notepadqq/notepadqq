#ifndef QTPROMISE_QPROMISE_H
#define QTPROMISE_QPROMISE_H

#include "qpromise_p.h"
#include "qpromiseexceptions.h"
#include "qpromiseglobal.h"
#include "qpromiseresolver.h"

// Qt
#include <QExplicitlySharedDataPointer>

namespace QtPromise {

template <typename T>
class QPromiseBase
{
public:
    using Type = T;

    template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count == 1, int>::type = 0>
    inline QPromiseBase(F resolver);

    template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count != 1, int>::type = 0>
    inline QPromiseBase(F resolver);

    QPromiseBase(const QPromiseBase<T>& other): m_d(other.m_d) {}
    QPromiseBase(const QPromise<T>& other): m_d(other.m_d) {}
    QPromiseBase(QPromiseBase<T>&& other) Q_DECL_NOEXCEPT { swap(other); }

    virtual ~QPromiseBase() { }

    QPromiseBase<T>& operator=(const QPromiseBase<T>& other) { m_d = other.m_d; return *this;}
    QPromiseBase<T>& operator=(QPromiseBase<T>&& other) Q_DECL_NOEXCEPT
    { QPromiseBase<T>(std::move(other)).swap(*this); return *this; }

    bool operator==(const QPromiseBase<T>& other) const { return (m_d == other.m_d); }
    bool operator!=(const QPromiseBase<T>& other) const { return (m_d != other.m_d); }

    void swap(QPromiseBase<T>& other) Q_DECL_NOEXCEPT { qSwap(m_d, other.m_d); }

    bool isFulfilled() const { return m_d->isFulfilled(); }
    bool isRejected() const { return m_d->isRejected(); }
    bool isPending() const { return m_d->isPending(); }

    template <typename TFulfilled, typename TRejected>
    inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
    then(const TFulfilled& fulfilled, const TRejected& rejected) const;

    template <typename TFulfilled>
    inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
    then(TFulfilled&& fulfilled) const;

    template <typename TRejected>
    inline typename QtPromisePrivate::PromiseHandler<T, std::nullptr_t>::Promise
    fail(TRejected&& rejected) const;

    template <typename THandler>
    inline QPromise<T> finally(THandler handler) const;

    template <typename THandler>
    inline QPromise<T> tap(THandler handler) const;

    template <typename THandler>
    inline QPromise<T> tapFail(THandler handler) const;

    template <typename E = QPromiseTimeoutException>
    inline QPromise<T> timeout(int msec, E&& error = E()) const;

    inline QPromise<T> delay(int msec) const;
    inline QPromise<T> wait() const;

public: // STATIC
    template <typename E>
    inline static QPromise<T> reject(E&& error);

protected:
    friend struct QtPromisePrivate::PromiseFulfill<QPromise<T>>;
    friend class QtPromisePrivate::PromiseResolver<T>;
    friend struct QtPromisePrivate::PromiseInspect;

    QExplicitlySharedDataPointer<QtPromisePrivate::PromiseData<T>> m_d;
};

template <typename T>
class QPromise : public QPromiseBase<T>
{
public:
    template <typename F>
    QPromise(F&& resolver): QPromiseBase<T>(std::forward<F>(resolver)) { }

    template <typename Functor>
    inline QPromise<T>
    each(Functor fn);

    template <typename Functor>
    inline QPromise<T>
    filter(Functor fn);

    template <typename Functor>
    inline typename QtPromisePrivate::PromiseMapper<T, Functor>::PromiseType
    map(Functor fn);

    template <typename Functor, typename Input>
    inline typename QtPromisePrivate::PromiseDeduce<Input>::Type
    reduce(Functor fn, Input initial);

    template <typename Functor, typename U = T>
    inline typename QtPromisePrivate::PromiseDeduce<typename U::value_type>::Type
    reduce(Functor fn);

public: // STATIC

    // DEPRECATED (remove at version 1)
    template <template <typename, typename...> class Sequence = QVector, typename ...Args>
    Q_DECL_DEPRECATED_X("Use QtPromise::all instead") static inline QPromise<QVector<T>>
    all(const Sequence<QPromise<T>, Args...>& promises);

    inline static QPromise<T> resolve(const T& value);
    inline static QPromise<T> resolve(T&& value);

private:
    friend class QPromiseBase<T>;
};

template <>
class QPromise<void> : public QPromiseBase<void>
{
public:
    template <typename F>
    QPromise(F&& resolver): QPromiseBase<void>(std::forward<F>(resolver)) { }

public: // STATIC

    // DEPRECATED (remove at version 1)
    template <template <typename, typename...> class Sequence = QVector, typename ...Args>
    Q_DECL_DEPRECATED_X("Use QtPromise::all instead") static inline QPromise<void>
    all(const Sequence<QPromise<void>, Args...>& promises);

    inline static QPromise<void> resolve();

private:
    friend class QPromiseBase<void>;
};

} // namespace QtPromise

#include "qpromise.inl"

#endif // QTPROMISE_QPROMISE_H
