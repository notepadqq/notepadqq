#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "include/EditorNS/languagecache.h"
#include "include/notepadqq.h"

// Self-contained benchmarking utilities.
#ifdef QT_DEBUG
#include <QDebug>
#include <QElapsedTimer>
#define STRINGIFY(X) #X
#define TIMER QElapsedTimer timer;timer.start()
#define TOTALTIME(X) qDebug().noquote() << STRINGIFY(X:) << timer.nsecsElapsed() << "nanosecs"
#else
#define TIMER
#define TOTALTIME(X)
#endif

namespace EditorNS {

Language::Language()
{

}

Language::Language(const Language& o) :
    id(o.id),
    name(o.name),
    mime(o.mime),
    mode(o.mode),
    fileNames(o.fileNames),
    fileExtensions(o.fileExtensions),
    firstNonBlankLine(o.firstNonBlankLine) 
{
}

Language::Language(Language&& o) noexcept :
    id(std::move(o.id)),
    name(std::move(o.name)),
    mime(std::move(o.mime)),
    mode(std::move(o.mode)),
    fileNames(std::move(o.fileNames)),
    fileExtensions(std::move(o.fileExtensions)),
    firstNonBlankLine(std::move(o.firstNonBlankLine))
{
}

Language& Language::operator=(const Language& o)
{
    id = o.id;
    name = o.name;
    mime = o.mime;
    mode = o.mode;
    fileNames = o.fileNames;
    fileExtensions = o.fileExtensions;
    firstNonBlankLine = o.firstNonBlankLine;
    return *this;
}

Language& Language::operator=(Language&& o)
{
    id = std::move(o.id);
    name = std::move(o.name);
    mime = std::move(o.mime);
    mode = std::move(o.mode);
    fileNames = std::move(o.fileNames);
    fileExtensions = std::move(o.fileExtensions);
    firstNonBlankLine = std::move(o.firstNonBlankLine);
    return *this;
}

LanguageCache::LanguageCache()
{
    TIMER;
    QFileInfo fileInfo(Notepadqq::editorPath());
    QString fileName = fileInfo.absolutePath() + "/Languages.json";
    QFile scriptFile(fileName);
    scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QJsonDocument json = QJsonDocument::fromJson(scriptFile.readAll());
    scriptFile.close();

    // Reserve the space we need to prevent resize and copy constructs.
    m_languages.reserve(json.object().count());

    // Begin iterating our QJsonDocument's object and adding languages.
    for (const auto& key : json.object().keys()) {
        auto mode = json.object().value(key).toObject();
        Language newMode;
        newMode.id = key;
        newMode.name = mode.value("name").toString();
        newMode.mime = mode.value("mime").toString();
        newMode.mode = mode.value("mode").toString();
        newMode.fileNames = mode.value("fileNames")
            .toVariant().toStringList();
        newMode.fileExtensions = mode.value("fileExtensions")
            .toVariant().toStringList();
        newMode.firstNonBlankLine = mode.value("firstNonBlankLine")
            .toVariant().toStringList();
        m_languages.append(std::move(newMode));
    }
    TOTALTIME("Language Cache Initialization");
}

const LanguageList& LanguageCache::languages()
{
    return m_languages;
}

int LanguageCache::lookupById(const QString& id)
{
    TIMER;
    auto end = m_languages.constEnd();
    for (auto it = m_languages.constBegin(); it != end; ++ it) {
        if (it->id == id) {
            TOTALTIME("Language Cache Lookup[ID]");
            return it - m_languages.constBegin();
        }
    }
    return -1;
}

int LanguageCache::lookupByFileName(const QString& fileName)
{
    TIMER;
    auto start = m_languages.constBegin();
    auto end = m_languages.constEnd();
    for (auto it = start; it != end; ++it) {
        if (it->fileNames.contains(fileName)) {
            TOTALTIME("Language Cache Lookup[File Name]");
            return it - start;
        }
    }
    return -1;
}

int LanguageCache::lookupByExtension(const QString& fileName)
{
    auto start = m_languages.constBegin();
    auto end = m_languages.constEnd();
    auto pos = fileName.lastIndexOf('.');
    if (pos == -1) {
        return -1;
    }

    auto ext = fileName.mid(pos+1);
    for (auto it = start; it != end; ++it) {
        if (it->fileExtensions.contains(ext, Qt::CaseInsensitive)) {
            return it - start;
        }
    }
    return -1;
}

LanguageCache& LanguageCache::getInstance()
{
    static LanguageCache instance;
    return instance;
}

}
