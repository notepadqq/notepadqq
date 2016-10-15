#include "include/Search/searchstring.h"
#include <QRegularExpression>

QString SearchString::toRaw(const QString &data, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions)
{
    QString rawSearch = data;
    if (searchMode == SearchHelpers::SearchMode::SpecialChars) {
        rawSearch = toRegex(data, searchOptions.MatchWholeWord);
        rawSearch = rawSearch.replace("\\\\", "\\");
    } else if (searchMode == SearchHelpers::SearchMode::PlainText){
        rawSearch = toRegex(data, searchOptions.MatchWholeWord);
    }
    return rawSearch;
}

QString SearchString::toRegex(const QString &data, bool matchWholeWord)
{
    QString regex = QRegularExpression::escape(data);
    if (matchWholeWord) {
        regex = "\\b" + regex + "\\b";
    }
    return regex;
}

QString SearchString::unescape(const QString &data)
{ 
    const int dataLength = data.size();
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
            } else if (data[i] == 'u' && i+4 <= dataLength) {
                int nHex = data.mid(++i,4).toInt(0, 16);
                c = QChar(nHex);
                i += 3;
            }
        }
        unescaped.append(c);
    }
    return unescaped;
}

