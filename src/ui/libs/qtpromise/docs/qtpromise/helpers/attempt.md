---
title: attempt
---

# QtPromise::attempt

*Since: 0.4.0*

```cpp
QtPromise::attempt(Functor functor, Args...) -> QPromise<R>

// With:
// - Functor: Function(Args...) -> R | QPromise<R>
```

Calls `functor` immediately and returns a promise fulfilled with the value returned by `functor`. Any synchronous exceptions will be turned into rejections on the returned promise. This is a convenient method that can be used instead of handling both synchronous and asynchronous exception flows.

The type `R` of the `output` promise depends on the type returned by the `functor` function. If `functor` returns a promise (or `QFuture`), the `output` promise is delayed and will be resolved by the returned promise.

```cpp
QPromise<QByteArray> download(const QUrl& url);

QPromise<QByteArray> process(const QUrl& url)
{
    return QtPromise::attempt([&]() {
        if (!url.isValid()) {
            throw InvalidUrlException();
        }

        return download(url);
    }
}

auto output = process(url);

// 'output' type: QPromise<QByteArray>
output.then([](const QByteArray& res) {
    // {...}
}).fail([](const InvalidUrlException& err) {
    // {...}
});
```
