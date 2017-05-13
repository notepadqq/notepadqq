#include "include/Search/filereplacer.h"

#include <QFile>
#include <QTextStream>

#include "include/docengine.h"

FileReplacer::FileReplacer(const SearchResult& results, const QString &replacement)
    : m_searchResult(results),
      m_replacement(replacement)
{ }

int FileReplacer::replaceAll(const DocResult& doc, QString& content, const QString& replacement)
{
    int numReplaced = 0;
    int offset = 0;
    const int replacementLength = replacement.length();

    for (const MatchResult& result : doc.results) {
        const int pos = result.positionInFile + offset;
        const int length = result.matchLength;

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
