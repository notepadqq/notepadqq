#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H
#include <QRegExp>
#include <QObject>
#include "qsciscintillaqq.h"
class searchengine : public QObject
{
    Q_OBJECT
public:
    searchengine(QObject *parent=0) : QObject(parent) {}
    virtual ~searchengine() {}

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
