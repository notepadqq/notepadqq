#include "include/languages.h"
#include "include/docengine.h"
#include <QFile>

Languages::Languages()
{
}

QString Languages::detectLanguage(QString fileName)
{
    QString lang = Languages::detectLanguageFromSpecialName(fileName);

    if (lang == "") {
        lang = Languages::detectLanguageFromExtension(fileName);
    }

    if (lang == "") {
        lang = DocEngine::getFileMimeType(fileName);

        if (lang.compare("text/plain", Qt::CaseInsensitive) == 0)
            lang = "";
    }

    // TODO Add last try to detect language from text content
    // (e.g. file starting with "<?xml" ==> "xml")

    return lang == "" ? "text/plain" : lang;
}

QString Languages::detectLanguageFromExtension(QString fileName)
{
    if(fileName.endsWith(".js", Qt::CaseInsensitive))
        return "text/javascript";

    else if(fileName.endsWith(".cpp", Qt::CaseInsensitive))
        return "text/x-c++hdr";

    else if(fileName.endsWith(".css", Qt::CaseInsensitive))
        return "text/css";

    else if(fileName.endsWith(".h", Qt::CaseInsensitive))
        return "text/x-c++src";

    else if(fileName.endsWith(".html", Qt::CaseInsensitive) ||
            fileName.endsWith(".htm", Qt::CaseInsensitive))
        return "text/html";

    else if(fileName.endsWith(".ts", Qt::CaseInsensitive))
        return "text/typescript";

    else if(fileName.endsWith(".xml", Qt::CaseInsensitive))
        return "text/xml";

    else
        return "";
}

QString Languages::detectLanguageFromSpecialName(QString fileName)
{
    QString file = QFile(fileName).fileName();

    if (file.compare("CmakeLists.txt", Qt::CaseInsensitive) == 0)
        return "cmake";

    else if (file.compare("makefile", Qt::CaseInsensitive) == 0 ||
             file.compare("gnumakefile", Qt::CaseInsensitive) == 0)
        return "makefile";

    else if (file.compare("SConstruct", Qt::CaseInsensitive) == 0 ||
             file.compare("SConscript", Qt::CaseInsensitive) == 0 ||
             file.compare("wscript", Qt::CaseInsensitive) == 0)
        return "text/x-python";

    else if (file.compare("Rakefile", Qt::CaseInsensitive) == 0)
        return "text/x-ruby";

    else
        return "";
}
