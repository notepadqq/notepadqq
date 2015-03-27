#include "include/Search/replaceinfilesworker.h"
#include "include/Search/searchinfilesworker.h"
#include "include/docengine.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

ReplaceInFilesWorker::ReplaceInFilesWorker(const FileSearchResult::SearchResult &searchResult, const QString &replacement)
    : m_searchResult(searchResult),
      m_replacement(replacement)
{

}

ReplaceInFilesWorker::~ReplaceInFilesWorker()
{

}

QPair<int, int> ReplaceInFilesWorker::getResult()
{
    QPair<int, int> result;

    m_resultMutex.lock();
    result.first = m_numOfOccurrencesReplaced;
    result.second = m_numOfFilesReplaced;
    m_resultMutex.unlock();

    return result;
}

void ReplaceInFilesWorker::run()
{
    for (FileSearchResult::FileResult fileResult : m_searchResult.fileResults) {
        emit progress(fileResult.fileName);

        m_stopMutex.lock();
        bool stop = m_stop;
        m_stopMutex.unlock();
        if (stop) {
            emit finished(true);
            return;
        }

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

        m_stopMutex.lock();
        stop = m_stop;
        m_stopMutex.unlock();
        if (stop) {
            emit finished(true);
            return;
        }

        int tmpNumReplaced = 0;

        // Replace in reverse order to make sure all the positions are still valid after each iteration.
        for (int i = fileResult.results.count() - 1; i >= 0; i--) {
            FileSearchResult::Result result = fileResult.results[i];
            decodedText.text.replace(result.matchStartPosition, result.matchEndPosition - result.matchStartPosition, m_replacement);
            tmpNumReplaced++;
        }

        if (tmpNumReplaced > 0) {

            bool fileWritten = false;

            do {
                retry = false;

                if (DocEngine::writeFromString(&f, decodedText)) {
                    fileWritten = true;
                } else {
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

            if (fileWritten) {
                m_resultMutex.lock();
                m_numOfFilesReplaced++;
                m_numOfOccurrencesReplaced += tmpNumReplaced;
                m_resultMutex.unlock();
            }
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
