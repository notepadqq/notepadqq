#ifndef __SEARCHSTRING_H__
#define __SEARCHSTRING_H__
#include "include/Search/searchhelpers.h"
#include <QString>

class SearchString {
public:
   /**
    * @brief Build a raw version of `data` to be used for the search.
    * @param `data`:          The string to be worked on.
    * @param `searchMode`:    The search mode to be used.
    * @param `searchOptions`: The search options to be used.
    * @return `QString`:      Converted string based on `searchMode` and `searchOptions`
    */
    static QString toRaw(const QString &data, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions);
   /**
    * @brief Convert string to its regex counterpart.
    * @param `data`:           The string to be worked on.
    * @param `matchWholeWord`: Whether to add boundary checks to converted string.
    * @return `QString`:       String converted for use in regex operations.
    */
    static QString toRegex(const QString &data, bool matchWholeWord);
   /**
    * @brief Unescape escape sequences in `data`
    * @param `data`:     The string to be worked on.
    * @return `QString`: `data` with escape sequences unescaped.
    */
    static QString unescape(const QString &data);
};

#endif //__SEARCHSTRING_H_
