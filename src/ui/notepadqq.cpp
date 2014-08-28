#include "include/notepadqq.h"

const QString Notepadqq::version = POINTVERSION;
const QString Notepadqq::membersUrl = "https://github.com/notepadqq/notepadqq/network/members";

QString Notepadqq::m_cachedEditorPath = QString();

QString Notepadqq::copyright()
{
    return QObject::trUtf8("Copyright © 2010-2014, Daniele Di Sarli");
}

QString Notepadqq::editorPath()
{
    if (!m_cachedEditorPath.isNull())
        return m_cachedEditorPath;

    QString def = QString("%1/editor/index.html").arg(qApp->applicationDirPath());
    if(!QFile(def).exists())
        def = QString("%1/../share/%2/editor/index.html").
                arg(qApp->applicationDirPath()).
                arg(qApp->applicationName().toLower());

    m_cachedEditorPath = def;
    return def;
}
