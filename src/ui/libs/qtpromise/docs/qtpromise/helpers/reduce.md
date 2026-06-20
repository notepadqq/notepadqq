---
title: reduce
---

# QtPromise::reduce

*Since: 0.5.0*

```cpp
QPromise::reduce(Sequence<T|QPromise<T>> values, Reducer reducer) -> QPromise<T>
QPromise::reduce(Sequence<T|QPromise<T>> values, Reducer reducer, R|QPromise<R> initialValue) -> QPromise<R>

// With:
// - Sequence: STL compatible container (e.g. QVector, etc.)
// - Reducer: Function(T|R accumulator, T item, int index) -> R|QPromise<R>
```

Iterates over `values` and [reduces the sequence to a single value](https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29) using the given `reducer` function and an optional `initialValue`. The type returned by `reducer` determines the type of the `output` promise. If `reducer` throws, `output` is rejected with the new exception.

If `reducer` returns a promise (or `QFuture`), then the result of the promise is awaited, before continuing with next iteration. If any promise in the `values` sequence is rejected or a promise returned by the `reducer` function is rejected, `output` immediately rejects with the error of the promise that rejected.

```cpp
// Concatenate the content of the given files, read asynchronously
auto output = QtPromise::reduce(QList<QUrl>{
    "file:f0.txt",   // contains "foo"
    "file:f1.txt",   // contains "bar"
    "file:f2.txt"    // contains "42"
}, [](const QString& acc, const QString& cur, int idx) {
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

::: warning IMPORTANT
The first time `reducer` is called, if no `initialValue` is provided, `accumulator` will be equal to the first value in the sequence, and `currentValue` to the second one (thus index will be `1`).
:::

See also: [`QPromise<T>::reduce`](../qpromise/reduce.md)
