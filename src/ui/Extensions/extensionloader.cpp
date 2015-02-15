#include "include/Extensions/extensionloader.h"
#include "include/mainwindow.h"
#include <QDirIterator>

QMap<QString, Extension*> ExtensionLoader::m_extensions;

ExtensionLoader::ExtensionLoader(QObject *parent) : QObject(parent)
{

}

ExtensionLoader::~ExtensionLoader()
{

}

void ExtensionLoader::loadExtensions(QString path)
{
    qRegisterMetaType<MainWindow*>("MainWindow*");
    qRegisterMetaType<Editor*>("Editor*");
    qRegisterMetaType<QAction*>("QAction*");

    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        Extension *ext = new Extension(it.next());
        m_extensions.insert(ext->id(), ext);
    }
}

QMap<QString, Extension*> ExtensionLoader::loadedExtensions()
{
    return m_extensions;
}
