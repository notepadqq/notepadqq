#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <vector>

#include <QObject>
#include <QPoint>
#include <QString>
#include <QThread>

#include "include/Search/searchhelpers.h"
#include <QRegularExpression>

#include <QDebug>

// TODO: Find a good home for SearchConfig, MatchResult, etc
struct SearchConfig {
    int scope;
    QString searchString;
    QString filePattern;
    QString directory;

    bool matchCase;
    bool matchWord;
    bool includeSubdirs;

    SearchHelpers::SearchMode searchMode;
};

struct MatchResult {

    QString getFormattedText(bool showFullText=false) const {
        return QString("<span style='white-space:pre-wrap;'>%1:\t"
                       "%2"
                       "<span style='background-color: #ffef0b; color: black;'>%3</span>"
                       "%4</span>")
                .arg(m_lineNumber)
                 // Natural tabs are way too large; just replace them.
                .arg(getPreMatchString(showFullText).replace('\t', "    ").toHtmlEscaped(),
                     getMatchString().replace('\t', "    ").toHtmlEscaped(),
                     getPostMatchString(showFullText).replace('\t', "    ").toHtmlEscaped());
    }

    QString getMatchString() const { return m_matchLineString.mid(m_matchIndex.x(), m_matchIndex.y()); }

    QString getPreMatchString(bool fullText=false) const {
        const int pos = m_matchIndex.x();

        // Cut off part of the text if it is too long and the caller did not request full text
        if(!fullText && pos > CUTOFF_LENGTH)
            return "..." + m_matchLineString.mid( std::max(0, pos-CUTOFF_LENGTH), std::min(CUTOFF_LENGTH, pos) );
        else
            return m_matchLineString.left(pos);
    }

    QString getPostMatchString(bool fullText=false) const {
        const int end = m_matchLineString.length();
        const int pos = m_matchIndex.x()+m_matchIndex.y();

        if(!fullText && end-pos > CUTOFF_LENGTH)
            return m_matchLineString.mid(pos, CUTOFF_LENGTH) + "...";
        else
            return m_matchLineString.right(end-pos);
    }

//private:
    // TODO: Don't use m_matchIndex. Use some qint64's instead
    QString m_matchLineString;
    QPoint m_matchIndex;
    qint64 m_matchOffset;
    qint32 m_lineNumber;

private:
    static const int CUTOFF_LENGTH; //Number of characters before/after match result that will be shown in preview
};



struct DocResult {

    QString getFormattedText() const {
        // TODO: May want to cut down file path as well, if it's too long
        return QString("%1 Results for: '<b>%2</b>'").arg(results.size()).arg(fileName.toHtmlEscaped());
    }

    QString fileName;
    std::vector<MatchResult> results;
};

struct SearchResult {
    std::vector<DocResult> results;

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
