#include "include/Search/replaceinfilesworker.h"
#include "include/Search/searchinfilesworker.h"
#include "include/docengine.h"
#include <QFile>
#include <QTextStream>

ReplaceInFilesWorker::ReplaceInFilesWorker(const FileSearchResult::SearchResult &searchResult, const QString &replacement)
    : m_searchResult(searchResult),
      m_replacement(replacement)
{

}

ReplaceInFilesWorker::~ReplaceInFilesWorker()
{

}

void ReplaceInFilesWorker::run()
{
    for (FileSearchResult::FileResult fileResult : m_searchResult.fileResults) {
        emit progress(fileResult.fileName);

        QFile f(fileResult.fileName);
        DocEngine::DecodedText decodedText = DocEngine::readToString(&f);
        if (decodedText.error) {
            // FIXME emit error, then get abort/retry/ignore
            continue;
        }

        for (FileSearchResult::Result result : fileResult.results) {
            decodedText.text.replace(result.matchStartPosition, result.matchEndPosition - result.matchStartPosition, m_replacement);
        }

        if (DocEngine::writeFromString(&f, decodedText) == false) {
            // Fixme emit error, then get abort/retry/ignore
        }
    }

    emit finished(false);
}

void ReplaceInFilesWorker::stop()
{
    m_stopMutex.lock();
    m_stop = true;
    m_stopMutex.unlock();
}
