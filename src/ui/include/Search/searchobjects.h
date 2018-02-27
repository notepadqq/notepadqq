#ifndef SEARCHOBJECTS_H
#define SEARCHOBJECTS_H

#include <QObject>
#include <QString>
#include <QVector>

#include "include/Search/searchhelpers.h"

class MainWindow;

struct SearchConfig {
    /**
     * @brief setScopeFromInt Sets searchScope to the given int. Helper function to avoid int<->SearchScope cast.
     * @param scopeAsInt Must be between 0 and 3.
     */
    void setScopeFromInt(int scopeAsInt);

    /**
     * @brief getScopeAsString Returns the config's current searchScope as a human-readable string
     */
    QString getScopeAsString() const;


    QString searchString;
    QString filePattern; // Only used if searchMode==ScopeFileSystem.
    QString directory;   // Only used if searchMode==ScopeFileSystem.
    MainWindow* targetWindow = nullptr; // Only used if searchMode is ScopeCurrentDocument or ScopeAllOpenDocuements

    bool matchCase      = false;
    bool matchWord      = false;
    bool includeSubdirs = false; // Only used if searchMode==ScopeFileSystem.

    enum SearchScope {
        ScopeCurrentDocument    = 0,
        ScopeAllOpenDocuments   = 1,
        ScopeFileSystem         = 2
    };
    SearchScope searchScope = ScopeCurrentDocument;

    enum SearchMode {
        ModePlainText               = 0,
        ModePlainTextSpecialChars   = 1,
        ModeRegex                   = 2
    };
    SearchMode searchMode = ModePlainText;
};

struct MatchResult {
    /**
     * @brief getMatchString Returns the match as a string
     */
    QString getMatchString() const;

    /**
     * @brief getPreMatchString Returns the part of the line before the match.
     * @param fullText If false, the text length is limited to CUTOFF_LENGTH characters
     */
    QString getPreMatchString(bool fullText=false) const;

    /**
     * @brief getPostMatchString Returns the part of the line after the match.
     * @param fullText If false, the text length is limited to CUTOFF_LENGTH characters
     */
    QString getPostMatchString(bool fullText=false) const;

    QString matchLineString; // The full text line where the match occured
    int lineNumber;          // The line number, starting at 1
    int positionInFile;      // The match's offset from the beginning of the file
    int positionInLine;      // The match's offset from the beginning of the line
    int matchLength;         // The match's length

private:
    static const int CUTOFF_LENGTH; //Number of characters before/after match result that will be shown in preview
};

namespace EditorNS { class Editor; }

struct DocResult {
    enum DocType {
        TypeNone,       // No source; don't try to open it and don't allow replacement
        TypeFile,       // It's a file on the user's file system
        TypeDocument    // It's one of MainWindow's open documents
    };
    DocType docType = TypeNone;

    // TODO: Only a workaround- we need some easy way to address Editors in the future.
    EditorNS::Editor* editor = nullptr; // Only used when docType==TypeDocument
    QString fileName;                   // Is a file path when docType==TypeFile and a file name when TypeDocument
    QVector<MatchResult> results;
};

enum class SearchUserInteraction {
    OpenDocument,
    OpenContainingFolder
};

struct SearchResult {

    /**
     * @brief countResults Returns the total number of MatchResults in all DocResults combined.
     */
    int countResults() const;

    QVector<DocResult> results;
};


#endif // SEARCHOBJECTS_H
