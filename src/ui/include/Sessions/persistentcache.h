#ifndef PERSISTENTCACHE_H
#define PERSISTENTCACHE_H

#include <QDir>

class PersistentCache {
public:

    /**
     * @brief Returns the path to where the session file is/should be located
     *        that contains all tabs when the main Notepadqq instance is closed.
     */
    static QString cacheSessionPath();

    /**
     * @brief Returns the path to the directory that contains the tab cache.
     */
    static QString cacheDirPath();

    /**
     * @brief Generates a QUrl to a file within the a directory.
     * @param parent The parent directory for the file.
     * @param fileName The file name for the file.
     * @return QUrl to a location within the the directory, file guaranteed not to exist yet.
     */
    static QUrl createValidCacheName(const QDir& parent, const QString& fileName);
};

#endif // PERSISTENTCACHE_H
