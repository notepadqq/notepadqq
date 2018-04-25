#ifndef _LANGUAGECACHE_H_
#define _LANGUAGECACHE_H_

#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>

/**
 * @brief Fast and efficient language cache/lookup for Notepadqq
 */
namespace EditorNS {
/**
 * @brief Language struct containing all the language data
 */
struct Language {
    QString id;
    QString name;
    QString mime;
    QString mode;
    QStringList fileNames;
    QStringList fileExtensions;
    QStringList firstNonBlankLine;
    Language() {};
    Language(const Language& o) = default;
    Language(Language&& o) = default;
    Language& operator=(const Language& o) = default;
    Language& operator=(Language&& o) = default;
};

typedef QVector<Language> LanguageList;

class LanguageCache
{
    public:
        static LanguageCache& getInstance();
        LanguageCache(LanguageCache const&) = delete;
        void operator=(LanguageCache const&) = delete;

        /**
         * @brief Returns a Language struct at the given position by value
         *        No bounds checking is performed in this function!
         * @param i
         * @return Language struct
         */
        const Language& operator [](int i) const {return m_languages[i];}
        
        /**
         * @brief Look up a language by its given Id and return an integer
         *        containing the position of the language in the cache.
         * @param id
         * @return int
         */
        int lookupById(const QString& id);

        /**
         * @brief Look up a language by its file name and return an integer
         *        containing the position of the language in the cache.
         * @param fileName
         * @return int
         */
        int lookupByFileName(const QString& fileName);

        /**
         * @brief Look up a language by the file extension, if any, and return
         *        an integer containing the position of the language in the
         *        cache.
         * @param fileName
         * @return int
         */
        int lookupByExtension(const QString& fileName);

        /**
         * @brief Return a list of all the languages currently available in
         *        cache.
         * @return QVector<Language>
         */
        const LanguageList& languages() {return m_languages;};
        
    private:
        LanguageCache();
        LanguageList m_languages;
};

}
#endif//_LANGUAGECACHE_H_
