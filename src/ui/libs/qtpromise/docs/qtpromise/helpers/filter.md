---
title: filter
---

# QtPromise::filter

*Since: 0.4.0*

```cpp
QtPromise::filter(Sequence<T> values, Filterer filterer) -> QPromise<Sequence<T>>

// With:
// - Sequence: STL compatible container (e.g. QVector, etc.)
// - Filterer: Function(T value, int index) -> bool
```

Iterates over `values` and [filters the sequence](https://en.wikipedia.org/wiki/Filter_%28higher-order_function%29) to another using the given `filterer` function. If `filterer` returns `true`, a copy of the item is put in the `output` sequence, otherwise, the item will not appear in `output`. If `filterer` throws, `output` is rejected with the new exception.

If `filterer` returns a promise (or `QFuture`), the `output` promise is delayed until all the promises are resolved. If any of the promises fail, `output` immediately rejects with the error of the promise that rejected, whether or not the other promises are resolved.

```cpp
auto output = QtPromise::filter(QVector{
    QUrl("http://a..."),
    QUrl("http://b..."),
    QUrl("http://c...")
}, [](const QUrl& url, ...) {
    return QPromise<bool>([&](auto resolve, auto reject) {
        // resolve(true) if 'url' is reachable, else resolve(false)
        // {...}
    });
});

// 'output' resolves as soon as all promises returned by
// 'filterer' are fulfilled or at least one is rejected.

// 'output' type: QPromise<QVector<QUrl>>
output.then([](const QVector<QUrl>& res) {
    // 'res' contains only reachable URLs
});
```

::: tip NOTE
The order of the output sequence values is guarantee to be the same as the original sequence, regardless of completion order of the promises returned by `filterer`.
:::

See also: [`QPromise<T>::filter`](../qpromise/filter.md)
