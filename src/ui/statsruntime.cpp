#include "include/statsruntime.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace StatsRuntime {

namespace {

// Returns the shared telemetry manager, parented to the application so it lives for the process lifetime.
QNetworkAccessManager* manager()
{
    static auto* networkManager = new QNetworkAccessManager(qApp);
    return networkManager;
}

} // namespace

bool isTransmissionDue(qint64 lastSeconds, qint64 nowSeconds)
{ return (nowSeconds - lastSeconds) >= 7 * 24 * 60 * 60; }

void startTimers(QObject* owner,
    std::function<void()> check,
    std::chrono::milliseconds startupDelay,
    std::chrono::milliseconds periodicInterval)
{
    auto periodicCheck = check;
    auto* startupTimer = new QTimer(owner);
    startupTimer->setTimerType(Qt::VeryCoarseTimer);
    startupTimer->setSingleShot(true);
    QObject::connect(startupTimer, &QTimer::timeout, owner, [check = std::move(check)]() { check(); });
    startupTimer->start(startupDelay);

    auto* periodicTimer = new QTimer(owner);
    periodicTimer->setTimerType(Qt::VeryCoarseTimer);
    QObject::connect(periodicTimer, &QTimer::timeout, owner, [periodicCheck] { periodicCheck(); });
    periodicTimer->start(periodicInterval);
}

QNetworkReply* post(const QNetworkRequest& request, const QByteArray& body, std::chrono::milliseconds timeout)
{ return post(manager(), request, body, timeout); }

QNetworkReply* post(QNetworkAccessManager* networkManager,
    const QNetworkRequest& request,
    const QByteArray& body,
    std::chrono::milliseconds timeout)
{
    QNetworkReply* reply = networkManager->post(request, body);
    auto* timer = new QTimer(reply);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, reply, [reply] { reply->abort(); });
    QObject::connect(reply, &QNetworkReply::finished, reply, [reply, timer] {
        timer->stop();
        timer->deleteLater();
        reply->deleteLater();
    });
    timer->start(timeout);
    return reply;
}

} // namespace StatsRuntime
