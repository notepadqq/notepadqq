#ifndef SESSIONS_H
#define SESSIONS_H

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

#include <vector>

struct TabData {
    QString filePath;
    QString cacheFilePath;
    int scrollX = 0;
    int scrollY = 0;
};

struct ViewData {
    std::vector<TabData> tabs;
};

/**
 * @brief Provides a convenience class to read session .xml files.
 *
 */
class SessionReader {
public:

    SessionReader(QFile& input)
        : m_reader(&input) { }

    /**
     * @brief Completely read the session data
     *
     * @param outSuccess pass a pointer to bool here to be informed about whether
     * the reading process has encountered any errors.
     *
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
     * TabWidget and its tabs.
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

#endif // SESSIONS_H
