#ifndef EDITORNS_DEFER_H
#define EDITORNS_DEFER_H

#include <QObject>
#include <QTimer>

#include <functional>
#include <utility>

namespace EditorNS {

// Queues a callback only while its QObject context remains alive.
inline void deferToObject(QObject* context, std::function<void()> callback)
{
    QTimer::singleShot(0, context, [callback = std::move(callback)] { callback(); });
}

} // namespace EditorNS

#endif
