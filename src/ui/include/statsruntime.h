#ifndef STATSRUNTIME_H
#define STATSRUNTIME_H

#include <chrono>
#include <functional>

class QObject;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

namespace StatsRuntime {

/**
 * @brief Return whether the weekly statistics interval has elapsed.
 */
bool isTransmissionDue(qint64 lastSeconds, qint64 nowSeconds);

/**
 * @brief Create the startup and periodic timers that drive the statistics check.
 */
void startTimers(QObject* owner,
    std::function<void()> check,
    std::chrono::milliseconds startupDelay,
    std::chrono::milliseconds periodicInterval);

/**
 * @brief Post telemetry with a bounded reply lifetime.
 */
QNetworkReply* post(const QNetworkRequest& request,
    const QByteArray& body,
    std::chrono::milliseconds timeout = std::chrono::seconds(10));

/**
 * @brief Post telemetry through a caller-supplied manager, primarily for tests.
 */
QNetworkReply* post(QNetworkAccessManager* manager,
    const QNetworkRequest& request,
    const QByteArray& body,
    std::chrono::milliseconds timeout);

} // namespace StatsRuntime

#endif // STATSRUNTIME_H
