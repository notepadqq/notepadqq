---
title: .map
---

# QPromise::map

*Since: 0.4.0*

```cpp
QPromise<Sequence<T>>::map(Mapper mapper) -> QPromise<QVector<R>>

// With:
// - Sequence: STL compatible container (e.g. QVector, etc.)
// - Mapper: Function(T value, int index) -> R | QPromise<R>
```

::: warning IMPORTANT
This method only applies to promise with sequence value.
:::

Iterates over all the promise values (i.e. `Sequence<T>`) and [maps the sequence](https://en.wikipedia.org/wiki/Map_%28higher-order_function%29) to another using the given `mapper` function. The type returned by `mapper` determines the type of the `output` promise. If `mapper` throws, `output` is rejected with the new exception.

If `mapper` returns a promise (or `QFuture`), the `output` promise is delayed until all the promises are resolved. If any of the promises fails, `output` immediately rejects with the error of the promise that rejected, whether or not the other promises are resolved.

```cpp
QPromise<QList<QUrl>> input = {...}

auto output = input.map([](const QUrl& url, int index) {
    return QPromise<QByteArray>([&](auto resolve, auto reject) {
        // download content at 'url' and resolve
        // {...}
    });
}).map([](const QByteArray& value, ...) {
    // process the downloaded QByteArray
    // {...}
    return DownloadResult(value);
});

// 'output' resolves as soon as all promises returned by
// 'mapper' are fulfilled or at least one is rejected.

// 'output' type: QPromise<QVector<DownloadResult>>
output.then([](const QVector<DownloadResult>& res) {
    // {...}
});
```

::: tip NOTE
The order of the output sequence values is guarantee to be the same as the original sequence, regardless of completion order of the promises returned by `mapper`.
:::

This function is provided for convenience and is similar to:

```cpp
promise.then([](const Sequence<T>& values) {
    return QtPromise::map(values, [](const T& value, int index) {
        return // {...}
    });
});
```

See also: [`QtPromise::map`](../helpers/map.md)
