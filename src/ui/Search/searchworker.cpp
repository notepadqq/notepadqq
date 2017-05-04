#include "include/Search/searchworker.h"

#include "include/Search/searchstring.h"
#include "include/docengine.h"
#include <QDirIterator>
#include <QMessageBox>

#include <algorithm>

const int MatchResult::CUTOFF_LENGTH = 60;

FileSearcher::FileSearcher(const SearchConfig& config)
    : QThread(nullptr),
      m_searchConfig(config)
{

}

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

QString trimEnd(QString str) {
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
            offset += matchLength;
            continue;
        }

        // std::upper_bound returns an iterator to the first element greater than column. This is the line column is on
        // since we want to start counting lines from 1. Otherwise we'd have to subtract 1.
        // TODO: line-1 sucks
        const auto it = std::upper_bound(linePosition.begin(), linePosition.end(), offset);
        const int line = std::distance(linePosition.begin(), it);

        MatchResult result;

        result.m_lineNumber = line;
        result.m_matchLineString = trimEnd(content.mid(linePosition[line-1], linePosition[line]-linePosition[line-1]));
        result.m_matchOffset = offset;
        result.m_matchIndex = QPoint(offset - linePosition[line-1], matchLength);

        results.results.push_back(result);

        offset += matchLength;
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

        MatchResult result;

        result.m_lineNumber = line;
        result.m_matchLineString = trimEnd(content.mid(linePosition[line-1], linePosition[line]-linePosition[line-1]));
        result.m_matchOffset = offset;
        result.m_matchIndex = QPoint(offset - linePosition[line-1], match.capturedLength());

        results.results.push_back(result);

        offset += match.capturedLength();
    }

    return results;
}

void FileSearcher::worker()
{
    QFlags<QRegularExpression::PatternOption> options = QRegularExpression::MultilineOption;
    QFlags<QDirIterator::IteratorFlag> dirIteratorOptions = QDirIterator::NoIteratorFlags;

    if (m_searchConfig.searchMode == SearchHelpers::SearchMode::Regex) {
        if (m_searchConfig.matchCase == false) {
            options |= QRegularExpression::CaseInsensitiveOption;
        }

        // TODO: Get rid of this one
        SearchHelpers::SearchOptions so;
        so.MatchWholeWord = m_searchConfig.matchWord;
        so.MatchCase = m_searchConfig.matchCase;
        QString rawSearch = SearchString::toRaw(m_searchConfig.searchString, m_searchConfig.searchMode, so);
        m_regex.setPattern(rawSearch);
        m_regex.setPatternOptions(options);
    } else if (m_searchConfig.searchMode == SearchHelpers::SearchMode::SpecialChars) {
        m_searchConfig.searchString = SearchString::unescape(m_searchConfig.searchString);
    }


    if (m_searchConfig.includeSubdirs) {
        dirIteratorOptions |= QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;
    }


    // TODO: Test with multiple filters
    QStringList filters = m_searchConfig.filePattern.split(",", QString::SkipEmptyParts);
    for (int i = 0; i < filters.count(); i++) { // TODO: For-each ?
        filters[i] = filters[i].trimmed();
    }

    QDirIterator it(m_searchConfig.directory, filters, QDir::Files | QDir::Readable | QDir::Hidden, dirIteratorOptions);

    QStringList fileList;

    while( it.hasNext() )
        fileList << it.next();

    emit resultProgress(0, fileList.size());

    int count = 1;
    for(auto fileName : fileList) {

        if(m_wantToStop)
            break;

        if (++count % 100 == 0)
            emit resultProgress(count, fileList.size());

        QFile f(fileName);
        DocEngine::DecodedText decodedText;
        decodedText = DocEngine::readToString(&f);

        // TODO: if(decodedText.error) ...

        f.close();

        DocResult res;
        if (m_searchConfig.searchMode == SearchHelpers::SearchMode::Regex) {
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

    // TODO: Dummy config
    m_searchConfig.searchString = "variable";
    m_searchConfig.filePattern = "*.cpp";
    m_searchConfig.directory = "/home/s3rius/dev/raspi-arm/qt5/qtbase/";
    m_searchConfig.matchWord = false;
    m_searchConfig.matchCase = false;
    m_searchConfig.includeSubdirs = true;
    m_searchConfig.searchMode = SearchHelpers::SearchMode::Regex; //PlainText

    auto t1 = QDateTime::currentMSecsSinceEpoch();
    worker();
    auto t2 = QDateTime::currentMSecsSinceEpoch();

    m_searchResult.m_timeToComplete = t2-t1;

    emit resultReady();
}
