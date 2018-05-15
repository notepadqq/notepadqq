#include "include/Search/searchstring.h"

#include <QRegularExpression>

QString SearchString::format(QString regex, SearchHelpers::SearchMode searchMode, const SearchHelpers::SearchOptions& searchOptions)
{
    // CodeMirror only knows regex search. So if user asks for "regular" search
    // we'll have to escape all regex characters.
    if (searchMode != SearchHelpers::SearchMode::Regex) {
        regex = QRegularExpression::escape(regex);
    }

    if (searchOptions.MatchWholeWord) {
        regex = "\\b" + regex + "\\b";
    }

    if (searchMode == SearchHelpers::SearchMode::SpecialChars) {
        regex = regex.replace("\\\\", "\\");
    }

    return regex;
}

QString SearchString::unescape(const QString &data)
{ 
    const int dataLength = data.size();
    QString unescaped;
    unescaped.reserve(dataLength);

    for (int i = 0; i < dataLength; i++) {
        QChar c = data[i];
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
