#include "include/Extensions/extension.h"
#include "include/Extensions/extensionsapi.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

Extension::Extension(QString path, QObject *parent) : QObject(parent)
{
    m_extensionId = path + "-" + QTime::currentTime().msec() + "-" + QString::number(rand() * 1048576);

    QFile fManifest(path + "/manifest.json");
    if (fManifest.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&fManifest);
        QString content = in.readAll();
        fManifest.close();

        QJsonParseError err;
        QJsonDocument manifest = QJsonDocument::fromJson(content.toUtf8(), &err);

        if (err.error != QJsonParseError::NoError) {
            failedToLoadExtension(path, "manifest.json: " + err.errorString());
            return;
        }

        m_name = manifest.object().value("name").toString();

        if (m_name.isEmpty()) {
            failedToLoadExtension(path, "name missing or invalid");
            return;
        }

    } else {
        failedToLoadExtension(path, "manifest.json missing");
        return;
    }


    QFile fUi(path + "/ui.js");
    if (fUi.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&fUi);
        QString content = in.readAll();
        fUi.close();

        m_uiScriptEngine = new QScriptEngine(this);

        QObject *nqq = ExtensionsApi::instance();
        QScriptValue nqq_val = m_uiScriptEngine->newQObject(nqq);
        m_uiScriptEngine->globalObject().setProperty("nqq", nqq_val);

        QObject *ext = this;
        QScriptValue ext_val = m_uiScriptEngine->newQObject(ext);
        m_uiScriptEngine->globalObject().setProperty("extension", ext_val);

        m_uiScriptEngine->evaluate(content);

    } else {
        failedToLoadExtension(path, "ui.js missing");
        return;
    }
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
