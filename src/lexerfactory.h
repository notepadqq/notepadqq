#ifndef LEXERFACTORY_H
#define LEXERFACTORY_H

#include <QList>
#include <QStringList>
#include <QHash>
#include <QString>
#include <QFileInfo>
#include <QSharedPointer>

// name="actionscript" ext="as mx" commentLine="//" commentStart="/*" commentEnd="*/"

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

class LexerFactory
{
public:
    LexerFactory();
    bool init();

    ShrPtrLangDefinition detectLanguage(QFileInfo info);
    QsciLexer*           createLexer(QFileInfo info,   QObject *parent = 0);
    QsciLexer*           createLexer(QString language, QObject *parent = 0);
private:
    QString langDefFile;
    QList<ShrPtrLangDefinition>          languages;
    QHash<QString, ShrPtrLangDefinition> languages_by_name;
    QHash<QString, ShrPtrLangDefinition> language_by_extension;
};

#endif // LEXERFACTORY_H
