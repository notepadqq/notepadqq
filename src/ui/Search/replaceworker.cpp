#include "include/Search/replaceworker.h"

#include "include/docengine.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include <QDebug>

ReplaceWorker::ReplaceWorker(const SearchResult& results, const QString &replacement)
    : m_searchResult(results),
      m_replacement(replacement)
{

}

void ReplaceWorker::validate()
{

    // TODO: Check if any MatchResults are overlapping. If so, we can't just blindly replace them.
    /*for (const DocResult& r : m_searchResult.results) {
        const std::vector<MatchResult>& mr = r.results;

    }*/

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
            // TODO: Handle errors
            continue;
        }

        int numReplaced = 0;
        qint64 offset = 0;
        const qint64 replacementLength = m_replacement.length();

        for (const MatchResult& result : matchResults) {
            const qint64 pos = result.m_matchOffset + offset;
            const qint64 length = result.m_matchIndex.y();

            decodedText.text.replace(pos, length, m_replacement);

            offset += replacementLength - length;
            numReplaced++;
        }

        if (numReplaced == 0)
            continue;

        if (!DocEngine::writeFromString(&f, decodedText)) {
            // TODO: Handle errors
            continue;
        }

        qDebug() << "File" << docResult.fileName << "had" << numReplaced << "of" << matchResults.size() << "replaced.";
        QThread::msleep(50);
    }

    emit resultReady();
}
