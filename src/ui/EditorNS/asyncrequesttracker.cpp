#include "include/EditorNS/asyncrequesttracker.h"

#include <QTimer>

#include <algorithm>
#include <limits>
#include <utility>

namespace EditorNS {

AsyncRequestTracker::AsyncRequestTracker(QObject* parent, std::chrono::milliseconds timeout)
    : QObject(parent)
    , m_timeout(timeout)
{
}

AsyncRequestTracker::~AsyncRequestTracker()
{ rejectAll(); }

void AsyncRequestTracker::trackPromise(unsigned int id,
    QString message,
    QtPromise::QPromiseResolve<QVariant> resolve,
    QtPromise::QPromiseReject<QVariant> reject)
{
    PendingRequest request;
    request.message = std::move(message);
    request.resolve = std::move(resolve);
    request.reject = std::move(reject);
    startTimer(id, std::move(request));
}

void AsyncRequestTracker::trackLegacy(unsigned int id,
    QString message,
    std::shared_ptr<std::promise<QVariant>> promise,
    std::function<void(QVariant)> callback)
{
    PendingRequest request;
    request.message = std::move(message);
    request.legacyPromise = std::move(promise);
    request.callback = std::move(callback);
    startTimer(id, std::move(request));
}

bool AsyncRequestTracker::resolve(unsigned int id, const QVariant& data)
{
    if (!m_pendingRequests.contains(id))
        return false;

    PendingRequest request = take(id);
    if (request.resolve)
        (*request.resolve)(data);
    if (request.legacyPromise)
        request.legacyPromise->set_value(data);
    if (request.callback) {
        QTimer::singleShot(0, [callback = std::move(request.callback), data] { callback(data); });
    }
    return true;
}

int AsyncRequestTracker::pendingCount() const
{ return m_pendingRequests.size(); }

void AsyncRequestTracker::rejectAll()
{
    QHash<unsigned int, PendingRequest> requests;
    requests.swap(m_pendingRequests);

    for (PendingRequest& request : requests) {
        if (request.timer) {
            request.timer->stop();
            delete request.timer;
        }

        const auto error = timeoutError(request.message);
        if (request.reject)
            (*request.reject)(error);
        if (request.legacyPromise)
            request.legacyPromise->set_exception(std::make_exception_ptr(error));
    }
}

void AsyncRequestTracker::reject(unsigned int id)
{
    if (!m_pendingRequests.contains(id))
        return;

    PendingRequest request = take(id);
    const auto error = timeoutError(request.message);
    if (request.reject)
        (*request.reject)(error);
    if (request.legacyPromise)
        request.legacyPromise->set_exception(std::make_exception_ptr(error));
}

AsyncRequestTracker::PendingRequest AsyncRequestTracker::take(unsigned int id)
{
    PendingRequest request = m_pendingRequests.take(id);
    if (request.timer) {
        request.timer->stop();
        delete request.timer;
        request.timer = nullptr;
    }
    return request;
}

void AsyncRequestTracker::startTimer(unsigned int id, PendingRequest request)
{
    Q_ASSERT(!m_pendingRequests.contains(id));

    auto* timer = new QTimer(this);
    timer->setSingleShot(true);
    request.timer = timer;
    m_pendingRequests.insert(id, std::move(request));

    connect(timer, &QTimer::timeout, this, [this, id] { reject(id); });
    timer->start(
        static_cast<int>(std::min<std::chrono::milliseconds::rep>(m_timeout.count(), std::numeric_limits<int>::max())));
}

std::runtime_error AsyncRequestTracker::timeoutError(const QString& message)
{ return std::runtime_error(QStringLiteral("Timed out waiting for editor reply to %1").arg(message).toStdString()); }

bool AsyncRequestTracker::isPending(unsigned int id) const
{ return m_pendingRequests.contains(id); }

} // namespace EditorNS
