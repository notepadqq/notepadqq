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

void Stats::init()
{
    // 0. Check whether the user wants us to collect stats. If not, return.
    // 1. Check when was last time stats were collected
    // 2. If it's been a week or more, transmit the new stats
    // 3. Start a timer that will send stats again in a week from now,
    //    in case the user never shuts down his computer.

    QTimer* t = new QTimer();
    QObject::connect(t, &QTimer::timeout, [t](){
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

        Stats::send(data);

        t->deleteLater();
    });

    // Start after 10 seconds: we don't want to take time to the startup sequence,
    // and we want the extensions to be fully loaded.
    t->start(10000);
}


void Stats::send(const QJsonObject &data) {
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
