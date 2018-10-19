#include "include/EditorNS/languageservice.h"

#include "include/notepadqq.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextStream>

namespace EditorNS {

LanguageService::LanguageService()
{
    QFileInfo fileInfo(Notepadqq::editorPath());
    QString fileName = fileInfo.absolutePath() + "/Languages.json";
    QFile scriptFile(fileName);
    scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QJsonDocument json = QJsonDocument::fromJson(scriptFile.readAll());
    scriptFile.close();

    // Reserve the space we need to prevent resize and copy constructs.
    m_languages.reserve(json.object().count());

    bool hasPlainText = false;
    // Begin iterating our QJsonDocument's object and adding languages.
    for (auto&& key : json.object().keys()) {
        if (key == "plaintext") hasPlainText = true;
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

    Q_ASSERT(hasPlainText);
}

const Language* LanguageService::lookupById(const QString& id)
{
    auto it = std::find_if (m_languages.begin(), m_languages.end(), [&id] (const Language& l) {
        return (l.id == id);
    });
    if (it == m_languages.end()) return nullptr;
    return &(*it);
}

const Language* LanguageService::lookupByFileName(const QString& fileName)
{
    auto it = std::find_if(m_languages.begin(), m_languages.end(), [&fileName] (const Language& l) {
        return (l.fileNames.contains(fileName));
    });
    if (it == m_languages.end()) return nullptr;
    return &(*it);
}

const Language* LanguageService::lookupByExtension(const QString& fileName)
{
    QFileInfo fi(fileName);
    auto ext = fi.suffix();
    auto it = std::find_if(m_languages.begin(), m_languages.end(), [&ext] (const Language& l) {
        return (l.fileExtensions.contains(ext,Qt::CaseInsensitive));
    });
    if (it == m_languages.end()) return nullptr;
    return &(*it);
}

const Language* LanguageService::lookupByContent(QString content)
{
    if (content.isEmpty()) {
        return nullptr;
    }
    QTextStream stream(&content);
    stream.skipWhiteSpace();
    auto test = stream.readLine();
    for (auto&& l : m_languages) {
        if (l.firstNonBlankLine.isEmpty()) continue;
        for (auto&& t : l.firstNonBlankLine) {
            if (test.contains(QRegularExpression(t))) {
                return &l;
            }
        }
    }
    return nullptr;
}

LanguageService& LanguageService::getInstance()
{
    static LanguageService instance;
    return instance;
}

}
