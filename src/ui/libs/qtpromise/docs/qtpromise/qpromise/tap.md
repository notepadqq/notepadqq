---
title: .tap
---

# QPromise::tap

*Since: 0.2.0*

```cpp
QPromise<T>::tap(Function handler) -> QPromise<T>
```

This `handler` allows to observe the value of the `input` promise, without changing the propagated value. The `output` promise will be resolved with the same value as the `input` promise (the `handler` returned value will be ignored). However, if `handler` throws, `output` is rejected with the new exception. Unlike [`finally`](finally.md), this handler is **not** called for rejections.

```cpp
QPromise<int> input = {...}
auto output = input.tap([](int res) {
    log(res);
}).then([](int res) {
    // {...}
});
```

If `handler` returns a promise (or QFuture), the `output` promise is delayed until the returned promise is resolved and under the same conditions: the delayed value is ignored, the error transmitted to the `output` promise.
