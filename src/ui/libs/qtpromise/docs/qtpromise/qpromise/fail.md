---
title: .fail
---

# QPromise::fail

*Since: 0.1.0*

```cpp
QPromise<T>::fail(Function onRejected) -> QPromise<T>
```

Shorthand to `promise.then(nullptr, onRejected)`, similar to the [`catch` statement](http://en.cppreference.com/w/cpp/language/try_catch):

```cpp
promise.fail([](const MyException&) {
    // {...}
}).fail(const QException&) {
    // {...}
}).fail(const std::exception&) {
    // {...}
}).fail() {
    // {...} catch-all
});
```
