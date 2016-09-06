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
    FileSearchResult::SearchResult searchResult;
    QFlags<QRegularExpression::PatternOption> options = QRegularExpression::NoPatternOption;
    QFlags<QDirIterator::IteratorFlag> dirIteratorOptions = QDirIterator::NoIteratorFlags;

    if(m_searchMode != SearchHelpers::SearchMode::PlainText) {
        if (m_searchOptions.MatchCase == false) {
            options |= QRegularExpression::CaseInsensitiveOption;
        }
        QString rawSearch = frmSearchReplace::rawSearchString(m_string, m_searchMode, m_searchOptions);
        m_regex.setPattern(rawSearch);
        m_regex.setPatternOptions(options);
        m_regex.optimize();
        multiLine = isMultilineMatch(m_regex.pattern());
    }


    if (m_searchOptions.IncludeSubDirs) {
        dirIteratorOptions |= QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;
    }
    
    QDirIterator it(m_path, m_filters, QDir::Files | QDir::Readable | QDir::Hidden, dirIteratorOptions);
    while (it.hasNext()) {
        if (m_stop) {
            emit finished(true);
            return;
        }

        const QString fileName = it.next();
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
        f.close();

        FileSearchResult::FileResult fileResult;

        if (multiLine) fileResult = searchMultiLineRegExp(fileName,decodedText.text);
        else fileResult = searchSingleLine(fileName,&decodedText.text);
        if (!fileResult.results.isEmpty()) searchResult.fileResults.append(fileResult);
    }
    searchResult.search = m_string;
    m_result = searchResult;

    emit finished(false);
}

void SearchInFilesWorker::stop()
{
    m_stopMutex.lock();
    m_stop = true;
    m_stopMutex.unlock();
}

bool SearchInFilesWorker::isMultilineMatch(const QString &pattern)
{
    return pattern.contains(QRegExp("\\\\r|\\\\n|\\\\[xX]0[aA]|\\\\[xX]0[Dd]"));
}

bool SearchInFilesWorker::matchesWholeWord(const int &index, const int &matchLength, const QString &data)
{
    QChar boundary;

    if (index !=0) {
        boundary = data.at(index-1);
        if (!boundary.isPunct() && !boundary.isSpace() && !boundary.isSymbol()) return false;
    }
    if (data.length() != index+matchLength) {
        boundary = data.at(index+matchLength);
        if (!boundary.isPunct() && !boundary.isSpace() && !boundary.isSymbol()) return false;
    }
    return true;
}

FileSearchResult::FileResult SearchInFilesWorker::searchSingleLine(const QString &fileName, QString *content)
{
    FileSearchResult::FileResult fileResult;

    QTextStream stream (content);
    QString line;

    int i = 0;
    int column;
    int streamPosition = 0;
    int lineLength;

    fileResult.fileName = fileName;
    if (m_searchMode == SearchHelpers::SearchMode::PlainText) {
        Qt::CaseSensitivity caseSensitive = m_searchOptions.MatchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;
        int matchLength = m_string.length();

        while (!(line=stream.readLine()).isNull() && !m_stop) {
            bool hasResult = true;
            lineLength = line.length();
            column = line.indexOf(m_string, 0, caseSensitive);
            while (column != -1 && !m_stop) {
                if (m_searchOptions.MatchWholeWord) hasResult = matchesWholeWord(column, matchLength, line);
                if (hasResult) fileResult.results.append(buildResult(i, column, streamPosition + column, line, matchLength));
                column = line.indexOf(m_string, column + matchLength, caseSensitive);
                m_matchCount++;
            }
            i++;
            if (content->midRef(streamPosition + lineLength, 2) == "\r\n") streamPosition += lineLength + 2;
            else streamPosition += lineLength + 1;
        }

    }else {
        QRegularExpressionMatch match;
        while (!(line=stream.readLine()).isNull() && !m_stop) {
            lineLength = line.length();
            match = m_regex.match(line);
            column = match.capturedStart();
            while (column != -1 && match.hasMatch() && !m_stop) {
                fileResult.results.append(buildResult(i, column, streamPosition + column, line, match.capturedLength()));
                match = m_regex.match(line, column + match.capturedLength());
                column = match.capturedStart();
                m_matchCount++;
            }
            i++;
            if (content->midRef(streamPosition + lineLength, 2) == "\r\n") streamPosition += lineLength + 2;
            else streamPosition += lineLength + 1; 
        }
    }
    return fileResult;

}

FileSearchResult::FileResult SearchInFilesWorker::searchMultiLineRegExp(const QString &fileName, const QString &content)
{
    FileSearchResult::FileResult fileResult;
    fileResult.fileName = fileName;
    
    int column = 0;
    int line = 0;
    int lastLine = 0;
    QString fullDoc;
    QVector<int> lineStart;
    QRegularExpressionMatch match;

    lineStart << 0;
    //Determine line endings and add them to a list.
    for (int i=0; i<content.size()-1; i++) {
        if (content.midRef(i,2) == "\r\n") {
            lineStart << i+2;
            i++;
        }else if(content.at(i) == '\r' || content.at(i) == '\n') {
            lineStart << i+1;
        }
    }

    m_regex.setPatternOptions(m_regex.patternOptions() | QRegularExpression::MultilineOption);
    match = m_regex.match(fullDoc);
    column = match.capturedStart();
    while(column != -1 && match.hasMatch()) {
        //Get the current line we are working on.
        for(int i=0; i < lineStart.length(); i++){
            if(lineStart[i] > column) {
                line = i-1;
                break;
            }
        }
        fileResult.results.append(buildResult(line, column, column, fullDoc, match.capturedLength()));
        match = m_regex.match(fullDoc,column + match.capturedLength());
        column = match.capturedStart();
    }
    
    return fileResult;
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

    if (column > 50) {
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

