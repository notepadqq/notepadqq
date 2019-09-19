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
    Language() {}
    Language(const Language& o) = default;
    Language(Language&& o) = default;
    Language& operator=(const Language& o) = default;
    Language& operator=(Language&& o) = default;
};

typedef QVector<Language> LanguageList;

class LanguageService
{
    public:
        static LanguageService& getInstance();
        LanguageService(LanguageService const&) = delete;
        void operator=(LanguageService const&) = delete;

        /**
         * @brief Look up a language by its given Id
         *        containing the position of the language in the cache.
         * @param id
         * @return const Language pointer
         */
        const Language* lookupById(const QString& id);

        /**
         * @brief Look up a language by its file name
         *        containing the position of the language in the cache.
         * @param fileName
         * @return const Language pointer
         */
        const Language* lookupByFileName(const QString& fileName);

        /**
         * @brief Look up a language by the file extension
         * @param fileName
         * @return const Language pointer
         */
        const Language* lookupByExtension(const QString& fileName);

        /**
         * @brief Look up a language by the content of the first few lines
         *        i.e. shebang
         * @param content
         * @return const Language pointer
         */
        const Language* lookupByContent(QString content);

        /**
         * @brief Return a list of all the languages currently available in
         *        cache.
         * @return const QVector<Language>
         */
        const LanguageList& languages() {return m_languages;}
        
    private:
        LanguageService();
        LanguageList m_languages;
};

}
#endif//_LANGUAGECACHE_H_
