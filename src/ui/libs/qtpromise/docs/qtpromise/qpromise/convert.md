---
title: .convert
---

# QPromise::convert

*Since: 0.7.0*

```cpp
QPromise<T>::convert<U>() -> QPromise<U>
```

This method converts the resolved value of `QPromise<T>` to the type `U`. Depending on types `T`
and `U`, it performs a [static cast](https://en.cppreference.com/w/cpp/language/static_cast), 
calls a [converting constructor](https://en.cppreference.com/w/cpp/language/converting_constructor), 
or tries to convert using [QVariant](https://doc.qt.io/qt-5/qvariant.html).

If `T` and `U` are [fundamental types](https://en.cppreference.com/w/cpp/language/types) or 
[enumerations](https://en.cppreference.com/w/cpp/language/enum), the result of the conversion is 
the same as calling `static_cast<U>` for type `T`:

```cpp
QPromise<int> input = {...}

// output type: QPromise<double>
auto output = input.convert<double>();

output.then([](double value) {
    // the value has been converted using static_cast
});
```

If `U` has a [converting constructor](https://en.cppreference.com/w/cpp/language/converting_constructor) 
from `T`, i.e., a non-explicit constructor with a single argument accepting `T`, it is used to
convert the value:

```cpp
QPromise<QByteArray> input = {...}

// output type: QPromise<QString>
auto output = input.convert<QString>();

output.then([](const QString& value) {
    // the value has been converted using static_cast that effectively calls QString(QByteArray)
});
```

::: tip NOTE
When using this method to convert to your own classes, make sure that the constructor meeting the 
converting constructor criteria actually performs conversion.
:::

::: tip NOTE 
If `U` is `void`, the resolved value of `QPromise<T>` is dropped. 
:::

Calling this method for `QPromise<QVariant>` tries to convert the resolved `QVariant` to type `U`
using the `QVariant` [conversion algorithm](https://doc.qt.io/qt-5/qvariant.html#using-canconvert-and-convert-consecutively). 
For example, this allows to convert a string contained in `QVariant` to number. If such a 
conversion fails, the promise is rejected with 
[`QPromiseConversionException`](../exceptions/conversion.md).

```cpp
// resolves to QVariant(int, 42) or QVariant(string, "foo")
QPromise<QVariant> input = {...}; 

auto output = input.convert<int>();

// output type: QPromise<int>
output.then([](int value) {
    // input was QVariant(int, 42), value is 42
})
.fail(const QPromiseConversionException& e) {
    // input was QVariant(string, "foo")
});
```

Conversion of `T` to `QVariant` using this method effectively calls `QVariant::fromValue<T>()`. 
All custom types must be registered with 
[`Q_DECLARE_METATYPE`](https://doc.qt.io/qt-5/qmetatype.html#Q_DECLARE_METATYPE) for this 
conversion to work:

```cpp
struct Foo {};
Q_DECLARE_METATYPE(Foo);

QPromise<Foo> input = {...}

auto output = input.convert<QVariant>();

// output type: QPromise<QVariant>
output.then([](const QVariant& value) {
    // value contains an instance of Foo
});
```

All other combinations of `T` and `U` are converted via `QVariant`. All non-Qt types should provide 
a [conversion function](https://doc.qt.io/qt-5/qmetatype.html#registerConverter), otherwise the 
promise is rejected with [`QPromiseConversionException`](../exceptions/conversion.md):

```cpp
struct Foo {};
Q_DECLARE_METATYPE(Foo);

QMetaType::registerConverter<Foo, QString>([](const Foo& foo) {
    return QString{...};
});

QPromise<Foo> input = {...}

auto output = input.convert<QVariant>();

// output type: QPromise<QString>
output.then([](const QString& value) {
    // value contains a result produced by the custom converter
})
.fail([](const QPromiseConversionException& e) {
    // QVariant was unable to convert Foo to QString 
})
```

::: warning IMPORTANT
Calling this method for `QPromise<void>` is not supported.
:::
