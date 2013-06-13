#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H
#include <QRegExp>
#include "qsciscintillaqq.h"
class searchengine
{
public:
    searchengine(bool regexp=false, bool casesense=false, bool wholeword=false, bool wrap=true, bool forward=true);

    QString pattern();

    void setWrap(bool yes);
    void setWholeWord(bool yes);
    void setForward(bool yes);
    bool getForward();
    void setRegExp(bool yes);
    void setCaseSensitive(bool yes);
    void setPattern(QString pattern);
    void setHaystack(QString haystack);
    void setContext(QsciScintillaqq *context);
    void setNewSearch(bool yes=true);

    int countOccurrences();
    int findString();
    int replace(QString with, bool all=false);


private:
    QString needle;
    QString haystack;
    QRegExp engine;
    QsciScintillaqq *context;
    bool casesense;
    bool wholeword;
    bool regexp;
    bool wrap;
    bool forward;
    bool newsearch;


    void prepareString();

};

#endif // SEARCHENGINE_H
