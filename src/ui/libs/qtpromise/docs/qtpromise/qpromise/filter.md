---
title: .filter
---

# QPromise::filter

*Since: 0.4.0*

```cpp
QPromise<Sequence<T>>::filter(Filter filterer) -> QPromise<Sequence<T>>

// With:
// - Sequence: STL compatible container (e.g. QVector, etc.)
// - Filterer: Function(T value, int index) -> bool
```

::: warning IMPORTANT
This method only applies to promise with sequence value.
:::

Iterates over all the promise values (i.e. `Sequence<T>`) and [filters the sequence](https://en.wikipedia.org/wiki/Filter_%28higher-order_function%29) to another using the given `filterer` function. If `filterer` returns `true`, a copy of the item is put in the `output` sequence, otherwise, the item will not appear in  `output`. If `filterer` throws, `output` is rejected with the new exception.

If `filterer` returns a promise (or `QFuture`), the `output` promise is delayed until all the promises are resolved. If any of the promises fail, `output` immediately rejects with the error of the promise that rejected, whether or not the other promises are resolved.

```cpp
QPromise<QList<QUrl>> input = {...}

auto output = input.filter([](const QUrl& url, ...) {
    return url.isValid();  // Keep only valid URLs
}).filter([](const QUrl& url, ...) {
    return QPromise<bool>([&](auto resolve, auto reject) {
        // resolve(true) if `url` is reachable, else resolve(false)
    });
});

// 'output' resolves as soon as all promises returned by
// 'filterer' are fulfilled or at least one is rejected.

// 'output' type: QPromise<QList<QUrl>>
output.then([](const QList<QUrl>& res) {
    // 'res' contains only reachable URLs
});
```

::: tip NOTE
The order of the output sequence values is guarantee to be the same as the original sequence, regardless of completion order of the promises returned by `filterer`.
:::

See also: [`QtPromise::filter`](../helpers/filter.md)
