# Qt Concurrent

QtPromise integrates with [QtConcurrent](https://doc.qt.io/qt-5/qtconcurrent-index.html) to simplify
chaining QFuture with QPromise.

## Convert

Converting `QFuture<T>` to `QPromise<T>` is done using the [`QtPromise::resolve`](helpers/resolve.md)
helper:

```cpp
QFuture<int> future = QtConcurrent::run([]() {
    // {...}
    return 42;
});

QPromise<int> promise = QtPromise::resolve(future);
```

or simply:

```cpp
auto promise = QtPromise::resolve(QtConcurrent::run([]() {
    // {...}
}));
```

## Chain

Returning a `QFuture<T>` in [`then`](qpromise/then.md)  or [`fail`](qpromise/fail.md) automatically
translate to `QPromise<T>`:

```cpp
QPromise<int> input = ...
auto output = input.then([](int res) {
    return QtConcurrent::run([]() {
        // {...}
        return QString{"42"};
    });
});

// output type: QPromise<QString>
output.then([](const QString& res) {
    // {...}
});
```

The `output` promise is resolved when the `QFuture` is [finished](https://doc.qt.io/qt-5/qfuture.html#isFinished).

## Error

Exceptions thrown from a QtConcurrent thread reject the associated promise with the exception as the
reason. For this to work, the exception should be a [`QException`](https://doc.qt.io/qt-5/qexception.html) 
or its subclass. Correct subclassing of [`QException`](https://doc.qt.io/qt-5/qexception.html) 
includes overriding its methods [`clone()`](https://doc.qt.io/qt-5/qexception.html#clone) and 
[`raise()`](https://doc.qt.io/qt-5/qexception.html#raise). Without these overrides the promise will 
be rejected with [`QException`](https://doc.qt.io/qt-5/qexception.html). 

Note that if you throw an exception that is not a subclass of `QException`, the promise will
be rejected with [`QUnhandledException`](https://doc.qt.io/qt-5/qunhandledexception.html#details)
(this restriction only applies to exceptions thrown from a QtConcurrent thread,
[read more](https://doc.qt.io/qt-5/qexception.html#details)). 

```cpp
class CustomException : public QException
{
public:
    void raise() const override { throw *this; }
    CustomException* clone() const override { return new CustomException{*this}; }
};

// {...}

QPromise<int> promise = ...
promise.then([](int res) {
    return QtConcurrent::run([]() {
        // {...}

        if (!success) {
            throw CustomException{};
        }

        return QString{"42"};
    });
}).fail([](const CustomException& error) {
    // {...}
});
```
