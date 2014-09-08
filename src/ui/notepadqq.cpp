#include "include/notepadqq.h"
#include <QFileInfo>

const QString Notepadqq::version = POINTVERSION;
const QString Notepadqq::contributorsUrl = "https://github.com/notepadqq/notepadqq/blob/master/CONTRIBUTORS.md";
const QString Notepadqq::website = "http://notepadqq.altervista.org";
QCommandLineParser *Notepadqq::m_commandLineParameters = 0;

QString Notepadqq::copyright()
{
    return QObject::trUtf8("Copyright © 2010-2014, Daniele Di Sarli");
}

QString Notepadqq::editorPath()
{
    QString def = QString("%1/../appdata/editor/index.html").
            arg(qApp->applicationDirPath());

    if(!QFile(def).exists())
        def = QString("%1/../share/%2/editor/index.html").
                arg(qApp->applicationDirPath()).
                arg(qApp->applicationName().toLower());

    return def;
}

QString Notepadqq::fileNameFromUrl(const QUrl &url)
{
    return QFileInfo(url.toDisplayString(
                         QUrl::RemoveScheme |
                         QUrl::RemovePassword |
                         QUrl::RemoveUserInfo |
                         QUrl::RemovePort |
                         QUrl::RemoveAuthority |
                         QUrl::RemoveQuery |
                         QUrl::RemoveFragment |
                         QUrl::PreferLocalFile )
                     ).fileName();
}

void Notepadqq::parseCommandLineParameters()
{
    if (m_commandLineParameters != 0)
        return;

    QCommandLineParser *parser = new QCommandLineParser();
    parser->setApplicationDescription("Text editor for developers");
    parser->addHelpOption();
    parser->addVersionOption();

    /*QCommandLineOption newDocumentOption("new-document",
                                         QObject::tr("Create a new document in an existing instance of %1.")
                                         .arg(QCoreApplication::applicationName()));
    parser.addOption(newDocumentOption);*/

    parser->addPositionalArgument("urls",
                                 QObject::tr("Files to open."),
                                 "[urls...]");

    parser->process(QCoreApplication::arguments());

    m_commandLineParameters = parser;
}

QCommandLineParser *Notepadqq::commandLineParameters()
{
    return m_commandLineParameters;
}
