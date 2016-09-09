#ifndef __SEARCHSTRING_H__
#define __SEARCHSTRING_H__
#include "include/Search/searchhelpers.h"
#include <QString>

class SearchString {
public:
    static QString toRaw(const QString &data, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions);
    static QString toRegex(const QString &data, bool matchWholeWord);
    static QString unescape(const QString &data);
};

#endif //__SEARCHSTRING_H_
