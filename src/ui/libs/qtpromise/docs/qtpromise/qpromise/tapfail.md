---
title: .tapFail
---

# QPromise::tapFail

*Since: 0.4.0*

```cpp
QPromise<T>::tapFail(Function handler) -> QPromise<T>
```

This `handler` allows to observe errors of the `input` promise without handling them - similar to [`finally`](finally.md) but **only** called on rejections. The `output` promise has the same type as the `input` one but also the same value or error. However, if `handler` throws, `output` is rejected with the new exception.

```cpp
QPromise<int> input = {...}
auto output = input.tapFail([](Error err) {
    log(err);
}).then([](int res) {
    return process(res);
}).fail([](Error err) {
    handle(err);
    return -1;
});
```

If `handler` returns a promise (or QFuture), the `output` promise is delayed until the returned promise is resolved and under the same conditions: the delayed value is ignored, the error transmitted to the `output` promise.
