#ifndef SEARCHINFILESWORKER_H
#define SEARCHINFILESWORKER_H

#include <QObject>
#include <QMutex>
#include "include/Search/filesearchresult.h"
#include "include/Search/frmsearchreplace.h"

class SearchInFilesWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchInFilesWorker(QString string, QString path, QStringList filters, frmSearchReplace::SearchMode searchMode, frmSearchReplace::SearchOptions searchOptions);
    ~SearchInFilesWorker();

    FileSearchResult::SearchResult getResult();

signals:
    void finished();
    void error(QString string);
    void progress(QString file);

public slots:
    void run();
    void stop();

private:
    QString m_string;
    QString m_path;
    QStringList m_filters;
    frmSearchReplace::SearchMode m_searchMode;
    frmSearchReplace::SearchOptions m_searchOptions;
    FileSearchResult::SearchResult m_result;
    QMutex m_resultMutex;
    bool m_stop = false;
    QMutex m_stopMutex;
    FileSearchResult::Result buildResult(const QRegularExpressionMatch &match, QString *content);
};

#endif // SEARCHINFILESWORKER_H
