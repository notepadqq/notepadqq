#include "include/Extensions/extension.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QDebug>
#include <QSettings>

namespace Extensions {

    Extension::Extension(QString path, QString serverSocketPath) : QObject(0)
    {
        m_extensionId = path + "-" + QTime::currentTime().msec() + "-" + QString::number(qrand());

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

            QString runtime = manifest.value("runtime").toString().toLower();
            QString runtimeVersion = manifest.value("runtimeVersion").toString().toLower();
            QString main = manifest.value("main").toString();

            if (runtime == "ruby") {

                QProcess *process = new QProcess(this);
                process->setProcessChannelMode(QProcess::ForwardedChannels);
                process->setWorkingDirectory(path);

                QStringList args;
                args << main;
                args << serverSocketPath;
                args << m_extensionId;

                QSettings s;
                QString nodePath = s.value("Extensions/Runtime_Ruby2.1", "").toString();

                connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(on_processError(QProcess::ProcessError)));

                process->start(nodePath, args);

                // FIXME Handle QProcess 'error' event
            }

        } else {
            failedToLoadExtension(path, "manifest.json missing");
            return;
        }

        // FIXME Load editor parts
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

    void Extension::on_processError(QProcess::ProcessError error)
    {
        if (error == QProcess::FailedToStart) {
            failedToLoadExtension(m_name, "failed to start. Check your runtime."); // FIXME Add info about the missing runtime
        } else if (error == QProcess::Crashed) {
            qWarning() << QString("%1 crashed.").arg(m_name).toStdWString().c_str();
        }
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
