#ifndef DOCENGINE_H
#define DOCENGINE_H

#include <QObject>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QFile>
#include <QUrl>
#include "editortabwidget.h"
#include "topeditorcontainer.h"

/**
 * @brief Provides methods for managing documents
 *
 * This class is responsible for reading files, monitoring
 * file changes, text encoding, etc.
 */
class DocEngine : public QObject
{
    Q_OBJECT
public:
    explicit DocEngine(QSettings *settings, TopEditorContainer *topEditorContainer, QObject *parent = 0);
    ~DocEngine();

    struct DecodedText {
        QString text;
        QTextCodec *codec = nullptr;
        bool bom = false;
        bool error = false;
    };

    /**
     * @brief Saves a document to the file system.
     * @param tabWidget tabWidget where the document is
     * @param tab tab of the tabWidget that identifies the document
     * @param outFileName Where to save the file. If it's an empty url,
     *                    then the file name is the same as the current
     *                    file name of the document.
     * @param copy If true, do not change the file name of the document to the
     *             new path. Just save a copy.
     * @return A MainWindow::saveFileResult.
     */
    int saveDocument(EditorTabWidget *tabWidget, int tab, QUrl outFileName = QUrl(), bool copy = false);

    void closeDocument(EditorTabWidget *tabWidget, int tab);

    QPair<int, int> findOpenEditorByUrl(const QUrl &filename) const;

    void monitorDocument(Editor *editor);
    void unmonitorDocument(Editor *editor);
    bool isMonitored(Editor *editor);

    bool loadDocuments(const QList<QUrl> &fileNames, EditorTabWidget *tabWidget);
    bool loadDocument(const QUrl &fileName, EditorTabWidget *tabWidget);
    bool reloadDocument(EditorTabWidget *tabWidget, int tab);
    bool reloadDocument(EditorTabWidget *tabWidget, int tab, QTextCodec *codec, bool bom);
    int addNewDocument(QString name, bool setFocus, EditorTabWidget *tabWidget);
    void reinterpretEncoding(Editor *editor, QTextCodec *codec, bool bom);
    static DocEngine::DecodedText readToString(QFile *file);
    static DocEngine::DecodedText readToString(QFile *file, QTextCodec *codec, bool bom);
    static bool writeFromString(QIODevice *io, const DecodedText &write);

private:
    QSettings *m_settings;
    TopEditorContainer *m_topEditorContainer;
    QFileSystemWatcher *m_fsWatcher;

    /**
     * @brief Read a file and puts the content into the provided Editor, clearing
     *        its history and marking it as clean. Tries to automatically
     *        detect the encoding.
     * @param file
     * @param editor
     * @return true if successful, false otherwise
     */
    bool read(QFile *file, Editor *editor);
    bool read(QFile *file, Editor *editor, QTextCodec *codec, bool bom);
    // FIXME Separate from reload
    bool loadDocuments(const QList<QUrl> &fileNames, EditorTabWidget *tabWidget, const bool reload, QTextCodec *codec, bool bom);

    /**
     * @brief Write the provided Editor content to the specified IO device, using
     *        the encoding and the BOM settings specified in the Editor.
     * @param io
     * @param editor
     * @return true if successful, false otherwise
     */
    bool write(QIODevice *io, Editor *editor);
    void monitorDocument(const QString &fileName);
    void unmonitorDocument(const QString &fileName);

    /**
     * @brief Decodes a byte array into a string, trying to guess the best
     *        codec.
     * @param contents
     * @return
     */
    static DecodedText decodeText(const QByteArray &contents);
    /**
     * @brief Decodes a byte array into a string, using the specified codec.
     * @param contents
     * @param codec
     * @param contentHasBOM Simply copied to the result struct.
     * @return
     */
    static DecodedText decodeText(const QByteArray &contents, QTextCodec *codec, bool contentHasBOM);

    static QByteArray getBomForCodec(QTextCodec *codec);
signals:
    /**
     * @brief The monitored file has changed. Remember to call
     *        monitorDocument() again if you want to keep monitoring it.
     * @param tabWidget
     * @param tab
     * @param removed true if the file has been removed from the disk
     */
    void fileOnDiskChanged(EditorTabWidget *tabWidget, int tab, bool removed);

    /**
     * @brief The document has been successfully saved. This event is
     *        not emitted if the document has just been copied to another
     *        location.
     * @param tabWidget
     * @param tab
     */
    void documentSaved(EditorTabWidget *tabWidget, int tab);

    void documentReloaded(EditorTabWidget *tabWidget, int tab);

    void documentLoaded(EditorTabWidget *tabWidget, int tab, bool wasAlreadyOpened);

public slots:

private slots:
    void documentChanged(QString fileName);
};

#endif // DOCENGINE_H
