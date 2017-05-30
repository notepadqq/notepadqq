#include "include/Sessions/sessions.h"

#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDateTime>

#include <vector>

#include "include/docengine.h"
#include "include/nqqtab.h"
#include "include/Sessions/persistentcache.h"


/* Session XML structure:
 *
 * <Notepadqq>
 *      <View>
 *          <Tab filePath="xxx" .../>
 *          <Tab filePath="xxx" .../>
 *      </View>
 *      <View>
 *          ...
 *      </View>
 *      ...
 * <Notepadqq>
 *
 *
 * All currently available attributes for <Tab>:
 * -> string filePath - path to the file if it exists
 * -> string cacheFilePath - path to the cache file if it exists
 * -> int scrollX - horizontal scroll position
 * -> int scrollY - vertical scroll position
 * -> string language - the display language of the document.
 * -> long int lastModified - optional, last modification date (in msecs since epoch) of the file point to in filePath
 * -> int active - optional, value is "1" if this tab is the open one in the tabview, otherwise "0".
 *
 * */


struct TabData {
    QString filePath;
    QString cacheFilePath;
    int scrollX = 0;
    int scrollY = 0;
    bool active = false;
    QString language;
    qint64 lastModified = 0;
};

struct ViewData {
    std::vector<TabData> tabs;
};

/**
 * @brief Provides a convenience class to read session .xml files.
 */
class SessionReader {
public:

    SessionReader(QFile& input)
        : m_reader(&input) { }

    /**
     * @brief Completely read the session data
     * @param outSuccess pass a pointer to bool here to be informed about whether
     *        the reading process has encountered any errors.
     * @return The data read from the session file.
     */
    std::vector<ViewData> readData(bool* outSuccess=nullptr);

    QString getError();

private:

    /**
     * @brief Helper functions to read specific parts of the xml structure
     */
    std::vector<ViewData> readViewData();
    std::vector<TabData> readTabData();

    QXmlStreamReader m_reader;
};


/**
 * @brief Provides a convenience class to write session .xml files.
 *
 * Note that a SessionWriter object must be successfully destroyed
 * in order to complete the writing process. (Aka the destructor must
 * be called)
 */
class SessionWriter {
public:

    SessionWriter(QFile& destination);

    ~SessionWriter();

    /**
     * @brief Write ViewData to the session file. ViewData is the representation of a
     *        TabWidget and its tabs.
     *
     * @param The ViewData to be written.
     */
    void addViewData(const ViewData& vd);


private:
    /**
     * @brief Helper function to write specific parts of the xml structure
     */
    void addTabData(const TabData& td);

    QXmlStreamWriter m_writer;
};

std::vector<ViewData> SessionReader::readData(bool* outSuccess) {
    std::vector<ViewData> result;

    if (m_reader.readNextStartElement()) {
        if (m_reader.name() == "Notepadqq") {
            result = readViewData();
        }
        else
            m_reader.raiseError(QObject::tr("Error reading session file"));
    };

    if (outSuccess != nullptr)
        *outSuccess = !m_reader.error();

    return result;
}

QString SessionReader::getError(){
    return m_reader.errorString();
}

std::vector<ViewData> SessionReader::readViewData() {
    std::vector<ViewData> result;

    while (m_reader.readNextStartElement()) {
        if (m_reader.name() == "View") {
            ViewData vd;
            vd.tabs = readTabData();
            result.push_back(vd);
        }
        else
            m_reader.skipCurrentElement();
    }


    return result;
}

std::vector<TabData> SessionReader::readTabData() {
    std::vector<TabData> result;

    while (m_reader.readNextStartElement()) {
        if (m_reader.name() == "Tab") {
            const QXmlStreamAttributes& attrs = m_reader.attributes();

            TabData td;
            td.filePath = attrs.value("filePath").toString();
            td.cacheFilePath = attrs.value("cacheFilePath").toString();
            td.scrollX = attrs.value("scrollX").toInt();
            td.scrollY = attrs.value("scrollY").toInt();
            td.language = attrs.value("language").toString();
            td.lastModified = attrs.value("lastModified").toLongLong();
            td.active = attrs.value("active").toInt() != 0;

            result.push_back(td);

            m_reader.readElementText();
        }
        else
            m_reader.skipCurrentElement();
    }

    return result;
}

