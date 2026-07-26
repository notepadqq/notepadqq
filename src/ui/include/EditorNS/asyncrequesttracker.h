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
#include <stdexcept>

#include <optional>

class QTimer;

namespace EditorNS {

class Editor;

/**
 * Owns bounded editor bridge requests until a matching reply, timeout, or teardown settles them.
 * Each tracked request owns a child timer; unknown and late reply IDs are ignored.
 */
class AsyncRequestTracker : public QObject {
public:
    /** Creates a tracker whose child request timers all use the supplied timeout. */
    AsyncRequestTracker(QObject* parent, std::chrono::milliseconds timeout);

    /** Rejects every still-pending request before QObject destroys the owned timers. */
    ~AsyncRequestTracker() override;

    /** Registers a QtPromise request and starts its timeout timer. */
    void trackPromise(unsigned int id,
        QString message,
        QtPromise::QPromiseResolve<QVariant> resolve,
        QtPromise::QPromiseReject<QVariant> reject);

    /** Registers a legacy future request; its callback is retained only for successful replies. */
    void trackLegacy(unsigned int id,
        QString message,
        std::shared_ptr<std::promise<QVariant>> promise,
        std::function<void(QVariant)> callback);

    /** Settles and removes a matching request, returning false for unknown or late reply IDs. */
    bool resolve(unsigned int id, const QVariant& data);

    /** Returns the number of requests that have not yet been settled. */
    int pendingCount() const;

    /** Removes all pending entries before rejecting them with std::runtime_error. */
    void rejectAll();

private:
    friend class Editor;

    /** Holds one completion mechanism and its tracker-owned single-shot timeout timer. */
    struct PendingRequest {
        QString message;
        std::optional<QtPromise::QPromiseResolve<QVariant>> resolve;
        std::optional<QtPromise::QPromiseReject<QVariant>> reject;
        std::shared_ptr<std::promise<QVariant>> legacyPromise;
        std::function<void(QVariant)> callback;
        QTimer* timer = nullptr;
    };

    /** Removes and rejects one request after its timer expires; legacy callbacks are not invoked. */
    void reject(unsigned int id);

    /** Removes a request and destroys its timer before user completion code runs. */
    PendingRequest take(unsigned int id);

    /** Inserts a request before starting its tracker-owned single-shot timer. */
    void startTimer(unsigned int id, PendingRequest request);

    /** Builds the common runtime error used for timeout and teardown rejection. */
    static std::runtime_error timeoutError(const QString& message);

    /** Lets Editor suppress deferred emission after this request has already settled. */
    bool isPending(unsigned int id) const;

    QHash<unsigned int, PendingRequest> m_pendingRequests;
    std::chrono::milliseconds m_timeout;
};

} // namespace EditorNS

#endif // ASYNCREQUESTTRACKER_H
