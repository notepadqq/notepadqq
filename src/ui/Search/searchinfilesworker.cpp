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
    FileSearchResult::SearchResult searchResult;
    searchResult.search = m_string;

    QFlags<QRegularExpression::PatternOption> options = QRegularExpression::MultilineOption;
    QFlags<QDirIterator::IteratorFlag> dirIteratorOptions = QDirIterator::NoIteratorFlags;

    if (m_searchMode == SearchHelpers::SearchMode::Regex) {
        if (m_searchOptions.MatchCase == false) {
            options |= QRegularExpression::CaseInsensitiveOption;
        }
        QString rawSearch = frmSearchReplace::rawSearchString(m_string, m_searchMode, m_searchOptions);
        m_regex.setPattern(rawSearch);
        m_regex.setPatternOptions(options);
    }else if (m_searchMode == SearchHelpers::SearchMode::SpecialChars) {
        QString rawSearch = frmSearchReplace::rawSearchString(m_string, m_searchMode, m_searchOptions);
        m_string = unescapeString(m_string);
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
        if (m_searchMode == SearchHelpers::SearchMode::Regex) fileResult = searchRegExp(fileName, decodedText.text);
        else fileResult = searchPlainText(fileName, decodedText.text);
        if (!fileResult.results.isEmpty()) searchResult.fileResults.append(fileResult);
    }
    m_result = searchResult;

    emit finished(false);
}

void SearchInFilesWorker::stop()
{
    QMutexLocker locker(&m_stopMutex);
    m_stop = true;
}

FileSearchResult::FileResult SearchInFilesWorker::searchPlainText(const QString &fileName, const QString &content)
{
    FileSearchResult::FileResult fileResult;
    fileResult.fileName = fileName;
    Qt::CaseSensitivity caseSense = m_searchOptions.MatchCase ? Qt::CaseSensitive : Qt::CaseInsensitive; 
    QVector<int> linePosition = getLinePositions(content);

    const int totalLines = linePosition.length();
    const int matchLength = m_string.length();
    bool hasResult;
    int column = 0;
    int line = 0;
    while ((column = content.indexOf(m_string, column, caseSense)) != -1 && !m_stop) {
        hasResult = m_searchOptions.MatchWholeWord ? matchesWholeWord(column, matchLength, content) : true;
        if (hasResult) {
            for (int i = line;i < totalLines; i++) {
                if (linePosition[i] > column) {
                    line = i-1;
                    if (hasResult) fileResult.results.append(buildResult(line, column - linePosition[line], column, content, matchLength));
                    break;
                }
            }
        }
        column += matchLength;
    }

    return fileResult;
}

FileSearchResult::FileResult SearchInFilesWorker::searchRegExp(const QString &fileName, const QString &content)
{
    FileSearchResult::FileResult fileResult;
    fileResult.fileName = fileName;
    
    int column = 0;
    int line = 0;
    QVector<int> linePosition = getLinePositions(content);
    QRegularExpressionMatch match;

    match = m_regex.match(content);
    column = match.capturedStart();
    while (column != -1 && match.hasMatch() && !m_stop) {
        for (int i=line; i < linePosition.length(); i++){
            if (linePosition[i] > column) {
                line = i-1;
                fileResult.results.append(buildResult(line, column - linePosition[line], column, content, match.capturedLength()));
                break;
            }
        }
        match = m_regex.match(content, column + match.capturedLength());
        column = match.capturedStart();
    }
    return fileResult;
}

bool SearchInFilesWorker::matchesWholeWord(const int &index, const int &matchLength, const QString &data)
{
    QChar boundary;

    if (index !=0) {
        boundary = data[index-1];
        if (!boundary.isPunct() && !boundary.isSpace() && !boundary.isSymbol()) return false;
    }
    if (data.length() != index+matchLength) {
        boundary = data[index+matchLength];
        if (!boundary.isPunct() && !boundary.isSpace() && !boundary.isSymbol()) return false;
    }
    return true;
}

FileSearchResult::SearchResult SearchInFilesWorker::getResult()
{
    QMutexLocker locker(&m_resultMutex);
    FileSearchResult::SearchResult r;
    r = m_result;

    return r;
}

QVector<int> SearchInFilesWorker::getLinePositions(const QString &data)
{
    const int dataSize = data.size();
    QVector<int> linePosition;

    linePosition << 0;
    for (int i = 0; i < dataSize; i++) {
        if (data[i] == '\r' && data[i+1] == '\n') {
            linePosition << i+2;
            i++;
        }else if (data[i] == '\r' || data[i] == '\n') {
            linePosition << i+1;
        }
    }

    linePosition << data.size();
    return linePosition;
}

QString SearchInFilesWorker::unescapeString(const QString &data)
{ 
    int dataLength = data.size();
    QString unescaped;
    QChar c;
    for (int i = 0; i < dataLength; i++) {
        c = data[i];
        if (c == '\\' && i != dataLength) {
            i++;
            if (data[i] == 'a') c = '\a';
            else if (data[i] == 'b') c = '\b';
            else if (data[i] == 'f') c = '\f';
            else if (data[i] == 'n') c = '\n';
            else if (data[i] == 'r') c = '\r';
            else if (data[i] == 't') c = '\t';
            else if (data[i] == 'v') c = '\v';
            else if (data[i] == 'x' && i+2 <= dataLength) {
                int nHex = data.mid(++i, 2).toInt(0, 16);
                c = QChar(nHex);
                i += 1;
            }else if (data[i] == 'u' && i+4 <= dataLength) {
                int nHex = data.mid(++i,4).toInt(0, 16);
                c = QChar(nHex);
                i += 3;
            }
        }
        unescaped.append(c);
    }
    qDebug() << unescaped;
    return unescaped;
}

FileSearchResult::Result SearchInFilesWorker::buildResult(const int &line, const int &column, const int &absoluteColumn, const QString &lineContent, const int &matchLen)
{
    FileSearchResult::Result res;

    if (absoluteColumn > 50) {
        res.previewBeforeMatch = lineContent.mid(absoluteColumn-50,50);
    }else {
        res.previewBeforeMatch = lineContent.mid(0,absoluteColumn);
    }
    res.match = lineContent.mid(absoluteColumn,matchLen);
    res.previewAfterMatch = lineContent.mid(absoluteColumn + matchLen,matchLen + 50);
    res.matchStartLine = line;
    res.matchStartCol = column;
    res.matchEndLine = line;
    res.matchEndCol = column+matchLen;
    res.matchStartPosition = absoluteColumn;
    res.matchEndPosition = absoluteColumn+matchLen;

    return res;
}

