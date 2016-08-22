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
     * @brief Returns the path to where the cache is located.
     *        Note the the cache directory will be created if it does not yet exist.
     */
    static QString cacheDirPath();

    /**
     * @brief Deletes the contents of the cache directory.
     */
    static bool clearCacheDir();

    /**
     * @brief Generates a QUrl to a location within the cache directory.
     * @param fileName The file name for the file. This must be a *file name*,
     *        not a *file path*.
     * @return QUrl to a location within the cache directory, file guaranteed not to exist yet.
     */
    static QUrl createValidCacheName(const QString& fileName);
};

#endif // PERSISTENTCACHE_H
