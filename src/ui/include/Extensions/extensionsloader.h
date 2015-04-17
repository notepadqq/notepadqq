#ifndef EXTENSIONLOADER_H
#define EXTENSIONLOADER_H

#include <QObject>
#include "include/Extensions/extension.h"
#include "include/Extensions/extensionsserver.h"

namespace Extensions {

    class ExtensionsLoader : public QObject
    {
        Q_OBJECT
    public:
        static QSharedPointer<ExtensionsServer> startExtensionsServer(QString address);
        static void loadExtensions(QString path);
        static QMap<QString, Extension *> loadedExtensions();
        static QSharedPointer<ExtensionsServer> extensionsServer();

    signals:

    public slots:

    private:
        explicit ExtensionsLoader(QObject *parent = 0);
        ~ExtensionsLoader();

        static QSharedPointer<ExtensionsServer> m_extensionsServer;
        static QMap<QString, Extension*> m_extensions;
    };

}

#endif // EXTENSIONLOADER_H
