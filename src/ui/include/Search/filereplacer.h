#ifndef FILEREPLACER_H
#define FILEREPLACER_H

#include "searchobjects.h"

#include <QObject>
#include <QThread>
#include <QVector>

/**
 * @brief The FileReplacer class can be used to replace text in strings and files.
 */
class FileReplacer : public QThread
{
    Q_OBJECT

public:

    /**
     * @brief FileReplacer Constructs a FileReplacer to replace all matches in the given SearchResult.
     *                     Use this to replace in ScopeFileSystem for ScopeFileSystem searches, and use
     *                     the static replaceAll() for replacing in documents.
     */
    FileReplacer(const SearchResult& results, const QString &replacement);

    /**
     * @brief cancel Orders the FileSearcher to stop searching at the earliest convenience.
     */
    void cancel() { m_wantToStop = true; }

    /**
     * @brief hasErrors Returns true if any errors have been encountered during the execution of the thread.
     */
    bool hasErrors() const { return !m_failedFiles.empty(); }

    /**
     * @brief getErrors Returns a vector containing the file paths where replacing created errors.
     */
    const QVector<QString>& getErrors() const { return m_failedFiles; }

    /**
     * @brief replaceAll Replaces all matches in 'content' with 'replacement'.
     * @param doc All of this DocResult's ResultMatches will be replaced
     * @param content This is the string that corresponds to the matches in 'doc'
     * @param replacement This is the replacement string
     * @return The number of successful replacements
     */
    static void replaceAll(const DocResult& doc, QString& content, const QString& replacement);

protected:
    void run() override;

signals:
    /**
     * @brief resultProgress Emitted periodically. 'current' is the number of successful replacements done,
     *                       'total' is the total number of replacements to be done.
     */
    void resultProgress(int current, int total);
    void resultReady();

private:
    SearchResult m_searchResult;
    QString m_replacement;

    bool m_wantToStop = false;
    QVector<QString> m_failedFiles;
};

#endif // FILEREPLACER_H
