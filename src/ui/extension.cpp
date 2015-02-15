#include "include/extension.h"
#include "include/Extensions/extensionsapi.h"
#include <QFile>
#include <QTextStream>

Extension::Extension(QObject *parent) : QObject(parent)
{
    QFile f("/home/daniele/Progetti/qt/notepadqq-exts/test1/ui.js");
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&f);
        QString content = in.readAll();

        m_uiScriptEngine = new QScriptEngine(this);

        QObject *nqq = ExtensionsApi::instance();
        QScriptValue nqq_val = m_uiScriptEngine->newQObject(nqq);
        m_uiScriptEngine->globalObject().setProperty("nqq", nqq_val);

        m_uiScriptEngine->evaluate(content);
    }
}

Extension::~Extension()
{

}

