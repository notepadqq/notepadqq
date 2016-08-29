#include "include/Search/searchinfilesworker.h"
#include "include/Search/frmsearchreplace.h"
#include "include/docengine.h"
#include <QDirIterator>
#include <QMessageBox>
#include <QThread>

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

    m_regex.setPattern(rawSearch);
    m_regex.setPatternOptions(options);

    //Pre-optimize here since we are typically going to be traversing many files.
    m_regex.optimize();


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

        FileSearchResult::FileResult structFileResult;

        // Run the search
        if (m_regex.pattern().contains(QStringLiteral("\\n"))) {
            //Do multi-line regex
        }else {
            structFileResult = searchSingleLineRegExp(fileName,decodedText.text);
        }

        f.close();

        if (!structFileResult.results.isEmpty()) {
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

FileSearchResult::FileResult SearchInFilesWorker::searchSingleLineRegExp(const QString &fileName, QString content)
{
    FileSearchResult::FileResult structFileResult;
    structFileResult.fileName = fileName;

    QTextStream stream (&content);
    QString line;
    int i = 0;
    int column;
    QRegularExpressionMatch match;
    while (!(line=stream.readLine()).isNull()) {
        if (m_stop) break;
        match = m_regex.match(line);
        column = match.capturedStart();
        while (column != -1 && !match.captured().isEmpty()) {
            // limit line length
            if (line.length() > 1024) line = line.left(1024);
            structFileResult.results.append(buildResult(i, column, line, match.capturedLength()));
            match = m_regex.match(line, column + match.capturedLength());
            column = match.capturedStart();
            m_matchCount++;
            if (m_matchCount%50) QThread::msleep(1);
        }
        i++;
    }
    return structFileResult;

}

FileSearchResult::SearchResult SearchInFilesWorker::getResult()
{
    FileSearchResult::SearchResult r;
    m_resultMutex.lock();
    r = m_result;
    m_resultMutex.unlock();

    return r;
}

FileSearchResult::Result SearchInFilesWorker::buildResult(int line, int column, const QString &lineContent, int matchLen)
{
    FileSearchResult::Result res;

    res.previewBeforeMatch = lineContent.left(column);
    res.match = lineContent.mid(column,matchLen);
    res.previewAfterMatch = lineContent.mid(column + matchLen);
    res.matchStartLine = line;
    res.matchStartCol = column;
    res.matchEndLine = line;
    res.matchEndCol = column+matchLen;
    res.matchStartPosition = 0;
    res.matchEndPosition = lineContent.length();

    return res;
}

