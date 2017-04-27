#include "include/notepadqq.h"
#include "include/Extensions/extensionsloader.h"
#include "include/Extensions/runtimesupport.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QCheckBox>
#include <QSettings>

const QString Notepadqq::version = POINTVERSION;
const QString Notepadqq::contributorsUrl = "https://github.com/notepadqq/notepadqq/blob/master/CONTRIBUTORS.md";
const QString Notepadqq::website = "http://notepadqq.altervista.org";

QString Notepadqq::copyright()
{
    return QObject::trUtf8("Copyright © 2010-%1, Daniele Di Sarli").arg(COPYRIGHT_YEAR);
}

QString Notepadqq::appDataPath(QString fileName)
{
#ifdef Q_OS_MACX
    QString def = QString("%1/../Resources/").
            arg(qApp->applicationDirPath());
#else
    QString def = QString("%1/../appdata/").
            arg(qApp->applicationDirPath());
#endif

    if(!QDir(def).exists())
        def = QString("%1/../../share/%2/").
                arg(qApp->applicationDirPath()).
                arg(qApp->applicationName().toLower());

    if (!fileName.isNull()) {
        def.append(fileName);
    }

    return def;
}

QString Notepadqq::editorPath()
{
    return appDataPath("editor/index.html");
}

QString Notepadqq::extensionToolsPath()
{
    return appDataPath("extension_tools");
}

QString Notepadqq::nodejsPath() {
    QSettings s;
    return s.value("Extensions/Runtime_Nodejs", "").toString();
}

QString Notepadqq::npmPath() {
    QSettings s;
    return s.value("Extensions/Runtime_Npm", "").toString();
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

QSharedPointer<QCommandLineParser> Notepadqq::getCommandLineArgumentsParser(const QStringList &arguments)
{
    QSharedPointer<QCommandLineParser> parser =
            QSharedPointer<QCommandLineParser>(new QCommandLineParser());

    parser->setApplicationDescription("Text editor for developers");
    parser->addHelpOption();
    parser->addVersionOption();

    QCommandLineOption newWindowOption("new-window",
                                         QObject::tr("Open a new window in an existing instance of %1.")
                                         .arg(QCoreApplication::applicationName()));
    parser->addOption(newWindowOption);

    parser->addPositionalArgument("urls",
                                 QObject::tr("Files to open."),
                                 "[urls...]");

    parser->process(arguments);

    return parser;
}

QString Notepadqq::extensionsPath()
{
    QSettings settings;

    QFileInfo f = QFileInfo(settings.fileName());
    return f.absoluteDir().absoluteFilePath("extensions");
}

QList<QString> Notepadqq::translations()
{
    QList<QString> out;

    QDir dir(":/translations");
    QStringList fileNames = dir.entryList(QStringList("notepadqq_*.qm"));

    // FIXME this can be removed if we create a .qm file for English too, which should exist for consistency purposes
    out.append("en");

    for (int i = 0; i < fileNames.size(); ++i) {
        // get locale extracted by filename
        QString langCode;
        langCode = fileNames[i]; // "notepadqq_de.qm"
        langCode.truncate(langCode.lastIndexOf('.')); // "notepadqq_de"
        langCode.remove(0, langCode.indexOf('_') + 1); // "de"

        out.append(langCode);
    }

    return out;
}
