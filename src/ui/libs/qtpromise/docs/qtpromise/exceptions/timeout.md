# QPromiseTimeoutException

*Since: 0.2.0*

This is the default exception thrown when reaching the time limit when using the
[`QPromise::timeout()`](../qpromise/timeout.md) method, for example:

```cpp
QPromise<int> input = {...}
auto output = input.timeout(2000)
    .fail([](const QPromiseTimeoutException& error) {
        // operation timed out after 2s!
    });
```
