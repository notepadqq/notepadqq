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
    SearchHelpers::SearchMode m_searchMode;
    SearchHelpers::SearchOptions m_searchOptions;
    FileSearchResult::SearchResult m_result;
    QMutex m_resultMutex;
    QMutex m_stopMutex;
    bool m_stop = false;
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
     * @brief Perform a search using regular expression matching
     * @param `fileName`
     * @param `content`
     * @return `FileSearchResult::FileResult` object containing all the matches
     *         found in the file
     */
    FileSearchResult::FileResult searchRegExp(const QString &fileName, const QString &content);

    /**
     * @brief Perform a search using plain text.
     * @param `fileName`
     * @param `content`
     * @return `FileSearchResult::FileResult` object containing all the matches
     *         found in the file
     */
    FileSearchResult::FileResult searchPlainText(const QString &fileName, const QString &content);

    /**
     * @brief Boundary check `match` at `index` to ensure it isn't part of another word
     * @param `index`
     * @param `matchLength`
     * @param `data`
     * @return bool value based on results of the string test.
     */
    bool matchesWholeWord(const int &index, const int &matchLength, const QString &data);

    /**
     * @brief Build a list of positions in the given QString where a new line begins.
     * @param `data`
     * @return QVector<int> containing line beginning positions.
     */
    QVector<int> getLinePositions(const QString &data);

    /**
     * @brief Unescapes a string to allow searching for \n, \r, \t, etc.
     * @param `data`
     * @return QString with character sequences unescaped.
     */
    QString unescapeString(const QString &data);
};

#endif // SEARCHINFILESWORKER_H
