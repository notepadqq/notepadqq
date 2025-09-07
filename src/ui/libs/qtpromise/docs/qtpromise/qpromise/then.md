---
title: .then
---

# QPromise::then

*Since: 0.1.0*

```cpp
QPromise<T>::then(Function onFulfilled, Function onRejected) -> QPromise<R>
QPromise<T>::then(Function onFulfilled) -> QPromise<R>
```

See [Promises/A+ `.then`](https://promisesaplus.com/#the-then-method) for details.

```cpp
QPromise<int> input = ...
auto output = input.then([](int res) {
    // called with the 'input' result if fulfilled
}, [](const ReasonType& reason) {
    // called with the 'input' reason if rejected
    // see QPromise<T>::fail for details
});
```

::: tip NOTE
`onRejected` handler is optional, in which case `output` will be rejected with the same reason as `input`. Also note that it's recommended to use the [`fail`](fail.md) shorthand to handle errors.
:::

The type `<R>` of the `output` promise depends on the return type of the `onFulfilled` handler:

```cpp
QPromise<int> input = {...}
auto output = input.then([](int res) {
    return QString::number(res);    // -> QPromise<QString>
});

// output type: QPromise<QString>
output.then([](const QString& res) {
    // {...}
});
```

::: tip NOTE
Only `onFulfilled` can change the promise type, `onRejected` **must** return the same type as `onFulfilled`. That also means if `onFulfilled` is `nullptr`, `onRejected` must return the same type as the `input` promise.
:::

```cpp
QPromise<int> input = ...
auto output = input.then([](int res) {
    return res + 4;
}, [](const ReasonType& reason) {
    return -1;
});
```

If `onFulfilled` doesn't return any value, the `output` type is `QPromise<void>`:

```cpp
QPromise<int> input = ...
auto output = input.then([](int res) {
    // {...}
});

// output type: QPromise<void>
output.then([]() {
    // `QPromise<void>` `onFulfilled` handler has no argument
});
```

You can also decide to skip the promise result by omitting the handler argument:

```cpp
QPromise<int> input = {...}
auto output = input.then([]( /* skip int result */ ) {
    // {...}
});
```

The `output` promise can be *rejected* by throwing an exception in either `onFulfilled` or `onRejected`:

```cpp
QPromise<int> input = {...}
auto output = input.then([](int res) {
    if (res == -1) {
        throw ReasonType();
    } else {
        return res;
    }
});

// output.isRejected() is true
```

If an handler returns a promise (or QFuture), the `output` promise is delayed and will be resolved by the returned promise.
