#ifndef REPLACEWORKER_H
#define REPLACEWORKER_H

#include <QObject>
#include <QThread>
#include <QVector>

#include "searchobjects.h"

/**
 * @brief This worker handles the replacement of portions of text in files,
 *        based on a search result.
 */
class FileReplacer : public QThread
{
    Q_OBJECT

public:
    FileReplacer(const SearchResult& results, const QString &replacement);

    void cancel() { m_wantToStop = true; }
    void validate();

    bool hasErrors() const { return !m_failedFiles.empty(); }
    const QVector<QString>& getErrors() const { return m_failedFiles; }

    static int replaceAll(const DocResult& doc, QString& content, const QString& replacement);

protected:
    void run() override;

signals:
    void resultProgress(int current, int total);
    void resultReady();

private:
    SearchResult m_searchResult;
    QString m_replacement;

    bool m_wantToStop = false;
    QVector<QString> m_failedFiles;
};

#endif // REPLACEWORKER_H
