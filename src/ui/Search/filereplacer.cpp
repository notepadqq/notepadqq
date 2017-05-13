#include "include/Search/filereplacer.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "include/docengine.h"

FileReplacer::FileReplacer(const SearchResult& results, const QString &replacement)
    : m_searchResult(results),
      m_replacement(replacement)
{ }

void FileReplacer::validate()
{
    // TODO: Check if any MatchResults are overlapping. If so, we can't just blindly replace them.
    for (const DocResult& r : m_searchResult.results) {
        const QVector<MatchResult>& mr = r.results;

        if (mr.isEmpty()) continue;

        const MatchResult* last = &(*mr.cbegin());
        for (auto it = mr.cbegin()+1; it != mr.cend(); ++it) {
            const int endLast = last->m_positionInFile + last->m_matchLength;
            const int startCurr = it->m_positionInFile;

            if (endLast > startCurr) {
                qDebug() << "There are overlapping items";
            }

            last = &(*it);
        }
    }

}

int FileReplacer::replaceAll(const DocResult& doc, QString& content, const QString& replacement)
{
    int numReplaced = 0;
    int offset = 0;
    const int replacementLength = replacement.length();

    for (const MatchResult& result : doc.results) {
        const int pos = result.m_positionInFile + offset;
        const int length = result.m_matchLength;

        content.replace(pos, length, replacement);

        offset += replacementLength - length;
        numReplaced++;
    }

    return numReplaced;
}

void FileReplacer::run()
{
    int count = 0;

    for (const DocResult& docResult : m_searchResult.results) {
        if(m_wantToStop)
            return;

        if (++count % 10 == 0)
            emit resultProgress(count, m_searchResult.results.size());

        QFile f(docResult.fileName);
        DocEngine::DecodedText decodedText;

        decodedText = DocEngine::readToString(&f);
        if (decodedText.error) {
            m_failedFiles.push_back(docResult.fileName);
            continue;
        }

        if (replaceAll(docResult, decodedText.text, m_replacement) == 0)
            continue;

        if (!DocEngine::writeFromString(&f, decodedText)) {
            m_failedFiles.push_back(docResult.fileName);
            continue;
        }
    }

    emit resultReady();
}
