#include "include/Extensions/extensionsloader.h"
#include "include/mainwindow.h"
#include <QDirIterator>

namespace Extensions {

    QSharedPointer<ExtensionsServer> ExtensionsLoader::m_extensionsServer;
    QMap<QString, Extension*> ExtensionsLoader::m_extensions;

    ExtensionsLoader::ExtensionsLoader(QObject *parent) : QObject(parent)
    {

    }

    ExtensionsLoader::~ExtensionsLoader()
    {

    }

    QSharedPointer<Extensions::ExtensionsServer> ExtensionsLoader::startExtensionsServer(QString address)
    {
        QSharedPointer<Extensions::RuntimeSupport> rts =
                QSharedPointer<Extensions::RuntimeSupport>(new Extensions::RuntimeSupport());

        m_extensionsServer = QSharedPointer<Extensions::ExtensionsServer>(
                    new Extensions::ExtensionsServer(rts));

        m_extensionsServer->startServer(address);

        return m_extensionsServer;
    }

    void ExtensionsLoader::loadExtensions(QString path)
    {
        QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            Extension *ext = new Extension(it.next()); // FIXME SharedPointer
            m_extensions.insert(ext->id(), ext);
        }
    }

    QMap<QString, Extension*> ExtensionsLoader::loadedExtensions()
    {
        return m_extensions;
    }

    QSharedPointer<ExtensionsServer> ExtensionsLoader::extensionsServer()
    {
        return m_extensionsServer;
    }

}
