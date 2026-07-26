#include "include/commandlineopenruntime.h"

#include <QPointer>

void CommandLineOpenRuntime::continueAfterLoading(
    const QtPromise::QPromise<void>& loading, QObject* owner, std::function<void()> continuation)
{
    const QPointer<QObject> guard(owner);
    loading.then([guard, continuation = std::move(continuation)] {
        if (guard) {
            continuation();
        }
    });
}
