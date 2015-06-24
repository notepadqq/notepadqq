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
        /**
         * @brief Starts an extensions server with a new unique name.
         * @return
         */
        static QSharedPointer<Extensions::ExtensionsServer> startExtensionsServer();
        static QSharedPointer<ExtensionsServer> startExtensionsServer(QString name);
        static void loadExtensions(QString path);
        static QMap<QString, QSharedPointer<Extension>> loadedExtensions();
        static QSharedPointer<ExtensionsServer> extensionsServer();
        static bool extensionRuntimePresent();

    signals:

    public slots:

    private:
        explicit ExtensionsLoader(QObject *parent = 0);
        ~ExtensionsLoader();

        static QSharedPointer<ExtensionsServer> m_extensionsServer;
        static QMap<QString, QSharedPointer<Extension>> m_extensions;
    };

}

#endif // EXTENSIONLOADER_H
