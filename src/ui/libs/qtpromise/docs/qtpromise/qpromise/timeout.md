---
title: .timeout
---

# QPromise::timeout

*Since: 0.2.0*

```cpp
QPromise<T>::timeout(int msec, any error = QPromiseTimeoutException) -> QPromise<T>
```

This method returns a promise that will be resolved with the `input` promise's fulfillment value or rejection reason. However, if the `input` promise is not fulfilled or rejected within `msec` milliseconds, the `output` promise is rejected with `error` as the reason ([`QPromiseTimeoutException`](../exceptions/timeout.md) by default).

```cpp
QPromise<int> input = {...}
auto output = input.timeout(2000)
    .then([](int res) {
        // operation succeeded within 2 seconds
    })
    .fail([](const QPromiseTimeoutException& e) {
        // operation timed out!
    });
```
