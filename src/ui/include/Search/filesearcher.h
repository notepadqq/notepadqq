#ifndef FILESEARCHER_H
#define FILESEARCHER_H

#include "searchhelpers.h"
#include "searchobjects.h"

#include <QObject>
#include <QRegularExpression>
#include <QThread>

/**
 * @brief The FileSearcher class contains the tools to search strings and files asynchronously and synchronously.
 *        Use prepareAsyncSearch() and run start() on the returned FileSearcher* object to search files
 *        asynchronously. Use searchPlainText() and searchRegExp() to search strings synchronously.
 */
class FileSearcher : public QThread {
    Q_OBJECT

public:

    /**
     * @brief prepareAsynchSearch Returns a FileSearcher* object to be used in async file search. Only use this for
     *                            ScopeFileSystem searches. Use the static functions for searching documents or strings.
     */
    static FileSearcher* prepareAsyncSearch(const SearchConfig& config);

    /**
     * @brief createRegexFromConfig Creates a RegularExpression based on the given config that can be used
     *                              in conjuncture with searchRegExp()
     */
    static QRegularExpression createRegexFromConfig(const SearchConfig& config);

    /**
     * @brief searchPlainText Searches a given string (synchronously)
     * @param config Contains the search string and other parameters for the search
     * @param content The string to be searched
     * @return A DocResult containing all found matches.
     */
    static DocResult searchPlainText(const SearchConfig& config, const QString& content);

    /**
     * @brief searchRegExp  Searches a given string via a RegularExpression (synchronously)
     * @param regex The RegExp to be used. Can be created  via createRegexFromString()
     * @param content The string to be searched
     * @return A DocResult containing all found matches.
     */
    static DocResult searchRegExp(const QRegularExpression& regex, const QString& content);

    /**
     * @brief cancel Orders the FileSearcher to stop searching at the earliest convenience. Won't immediately stop.
     */
    void cancel() { m_wantToStop = true; }

    /**
     * @brief getResult Returns a SearchResult item with all found results. Will be empty until the FileSearcher
     *                  completely finished its search and emitted resultReady()
     */
    const SearchResult& getResult() { return m_searchResult; }

signals:
    /**
     * @brief resultProgress is emitted periodically. 'Processed' is the number of files already searched.
     *                       'Total' is the total number of files to be searched.
     */
    void resultProgress(int processed, int total);
    void resultReady();

protected:
    void run() override;

private:
    FileSearcher(const SearchConfig& config);

    SearchConfig m_searchConfig;
    QRegularExpression m_regex;
    bool m_wantToStop = false;
    SearchResult m_searchResult;
};

#endif // FILESEARCHER_H
