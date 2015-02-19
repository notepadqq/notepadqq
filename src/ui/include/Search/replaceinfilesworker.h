#ifndef REPLACEINFILESWORKER_H
#define REPLACEINFILESWORKER_H

#include <QObject>
#include <QMutex>
#include "include/Search/filesearchresult.h"

/**
 * @brief This worker handles the replacement of portions of text in files,
 *        based on a search result.
 */
class ReplaceInFilesWorker : public QObject
{
    Q_OBJECT
public:
    explicit ReplaceInFilesWorker(const FileSearchResult::SearchResult &searchResult, const QString &replacement);
    ~ReplaceInFilesWorker();

    QPair<int, int> getResult();

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

    /**
     * @brief Error writing a file. You can handle this signal
     *        and show a message box asking the user to abort/retry/ignore.
     *        Assign the result of the message box to "operation".
     *        Make sure to connect to this signal with
     *        Qt::BlockingQueuedConnection
     * @param message
     * @param operation
     */
    void errorWritingFile(const QString &message, int &operation);

public slots:
    void run();
    void stop();

private:
    FileSearchResult::SearchResult m_searchResult;
    QString m_replacement;
    bool m_stop = false;
    QMutex m_stopMutex;

    QMutex m_resultMutex;
    int m_numOfFilesReplaced = 0;
    int m_numOfOccurrencesReplaced = 0;
};

#endif // REPLACEINFILESWORKER_H
