#include "include/Sessions/sessions.h"

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
 * -> long int lastModified - optional, last modification date (in msecs since epoch) of the file point to in filePath
 * -> int active - optional, value is "1" if this tab is the open one in the tabview, otherwise "0".
 *
 * */

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
    if (td.lastModified != 0)
        attrs.push_back(QXmlStreamAttribute("lastModified", QString::number(td.lastModified)));

    if (td.active)
        attrs.push_back(QXmlStreamAttribute("active", "1"));

    m_writer.writeAttributes(attrs);

    m_writer.writeEndElement();
}
