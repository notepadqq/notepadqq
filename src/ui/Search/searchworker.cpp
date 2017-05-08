#include "include/Search/searchworker.h"

#include <QDirIterator>
#include <QMessageBox>
#include <QDebug>

#include <vector>
#include <algorithm>

#include "include/Search/searchstring.h"
#include "include/docengine.h"

const int MatchResult::CUTOFF_LENGTH = 60;

FileSearcher::FileSearcher(const SearchConfig& config)
    : QThread(nullptr),
      m_searchConfig(config)
{ }

/**
 * @brief matchesWholeWord Returns true if the substring at data.mid(index,matchLength) is a whole word.
 *                         This means it's preceeded and followed by either a whitespace, symbol or punctuation.
 */
bool matchesWholeWord(int index, int matchLength, const QString &data)
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

/**
 * @brief getLinePositions Returns a vector with the positions of all line beginnings of the given string.
 */
std::vector<int> getLinePositions(const QString &data)
{
    const int dataSize = data.size();
    std::vector<int> linePosition;

    linePosition.push_back(0);

    for (int i = 0; i < dataSize; i++) {
        if (data[i] == '\r' && data[i+1] == '\n') {
            linePosition.push_back(i+2);
            i++;
        } else if (data[i] == '\r' || data[i] == '\n') {
            linePosition.push_back(i+1);
        }
    }

    linePosition.push_back(dataSize);
    return linePosition;
}

/**
 * @brief trimEnd Returns the string with all whitespace trimmed from the end.
 *                Taken from the QString source code.
 */
QString trimEnd(const QString& str) {
    const QChar *begin = str.cbegin();
    const QChar *end = str.cend();

    while (begin < end && end[-1].isSpace())
        end--;

    return QString(begin,end-begin);
}

DocResult FileSearcher::searchPlainText(const QString& content)
{
    DocResult results;

    Qt::CaseSensitivity caseSense = m_searchConfig.matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;
    const std::vector<int> linePosition = getLinePositions(content);

    const int matchLength = m_searchConfig.searchString.length();
    int offset = 0;

    while((offset = content.indexOf(m_searchConfig.searchString, offset, caseSense)) != -1) {
        if (m_searchConfig.matchWord && !matchesWholeWord(offset, matchLength, content) ) {
            offset += 1;
            continue;
        }

        // std::upper_bound returns an iterator to the first item greater than 'offset'. This is the line after the match.
        // Substract 1 to get the line the match is on.
        const auto it = std::upper_bound(linePosition.begin(), linePosition.end(), offset);
        const int line = std::distance(linePosition.begin(), it);
        const int lineStart = linePosition[line-1];
        const int lineEnd = linePosition[line];

        MatchResult result;
        result.m_lineNumber = line;
        result.m_matchLineString = trimEnd(content.mid(lineStart, lineEnd-lineStart));
        result.m_positionInFile = offset;
        result.m_positionInLine = offset - lineStart;
        result.m_matchLength = matchLength;
        results.results.push_back(result);

        offset += 1;
    }

    return results;
}

DocResult FileSearcher::searchRegExp(const QString &content)
{
    DocResult results;

    int offset = 0;
    std::vector<int> linePosition = getLinePositions(content);

    QRegularExpressionMatch match;
    for(;;) {
        match = m_regex.match(content, offset);

        if(!match.hasMatch())
            break;

        offset = match.capturedStart();
        const auto it = std::upper_bound(linePosition.begin(), linePosition.end(), offset);
        const int line = std::distance(linePosition.begin(), it);
        const int lineStart = linePosition[line-1];
        const int lineEnd = linePosition[line];

        MatchResult result;
        result.m_lineNumber = line;
        result.m_matchLineString = trimEnd(content.mid(lineStart, lineEnd-lineStart));
        result.m_positionInFile = offset;
        result.m_positionInLine = offset - lineStart;
        result.m_matchLength = match.capturedLength();
        results.results.push_back(result);

        offset += 1;
    }

    return results;
}

void FileSearcher::worker()
{
    if (m_searchConfig.searchMode == SearchConfig::ModeRegex) {
        const QFlags<QRegularExpression::PatternOption> options = m_searchConfig.matchCase ?
                    QRegularExpression::MultilineOption :
                    QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption;

        const QString regex = m_searchConfig.matchWord ?
                    "\\b" + m_searchConfig.searchString + "\\b" : m_searchConfig.searchString;

        m_regex.setPattern(regex);
        m_regex.setPatternOptions(options);

    } else if (m_searchConfig.searchMode == SearchConfig::ModePlainTextSpecialChars) {
        m_searchConfig.searchString = SearchString::unescape(m_searchConfig.searchString);
    }

    const QFlags<QDirIterator::IteratorFlag> dirIteratorOptions = m_searchConfig.includeSubdirs ?
                (QDirIterator::Subdirectories | QDirIterator::FollowSymlinks) :
                QDirIterator::NoIteratorFlags;

    // Split contents of the file pattern string and sanitize it for use
    QStringList filters = m_searchConfig.filePattern.split(',', QString::SkipEmptyParts);
    for (QString& item : filters)
        item = item.trimmed();

    // Create a list of all files that will be read.
    QDirIterator it(m_searchConfig.directory, filters, QDir::Files | QDir::Readable | QDir::Hidden, dirIteratorOptions);
    QStringList fileList;

    while( it.hasNext() )
        fileList << it.next();

    const int listSize = fileList.size();
    emit resultProgress(0, listSize);

    // Start the actual search
    int count = 0;
    for(const auto& fileName : fileList) {
        if(m_wantToStop)
            break;

        if (++count % 100 == 0)
            emit resultProgress(count, listSize);

        QFile f(fileName);
        DocEngine::DecodedText decodedText;
        decodedText = DocEngine::readToString(&f);
        f.close();

        if (decodedText.error) {
            // File could not be read. We'll ignore this error since it should never happen. QDirIterator only iterates over
            // readable files and DocEngine only reads the file. But if it happens we can skip the rest, just in case.
            continue;
        }

        DocResult res;
        if (m_searchConfig.searchMode == SearchConfig::ModeRegex) {
            res = std::move(searchRegExp(decodedText.text));
        } else {
            res = std::move(searchPlainText(decodedText.text));
        }

        if(!res.results.empty()) {
            res.fileName = fileName;
            m_searchResult.results.push_back(res);
        }
    }
}

void FileSearcher::run() {

    auto t1 = QDateTime::currentMSecsSinceEpoch();
    worker();
    auto t2 = QDateTime::currentMSecsSinceEpoch();

    m_searchResult.m_timeToComplete = t2-t1;

    emit resultReady();
}
