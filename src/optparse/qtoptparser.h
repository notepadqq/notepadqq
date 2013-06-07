#ifndef QTOPTPARSER_H
#define QTOPTPARSER_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QPair>
#include <QList>
#include <QStringList>

class QtOptParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString _cmdShortSwitch READ shortSwitch WRITE setShortSwitch)
    Q_PROPERTY(QString _cmdLongSwitch  READ longSwitch  WRITE setLongSwitch )

public:
    explicit QtOptParser(QObject *parent = 0);

    const QString & shortSwitch() const;
    void setShortSwitch(const QString & value);

    const QString & longSwitch() const;
    void setLongSwitch(const QString & value);

    void addSwitch(const QString & shortName, const QString & longName);
    void addOption(const QString & shortName, const QString & longName);

    bool parse( const QStringList & arguments );

    bool hasOption( const QString & name );
    QString getOptValue( const QString & name );

    void printHelp   ();
    void printVersion();

signals:

public slots:

private:

    class QtOption : public QPair<QString, QString> {
    public:
        QtOption() : QPair<QString, QString>() { }
        QtOption( const QString & shortN, const QString & longN, bool hasValue )
            : QPair<QString, QString>(shortN, longN), _hasValue(hasValue) { }

        const QString & shortName   () const { return first ;  }
        const QString & longName    () const { return second;  }
        const QString & description () const { return _desc ;  }

        void setDescription(const QString & v) { _desc = v; }

        bool hasValue() const { return _hasValue; }

        const QString & value() const { return _value; }
        void setValue(const QString & v) { _value = v; }

    private:
        bool _hasValue;
        QString _value;
        QString _desc;
    };

    QString _cmdShortSwitch;
    QString _cmdLongSwitch ;

    QMap <QString, QtOption> _appOpts;        // true if it's an option, false if it's a switch
    QList<QtOption>          _appOptsDinst;   // distinct list
    QHash<QString, QtOption> _parsedOpts;     // name->value collection
    QStringList              _positionalOpts; // list of positional options
};

#endif // QTOPTPARSER_H