SessionWriter::SessionWriter(QFile& destination)
    : m_writer(&destination)
{
    m_writer.setAutoFormatting(true);

    m_writer.writeStartDocument();
    m_writer.writeStartElement("Notepadqq");
}

SessionWriter::~SessionWriter(){
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
}

void SessionWriter::addViewData(const ViewData& vd){
    if (vd.tabs.empty())
        return;

    m_writer.writeStartElement("View");

    for (auto&& tab : vd.tabs)
        addTabData(tab);

    m_writer.writeEndElement();
}

void SessionWriter::addTabData(const TabData& td){
    m_writer.writeStartElement("Tab");

    QXmlStreamAttributes attrs;
    attrs.push_back(QXmlStreamAttribute("filePath", td.filePath));
    attrs.push_back(QXmlStreamAttribute("cacheFilePath", td.cacheFilePath));
    attrs.push_back(QXmlStreamAttribute("scrollX", QString::number(td.scrollX)));
    attrs.push_back(QXmlStreamAttribute("scrollY", QString::number(td.scrollY)));

    // A few attributes aren't often used, so we'll only write them into the file if they're
    // set to a non-default value as to not clutter up the xml file.
    if (!td.language.isEmpty())
        attrs.push_back(QXmlStreamAttribute("language", td.language));

    if (td.lastModified != 0)
        attrs.push_back(QXmlStreamAttribute("lastModified", QString::number(td.lastModified)));

    if (td.active)
        attrs.push_back(QXmlStreamAttribute("active", "1"));

    m_writer.writeAttributes(attrs);

    m_writer.writeEndElement();
}

