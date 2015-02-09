#ifndef FILESEARCHRESULT_H
#define FILESEARCHRESULT_H

#include <QString>
#include <QList>

class FileSearchResult {
public:

    struct Result {
        QString line;
        int lineNumber;
        int lineMatchStart;
        int lineMatchEnd;
    };

    struct FileResult {
        QString fileName;
        QList<Result> results;
    };

    struct SearchResult {
        QString search;
        QList<FileResult> fileResults;
    };

};

#endif // FILESEARCHRESULT_H

