---
title: .timeout
---

# QPromise::timeout

*Since: 0.2.0*

```cpp
QPromise<T>::timeout(int msec, any error = QPromiseTimeoutException) -> QPromise<T>
```

This method returns a promise that will be resolved with the `input` promise's fulfillment value
or rejection reason. However, if the `input` promise is not fulfilled or rejected within `msec`
milliseconds, the `output` promise is rejected with `error` as the reason ([`QPromiseTimeoutException`](../exceptions/timeout.md)
by default).

```cpp
QPromise<int> input = {...}
auto output = input.timeout(2000)
    .then([](int res) {
        // operation succeeded within 2 seconds
    })
    .fail([](const QPromiseTimeoutException& error) {
        // operation timed out!
    });
```

---

*Since: 0.6.0*

```cpp
QPromise<T>::timeout(std::chrono::milliseconds msec, any error = QPromiseTimeoutException) -> QPromise<T>
```

This is a convenience overload accepting [durations from the C++ Standard Library](https://en.cppreference.com/w/cpp/chrono/duration).

```cpp
QPromise<int> input = {...}
auto output = input.timeout(std::chrono::seconds{2})
    .then([](int res) {
        // operation succeeded within 2 seconds
    })
    .fail([](const QPromiseTimeoutException& error) {
        // operation timed out!
    });
```

C++14 alternative:

```cpp
using namespace std::chrono_literals; 

QPromise<int> input = {...}
auto output = input.timeout(2s)
    .then([](int res) {
        // operation succeeded within 2 seconds
    })
    .fail([](const QPromiseTimeoutException& error) {
        // operation timed out!
    });
```
