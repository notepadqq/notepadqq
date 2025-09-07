---
title: .finally
---

# QPromise::finally

*Since: 0.1.0*

```cpp
QPromise<T>::finally(Function handler) -> QPromise<T>
```

This `handler` is **always** called, without any argument and whatever the `input` promise state (fulfilled or rejected). The `output` promise has the same type as the `input` one but also the same value or error. The finally `handler` **can not modify the fulfilled value** (the returned value is ignored), however, if `handler` throws, `output` is rejected with the new exception.

```cpp
auto output = input.finally([]() {
    // {...}
});
```

If `handler` returns a promise (or QFuture), the `output` promise is delayed until the returned promise is resolved and under the same conditions: the delayed value is ignored, the error transmitted to the `output` promise.
