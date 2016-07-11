#ifndef SEARCHHELPERS_H
#define SEARCHHELPERS_H

class SearchHelpers
{

public:

    enum class SearchMode {
        PlainText = 1,
        SpecialChars = 2,
        Regex = 3
    };

    struct SearchOptions {
        unsigned MatchCase : 1;
        unsigned MatchWholeWord : 1;
        unsigned SearchFromStart : 1;
        unsigned IncludeSubDirs : 1;

        SearchOptions() : MatchCase(0), MatchWholeWord(0),
        SearchFromStart(0), IncludeSubDirs(0) { }
    };

};

#endif // SEARCHHELPERS_H
