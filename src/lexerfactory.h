#ifndef LEXERFACTORY_H
#define LEXERFACTORY_H

#include <QList>
#include <QStringList>
#include <QHash>
#include <QColor>
#include <QString>
#include <QFileInfo>
#include <QSharedPointer>

namespace stylename
{
    extern const char* GLOBAL_OVERRIDE;
    extern const char* DEFAULT;
    extern const char* INDENT_GUIDELINE;
    extern const char* BRACE_HIGHLIGHT;
    extern const char* BAD_BRACE;
    extern const char* CURRENT_LINE;
    extern const char* SELECTED_TEXT;
    extern const char* CARET;
    extern const char* EDGE;
    extern const char* LINE_NUMBER_MARGIN;
    extern const char* FOLD;
    extern const char* FOLD_MARGIN;
    extern const char* WHITE_SPACE_SYMBOL;
    extern const char* SMART_HIGHLIGHTING;
    extern const char* FIND_MARK;
    extern const char* MARK1;
    extern const char* MARK2;
    extern const char* MARK3;
    extern const char* MARK4;
    extern const char* MARK5;
    extern const char* INCR_HIGHLIGHT_ALL;
    extern const char* TAG_MATCH_HIGHLIGHT;
    extern const char* TAG_ATTRIBUTE;
    extern const char* ACTV_TAB_FOCUS;
    extern const char* ACTV_TAB_UNFOCUS;
    extern const char* ACTV_TAB_TEXT;
    extern const char* INACTIVE_TAB;
}

class QsciLexer;

struct KeywordCollection
{
    QString     type;
    QStringList keywords;
}; typedef QSharedPointer<KeywordCollection> ShrPtrKeywordCollection;

struct LangDefinition
{
    QString     name;
    QStringList file_extensions;
    QString     comment_line;
    QString     comment_start;
    QString     comment_end;

    QList<ShrPtrKeywordCollection>          keywords;
    QHash<QString, ShrPtrKeywordCollection> keywords_by_class;

    LangDefinition()
        : comment_line(QString::null), comment_start(QString::null), comment_end(QString::null)
    {

    }
}; typedef QSharedPointer<LangDefinition> ShrPtrLangDefinition;

struct WordsStyle
{
    QString name;
    qint16  style_id;
    QColor  fg_color;
    QColor  bg_color;
    QString font_name;
    qint16  font_style;
    qint16  font_size;
    QString keyword_class;
}; typedef QSharedPointer<WordsStyle> ShrPtrWordsStyle;

struct StylerDefinition
{
    QString name;
    QString desc;
    QList<ShrPtrWordsStyle>          words_stylers;
    QHash<QString, ShrPtrWordsStyle> words_stylers_by_name;

}; typedef QSharedPointer<StylerDefinition> ShrPtrStylerDefinition;

class LexerFactory
{
public:
    LexerFactory();
    bool init();
    bool setColorSchemeFile(QString filePath);

    ShrPtrLangDefinition   detectLanguage(QFileInfo info);
    QsciLexer*             createLexer(QFileInfo info,   QObject *parent = 0);
    QsciLexer*             createLexer(QString language, QObject *parent = 0);
    ShrPtrStylerDefinition getGlobalStyler() const;
    QHash<QString,QString> languages();
protected:
    bool parseLanguageDefinitions   ();
    bool parseColorSchemeDefinitions();
    QsciLexer* applyColorScheme(ShrPtrLangDefinition lang, QsciLexer* lex);


private:
    QString langDefFile;
    QString stylersDefFile;
    QList<ShrPtrLangDefinition>            _languages;
    QHash<QString, ShrPtrLangDefinition>   languages_by_name;
    QHash<QString, ShrPtrLangDefinition>   language_by_extension;
    QList<ShrPtrStylerDefinition>          stylers;
    QHash<QString, ShrPtrStylerDefinition> stylers_by_name;
    ShrPtrStylerDefinition                 global_styler;
};

#endif // LEXERFACTORY_H
