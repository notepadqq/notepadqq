#include "include/Extensions/extensionsloader.h"
#include "include/mainwindow.h"
#include <QDirIterator>

namespace Extensions {

    QSharedPointer<ExtensionsServer> ExtensionsLoader::m_extensionsServer;
    QMap<QString, QSharedPointer<Extension>> ExtensionsLoader::m_extensions;

    ExtensionsLoader::ExtensionsLoader(QObject *parent) : QObject(parent)
    {

    }

    ExtensionsLoader::~ExtensionsLoader()
    {

    }

    QSharedPointer<Extensions::ExtensionsServer> ExtensionsLoader::startExtensionsServer()
    {
        QString name = "notepadqq-exts-";
        name += QString::number(QDateTime::currentMSecsSinceEpoch());
        name += "-";
        name += QString::number(qrand());

        return startExtensionsServer(name);
    }

    QSharedPointer<Extensions::ExtensionsServer> ExtensionsLoader::startExtensionsServer(QString name)
    {
        QSharedPointer<Extensions::RuntimeSupport> rts =
                QSharedPointer<Extensions::RuntimeSupport>(new Extensions::RuntimeSupport());

        m_extensionsServer = QSharedPointer<Extensions::ExtensionsServer>(
                    new Extensions::ExtensionsServer(rts));

        m_extensionsServer->startServer(name);

        return m_extensionsServer;
    }

    void ExtensionsLoader::loadExtensions(QString path)
    {
        if (m_extensionsServer.isNull()) {
            return;
        }

        QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            QSharedPointer<Extension> ext = QSharedPointer<Extension>(
                    new Extension(it.next(), m_extensionsServer->socketPath()));
            m_extensions.insert(ext->id(), ext);
        }
    }

    QMap<QString, QSharedPointer<Extension>> ExtensionsLoader::loadedExtensions()
    {
        return m_extensions;
    }

    QSharedPointer<ExtensionsServer> ExtensionsLoader::extensionsServer()
    {
        return m_extensionsServer;
    }

}
