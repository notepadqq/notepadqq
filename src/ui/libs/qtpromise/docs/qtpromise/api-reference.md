# API Reference

## Functions

* [`QPromise<T>::QPromise`](qpromise/constructor.md)
* [`QPromise<T>::delay`](qpromise/delay.md)
* [`QPromise<T>::each`](qpromise/each.md)
* [`QPromise<T>::fail`](qpromise/fail.md)
* [`QPromise<T>::filter`](qpromise/filter.md)
* [`QPromise<T>::finally`](qpromise/finally.md)
* [`QPromise<T>::isFulfilled`](qpromise/isfulfilled.md)
* [`QPromise<T>::isPending`](qpromise/ispending.md)
* [`QPromise<T>::isRejected`](qpromise/isrejected.md)
* [`QPromise<T>::map`](qpromise/map.md)
* [`QPromise<T>::reduce`](qpromise/reduce.md)
* [`QPromise<T>::tap`](qpromise/tap.md)
* [`QPromise<T>::tapFail`](qpromise/tapfail.md)
* [`QPromise<T>::then`](qpromise/then.md)
* [`QPromise<T>::timeout`](qpromise/timeout.md)
* [`QPromise<T>::wait`](qpromise/wait.md)

## Static Functions

* [`[static] QPromise<T>::reject`](qpromise/reject.md)
* [`[static] QPromise<T>::resolve`](qpromise/resolve.md)

## Helpers

* [`QtPromise::all`](helpers/all.md)
* [`QtPromise::attempt`](helpers/attempt.md)
* [`QtPromise::connect`](helpers/connect.md)
* [`QtPromise::each`](helpers/each.md)
* [`QtPromise::filter`](helpers/filter.md)
* [`QtPromise::map`](helpers/map.md)
* [`QtPromise::reduce`](helpers/reduce.md)
* [`QtPromise::resolve`](helpers/resolve.md)

## Exceptions

* [`QPromiseCanceledException`](exceptions/canceled.md)
* [`QPromiseContextException`](exceptions/context.md)
* [`QPromiseTimeoutException`](exceptions/timeout.md)
* [`QPromiseUndefinedException`](exceptions/undefined.md)

## Deprecations

* `[static] QPromise<T>::all`: use [`QtPromise::all`](helpers/all.md) instead (since 0.5.0)
* `QtPromise::qPromise`: use [`QtPromise::resolve`](helpers/resolve.md) instead (since 0.5.0)
* `QtPromise::qPromiseAll`: use [`QtPromise::all`](helpers/all.md) instead (since 0.5.0)
