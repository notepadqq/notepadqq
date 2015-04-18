#include "include/Extensions/extension.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTime>
#include <QDebug>

namespace Extensions {

    Extension::Extension(QString path, QString serverSocketPath) : QObject(0)
    {
        m_extensionId = path + "-" + QTime::currentTime().msec() + "-" + QString::number(rand() * 1048576);

        QFile fManifest(path + "/manifest.json");
        if (fManifest.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&fManifest);
            QString content = in.readAll();
            fManifest.close();

            QJsonParseError err;
            QJsonDocument manifestDoc = QJsonDocument::fromJson(content.toUtf8(), &err);

            if (err.error != QJsonParseError::NoError) {
                failedToLoadExtension(path, "manifest.json: " + err.errorString());
                return;
            }

            QJsonObject manifest = manifestDoc.object();
            m_name = manifest.value("name").toString();

            if (m_name.isEmpty()) {
                failedToLoadExtension(path, "name missing or invalid");
                return;
            }

            QProcess *process = new QProcess(this);
            QStringList args;
            args << path + "/main.js";
            args << serverSocketPath;
            process->start("node", args); // FIXME Start package.json

        } else {
            failedToLoadExtension(path, "manifest.json missing");
            return;
        }


        /*QFile fUi(path + "/ui.js");
        if (fUi.open(QFile::ReadOnly | QFile::Text)) {


        } else {
            failedToLoadExtension(path, "ui.js missing");
            return;
        }*/
    }

    Extension::~Extension()
    {

    }

    void Extension::failedToLoadExtension(QString path, QString reason)
    {
        // FIXME Mark extension as broken
        qWarning() << QString("Failed to load %1: %2").arg(path).arg(reason).toStdString().c_str();
    }

    QString Extension::id() const
    {
        return m_extensionId;
    }

    QString Extension::name() const
    {
        return m_name;
    }

}
