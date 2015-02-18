#ifndef FILESEARCHRESULT_H
#define FILESEARCHRESULT_H

#include <QString>
#include <QList>
#include <QVariant>

class FileSearchResult {
public:

    struct Result {
        QString previewBeforeMatch;
        QString match;
        QString previewAfterMatch;
        int matchStartLine;
        int matchStartCol;
        int matchEndLine;
        int matchEndCol;
        int matchStartPosition;
        int matchEndPosition;

        operator QVariant() const
        {
            return QVariant::fromValue(*this);
        }
    };


    struct FileResult {
        QString fileName;
        QList<Result> results;

        operator QVariant() const
        {
            return QVariant::fromValue(*this);
        }
    };

    struct SearchResult {
        QString search;
        QList<FileResult> fileResults;

        operator QVariant() const
        {
            return QVariant::fromValue(*this);
        }
    };

};

Q_DECLARE_METATYPE(FileSearchResult::Result)
Q_DECLARE_METATYPE(FileSearchResult::FileResult)
Q_DECLARE_METATYPE(FileSearchResult::SearchResult)

#endif // FILESEARCHRESULT_H

