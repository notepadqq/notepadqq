#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "include/EditorNS/languagecache.h"
#include "include/notepadqq.h"

namespace EditorNS {

LanguageCache::LanguageCache()
{
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
}

const LanguageList& LanguageCache::languages()
{
    return m_languages;
}

int LanguageCache::lookupById(const QString& id)
{
    auto it = std::find_if (m_languages.begin(), m_languages.end(), [&id] (const Language& l) {
        return (l.id == id);
    });
    if (it != m_languages.end())
        return it - m_languages.begin();
    return -1;
}

int LanguageCache::lookupByFileName(const QString& fileName)
{
    auto it = std::find_if(m_languages.begin(), m_languages.end(), [&fileName] (const Language& l) {
        return (l.fileNames.contains(fileName));
    });
    if (it != m_languages.end())
        return it - m_languages.begin();
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
