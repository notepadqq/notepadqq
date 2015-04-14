#ifndef EXTENSIONLOADER_H
#define EXTENSIONLOADER_H

#include <QObject>
#include "include/Extensions/extension.h"
#include "include/Extensions/extensionsserver.h"

namespace Extensions {

    class ExtensionLoader : public QObject
    {
        Q_OBJECT
    public:
        static void startExtensionServer(QString address);
        static void loadExtensions(QString path);
        static QMap<QString, Extension *> loadedExtensions();

    signals:

    public slots:

    private:
        explicit ExtensionLoader(QObject *parent = 0);
        ~ExtensionLoader();

        static QSharedPointer<ExtensionsServer> m_extensionsServer;
        static QMap<QString, Extension*> m_extensions;
    };

}

#endif // EXTENSIONLOADER_H
