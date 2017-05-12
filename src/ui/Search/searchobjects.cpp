#include "include/Search/searchobjects.h"

void SearchConfig::setScopeFromInt(int scopeAsInt) {
    if(scopeAsInt>0 && scopeAsInt<3) searchScope = static_cast<SearchScope>(scopeAsInt);
}

QString SearchConfig::getScopeAsString() const {
    switch(searchScope){
    case ScopeCurrentDocument:  return QObject::tr("Current Document");
    case ScopeAllOpenDocuments: return QObject::tr("All Documents");
    case ScopeFileSystem:       return QObject::tr("File System");
    default:                    return QObject::tr("Invalid");
    }
}

QString MatchResult::getMatchString() const {
    return m_matchLineString.mid(m_positionInLine, m_matchLength);
}

QString MatchResult::getPreMatchString(bool fullText) const {
    const int pos = m_positionInLine;

    // Cut off part of the text if it is too long and the caller did not request full text
    if (!fullText && pos > CUTOFF_LENGTH)
        return "..." + m_matchLineString.mid( std::max(0, pos-CUTOFF_LENGTH), std::min(CUTOFF_LENGTH, pos) );
    else
        return m_matchLineString.left(pos);
}

QString MatchResult::getPostMatchString(bool fullText) const {
    const int end = m_matchLineString.length();
    const int pos = m_positionInLine + m_matchLength;

    if (!fullText && end-pos > CUTOFF_LENGTH)
        return m_matchLineString.mid(pos, CUTOFF_LENGTH) + "...";
    else
        return m_matchLineString.right(end-pos);
}

int SearchResult::countResults() const {
    int total = 0;

    for(const DocResult& docResult : results)
        total += docResult.results.size();

    return total;
}
