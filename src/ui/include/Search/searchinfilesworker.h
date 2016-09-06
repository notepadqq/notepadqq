#ifndef SEARCHINFILESWORKER_H
#define SEARCHINFILESWORKER_H

#include <QObject>
#include <QMutex>
#include <QRegularExpression>
#include "include/Search/filesearchresult.h"
#include "include/Search/searchhelpers.h"

class SearchInFilesWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchInFilesWorker(const QString &string, const QString &path, const QStringList &filters, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions);
    ~SearchInFilesWorker();

    FileSearchResult::SearchResult getResult();

signals:
    /**
     * @brief The worker finished its work.
     * @param stopped if true, the worker did not complete its operations
     *        (e.g. because of an error or because it was manually stopped).
     */
    void finished(bool stopped);
    void error(QString string);
    void progress(QString file);

    /**
     * @brief Currently unused, meant for a live-updating version of search results
     * @param `fileName`
     * @param `line`
     * @param `column`
     * @param `lineContent`
     * @param `matchLen`
     */
    //void matchFound(const QString &fileName, int line, int column, const QString &lineContent, int matchLen);

    /**
     * @brief Error reading a file. You can handle this signal
     *        and show a message box asking the user to abort/retry/ignore.
     *        Assign the result of the message box to "operation".
     *        Make sure to connect to this signal with
     *        Qt::BlockingQueuedConnection
     * @param message
     * @param operation
     */
    void errorReadingFile(const QString &message, int &operation);

public slots:
    void run();
    void stop();

private:
    QString m_string;
    QString m_path;
    QStringList m_filters;
    int m_matchCount;
    SearchHelpers::SearchMode m_searchMode;
    SearchHelpers::SearchOptions m_searchOptions;
    FileSearchResult::SearchResult m_result;
    QMutex m_resultMutex;
    bool m_stop = false;
    QMutex m_stopMutex;
    QRegularExpression m_regex;

   /**
    * @brief Build FileSearchResult::Result object using the parameters given.
    * @param `line`
    * @param `column`
    * @param `absoluteColumn`
    * @param `lineContent`
    * @param `matchLen`
    * @return `FileSearchResult::Result` object to be used for final search results.
    */
    FileSearchResult::Result buildResult(const int &line, 
            const int &column, 
            const int &absoluteColumn, 
            const QString &lineContent, 
            const int &matchLen);
   /**
    * @brief Perform a search using single line regular expression matching
    * @param `fileName`
    * @param `content`
    * @return `FileSearchResult::FileResult` object containing all the matches
    *         found in the file
    */
    FileSearchResult::FileResult searchSingleLine(const QString &fileName, QString *content);

   /**
    * @brief Perform a search using multi line regular expression matching
    * @param `fileName`
    * @param `content`
    * @return `FileSearchResult::FileResult` object containing all the matches
    *         found in the file
    */
    FileSearchResult::FileResult searchMultiLineRegExp(const QString &fileName, QString &content);
   /**
    * @brief Boundary check `match` at `index` to ensure it isn't part of another word
    * @param `index`
    * @param `matchLength`
    * @param `data`
    * @return bool value based on results of the string test.
    */
    bool matchesWholeWord(const int &index, const int &matchLength, const QString &data);

   /*
    * @brief Check pattern content to determine if a multiline pattern exists.
    * @param `pattern`
    * @return bool value based on results of the string test.
    */
    bool isMultilineMatch(const QString &pattern);
};

#endif // SEARCHINFILESWORKER_H
