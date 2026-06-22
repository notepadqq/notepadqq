---
title: .fail
---

# QPromise::fail

*Since: 0.1.0*

```cpp
QPromise<T>::fail(Function onRejected) -> QPromise<T>
```

Shorthand to [`promise.then(nullptr, onRejected)`](then.md) for handling errors in promise chains,
similar to the native C++ [`catch` statement](http://en.cppreference.com/w/cpp/language/try_catch):

```cpp
promise.fail([](const MyException& error) {
    // {...}
}).fail([](const QException& error) {
    // {...}
}).fail([](const std::exception& error) {
    // {...}
}).fail([]() {
    // {...} catch-all
});
```

See also: [`QPromise::then`](then.md)
