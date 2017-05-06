#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QRegularExpression>
#include <QVector>

#include "include/Search/searchhelpers.h"

// TODO: Find a good home for SearchConfig, MatchResult, etc
struct SearchConfig {
    int scope;
    QString searchString;
    QString filePattern;
    QString directory;

    bool matchCase;
    bool matchWord;
    bool includeSubdirs;

    enum SearchMode {
        ModeRegex,
        ModePlainText,
        ModePlainTextSpecialChars
    };
    SearchMode searchMode;

    //SearchHelpers::SearchMode searchMode;
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

//private:
    QString m_matchLineString;

    int m_lineNumber;
    int m_positionInFile;
    int m_positionInLine;
    int m_matchLength;

    bool m_selected = false;

private:
    static const int CUTOFF_LENGTH; //Number of characters before/after match result that will be shown in preview
};



struct DocResult {
    QString fileName;
    QVector<MatchResult> results;
};

struct SearchResult {

    int countResults() const {
        int total = 0;

        for(const DocResult& docResult : results)
            total += docResult.results.size();

        return total;
    }

    QVector<DocResult> results;

    // TODO: Debug value
    qint64 m_timeToComplete;
};

class FileSearcher : public QThread {
    Q_OBJECT

public:
    FileSearcher(const SearchConfig& config);
    void cancel() { m_wantToStop = true; }
    const SearchResult& getResult() { return m_searchResult; }

signals:
    void resultProgress(int processed, int total);
    void resultReady();

protected:
    void run() override;

private:
    DocResult searchPlainText(const QString& content);
    DocResult searchRegExp(const QString& content);

    // TODO: Integrate this into run()
    void worker();

    SearchConfig m_searchConfig;
    QRegularExpression m_regex;
    bool m_wantToStop = false;
    SearchResult m_searchResult;
};

#endif // SEARCHWORKER_H
