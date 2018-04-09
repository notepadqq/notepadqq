#include "include/stats.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSysInfo>
#include "include/notepadqq.h"

void Stats::init()
{
    // 0. Check whether the user wants us to collect stats. If not, return.
    // 1. Check when was last time stats were collected
    // 2. If it's been a week or more, transmit the new stats
    // 3. Start a timer that will send stats again in a week from now,
    //    in case the user never shuts down his computer.

    QJsonObject data;
    data["version"] = QString(POINTVERSION);
    data["qt_version"] = QString(qVersion());
    data["os"] = QSysInfo::productType();
    data["os_version"] = QSysInfo::productVersion();

    // The other supported fields for now are:
    /*
     - extensions      (string)
     - extension_count (int)
    */

    Stats::send(data);
}


void Stats::send(const QJsonObject &data) {
    QUrl url("https://notepadqq.com/api/stat/post.php");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/javascript");

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QObject::connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply){
        manager->deleteLater();
    });

    QJsonDocument doc;
    doc.setObject(data);

    manager->post(request, doc.toJson());
}
