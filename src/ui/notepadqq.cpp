#include "include/notepadqq.h"

const QString Notepadqq::version = POINTVERSION;
const QString Notepadqq::membersUrl = "https://github.com/notepadqq/notepadqq/network/members";

QString Notepadqq::copyright()
{
    return QObject::trUtf8("Copyright © 2010-2014, Daniele Di Sarli");
}

QString Notepadqq::editorPath()
{
    return QString("%1/editor/index.html").arg(qApp->applicationDirPath());;
}
