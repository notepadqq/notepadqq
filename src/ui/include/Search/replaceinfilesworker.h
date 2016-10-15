#ifndef REPLACEINFILESWORKER_H
#define REPLACEINFILESWORKER_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include "include/Search/filesearchresult.h"

/**
 * @brief This worker handles the replacement of portions of text in files,
 *        based on a search result.
 */
class ReplaceInFilesWorker : public QThread
{
    Q_OBJECT

public:
    explicit ReplaceInFilesWorker(QObject* parent, const FileSearchResult::SearchResult &searchResult, const QString &replacement);
    ~ReplaceInFilesWorker();
    void run();

signals:

    /**
     * @brief The worker finished its work.
     * @param stopped if true, the worker did not complete its operations
     *        (e.g. because of an error or because it was manually stopped).
     */
    void resultReady(int replaceCount, int fileCount, bool stopped);

   /**
    * @brief The worker has encountered an error.
    * @param QString e: The error message.
    */
    void error(const QString &e);

   /**
    * @brief Tell main thread the current progress of the replace operation.
    * @param QString file:  The current file being worked on.
    */
    void progress(QString file, const bool replace = true);

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
    void stop();

private:
    FileSearchResult::SearchResult m_searchResult;
    QMutex m_mutex;
    QString m_replacement;
    bool m_stop = false;
    int m_fileCount = 0;
    int m_replaceCount = 0;

   /**
    * @brief Build results from `m_fileCount` and `m_replaceCount` and emit
    *        a signal containing them.
    */
    void sendResults();
};

#endif // REPLACEINFILESWORKER_H
