#include "include/Search/searchinfilesworker.h"
#include "include/Search/frmsearchreplace.h"
#include "include/docengine.h"
#include <QDirIterator>
#include <QRegularExpression>
#include <QMessageBox>

SearchInFilesWorker::SearchInFilesWorker(const QString &string, const QString &path, const QStringList &filters, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions)
    : m_string(string),
      m_path(path),
      m_filters(filters),
      m_searchMode(searchMode),
      m_searchOptions(searchOptions)
{

}

SearchInFilesWorker::~SearchInFilesWorker()
{

}

void SearchInFilesWorker::run()
{
    // Search string converted to a regex
    QString rawSearch = frmSearchReplace::rawSearchString(m_string, m_searchMode, m_searchOptions);

    QFlags<QRegularExpression::PatternOption> options = QRegularExpression::NoPatternOption;
    if (m_searchOptions.MatchCase == false) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }

    QRegularExpression regex(rawSearch, options);

    // Search result structure
    FileSearchResult::SearchResult structSearchResult;
    structSearchResult.search = m_string;

    QFlags<QDirIterator::IteratorFlag> dirIteratorOptions = QDirIterator::NoIteratorFlags;
    if (m_searchOptions.IncludeSubDirs) {
        dirIteratorOptions |= QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;
    }

    // Iterator used to find files in the specified directory
    QDirIterator it(m_path, m_filters, QDir::Files | QDir::Readable | QDir::Hidden, dirIteratorOptions);

    // Total number of matches in all the files
    //int totalFileMatches = 0;
    // Number of files that contain matches
    //int totalFiles = 0;

    while (it.hasNext()) {
        m_stopMutex.lock();
        bool stop = m_stop;
        m_stopMutex.unlock();
        if (stop) {
            emit finished(true);
            return;
        }

        QString fileName = it.next();
        emit progress(fileName);

        // Number of matches in the current file
        int curFileMatches = 0;

        // Read the file into a string.
        QFile f(fileName);
        DocEngine::DecodedText decodedText;
        bool retry;

        do {
            retry = false;
            decodedText = DocEngine::readToString(&f);
            if (decodedText.error) {
                // Error reading from file: show message box

                int result = QMessageBox::StandardButton::NoButton;
                emit errorReadingFile(tr("Error reading %1").arg(fileName), result);

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

        QString content = decodedText.text;

        // Search result structure
        FileSearchResult::FileResult structFileResult;
        structFileResult.fileName = fileName;

        // Run the search
        QRegularExpressionMatchIterator i = regex.globalMatch(content);
        while (i.hasNext())
        {
            //emit progress(fileName + "\n" + QString::number(curFileMatches));
            m_stopMutex.lock();
            bool stop = m_stop;
            m_stopMutex.unlock();
            if (stop) {
                f.close();
                emit finished(true);
                return;
            }

            QRegularExpressionMatch match = i.next();
            QStringList matches = match.capturedTexts();

            if (!matches[0].isEmpty()) {
                structFileResult.results.append(buildResult(match, &content));

                curFileMatches++;
                //totalFileMatches++;
            }
        }

        f.close();

        if (curFileMatches > 0) {
            structSearchResult.fileResults.append(structFileResult);

            //totalFiles++;
        }
    }

    m_resultMutex.lock();
    m_result = structSearchResult;
    m_resultMutex.unlock();

    emit finished(false);
}

void SearchInFilesWorker::stop()
{
    m_stopMutex.lock();
    m_stop = true;
    m_stopMutex.unlock();
}

FileSearchResult::SearchResult SearchInFilesWorker::getResult()
{
    FileSearchResult::SearchResult r;
    m_resultMutex.lock();
    r = m_result;
    m_resultMutex.unlock();

    return r;
}

FileSearchResult::Result SearchInFilesWorker::buildResult(const QRegularExpressionMatch &match, QString *content)
{
    FileSearchResult::Result res;

    // Regex used to detect newlines
    static const QRegularExpression newLine("\n|\r\n|\r");

    // Position (from byte 0) of the start of the found word
    int capturedPosStart = match.capturedStart();

    // Position (from byte 0) of the end of the found word
    int capturedPosEnd = match.capturedEnd(match.lastCapturedIndex());

    // Position (from byte 0) of the start of the first line of the found word
    int firstLinePosStart = content->lastIndexOf(newLine, capturedPosStart) + 1;

    // Position (from byte 0) of the end of the first line of the found word
    //int firstLinePosEnd = content->indexOf(newLine, capturedPosStart);

    // Position (from byte 0) of the start of the last line of the found word
    int lastLinePosStart = content->lastIndexOf(newLine, capturedPosEnd - 1) + 1;

    // Position (from byte 0) of the end of the last line of the found word
    int lastLinePosEnd = content->indexOf(newLine, capturedPosStart);

    // String composed by all the lines that contain the found word.
    QString previewString = content->mid(firstLinePosStart, lastLinePosEnd - firstLinePosStart);

    // All the lines in wholeLine
    QStringList matchLines = previewString.split(newLine, QString::KeepEmptyParts);

    // Number of the first line of the found word
    int count1 = content->leftRef(firstLinePosStart).count("\r\n");
    int count2 = content->leftRef(firstLinePosStart).count("\r");
    int count3 = content->leftRef(firstLinePosStart).count("\n");
    int firstLineNumber = qMax(count1, qMax(count2, count3));
    int lastLineNumber = firstLineNumber + matchLines.count() - 1;

    // Position (from the start of the first line) of the start of the found word
    int capturedColStartInFirstLine = capturedPosStart - firstLinePosStart;

    // Position (from the start of the last line) of the end of the found word.
    int capturedColEndInLastLine = capturedPosEnd - lastLinePosStart;


    // Result
    res.previewBeforeMatch = previewString.mid(0, capturedColStartInFirstLine);
    res.match = previewString.mid(capturedColStartInFirstLine, capturedPosEnd - capturedPosStart);
    res.previewAfterMatch = previewString.mid(capturedColEndInLastLine);
    res.matchStartLine = firstLineNumber;
    res.matchStartCol = capturedColStartInFirstLine;
    res.matchEndLine = lastLineNumber;
    res.matchEndCol = capturedColEndInLastLine;
    res.matchStartPosition = capturedPosStart;
    res.matchEndPosition = capturedPosEnd;

    return res;
}
