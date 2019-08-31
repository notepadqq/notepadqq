---
title: .reduce
---

# QPromise::reduce

*Since: 0.5.0*

```cpp
QPromise<Sequence<T>>::reduce(Reducer reducer) -> QPromise<T>
QPromise<Sequence<T>>::reduce(Reducer reducer, R|QPromise<R> initialValue) -> QPromise<R>

// With:
// - Sequence: STL compatible container (e.g. QVector, etc.)
// - Reducer: Function(T|R accumulator, T item, int index) -> R|QPromise<R>
```

::: warning IMPORTANT
This method only applies to promise with sequence value.
:::

Iterates over all the promise values (i.e. `Sequence<T>`) and [reduces the sequence to a single value](https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29) using the given `reducer` function and an optional `initialValue`. The type returned by `reducer` determines the type of the `output` promise.

See [`QtPromise::reduce`](../helpers/reduce.md) for details, this method is provided for convenience and is similar to:

```cpp
promise.then([](const T& values) {
    return QtPromise::reduce(values, reducer, initialValue);
})
```

For example:

```cpp
auto input = QtPromise::resolve(QList<QUrl>{
    "file:f0.txt",   // contains "foo"
    "file:f1.txt",   // contains "bar"
    "file:f2.txt"    // contains "42"
});

// Concatenate the content of the given files, read asynchronously
auto output = input.reduce([](const QString& acc, const QString& cur, int idx) {
    return readAsync(cur).then([=](const QString& res) {
        return QString("%1;%2:%3").arg(acc).arg(idx).arg(res);
    });
}, QString("index:text"));

// 'output' resolves as soon as all promises returned by
// 'reducer' are fulfilled or at least one is rejected.

// 'output' type: QPromise<QString>
output.then([](const QString& res) {
    // res == "index:text;0:foo;1:bar;2:42"
    // {...}
});
```

See also: [`QtPromise::reduce`](../helpers/reduce.md)
