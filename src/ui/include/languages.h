#ifndef LANGUAGES_H
#define LANGUAGES_H

#include <QString>

class Languages
{
public:
    Languages();
    static QString detectLanguage(QString fileName);

private:
    static QString detectLanguageFromExtension(QString fileName);
    static QString detectLanguageFromSpecialName(QString fileName);
};

#endif // LANGUAGES_H