namespace Sessions {

bool saveSession(DocEngine* docEngine, NqqSplitPane* pane, QString sessionPath, QString cacheDirPath)
{
    const bool cacheModifiedFiles = !cacheDirPath.isEmpty();

    QDir cacheDir;

    // Clear the cache directory by deleting and recreating it.
    if (cacheModifiedFiles) {
        cacheDir = QDir(cacheDirPath);

        bool success = false;

        if (cacheDir.exists())
            success = cacheDir.removeRecursively();

        success |= cacheDir.mkpath(cacheDirPath);

        if(!success)
            return false;
    }

    std::vector<ViewData> viewData;

    //Loop through all tabwidgets and their tabs
    for (NqqTabWidget* tabWidget : pane->getAllTabWidgets()) {

        if(tabWidget->isEmpty())
            continue;

        viewData.push_back( ViewData() );
        ViewData& currentViewData = viewData.back();

        for (NqqTab* tab : tabWidget->getAllTabs()) {
            Editor* editor = tab->m_editor;
            bool isClean = editor->isClean();
            bool isOrphan = editor->fileName().isEmpty();

            if (isOrphan && !cacheModifiedFiles)
                continue; // Don't save temporary files if we're not caching tabs

            TabData td;

            if (!isClean && cacheModifiedFiles) {
                // Tab is dirty, meaning it needs to be cached.
                QUrl cacheFilePath = PersistentCache::createValidCacheName(cacheDir, tab->getTabTitle());

                td.cacheFilePath = cacheFilePath.toLocalFile();

                if (docEngine->saveDocumentProper(editor, cacheFilePath, true) != DocEngine::saveFileResult_Saved) {
                    return false;
                }
            } else if (isOrphan) {
                // Since we didn't cache the file and it is an orphan, we won't save it in the session.
                continue;
            }
            // Else tab is an openened unmodified file, we don't have to do anything special.

            td.filePath = !isOrphan ? editor->fileName().toLocalFile() : "";

            // Finally save other misc information about the tab.
            const auto& scrollPos = editor->scrollPosition();
            td.scrollX = scrollPos.first;
            td.scrollY = scrollPos.second;
            td.active = tabWidget->getCurrentTab() == tab;
            td.language = editor->language();

            // If we're caching and there's a file opened in the tab we want to inform the
            // user whether the file's contents have changed since Nqq was last opened.
            // For this we save and later compare the modification date.
            if (!isOrphan && cacheModifiedFiles) {
                // As a special case, if the file has *already* changed we set the modification
                // time to 1 so we always trigger the warning.
                if (editor->fileOnDiskChanged())
                    td.lastModified = 1;
                else
                    td.lastModified = QFileInfo(td.filePath).lastModified().toMSecsSinceEpoch();
            }

            currentViewData.tabs.push_back( td );

        } // end for
    } // end for

    // Write all information to a session file
    QFile file(sessionPath);
    file.open(QIODevice::WriteOnly);

    if (!file.isOpen())
        return false;

    SessionWriter sessionWriter(file);

    for (const auto& view : viewData)
        sessionWriter.addViewData(view);

    return true;
}

void loadSession(DocEngine* docEngine, NqqSplitPane* pane, QString sessionPath)
{
    QFile file(sessionPath);
    file.open(QIODevice::ReadOnly);

    if (!file.isOpen())
        return;

    SessionReader reader(file);

    bool success = false;
    const auto& views = reader.readData(&success);

    if (!success || views.empty()) {
        return;
    }

    NqqTabWidget* currTabWidget = pane->getCurrentTabWidget();

    for (const auto& view : views) {
        for (const TabData& tabData : view.tabs) {
            const QFileInfo fileInfo(tabData.filePath);
            const bool fileExists = fileInfo.exists();
            const bool cacheFileExists = QFileInfo(tabData.cacheFilePath).exists();

            const QUrl fileUrl = QUrl::fromLocalFile(tabData.filePath);
            const QUrl cacheFileUrl = QUrl::fromLocalFile(tabData.cacheFilePath);

            // This is the file to load the document from
            const QUrl& loadUrl = cacheFileExists ? cacheFileUrl : fileUrl;

            Editor* editor = docEngine->loadDocumentProper(loadUrl);

            if(!editor)
                continue;

            NqqTab* tab = new NqqTab(editor);

            if (!currTabWidget)
                currTabWidget = pane->createNewTabWidget(tab);
            else
                currTabWidget->attachTab(tab);

            if (cacheFileExists) {
                editor->markDirty();
                editor->setLanguageFromFileName();
                // Since we loaded from cache we want to unmonitor the cache file.
                docEngine->unmonitorDocument(editor);
            }

            if (fileExists) {
                editor->setFileName(fileUrl);
                docEngine->monitorDocument(editor);
                tab->setTabTitle( fileUrl.fileName() );
            } else {
                editor->setFileName(QUrl());
                tab->setTabTitle(docEngine->getNewDocumentName());
            }

            // If we're loading an existing file from cache we want to inform the user whether
            // the file has changed since Nqq was last closed. For this we can compare the
            // file's last modification date.
            if (fileExists && cacheFileExists && tabData.lastModified != 0) {
                auto lastModified = fileInfo.lastModified().toMSecsSinceEpoch();

                if (lastModified > tabData.lastModified) {
                    editor->setFileOnDiskChanged(true);
                }
            }

            // If the orig. file does not exist but *should* exist, we inform the user of its removal.
            if (!fileExists && !fileUrl.isEmpty()) {
                //TODO: maybe just catch this through editor->nqqtab->etc?
                //editor->setFileOnDiskChanged(true);
                //emit docEngine->fileOnDiskChanged(tabW, idx, true);
            }


            if(tabData.active) currTabWidget->makeCurrent(tab);

            if(!tabData.language.isEmpty()) editor->setLanguage(tabData.language);

            editor->setScrollPosition(tabData.scrollX, tabData.scrollY);
            editor->clearFocus();

        } // end for

        currTabWidget = nullptr;

    } // end for

    return;
}

} // namespace Sessions
