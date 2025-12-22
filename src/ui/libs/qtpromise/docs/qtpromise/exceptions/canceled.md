---
title: QPromiseCanceledException
---

# QPromiseCanceledException

*Since: 0.1.0*

This exception is thrown for promise created from a [`QFuture`](../qtconcurrent.md) which has been canceled (e.g. using [`QFuture::cancel()`](http://doc.qt.io/qt-5/qfuture.html#cancel)), for example:

```cpp
auto output = QtPromise::resolve(future)
    .fail([](const QPromiseCanceledException&) {
        // `future` has been canceled!
    });
```

::: tip NOTE
QtPromise doesn't support promise cancelation (yet?)
:::
