#include "include/Extensions/extensionloader.h"
#include "include/mainwindow.h"
#include <QDirIterator>

QList<Extension*> ExtensionLoader::m_extensions;

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

    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        QString ext = it.next();
        m_extensions.append(new Extension(ext));
    }
}
