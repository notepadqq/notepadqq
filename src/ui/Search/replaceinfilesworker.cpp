#include "include/Search/replaceinfilesworker.h"
#include "include/Search/searchinfilesworker.h"
#include "include/docengine.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

ReplaceInFilesWorker::ReplaceInFilesWorker(QObject* parent, const FileSearchResult::SearchResult &searchResult, const QString &replacement)
    : m_searchResult(searchResult),
      m_replacement(replacement)
{
    setParent(parent);
}

ReplaceInFilesWorker::~ReplaceInFilesWorker()
{

}

void ReplaceInFilesWorker::sendResults()
{
    emit resultReady(m_replaceCount, m_fileCount, m_stop);
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
                    sendResults();
                    return;
                } else if (result == QMessageBox::StandardButton::Retry) {
                    retry = true;
                } else {
                    continue;
                }
            }
        } while (retry);

        if (m_stop) {
            sendResults();
            return;
        }

        int tmpNumReplaced = 0;

        // Replace in reverse order to make sure all the positions are still valid after each iteration.
        for (int i = fileResult.results.count() - 1; i >= 0; i--) {
            if (m_stop) {
                sendResults();
                return;
            }
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
                        sendResults(); 
                        return;
                    } else if (result == QMessageBox::StandardButton::Retry) {
                        retry = true;
                    } else {
                        continue;
                    }
                }
            } while (retry);

            if (fileWritten) {
                m_fileCount++;
                m_replaceCount += tmpNumReplaced;
            }
        }
    }
    sendResults();
}

void ReplaceInFilesWorker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stop = true;
}
