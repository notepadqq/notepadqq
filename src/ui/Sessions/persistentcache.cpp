#include "include/Sessions/persistentcache.h"
#include "include/notepadqq.h"

QString PersistentCache::cacheSessionPath() {
    static QString cachePath = QFileInfo(QSettings().fileName()).dir().absolutePath().append("/session.xml");
    return cachePath;
}

QString PersistentCache::cacheDirPath() {
    static QString tabpath = QFileInfo(QSettings().fileName()).dir().absolutePath().append("/tabCache");
    return tabpath;
}

QUrl PersistentCache::createValidCacheName(const QDir& parent, const QString &fileName)
{
    QUrl cacheFile;
    QString partialPath = parent.absolutePath() + "/" + fileName;
    QFileInfo fileInfo;

    // To prevent name collision, a random suffix will be appended to each file.
    do {
        cacheFile = QUrl::fromLocalFile(partialPath + "." + QString::number(rand()));
        fileInfo = QFileInfo(cacheFile.toLocalFile());
    } while (fileInfo.exists());

    // Make sure an absolute file path is returned, otherwise there could be problems
    // with loading the files later.
    return QUrl::fromLocalFile(fileInfo.absoluteFilePath());
}
