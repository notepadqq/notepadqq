#ifndef REPLACEINFILESWORKER_H
#define REPLACEINFILESWORKER_H

#include <QObject>
#include <QMutex>
#include "include/Search/filesearchresult.h"
#include "include/Search/frmsearchreplace.h"

class ReplaceInFilesWorker : public QObject
{
    Q_OBJECT
public:
    explicit ReplaceInFilesWorker(FileSearchResult::SearchResult searchResult, QString replacement);
    ~ReplaceInFilesWorker();

signals:
    void finished();
    void error(QString string);
    void progress(QString file);

public slots:
    void run();
    void stop();

private:
    FileSearchResult::SearchResult m_searchResult;
    QString m_replacement;
    QMutex m_resultMutex;
    bool m_stop = false;
    QMutex m_stopMutex;
};

#endif // REPLACEINFILESWORKER_H
