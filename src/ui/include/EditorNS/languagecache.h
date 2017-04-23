#ifndef _LANGUAGECACHE_H_
#define _LANGUAGECACHE_H_

#include <QString>
#include <QList>
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
    Language(const Language& o);
    Language(Language&& o) noexcept;
    Language& operator=(const Language& o);
    Language& operator=(Language&& o);
};

typedef QVector<Language> LanguageList;

class LanguageCache
{
    public:
        LanguageCache();
        static LanguageCache& getInstance();
        LanguageCache(LanguageCache const&) = delete;
        void operator=(LanguageCache const&) = delete;

        /**
         * @brief Returns a Language struct at the given position by value
         *        No bounds checking is performed in this function!
         * @param i
         * @return Language struct
         */
        Language operator [](int i) const {return m_languages[i];}
        
        /**
         * @brief Returns a reference to the Language struct at the given
         *        position. No bounds checking is performed in this function!
         * @param i
         * @return Language struct reference.
         */
        Language& operator [](int i) {return m_languages[i];}

        /**
         * @brief Returns a const reference to the Language struct at
         *        the given position 'i'
         * @param i
         * @return const Language struct reference
         */
        const Language& at(const int& i) {return m_languages.at(i);}

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
        const LanguageList& languages();
        
    private:
        LanguageList m_languages;
};

}
#endif//_LANGUAGECACHE_H_
