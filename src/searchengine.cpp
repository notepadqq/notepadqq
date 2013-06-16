#include "searchengine.h"

searchengine::searchengine(bool regexp, bool casesense, bool wholeword, bool wrap, bool forward)
{
    this->regexp = regexp;
    this->casesense = casesense;
    this->wholeword = wholeword;
    this->wrap = wrap;
    this->forward = forward;
    this->newsearch = true;
    this->context = 0;
}

QString searchengine::pattern()
{
    return this->needle;
}

void searchengine::setForward(bool yes) {
    this->forward = yes;
}

bool searchengine::getForward() {
    return this->forward;
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
    if(needle.length() == 0){
        return -1;
    }

    int line=0,index=0;
    this->context->getCursorPosition(&line,&index);
    this->context->findFirst(needle,regexp,casesense,wholeword,wrap,forward,line,index);

    if(!forward) {
        this->context->findNext();
    }

    this->setNewSearch(false);

    return 0;
}

int searchengine::replace(QString with, bool all)
{
    bool foundFirst = false;
    int occurrences = 0;
    if(!this->context) {
        return -1;
    }

    //Set search to non-wrapping mode if we're replacing all occurrences to avoid infinite loop.
    if(all) {
        foundFirst = this->context->findFirst(needle,regexp,casesense,wholeword,false,forward,0,0);
    }else{
        int line=0,index=0;
        if(!this->newsearch){
            this->context->replace(with);
        }else {
            if(forward) {
                this->context->getCursorPosition(&line,&index);
                if(!this->context->findFirst(needle,regexp,casesense,wholeword,false,false,line,index)){
                    this->context->findFirst(needle,regexp,casesense,wholeword,false,true,line,index);
                }
                this->context->replace(with);
            }else {
                this->context->findFirst(needle,regexp,casesense,wholeword,wrap,false,line,index);
                this->context->replace(with);
            }
        }
        this->context->getCursorPosition(&line,&index);

        this->context->findFirst(needle,regexp,casesense,wholeword,wrap,forward,line,index);
        this->newsearch = false;
    }

    if(foundFirst) {
        int line=0,index=0;

        if(all) {
            this->context->beginUndoAction();
            int curline=0,curindex=0;
            this->context->getCursorPosition(&curline,&curindex);
            while(this->context->findFirst(needle,regexp,casesense,wholeword,false,forward,line,index)) {
                occurrences++;
                this->context->replace(with);
                this->context->getCursorPosition(&line,&index);
            }
            this->context->endUndoAction();
            this->context->setCursorPosition(curline,curindex);
            this->context->ensureCursorVisible();
        }

        return occurrences;
    }
    return 0;
}
