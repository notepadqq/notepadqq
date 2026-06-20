#ifndef QTPROMISE_QPROMISEHELPERS_H
#define QTPROMISE_QPROMISEHELPERS_H

#include "qpromise_p.h"
#include "qpromisehelpers_p.h"

namespace QtPromise {

template <typename T>
static inline typename QtPromisePrivate::PromiseDeduce<T>::Type
resolve(T&& value)
{
    using namespace QtPromisePrivate;
    using PromiseType = typename PromiseDeduce<T>::Type;
    using ValueType = typename PromiseType::Type;
    using ResolveType = QPromiseResolve<ValueType>;
    using RejectType = QPromiseReject<ValueType>;

    return PromiseType([&](ResolveType&& resolve, RejectType&& reject) {
        PromiseFulfill<Unqualified<T>>::call(
            std::forward<T>(value),
            std::forward<ResolveType>(resolve),
            std::forward<RejectType>(reject));
    });
}

template <typename T>
static inline QPromise<T>
resolve(QPromise<T> value)
{
    return std::move(value);
}

static inline QPromise<void>
resolve()
{
    return QPromise<void>([](const QPromiseResolve<void>& resolve) {
        resolve();
    });
}

template <typename T, template <typename, typename...> class Sequence = QVector, typename ...Args>
static inline QPromise<QVector<T>>
all(const Sequence<QPromise<T>, Args...>& promises)
{
    const int count = static_cast<int>(promises.size());
    if (count == 0) {
        return QtPromise::resolve(QVector<T>{});
    }

    return QPromise<QVector<T>>([=](
        const QPromiseResolve<QVector<T>>& resolve,
        const QPromiseReject<QVector<T>>& reject) {

        QSharedPointer<int> remaining(new int(count));
        QSharedPointer<QVector<T>> results(new QVector<T>(count));

        int i = 0;
        for (const auto& promise: promises) {
            promise.then([=](const T& res) mutable {
                (*results)[i] = res;
                if (--(*remaining) == 0) {
                    resolve(*results);
                }
            }, [=]() mutable {
                if (*remaining != -1) {
                    *remaining = -1;
                    reject(std::current_exception());
                }
            });

            i++;
        }
    });
}

template <template <typename, typename...> class Sequence = QVector, typename ...Args>
static inline QPromise<void>
all(const Sequence<QPromise<void>, Args...>& promises)
{
    const int count = static_cast<int>(promises.size());
    if (count == 0) {
        return QtPromise::resolve();
    }

    return QPromise<void>([=](
        const QPromiseResolve<void>& resolve,
        const QPromiseReject<void>& reject) {

        QSharedPointer<int> remaining(new int(count));

        for (const auto& promise: promises) {
            promise.then([=]() {
                if (--(*remaining) == 0) {
                    resolve();
                }
            }, [=]() {
                if (*remaining != -1) {
                    *remaining = -1;
                    reject(std::current_exception());
                }
            });
        }
    });
}

template <typename Functor, typename... Args>
static inline typename QtPromisePrivate::PromiseFunctor<Functor, Args...>::PromiseType
attempt(Functor&& fn, Args&&... args)
{
    using namespace QtPromisePrivate;
    using FunctorType = PromiseFunctor<Functor, Args...>;
    using PromiseType = typename FunctorType::PromiseType;
    using ValueType = typename PromiseType::Type;

    // NOTE: std::forward<T<U>>: MSVC 2013 fails when forwarding
    // template type (error: "expects 4 arguments - 0 provided").
    // However it succeeds with type alias.
    // TODO: should we expose QPromise::ResolveType & RejectType?
    using ResolveType = QPromiseResolve<ValueType>;
    using RejectType = QPromiseReject<ValueType>;

    return PromiseType(
        [&](ResolveType&& resolve, RejectType&& reject) {
            PromiseDispatch<typename FunctorType::ResultType>::call(
                std::forward<ResolveType>(resolve),
                std::forward<RejectType>(reject),
                std::forward<Functor>(fn),
                std::forward<Args>(args)...);
        });
}

template <typename Sender, typename Signal>
static inline typename QtPromisePrivate::PromiseFromSignal<Signal>
connect(const Sender* sender, Signal signal)
{
    using namespace QtPromisePrivate;
    using T = typename PromiseFromSignal<Signal>::Type;

    return QPromise<T>(
        [&](const QPromiseResolve<T>& resolve, const QPromiseReject<T>& reject) {
            QPromiseConnections connections;
            connectSignalToResolver(connections, resolve, sender, signal);
            connectDestroyedToReject(connections, reject, sender);
        });
}

template <typename FSender, typename FSignal, typename RSender, typename RSignal>
static inline typename QtPromisePrivate::PromiseFromSignal<FSignal>
connect(const FSender* fsender, FSignal fsignal, const RSender* rsender, RSignal rsignal)
{
    using namespace QtPromisePrivate;
    using T = typename PromiseFromSignal<FSignal>::Type;

    return QPromise<T>(
        [&](const QPromiseResolve<T>& resolve, const QPromiseReject<T>& reject) {
            QPromiseConnections connections;
            connectSignalToResolver(connections, resolve, fsender, fsignal);
            connectSignalToResolver(connections, reject, rsender, rsignal);
            connectDestroyedToReject(connections, reject, fsender);
        });
}

template <typename Sender, typename FSignal, typename RSignal>
static inline typename QtPromisePrivate::PromiseFromSignal<FSignal>
connect(const Sender* sender, FSignal fsignal, RSignal rsignal)
{
    return connect(sender, fsignal, sender, rsignal);
}

template <typename Sequence, typename Functor>
static inline QPromise<Sequence>
each(const Sequence& values, Functor&& fn)
{
    return QPromise<Sequence>::resolve(values).each(std::forward<Functor>(fn));
}

template <typename Sequence, typename Functor>
static inline typename QtPromisePrivate::PromiseMapper<Sequence, Functor>::PromiseType
map(const Sequence& values, Functor fn)
{
    using namespace QtPromisePrivate;
    using MapperType = PromiseMapper<Sequence, Functor>;
    using ResType = typename MapperType::ResultType::value_type;
    using RetType = typename MapperType::ReturnType;

    int i = 0;

    std::vector<QPromise<ResType>> promises;
    for (const auto& v : values) {
        promises.push_back(QPromise<ResType>([&](
            const QPromiseResolve<ResType>& resolve,
            const QPromiseReject<ResType>& reject) {
                PromiseFulfill<RetType>::call(fn(v, i), resolve, reject);
            }));

        i++;
    }

    return QtPromise::all(promises);
}

template <typename Sequence, typename Functor>
static inline QPromise<Sequence>
filter(const Sequence& values, Functor fn)
{
    return QtPromise::map(values, fn)
        .then([=](const QVector<bool>& filters) {
            Sequence filtered;

            auto filter = filters.begin();
            for (auto& value : values) {
                if (*filter) {
                    filtered.push_back(std::move(value));
                }

                filter++;
            }

            return filtered;
        });
}

template <typename T, template <typename...> class Sequence = QVector, typename Reducer, typename Input, typename ...Args>
static inline typename QtPromisePrivate::PromiseDeduce<Input>::Type
reduce(const Sequence<T, Args...>& values, Reducer fn, Input initial)
{
    using namespace QtPromisePrivate;
    using PromiseType = typename PromiseDeduce<T>::Type;
    using ValueType = typename PromiseType::Type;

    int idx = 0;

    auto promise = QtPromise::resolve(std::move(initial));
    for (const auto& value : values) {
        auto input = QtPromise::resolve(value);
        promise = promise.then([=]() {
            return input.then([=](const ValueType& cur) {
                return fn(PromiseInspect::get(promise)->value().data(), cur, idx);
            });
        });

        idx++;
    }

    return promise;
}

template <typename T, template <typename...> class Sequence = QVector, typename Reducer, typename ...Args>
static inline typename QtPromisePrivate::PromiseDeduce<T>::Type
reduce(const Sequence<T, Args...>& values, Reducer fn)
{
    using namespace QtPromisePrivate;
    using PromiseType = typename PromiseDeduce<T>::Type;
    using ValueType = typename PromiseType::Type;

    Q_ASSERT(values.size());

    int idx = 1;

    auto it = values.begin();
    auto promise = QtPromise::resolve(*it);
    for (++it; it != values.end(); ++it) {
        auto input = QtPromise::resolve(*it);
        promise = promise.then([=]() {
            return input.then([=](const ValueType& cur) {
                return fn(PromiseInspect::get(promise)->value().data(), cur, idx);
            });
        });

        idx++;
    }

    return promise;
}

// DEPRECATIONS (remove at version 1)

template <typename... Args>
Q_DECL_DEPRECATED_X("Use QtPromise::resolve instead")
static inline auto
qPromise(Args&&... args)
    -> decltype(QtPromise::resolve(std::forward<Args>(args)...))
{
    return QtPromise::resolve(std::forward<Args>(args)...);
}

template <typename... Args>
Q_DECL_DEPRECATED_X("Use QtPromise::all instead")
static inline auto
qPromiseAll(Args&&... args)
    -> decltype(QtPromise::all(std::forward<Args>(args)...))
{
    return QtPromise::all(std::forward<Args>(args)...);
}

} // namespace QtPromise

#endif // QTPROMISE_QPROMISEHELPERS_H
