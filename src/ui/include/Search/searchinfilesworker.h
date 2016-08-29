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
     * @param fileName
     * @param line
     * @param column
     * @param lineContent
     * @param matchLen
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
    FileSearchResult::Result buildResult(int line, int column, const QString &lineContent, int matchLen);
   /**
    * @brief Perform a search using single line regular expression matching
    * @param fileName
    * @param data
    */
    FileSearchResult::FileResult searchSingleLineRegExp(const QString &fileName, QString content);
};

#endif // SEARCHINFILESWORKER_H
