#ifndef FILESEARCHRESULT_H
#define FILESEARCHRESULT_H

#include <QString>
#include <QList>
#include <QVariant>

class FileSearchResult {
public:

    struct Result {
        QString previewLine;
        int lineNumber;
        int lineMatchStart;
        int lineMatchEnd;

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

