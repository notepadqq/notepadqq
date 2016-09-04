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
    bool multiLine = false; 
    
    QFlags<QRegularExpression::PatternOption> options = QRegularExpression::NoPatternOption;
    if(m_searchMode != SearchHelpers::SearchMode::PlainText) {
        if (m_searchOptions.MatchCase == false) {
            options |= QRegularExpression::CaseInsensitiveOption;
        }
        QString rawSearch = frmSearchReplace::rawSearchString(m_string, m_searchMode, m_searchOptions);
        m_regex.setPattern(rawSearch);
        m_regex.setPatternOptions(options);
        m_regex.optimize();
        multiLine = m_regex.pattern().contains(QStringLiteral("\\n"));
    }

    // Search result structure
    FileSearchResult::SearchResult structSearchResult;
    structSearchResult.search = m_string;

    QFlags<QDirIterator::IteratorFlag> dirIteratorOptions = QDirIterator::NoIteratorFlags;
    if (m_searchOptions.IncludeSubDirs) {
        dirIteratorOptions |= QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;
    }

    // Iterator used to find files in the specified directory
    QDirIterator it(m_path, m_filters, QDir::Files | QDir::Readable | QDir::Hidden, dirIteratorOptions);

    while (it.hasNext()) {
        if (m_stop) {
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
        if (multiLine) structFileResult = searchMultiLineRegExp(fileName,decodedText.text);
        else structFileResult = searchSingleLineRegExp(fileName,&decodedText.text);
        
        f.close();

        if (!structFileResult.results.isEmpty()) {
            structSearchResult.fileResults.append(structFileResult);
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

bool SearchInFilesWorker::matchesWholeWord(const int &index, const QString &data, const QString &match)
{
    QChar charBefore;
    QChar charAfter;
    int matchLength = match.length();
    int dataLength = data.length();
    bool result = false;

    if (index !=0) {
        charBefore = data.at(index-1);
        if(charBefore.isPunct() || charBefore.isSpace() || charBefore.isSymbol()) result = true;
        else return false;
    }

    if(index+matchLength != dataLength) {
        charAfter = data.at(index+matchLength);
        if(charAfter.isPunct() || charAfter.isSpace() || charAfter.isSymbol()) result = true;
        else return false;
    }

            
    return result;
}

FileSearchResult::FileResult SearchInFilesWorker::searchSingleLineRegExp(const QString &fileName, QString *content)
{
    FileSearchResult::FileResult structFileResult;
    structFileResult.fileName = fileName;

    QTextStream stream (content);
    QString line;
    int i = 0;
    int column;
    int streamPosition = 0;
    int lineLength;

    //Regex for plain text is slow, experimental plain text search for speed improvements.
    if(m_searchMode == SearchHelpers::SearchMode::PlainText) {
        Qt::CaseSensitivity caseSensitive = m_searchOptions.MatchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;
        int resultLength = m_string.length();

        while (!(line=stream.readLine()).isNull()) {
            if(m_stop) break;
            lineLength = line.length();
            column = line.indexOf(m_string, 0, caseSensitive);
            while (column != -1) {
                if (m_stop) break;
                if (m_searchOptions.MatchWholeWord) {
                    if (matchesWholeWord(column, line, m_string)) {
                        structFileResult.results.append(buildResult(i, column, streamPosition + column, line, resultLength));
                    }
                }else {
                    structFileResult.results.append(buildResult(i, column, streamPosition + column, line, resultLength));
                }
                column = line.indexOf(m_string, column + resultLength, caseSensitive);
                m_matchCount++;
            }
            i++;
            if (content->midRef(streamPosition + lineLength, 2) == "\r\n") streamPosition += lineLength + 2;
            else streamPosition += lineLength + 1;
        }

    }else {
        QRegularExpressionMatch match;
        while (!(line=stream.readLine()).isNull()) {
            if (m_stop) break;
            lineLength = line.length();
            match = m_regex.match(line);
            column = match.capturedStart();
            while (column != -1 && match.hasMatch()) {
                if(m_stop) break;
                structFileResult.results.append(buildResult(i, column, streamPosition + column, line, match.capturedLength()));
                match = m_regex.match(line, column + match.capturedLength());
                column = match.capturedStart();
                m_matchCount++;
                if (m_matchCount%50) QThread::usleep(1);
            }
            i++;
            //Check line ending per line to be safe.
            if (content->midRef(streamPosition + lineLength, 2) == "\r\n") streamPosition += lineLength + 2;
            else streamPosition += lineLength + 1; 
        }
    }
    return structFileResult;

}

FileSearchResult::FileResult SearchInFilesWorker::searchMultiLineRegExp(const QString &fileName, QString content)
{
    FileSearchResult::FileResult structFileResult;
    structFileResult.fileName = fileName;

    int column = 0;
    int line = 0;
    static QString fullDoc;
    static QVector<int> lineStart;
    QRegularExpression tmpRegExp = m_regex;

    QTextStream stream (&content);
    fullDoc = stream.readAll();
    fullDoc.remove(QLatin1Char('\r'));

    lineStart.clear();
    lineStart << 0;
    for (int i=0; i<fullDoc.size()-1; i++) {
        if (fullDoc[i] == QLatin1Char('\n')) {
            lineStart << i+1;
        }
    }
    if (tmpRegExp.pattern().endsWith(QStringLiteral("$"))) {
        fullDoc += QLatin1Char('\n');
        QString newPatern = tmpRegExp.pattern();
        newPatern.replace(QStringLiteral("$"), QStringLiteral("(?=\\n)"));
        tmpRegExp.setPattern(newPatern);
    }

    QRegularExpressionMatch match;
    match = tmpRegExp.match(fullDoc);
    column = match.capturedStart();
    while (column != -1 && !match.captured().isEmpty()) {
        if (m_stop) break;
        // search for the line number of the match
        int i;
        line = -1;
        for (i = 1; i < lineStart.size(); i++) {
            if (lineStart.at(i) > column) {
                line = i-1;
                break;
            }
        }
        if (line == -1) {
            break;
        }
        int matchLen = match.capturedLength();
        structFileResult.results.append(buildResult(line, (column - lineStart.at(line)), column, fullDoc.mid(lineStart.at(line), column - lineStart.at(line)) + match.captured(), matchLen));
        match = tmpRegExp.match(fullDoc, column + matchLen);
        column = match.capturedStart();
        m_matchCount++;
        // NOTE: This sleep is here so that the main thread will get a chance to
        // handle any stop button clicks if there are a lot of matches
        if (m_matchCount%50) QThread::usleep(1);
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

FileSearchResult::Result SearchInFilesWorker::buildResult(const int &line, const int &column, const int &absoluteColumn, const QString &lineContent, const int &matchLen)
{
    FileSearchResult::Result res;

    if(column > 50) {
        res.previewBeforeMatch = lineContent.mid(column-50,50);
    }else {
        res.previewBeforeMatch = lineContent.mid(0,column);
    }
    res.match = lineContent.mid(column,matchLen);
    res.previewAfterMatch = lineContent.mid(column + matchLen,matchLen + 50);
    res.matchStartLine = line;
    res.matchStartCol = column;
    res.matchEndLine = line;
    res.matchEndCol = column+matchLen;
    res.matchStartPosition = absoluteColumn;
    res.matchEndPosition = absoluteColumn+matchLen;

    return res;
}

