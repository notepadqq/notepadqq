#include "include/filesystemwatcher.h"

#include <QDebug>

FileSystemWatcher::FileSystemWatcher()
{
    connect(&m_fsWatcher, &QFileSystemWatcher::fileChanged, this, &FileSystemWatcher::fileChanged);
}

void FileSystemWatcher::monitor(EditorNS::Editor* ed)
{
    QString path = ed->fileName().toLocalFile();

    if(path.isEmpty()) {
        qDebug() << "Attempting to monitor Editor with no file.";
        return;
    }

    m_watchedEditors.emplace(path, ed);
    m_fsWatcher.addPath(path);
}

void FileSystemWatcher::unmonitor(EditorNS::Editor* ed)
{
    QString path = ed->fileName().toLocalFile();

    auto it = m_watchedEditors.find(path);

    if(it == m_watchedEditors.end()) {
        qDebug() << "Attempting to unmonitor Editor that isn't monitored";
        return;
    }

    qDebug() << "unmonitoring..";

    m_watchedEditors.erase(it);
    m_fsWatcher.removePath(path);
}

void FileSystemWatcher::fileChanged(QString filePath)
{
    qDebug() << "fileChanged";

    EditorNS::Editor* ed = m_watchedEditors.at(filePath);

    if (QFile::exists(filePath))
        ed->eventDocumentContentChanged();
    else
        ed->eventDocumentRemoved();

    // QFileSystemWatch automatically unregisters after one signal. Got to remove from list though.
    m_watchedEditors.erase(m_watchedEditors.find(filePath));
    //m_fsWatcher.addPath(filePath);

    qDebug() << "File " << filePath << "has changed";
}
