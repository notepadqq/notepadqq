#ifndef COMMANDLINEOPENRUNTIME_H
#define COMMANDLINEOPENRUNTIME_H

#include <QtPromise>

#include <functional>

class QObject;

namespace CommandLineOpenRuntime {
// Continues command-line setup after loading completes while respecting the QObject owner's lifetime.
void continueAfterLoading(const QtPromise::QPromise<void>& loading, QObject* owner, std::function<void()> continuation);
} // namespace CommandLineOpenRuntime

#endif
