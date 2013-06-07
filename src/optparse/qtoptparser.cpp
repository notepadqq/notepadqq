#include "qtoptparser.h"
#include "constants.h"
#include <QTextStream>
#include <QApplication>
#include <QMessageBox>
#define STR_NOT_LEGAL(x) ( x.isEmpty() || x.isNull() )


QtOptParser::QtOptParser(QObject *parent) :
    QObject(parent), _cmdLongSwitch("--")
{
#ifdef Q_OS_WIN32
    _cmdShortSwitch = "/";
#else
    _cmdShortSwitch = "-";
#endif
}

const QString & QtOptParser::shortSwitch() const
{
    return _cmdShortSwitch;
}

void QtOptParser::setShortSwitch(const QString & value)
{
    _cmdShortSwitch = value;
}

const QString & QtOptParser::longSwitch() const
{
    return _cmdLongSwitch;
}

void QtOptParser::setLongSwitch(const QString & value)
{
    _cmdLongSwitch = value;
}

void QtOptParser::addSwitch(const QString & shortName, const QString & longName)
{
    if ( STR_NOT_LEGAL(shortName) && STR_NOT_LEGAL(longName) )
        return;
    if ( _appOpts.contains( shortName ) || _appOpts.contains( longName ) )
        return;
    QtOption o(shortName, longName, false);
    if ( !STR_NOT_LEGAL(shortName) )
        _appOpts.insert(shortName, o);
    if ( !STR_NOT_LEGAL(longName)  )
        _appOpts.insert(longName, o);
    _appOptsDinst.append(o);
}

void QtOptParser::addOption(const QString & shortName, const QString & longName)
{
    if ( STR_NOT_LEGAL(shortName) && STR_NOT_LEGAL(longName) )
        return;
    if ( _appOpts.contains( shortName ) || _appOpts.contains( longName ) )
        return;
    QtOption o(shortName, longName, true);
    if ( !STR_NOT_LEGAL(shortName) )
        _appOpts.insert(shortName, o);
    if ( !STR_NOT_LEGAL(longName)  )
        _appOpts.insert(longName, o);
    _appOptsDinst.append(o);
}

bool QtOptParser::parse( const QStringList & args )
{
    _parsedOpts    .clear();
    _positionalOpts.clear();

    for ( int i = 0; i < args.length(); ++i ) {
        QString name;

        if ( args[i].startsWith(shortSwitch()) )
            name = args[i].mid(shortSwitch().length());
        else if ( args[i].startsWith(longSwitch()) )
            name = args[i].mid(longSwitch().length());
        else  {
            _positionalOpts.append(args[i]);
            continue;
        }

        if ( _appOpts.contains( name ) ) {
            QtOption o = _appOpts[name];
            if ( o.hasValue() )
                o.setValue( args.at( ++i ) );
            if ( !STR_NOT_LEGAL(o.shortName()) )
                _parsedOpts.insert(o.shortName(), o);
            if ( !STR_NOT_LEGAL(o.longName()) )
                _parsedOpts.insert(o.longName (), o);
        } else {
            return false; // BAD ARGUMENT SPECIFIED
        }
    }
    return true;
}

bool QtOptParser::hasOption( const QString & name )
{
    return _parsedOpts.contains(name);
}

QString QtOptParser::getOptValue( const QString & name )
{
    if ( !hasOption(name) || !_parsedOpts[name].hasValue() )
        return QString::null;
    return _parsedOpts[name].value();
}


// Helpers for displaying messages. Note that there is no console on Windows.
#ifdef Q_OS_WIN
// Format as <pre> HTML
static inline void toHtml(QString &t)
{
    t.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    t.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    t.replace(QLatin1Char('>'), QLatin1String("&gt;"));
    t.insert(0, QLatin1String("<html><pre>"));
    t.append(QLatin1String("</pre></html>"));
}

static void displayHelpText(QString t) // No console on Windows.
{
    toHtml(t);
    QMessageBox::information(0, qApp->applicationName(), t);
}

static void displayError(const QString &t) // No console on Windows.
{
    QMessageBox::critical(0, qApp->applicationName(), t);
}

#else

static void displayHelpText(const QString &t)
{
    qWarning("%s", qPrintable(t));
}

static void displayError(const QString &t)
{
    qCritical("%s", qPrintable(t));
}

#endif

static void printVersionS()
{
    QString version;
    QTextStream str(&version);
    str << '\n' << qApp->applicationName() << ' ' << VERSION << " based on Qt " << qVersion() << "\n\n";
    displayHelpText(version);
}

static void printHelpS(const QString &a0, const QString & options)
{
    QString help;
    QTextStream str(&help);
    str << "Usage: " << a0 << "\n" << options;
    displayHelpText(help);
}

void QtOptParser::printHelp()
{
    QString dump;
    QTextStream str(&dump);

    foreach ( QtOption o, _appOptsDinst ) {
        if ( !STR_NOT_LEGAL(o.shortName()) )
            str << "\t" << shortSwitch() << o.shortName();
        if ( !STR_NOT_LEGAL(o.longName()) )
            str << "\t" << longSwitch() << o.longName();
        str << "\n";
    }
    printHelpS( QFileInfo(qApp->applicationFilePath()).baseName(), dump );
}

void QtOptParser::printVersion()
{
    printVersionS();
}

