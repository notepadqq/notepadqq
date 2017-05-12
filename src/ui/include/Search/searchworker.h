#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QRegularExpression>
#include <QVector>

#include "include/Search/searchhelpers.h"


class MainWindow;

// TODO: Find a good home for SearchConfig, MatchResult, etc
struct SearchConfig {

    /**
     * @brief setScopeFromInt Sets searchScope to the given int. Helper function to avoid int<->SearchScope cast.
     * @param scopeAsInt Must be between 0 and 3.
     */
    void setScopeFromInt(int scopeAsInt) {
        if(scopeAsInt>0 && scopeAsInt<3) searchScope = static_cast<SearchScope>(scopeAsInt);
    }

    /**
     * @brief getScopeAsString Returns a readable label for the config's current searchScope
     */
    QString getScopeAsString() const {
        switch(searchScope){
        case ScopeCurrentDocument:  return QObject::tr("Current Document");
        case ScopeAllOpenDocuments: return QObject::tr("All Documents");
        case ScopeFileSystem:       return QObject::tr("File System");
        default:                    return "Invalid";
        }
    }

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
    QString getMatchString() const { return m_matchLineString.mid(m_positionInLine, m_matchLength); }

    QString getPreMatchString(bool fullText=false) const {
        const int pos = m_positionInLine;

        // Cut off part of the text if it is too long and the caller did not request full text
        if(!fullText && pos > CUTOFF_LENGTH)
            return "..." + m_matchLineString.mid( std::max(0, pos-CUTOFF_LENGTH), std::min(CUTOFF_LENGTH, pos) );
        else
            return m_matchLineString.left(pos);
    }

    QString getPostMatchString(bool fullText=false) const {
        const int end = m_matchLineString.length();
        const int pos = m_positionInLine + m_matchLength;

        if(!fullText && end-pos > CUTOFF_LENGTH)
            return m_matchLineString.mid(pos, CUTOFF_LENGTH) + "...";
        else
            return m_matchLineString.right(end-pos);
    }

    // TODO: Decide if these remain public vars or not.
//private:
    QString m_matchLineString;

    int m_lineNumber;
    int m_positionInFile;
    int m_positionInLine;
    int m_matchLength;

private:
    static const int CUTOFF_LENGTH; //Number of characters before/after match result that will be shown in preview
};



struct DocResult {
    QString fileName;
    QVector<MatchResult> results;

    enum DocType {
        TypeNone,
        TypeFile,
        TypeDocument
    };
    DocType docType = TypeNone;
};

struct SearchResult {

    /**
     * @brief countResults Returns the total number of MatchResults in all DocResults combined.
     */
    int countResults() const {
        int total = 0;

        for(const DocResult& docResult : results)
            total += docResult.results.size();

        return total;
    }

    QVector<DocResult> results;

    // TODO: Debug value
    qint64 m_timeToComplete = 0;
};

class FileSearcher : public QThread {
    Q_OBJECT

public:
    FileSearcher(const SearchConfig& config);
    void cancel() { m_wantToStop = true; }
    const SearchResult& getResult() { return m_searchResult; }

    static QRegularExpression createRegexFromConfig(const SearchConfig& config);
    static DocResult searchPlainText(const SearchConfig& config, const QString& content);
    static DocResult searchRegExp(const QRegularExpression& regex, const QString& content);

signals:
    void resultProgress(int processed, int total);
    void resultReady();

protected:
    void run() override;

private:
    // TODO: Integrate this into run()
    void worker();

    SearchConfig m_searchConfig;
    QRegularExpression m_regex;
    bool m_wantToStop = false;
    SearchResult m_searchResult;
};

#endif // SEARCHWORKER_H
