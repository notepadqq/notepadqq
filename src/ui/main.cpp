#include "include/globals.h"
#include "include/mainwindow.h"
#include "include/notepadqq.h"
#include "include/EditorNS/editor.h"
#include "include/singleapplication.h"
#include "include/Extensions/extensionsloader.h"
#include "include/nqqsettings.h"
#include "include/stats.h"
#include <QObject>
#include <QFile>
#include <QtGlobal>
#include <QTranslator>
#include <QLocale>
#include <QDateTime>
#include <unistd.h> // For getuid

#ifdef QT_DEBUG
#include <QElapsedTimer>
#endif

void forceDefaultSettings();
void loadExtensions();

int main(int argc, char *argv[])
{
    QTranslator translator;
#ifdef QT_DEBUG
    QElapsedTimer __aet_timer;
    __aet_timer.start();
    qDebug() << "Start-time benchmark started.";

    printerrln("WARNING: Notepadqq is running in DEBUG mode.");
#endif

    // Initialize random number generator
    qsrand(QDateTime::currentDateTimeUtc().time().msec() + qrand());

#if QT_VERSION > QT_VERSION_CHECK(5, 6, 0)
    SingleApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    SingleApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    SingleApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Notepadqq");
    QCoreApplication::setApplicationName("Notepadqq");
    QCoreApplication::setApplicationVersion(Notepadqq::version);

    QSettings::setDefaultFormat(QSettings::IniFormat);


    NqqSettings::ensureBackwardsCompatibility();
    NqqSettings& settings = NqqSettings::getInstance();
    settings.General.setNotepadqqVersion(POINTVERSION);

    forceDefaultSettings();

    // Initialize from system locale on first run, if no system locale is
    // set, our default will be used instead.
    if (settings.General.getLocalization().isEmpty()) {
        QLocale locale;
        // ISO 639 dictates language code will always be 2 letters
        if (locale.name().size() >= 2) {
            settings.General.setLocalization(locale.name().left(2));
        } else {
            settings.General.setLocalization("en");
        }
    }

    QString langCode = settings.General.getLocalization();
    if (translator.load(QLocale(langCode),
                        QString("%1").arg(qApp->applicationName().toLower()),
                        QString("_"),
                        QString(":/translations"))) {
        a.installTranslator(&translator);
    } else {
        settings.General.setLocalization("en");
    }
    // Check for "run-and-exit" options like -h or -v
    const auto parser = Notepadqq::getCommandLineArgumentsParser(QApplication::arguments());

    // Check if we're running as root
    if( getuid() == 0 && !parser->isSet("allow-root") ) {
        qWarning() << QObject::tr("Running Notepadqq as root is not recommended. Use --allow-root if you really want to.");
        return EXIT_SUCCESS;
    }

    if (a.attachToOtherInstance()) {
        return EXIT_SUCCESS;
    }

    // Arguments received from another instance
    QObject::connect(&a, &SingleApplication::receivedArguments, &a, [=](const QString &workingDirectory, const QStringList &arguments) {
        QSharedPointer<QCommandLineParser> parser = Notepadqq::getCommandLineArgumentsParser(arguments);
        if (parser->isSet("new-window")) {
            // Open a new window
            MainWindow *win = new MainWindow(workingDirectory, arguments, 0);
            win->show();
        } else {
            // Send the args to the last focused window
            MainWindow *win = MainWindow::lastActiveInstance();
            if (win != nullptr) {
                win->openCommandLineProvidedUrls(workingDirectory, arguments);

                // Activate the window
                win->hide();
                win->show();
                win->setWindowState((win->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
                win->raise();
                win->activateWindow();
            }
        }
    });

    // There are no other instances: start a new server.
    a.startServer();

    Editor::addEditorToBuffer();

    QFile file(Notepadqq::editorPath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Can't open file: " + file.fileName();
        return EXIT_FAILURE;
    }
    file.close();

    if (Extensions::ExtensionsLoader::extensionRuntimePresent()) {
        Extensions::ExtensionsLoader::startExtensionsServer();
        Extensions::ExtensionsLoader::loadExtensions(Notepadqq::extensionsPath());
    } else {
#ifdef QT_DEBUG
        qDebug() << "Extension support is not installed.";
#endif
    }

    MainWindow *w = new MainWindow(QApplication::arguments(), 0);
    w->show();

#ifdef QT_DEBUG
    qint64 __aet_elapsed = __aet_timer.nsecsElapsed();
    qDebug() << QString("Started in " + QString::number(__aet_elapsed / 1000 / 1000) + "msec").toStdString().c_str();
#endif

    Stats::init();

    return a.exec();
}

void forceDefaultSettings()
{
    NqqSettings& s = NqqSettings::getInstance();

    // Use tabs to indent makefile by default
    if(!s.Languages.hasUseDefaultSettings("makefile")) {
        s.Languages.setUseDefaultSettings("makefile", false);
        s.Languages.setIndentWithSpaces("makefile", false);
    }

    // Use two spaces to indent ruby by default
    if(!s.Languages.hasUseDefaultSettings("ruby")) {
        s.Languages.setUseDefaultSettings("ruby", false);
        s.Languages.setTabSize("ruby", 2);
        s.Languages.setIndentWithSpaces("ruby",true);
    }


}
