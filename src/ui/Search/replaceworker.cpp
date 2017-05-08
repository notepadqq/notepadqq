#include "include/Search/replaceworker.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "include/docengine.h"

ReplaceWorker::ReplaceWorker(const SearchResult& results, const QString &replacement)
    : m_searchResult(results),
      m_replacement(replacement)
{ }

void ReplaceWorker::validate()
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

void ReplaceWorker::run()
{
    int count = 0;

    for (const DocResult& docResult : m_searchResult.results) {
        const QVector<MatchResult>& matchResults = docResult.results;

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

        int numReplaced = 0;
        int offset = 0;
        const int replacementLength = m_replacement.length();

        for (const MatchResult& result : matchResults) {
            const int pos = result.m_positionInFile + offset;
            const int length = result.m_matchLength;

            decodedText.text.replace(pos, length, m_replacement);

            offset += replacementLength - length;
            numReplaced++;
        }

        if (numReplaced == 0)
            continue;

        if (!DocEngine::writeFromString(&f, decodedText)) {
            m_failedFiles.push_back(docResult.fileName);
            continue;
        }

        //qDebug() << "File" << docResult.fileName << "had" << numReplaced << "of" << matchResults.size() << "replaced.";
        //QThread::msleep(50);
    }

    emit resultReady();
}
