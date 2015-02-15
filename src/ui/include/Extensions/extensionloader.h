#ifndef EXTENSIONLOADER_H
#define EXTENSIONLOADER_H

#include <QObject>
#include "include/Extensions/extension.h"

class ExtensionLoader : public QObject
{
    Q_OBJECT
public:
    static void loadExtensions(QString path);

signals:

public slots:

private:
    explicit ExtensionLoader(QObject *parent = 0);
    ~ExtensionLoader();

    static QList<Extension*> m_extensions;
};

#endif // EXTENSIONLOADER_H
