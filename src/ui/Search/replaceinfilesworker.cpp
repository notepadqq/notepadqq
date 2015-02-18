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
        DocEngine::DecodedText decodedText;
        bool retry;

        do {
            retry = false;
            decodedText = DocEngine::readToString(&f);
            if (decodedText.error) {
                // Error reading from file: show message box

                int result = QMessageBox::StandardButton::NoButton;
                emit errorReadingFile(tr("Error reading %1").arg(fileResult.fileName), result);

                if (result == QMessageBox::StandardButton::Abort) {
                    emit finished(true);
                    return;
                } else if (result == QMessageBox::StandardButton::Retry) {
                    retry = true;
                } else {
                    continue;
                }
            }
        } while (retry);

        for (FileSearchResult::Result result : fileResult.results) {
            decodedText.text.replace(result.matchStartPosition, result.matchEndPosition - result.matchStartPosition, m_replacement);
        }

        do {
            retry = false;
            if (DocEngine::writeFromString(&f, decodedText) == false) {
                // Error writing to file: show message box

                int result = QMessageBox::StandardButton::NoButton;
                emit errorReadingFile(tr("Error writing %1").arg(fileResult.fileName), result);

                if (result == QMessageBox::StandardButton::Abort) {
                    emit finished(true);
                    return;
                } else if (result == QMessageBox::StandardButton::Retry) {
                    retry = true;
                } else {
                    continue;
                }
            }
        } while (retry);
    }

    emit finished(false);
}

void ReplaceInFilesWorker::stop()
{
    m_stopMutex.lock();
    m_stop = true;
    m_stopMutex.unlock();
}
