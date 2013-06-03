#include "searchengine.h"
#include <QDebug>
#include <QStringList>
searchengine::searchengine()
{
}

searchengine::searchengine(bool regexp, bool casesense, bool wholeword, bool wrap, bool forward)
{
    this->regexp = regexp;
    this->casesense = casesense;
    this->wholeword = wholeword;
    this->wrap = wrap;
    this->forward = forward;
    this->newsearch = true;
}

QString searchengine::pattern()
{
    return this->needle;
}

void searchengine::setForward(bool yes) {
    this->forward = yes;
}

void searchengine::setWrap(bool yes) {
    this->wrap = yes;
}

void searchengine::setWholeWord(bool yes) {
    this->wholeword = yes;
}

void searchengine::setCaseSensitive(bool yes) {
    this->casesense = yes;
    if(this->casesense) {
        this->engine.setCaseSensitivity(Qt::CaseSensitive);
    }else {
        this->engine.setCaseSensitivity(Qt::CaseInsensitive);
    }
}

void searchengine::setRegExp(bool yes) {
    this->regexp = yes;
}

void searchengine::setPattern(QString pattern) {
    this->needle = pattern;
}

void searchengine::setHaystack(QString haystack) {
    this->haystack = haystack;
}

void searchengine::setContext(QsciScintillaqq *context)
{
    this->context = context;
}

void searchengine::setNewSearch(bool yes)
{
    this->newsearch = yes;
}

void searchengine::prepareString()
{
    QString final(this->needle);
    if((!this->regexp)) {
        final = QRegExp::escape(final);
    }

    if(this->wholeword) {
        final.append("\\b").prepend("\\b");
    }

    this->engine.setPattern(final);
}

int searchengine::countOccurrences()
{
    int count=0;
    int pos=0;

    prepareString();

    while ((pos = this->engine.indexIn(haystack, pos)) != -1) {
        count++;
        pos += this->engine.matchedLength();
    }
    return count;
}

int searchengine::findString()
{
    if((this->newsearch)||(!context->findNext())) {
        this->context->findFirst(needle,regexp,casesense,wholeword,wrap,forward,-1,-1);
        this->setNewSearch(false);
    }
    return 0;
}
