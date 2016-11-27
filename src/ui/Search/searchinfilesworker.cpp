#include "include/Search/searchinfilesworker.h"
#include "include/Search/searchstring.h"
#include "include/docengine.h"
#include <QDirIterator>
#include <QMessageBox>

SearchInFilesWorker::SearchInFilesWorker(QObject* parent, const QString &string, const QString &path, const QStringList &filters, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions)
    : m_string(string),
      m_path(path),
      m_filters(filters),
      m_searchMode(searchMode),
      m_searchOptions(searchOptions)
{
    setParent(parent);
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
        QString rawSearch = SearchString::toRaw(m_string, m_searchMode, m_searchOptions);
        m_regex.setPattern(rawSearch);
        m_regex.setPatternOptions(options);

    } else if (m_searchMode == SearchHelpers::SearchMode::SpecialChars) {
        m_string = SearchString::unescape(m_string);
    }


    if (m_searchOptions.IncludeSubDirs) {
        dirIteratorOptions |= QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;
    }
    
    QDirIterator it(m_path, m_filters, QDir::Files | QDir::Readable | QDir::Hidden, dirIteratorOptions);
    while (it.hasNext()) {
        if (m_stop) {
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
        if (m_searchMode == SearchHelpers::SearchMode::Regex) {
            fileResult = searchRegExp(fileName, decodedText.text);
        } else {
            fileResult = searchPlainText(fileName, decodedText.text);
        }

        if (!fileResult.results.isEmpty()) {
            searchResult.fileResults.append(fileResult);
        }
    }
    m_result = searchResult;

    emit resultReady(m_result);
}

void SearchInFilesWorker::stop()
{
    QMutexLocker locker(&m_mutex);
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
            for (int i = line; i < totalLines; i++) {
                if (linePosition[i] > column) {
                    line = i-1;
                    if (hasResult) {
                        fileResult.results.append(buildResult(line, column - linePosition[line], column, content, matchLength));
                    }
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
        for (int i = line; i < linePosition.length(); i++){
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

    if (index != 0) {
        boundary = data[index-1];
        if (!boundary.isPunct() && !boundary.isSpace() && !boundary.isSymbol()) {
            return false;
        }
    }
    if (data.length() != index+matchLength) {
        boundary = data[index+matchLength];
        if (!boundary.isPunct() && !boundary.isSpace() && !boundary.isSymbol()) {
            return false;
        }
    }
    return true;
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
        } else if (data[i] == '\r' || data[i] == '\n') {
            linePosition << i+1;
        }
    }

    linePosition << data.size();
    return linePosition;
}

FileSearchResult::Result SearchInFilesWorker::buildResult(const int &line, const int &column, const int &absoluteColumn, const QString &lineContent, const int &matchLen)
{
    FileSearchResult::Result res;

    if (absoluteColumn > 50) {
        res.previewBeforeMatch = lineContent.mid(absoluteColumn-50, 50);
    } else {
        res.previewBeforeMatch = lineContent.mid(0, absoluteColumn);
    }
    res.match = lineContent.mid(absoluteColumn, matchLen);
    res.previewAfterMatch = lineContent.mid(absoluteColumn + matchLen, matchLen + 50);
    res.matchStartLine = line;
    res.matchStartCol = column;
    res.matchEndLine = line;
    res.matchEndCol = column + matchLen;
    res.matchStartPosition = absoluteColumn;
    res.matchEndPosition = absoluteColumn + matchLen;

    // Efficiently cut leading lines from previewBeforeMatch
    {
        int i;
        for (i = res.previewBeforeMatch.length() - 1; i >= 0; i--) {
            if (res.previewBeforeMatch[i] == '\r' || res.previewBeforeMatch[i] == '\n') {
                break;
            }
        }
        res.previewBeforeMatch = res.previewBeforeMatch.mid(i);
    }

    // Efficiently cut trailing lines from previewAfterMatch
    {
        int i, pos = -1;
        for (i = 0; i < res.previewAfterMatch.length(); i++) {
            if (res.previewAfterMatch[i] == '\r' || res.previewAfterMatch[i] == '\n') {
                pos = i;
                break;
            }
        }
        res.previewAfterMatch = res.previewAfterMatch.mid(0, pos);
    }


    return res;
}

