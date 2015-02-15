#ifndef EXTENSION_H
#define EXTENSION_H

#include <QObject>
#include <QScriptEngine>

class Extension : public QObject
{
    Q_OBJECT
public:
    explicit Extension(QString path, QObject *parent = 0);
    ~Extension();

    Q_INVOKABLE QString id() const;
    Q_INVOKABLE QString name() const;
signals:

public slots:

private:
    QString m_extensionId;
    QScriptEngine *m_uiScriptEngine;
    void failedToLoadMessage(QString path);
};

#endif // EXTENSION_H
