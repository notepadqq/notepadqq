#include "include/stats.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSysInfo>
#include <QTimer>
#include "include/notepadqq.h"
#include "include/Extensions/extensionsloader.h"

bool Stats::m_longTimerRunning = false;

void Stats::init()
{
    // Check whether the user wants us to collect stats. If not, return.
    NqqSettings &settings = NqqSettings::getInstance();
    if (!settings.General.getCollectStatistics()) {
        return;
    }

    // Start a timer that will check very soon if we need to send stats.
    QTimer* t = new QTimer();
    QObject::connect(t, &QTimer::timeout, [t](){
        Stats::check();
        t->deleteLater();
    });

    // Start after 10 seconds: we don't want to take time to the startup sequence,
    // and we want the extensions to be fully loaded.
    t->start(10000);


    // Also start another timer that will periodically check if a week has passed and
    // it's time to transmit new information.
    if (!m_longTimerRunning) {
        QTimer* tlong = new QTimer();
        QObject::connect(tlong, &QTimer::timeout, [t](){
            Stats::check();
        });

        tlong->start(12*60*60*1000); // Check every ~12 hours.

        m_longTimerRunning = true;
    }
}

void Stats::check() {
    // Check whether the user wants us to collect stats. If not, return.
    NqqSettings &settings = NqqSettings::getInstance();
    if (!settings.General.getCollectStatistics()) {
        return;
    }

    // TODO: Check last time that we sent stats, and atomically update the value.

    QJsonObject data;
    data["version"] = QString(POINTVERSION);
    data["qt_version"] = QString(qVersion());

#if QT_VERSION >= 0x050400
    data["os"] = QSysInfo::productType();
    data["os_version"] = QSysInfo::productVersion();
#endif

    auto extensions = Extensions::ExtensionsLoader::loadedExtensions();
    QJsonArray exts;
    for (const auto &ext : extensions.values()) { exts.append(ext->name()); }

    data["extensions"] = QString(QJsonDocument(exts).toJson());
    data["extension_count"] = extensions.count();

    Stats::remoteApiSend(data);
}

void Stats::remoteApiSend(const QJsonObject &data) {
    QUrl url("https://notepadqq.com/api/stat/post.php");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/javascript");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *){
        manager->deleteLater();
    });

    QJsonDocument doc;
    doc.setObject(data);

    manager->post(request, doc.toJson());
}
