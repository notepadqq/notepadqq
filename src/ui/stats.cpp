#include "include/stats.h"

#include "include/Extensions/extensionsloader.h"
#include "include/notepadqq.h"

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QSysInfo>
#include <QTimer>
#include <QUrl>

bool Stats::m_longTimerRunning = false;
bool Stats::m_isFirstNotepadqqRun = false;

#define DIALOG_NEVER_SHOWN 0
#define DIALOG_ALREADY_SHOWN 1
#define DIALOG_FIRST_TIME_IGNORED 2

void Stats::init()
{
    NqqSettings &settings = NqqSettings::getInstance();

    Stats::askUserPermission();

    // Check whether the user wants us to collect stats. If not, return.
    if (!settings.General.getCollectStatistics()) {
        return;
    }

    // Start a timer that will check very soon if we need to send stats.
    QTimer* t = new QTimer();
    t->setTimerType(Qt::VeryCoarseTimer);
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
        tlong->setTimerType(Qt::VeryCoarseTimer);
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

    // Check if it is time to send the stats (i.e. a week has passed).
    // If not, return.
    if (!isTimeToSendStats()) {
        return;
    }
    settings.General.setLastStatisticTransmissionTime(currentUnixTimestamp());


    QJsonObject data;
    data["version"] = QString(POINTVERSION);
#ifndef BUILD_SNAP
    data["qt_version"] = QString(qVersion());
#else
    data["qt_version"] = QString(qVersion()) + " (Snap)";
#endif

#if QT_VERSION >= 0x050400
    data["os"] = QSysInfo::productType();
    data["os_version"] = QSysInfo::productVersion();
#endif

    auto extensions = Extensions::ExtensionsLoader::loadedExtensions();
    QJsonArray exts;
    for (const auto &ext : extensions.values()) { exts.append(ext->name()); }

    data["extensions"] = QString(QJsonDocument(exts).toJson(QJsonDocument::Compact));
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

    manager->post(request, doc.toJson(QJsonDocument::Compact));
}

void Stats::askUserPermission() {
    NqqSettings &settings = NqqSettings::getInstance();
    int dialogShown = settings.General.getStatisticsDialogShown();

    if (dialogShown == DIALOG_FIRST_TIME_IGNORED && !m_isFirstNotepadqqRun) {

        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("<h3>" + QObject::tr("Would you like to help?") + "</h3>");
        msgBox.setInformativeText("<html><body>"
            "<p>" + QObject::tr("You can help to improve Notepadqq by allowing us to collect <b>anonymous statistics</b>.") + "</p>" +
            "<b>" + QObject::tr("What will we collect?") + "</b><br>" +
            QObject::tr(
                "We will collect information such as the version of Qt, the version of the OS, or the number of extensions.<br>"
                "You don't have to trust us: Notepadqq is open source, so you can %1check by yourself%2 😊").
                      arg("<a href=\"https://github.com/notepadqq/notepadqq/blob/master/src/ui/stats.cpp\">").
                      arg("</a>") +
            "</body></html>");

        QAbstractButton *ok = msgBox.addButton(QObject::tr("Okay, I agree"), QMessageBox::AcceptRole);
        msgBox.addButton(QObject::tr("No"), QMessageBox::RejectRole);

        msgBox.exec();

        settings.General.setStatisticsDialogShown(DIALOG_ALREADY_SHOWN);

        if (msgBox.clickedButton() == ok) {
            settings.General.setCollectStatistics(true);
        } else {
            settings.General.setCollectStatistics(false);
        }

    } else if (dialogShown == DIALOG_NEVER_SHOWN) {
        // Set m_isFirstNotepadqqRun to true, so that next executions of this method within
        // the current process won't show the dialog even if we're setting
        // statisticsDialogShown = DIALOG_FIRST_TIME_IGNORED.
        m_isFirstNotepadqqRun = true;
        settings.General.setStatisticsDialogShown(DIALOG_FIRST_TIME_IGNORED);
    }
}

bool Stats::isTimeToSendStats() {
    NqqSettings &settings = NqqSettings::getInstance();
    return (currentUnixTimestamp() - settings.General.getLastStatisticTransmissionTime()) >= 7*24*60*60*1000;
}

qint64 Stats::currentUnixTimestamp() {
#if QT_VERSION >= 0x050800
    return QDateTime::currentDateTime().toSecsSinceEpoch();
#else
    return QDateTime::currentDateTime().toTime_t();
#endif
}
