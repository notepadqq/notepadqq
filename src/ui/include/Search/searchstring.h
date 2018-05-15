#ifndef SEARCHSTRING_H
#define SEARCHSTRING_H
#include "include/Search/searchhelpers.h"

#include <QString>

class SearchString {
public:
   /**
    * @brief Formats a search string for use in CodeMirror.
    * @param regex          The string to be worked on, either regex or not.
    * @param searchMode     If mode==regex, will return an escaped string
    * @param searchOptions  Handles wholeWord search option
    * @return QString       A regex string
    */
    static QString format(QString regex, SearchHelpers::SearchMode searchMode, const SearchHelpers::SearchOptions &searchOptions);

   /**
    * @brief Unescape escape sequences in `data`
    * @param `data`:     The string to be worked on.
    * @return `QString`: `data` with escape sequences unescaped.
    */
    static QString unescape(const QString &data);
};

#endif // SEARCHSTRING_H
