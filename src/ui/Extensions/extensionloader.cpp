#include "include/Extensions/extensionloader.h"
#include "include/mainwindow.h"
#include <QDirIterator>

namespace Extensions {

    QSharedPointer<ExtensionsServer> ExtensionLoader::m_extensionsServer;
    QMap<QString, Extension*> ExtensionLoader::m_extensions;

    ExtensionLoader::ExtensionLoader(QObject *parent) : QObject(parent)
    {

    }

    ExtensionLoader::~ExtensionLoader()
    {

    }

    void ExtensionLoader::startExtensionServer(QString address)
    {
        QSharedPointer<Extensions::RuntimeSupport> rts =
                QSharedPointer<Extensions::RuntimeSupport>(new Extensions::RuntimeSupport());

        m_extensionsServer = QSharedPointer<Extensions::ExtensionsServer>(
                    new Extensions::ExtensionsServer(rts));

        m_extensionsServer->startServer(address);
    }

    void ExtensionLoader::loadExtensions(QString path)
    {
        QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            Extension *ext = new Extension(it.next()); // FIXME SharedPointer
            m_extensions.insert(ext->id(), ext);
        }
    }

    QMap<QString, Extension*> ExtensionLoader::loadedExtensions()
    {
        return m_extensions;
    }

}
