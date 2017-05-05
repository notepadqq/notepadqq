#ifndef REPLACEWORKER_H
#define REPLACEWORKER_H

#include <QObject>
#include <QThread>

#include "searchworker.h"

/**
 * @brief This worker handles the replacement of portions of text in files,
 *        based on a search result.
 */
class ReplaceWorker : public QThread
{
    Q_OBJECT

public:
    ReplaceWorker(const SearchResult& results, const QString &replacement);

    void cancel() { m_wantToStop = true; }
    void validate();

protected:
    void run() override;

signals:
    void resultProgress(int current, int total);
    void resultReady();

private:
    SearchResult m_searchResult;
    QString m_replacement;

    bool m_wantToStop = false;
};

#endif // REPLACEWORKER_H
