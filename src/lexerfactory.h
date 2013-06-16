#ifndef LEXERFACTORY_H
#define LEXERFACTORY_H

#include <QList>
#include <QStringList>
#include <QHash>
#include <QString>
#include <QFileInfo>

// name="actionscript" ext="as mx" commentLine="//" commentStart="/*" commentEnd="*/"

class QsciLexer;

struct KeywordCollection
{
    QString     type;
    QStringList keywords;
};

struct LangDefinition
{
    QString     name;
    QStringList file_extensions;
    QString     comment_line;
    QString     comment_start;
    QString     comment_end;

    QList<KeywordCollection>          keywords;
    QHash<QString, KeywordCollection> keywords_by_class;

    LangDefinition()
        : comment_line(QString::null), comment_start(QString::null), comment_end(QString::null)
    {

    }
};

class LexerFactory
{
public:
    LexerFactory();
    bool init();

    LangDefinition detectLanguage(QFileInfo info);
    QsciLexer*     createLexer(QFileInfo info,   QObject *parent = 0);
    QsciLexer*     createLexer(QString language, QObject *parent = 0);
private:
    QString langDefFile;
    QList<LangDefinition>          languages;
    QHash<QString, LangDefinition> languages_by_name;
    QHash<QString, LangDefinition> language_by_extension;
};

#endif // LEXERFACTORY_H
