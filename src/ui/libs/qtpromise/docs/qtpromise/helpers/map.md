---
title: map
---

# QtPromise::map

*Since: 0.4.0*

```cpp
QtPromise::map(Sequence<T> values, Mapper mapper) -> QPromise<QVector<R>>

// With:
// - Sequence: STL compatible container (e.g. QVector, etc.)
// - Mapper: Function(T value, int index) -> R | QPromise<R>
```

Iterates over `values` and [maps the sequence](https://en.wikipedia.org/wiki/Map_%28higher-order_function%29) to another using the given `mapper` function. The type returned by `mapper` determines the type of the `output` promise. If `mapper` throws, `output` is rejected with the new exception.

If `mapper` returns a promise (or `QFuture`), the `output` promise is delayed until all the promises are resolved. If any of the promises fails, `output` immediately rejects with the error of the promise that rejected, whether or not the other promises are resolved.

```cpp
auto output = QtPromise::map(QVector{
    QUrl("http://a..."),
    QUrl("http://b..."),
    QUrl("http://c...")
}, [](const QUrl& url, ...) {
    return QPromise<QByteArray>([&](auto resolve, auto reject) {
        // download content at url and resolve
        // {...}
    });
});

// 'output' resolves as soon as all promises returned by
// 'mapper' are fulfilled or at least one is rejected.

// 'output' type: QPromise<QVector<QByteArray>>
output.then([](const QVector<QByteArray>& res) {
    // {...}
});
```

::: tip NOTE
The order of the output sequence values is guarantee to be the same as the original sequence, regardless of completion order of the promises returned by `mapper`.
:::

See also: [`QPromise<T>::map`](../qpromise/map.md)
