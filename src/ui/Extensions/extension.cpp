#include "include/Extensions/extension.h"
#include "include/Extensions/extensionsapi.h"
#include <QFile>
#include <QTextStream>

Extension::Extension(QString path, QObject *parent) : QObject(parent)
{
    m_extensionId = path + "-" + QTime::currentTime().msec() + "-" + QString::number(rand() * 1048576);

    QFile f(path + "/ui.js");
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&f);
        QString content = in.readAll();

        m_uiScriptEngine = new QScriptEngine(this);

        QObject *nqq = ExtensionsApi::instance();
        QScriptValue nqq_val = m_uiScriptEngine->newQObject(nqq);
        m_uiScriptEngine->globalObject().setProperty("nqq", nqq_val);

        QObject *ext = this;
        QScriptValue ext_val = m_uiScriptEngine->newQObject(ext);
        m_uiScriptEngine->globalObject().setProperty("extension", ext_val);

        m_uiScriptEngine->evaluate(content);
    } else {
        failedToLoadMessage(path);
    }
}

Extension::~Extension()
{

}

void Extension::failedToLoadMessage(QString path)
{
    qWarning() << QString("Failed to load " + path).toStdString().c_str();
}

QString Extension::id() const
{
    return m_extensionId;
}

QString Extension::name() const
{
    return "Unnamed Extension";
}
