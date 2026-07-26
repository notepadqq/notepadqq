#ifndef ASYNCREQUESTTRACKER_H
#define ASYNCREQUESTTRACKER_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtPromise>

#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <stdexcept>

class QTimer;

namespace EditorNS {

class AsyncRequestTracker : public QObject {
public:
    AsyncRequestTracker(QObject* parent, std::chrono::milliseconds timeout);
    ~AsyncRequestTracker() override;

    void trackPromise(unsigned int id, QString message, QtPromise::QPromiseResolve<QVariant> resolve,
        QtPromise::QPromiseReject<QVariant> reject);
    void trackLegacy(unsigned int id, QString message, std::shared_ptr<std::promise<QVariant>> promise,
        std::function<void(QVariant)> callback);

    bool resolve(unsigned int id, const QVariant& data);
    int pendingCount() const;
    void rejectAll();

private:
    struct PendingRequest {
        QString message;
        std::optional<QtPromise::QPromiseResolve<QVariant>> resolve;
        std::optional<QtPromise::QPromiseReject<QVariant>> reject;
        std::shared_ptr<std::promise<QVariant>> legacyPromise;
        std::function<void(QVariant)> callback;
        QTimer* timer = nullptr;
    };

    void reject(unsigned int id);
    PendingRequest take(unsigned int id);
    void startTimer(unsigned int id, PendingRequest request);
    static std::runtime_error timeoutError(const QString& message);

    QHash<unsigned int, PendingRequest> m_pendingRequests;
    std::chrono::milliseconds m_timeout;
};

} // namespace EditorNS

#endif // ASYNCREQUESTTRACKER_H
