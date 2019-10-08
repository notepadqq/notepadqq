# Getting Started

## Installation

QtPromise is a [header-only](https://en.wikipedia.org/wiki/Header-only) library, simply download the [latest release](https://github.com/simonbrunel/qtpromise/releases/latest) (or [`git submodule`](https://git-scm.com/docs/git-submodule)) and include `qtpromise.pri` from your project `.pro`.

### qpm

Alternatively and **only** if your project relies on [qpm](https://www.qpm.io/), you can install QtPromise as follow:

```bash
qpm install com.github.simonbrunel.qtpromise
```

## Usage

The recommended way to use QtPromise is to include the single module header:

```cpp
#include <QtPromise>
```

## Example

Let's first make the code more readable by using the library namespace:

```cpp
using namespace QtPromise;
```

This `download` function creates a [promise from callbacks](qpromise/constructor.md) which will be resolved when the network request is finished:

```cpp
QPromise<QByteArray> download(const QUrl& url)
{
    return QPromise<QByteArray>([&](
        const QPromiseResolve<QByteArray>& resolve,
        const QPromiseReject<QByteArray>& reject) {

        QNetworkReply* reply = manager->get(QNetworkRequest(url));
        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
                resolve(reply->readAll());
            } else {
                reject(reply->error());
            }

            reply->deleteLater();
        });
    });
}
```

The following method `uncompress` data in a separate thread and returns a [promise from QFuture](qtconcurrent.md):

```cpp
QPromise<Entries> uncompress(const QByteArray& data)
{
    return QtPromise::resolve(QtConcurrent::run([](const QByteArray& data) {
        Entries entries;

        // {...} uncompress data and parse content.

        if (error) {
            throw MalformedException();
        }

        return entries;
    }, data));
}
```

It's then easy to chain the whole asynchronous process using promises:
- initiate the promise chain by downloading a specific URL,
- [`then`](qpromise/then.md) *and only if download succeeded*, uncompress received data,
- [`then`](qpromise/then.md) validate and process the uncompressed entries,
- [`finally`](qpromise/finally.md) perform operations whatever the process succeeded or failed,
- and handle specific errors using [`fail`](qpromise/fail.md).

```cpp
download(url).then(&uncompress).then([](const Entries& entries) {
    if (entries.isEmpty()) {
        throw UpdateException("No entries");
    }
    // {...} process entries
}).finally([]() {
    // {...} cleanup
}).fail([](QNetworkReply::NetworkError err) {
    // {...} handle network error
}).fail([](const UpdateException& err) {
    // {...} handle update error
}).fail([]() {
    // {...} catch all
});
```
