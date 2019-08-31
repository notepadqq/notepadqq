#ifndef DOCENGINE_H
#define DOCENGINE_H

#include "editortabwidget.h"
#include "topeditorcontainer.h"

#include <QFile>
#include <QFileSystemWatcher>
#include <QObject>
#include <QUrl>

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
    explicit DocEngine(TopEditorContainer *topEditorContainer, QObject *parent = nullptr);
    ~DocEngine();

    struct DecodedText {
        QString text;
        QTextCodec *codec = nullptr;
        bool bom = false;
        bool error = false;
    };

    enum FileSizeAction {
        FileSizeActionAsk,
        FileSizeActionYesToAll,
        FileSizeActionNoToAll
    };

    enum ReloadAction {
        ReloadActionDont,   // Don't reload documents, instead just focus them
        ReloadActionAsk,    // Ask user to reload if it would cause unsaved changes to be discarded
        ReloadActionDo      // Always reload documents
    };

    /**
     * @brief The DocumentLoader struct is an aggregation of all possible arguments for document loading.
     *        Only setTabWidget and setUrl(s) are necessary settings. All others have sensible defaults.
     *        Create new instances of this class using DocEngine::getDocumentLoader()
     */
    struct DocumentLoader {
        // Set the URL(s) of files to be loaded
        DocumentLoader& setUrl(const QUrl& url) { this->urls << url; return *this; }
        DocumentLoader& setUrls(const QList<QUrl>& urls) { this->urls = urls; return *this; }

        // Set how files should be handled that trigger a file-size warning.
        DocumentLoader& setFileSizeWarning(FileSizeAction fsa) { fileSizeAction = fsa; return *this; }

        // If true, the documents' parent directory will be remembered as the last opened dir.
        DocumentLoader& setRememberLastDir(bool rld) { rememberLastDir = rld; return *this; }

        // Set if document has Byte Order Marks set
        DocumentLoader& setBOM(bool setBom) { bom = setBom; return *this; }

        // Sets the TextCodec to decode the file as.
        DocumentLoader& setTextCodec(QTextCodec* codec) { textCodec = codec; return *this; }

        // Set the TabWidget the documents should be loaded into
        DocumentLoader& setTabWidget(EditorTabWidget* tw) { tabWidget = tw; return *this; }

        // Determines how already opened documents should be treated.
        DocumentLoader& setReloadAction(ReloadAction reload) { reloadAction = reload; return *this; }

        // Index of the URL that must be loaded with highest priority, for example because
        // it will be the one with user focus. With constants ALL_MINIMUM_PRIORITY and
        // ALL_MAXIMUM_PRIORITY, all URLs will be loaded with the same low or high priority.
        // This parameter has effect only for background executions (i.e. executeInBackground()).
        DocumentLoader& setPriorityIdx(int idx) { priorityIdx = idx; return *this; }

        // Set whether, after an Editor has been created and his content has been loaded,
        // the document loader should take care of things like assigning a file path to the
        // editor, setting the syntax highlighting, enabling monitoring and so on.
        // If not nullptr, runs the specified function to allow manual initialization instead
        // of the internal one. Note that, in case of a document reload, the internal
        // initialization is used regardless of this parameter. Similarly, this parameter
        // is ignored for non-background executions (i.e. execute()).
        // This is useful for example in case of loading tabs from a session, where in some
        // cases a fake file path (different from the data source) must be assigned to the
        // Editor during its loading.
        //
        // If this function is called, it will be called asynchronously. However, the function
        // itself must be synchronous so that the DocumentLoader knows when to emit the
        // DocumentLoaded event.
        DocumentLoader& setManualEditorInitialization(
                std::function<void(QSharedPointer<Editor> editor, const QUrl& url)> f) {
            manualEditorInitialization = f; return *this;
        }

        /**
         * @brief execute Runs the load operation.
         */
        QPromise<void> execute() {
            Q_ASSERT(tabWidget != nullptr);
            return docEngine.loadDocuments(*this);
        }

        QList<std::pair<QSharedPointer<Editor>, QPromise<QSharedPointer<Editor>>>> executeInBackground() {
            Q_ASSERT(tabWidget != nullptr);
            return docEngine.loadDocumentsInBackground(*this);
        }

        static constexpr int ALL_MINIMUM_PRIORITY = -1;
        static constexpr int ALL_MAXIMUM_PRIORITY = -2;

        // See here for the arguments' default values
        QList<QUrl> urls;
        EditorTabWidget* tabWidget      = nullptr;
        QTextCodec* textCodec           = nullptr;
        ReloadAction reloadAction       = ReloadActionAsk;
        bool rememberLastDir            = true;
        bool bom                        = false;
        FileSizeAction fileSizeAction   = FileSizeActionAsk;
        int priorityIdx                 = ALL_MAXIMUM_PRIORITY;
        std::function<void(QSharedPointer<Editor> editor, const QUrl& url)> manualEditorInitialization = nullptr;

    private:
        friend class DocEngine;
        DocumentLoader(DocEngine& eng) : docEngine(eng) {}
        DocEngine& docEngine;
    };

    /**
     * @brief getDocumentLoader Creates a new DocumentLoader with this DocEngine as its parent.
     *        Use this object to load or reload documents.
     */
    DocumentLoader getDocumentLoader() { return DocumentLoader(*this); }

    /**
     * Describes the result of a save process.
     * For example, if the user cancels the save dialog, \p saveFileResult_Canceled is returned.
     */
    enum saveFileResult {
         saveFileResult_Saved,      /** The file was saved  */
        saveFileResult_Canceled     /** The save process was canceled */
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
    void monitorDocument(QSharedPointer<Editor> editor);
    void unmonitorDocument(Editor *editor);
    void unmonitorDocument(QSharedPointer<Editor> editor);
    bool isMonitored(Editor *editor);

    int addNewDocument(QString name, bool setFocus, EditorTabWidget *tabWidget);
    void reinterpretEncoding(QSharedPointer<Editor> editor, QTextCodec *codec, bool bom);
    static DocEngine::DecodedText readToString(QFile *file);
    static DocEngine::DecodedText readToString(QFile *file, QTextCodec *codec, bool bom);
    static bool writeFromString(QIODevice *io, const DecodedText &write);

    /**
     * @brief Write the provided Editor content to the specified IO device, using
     *        the encoding and the BOM settings specified in the Editor.
     * @param io
     * @param editor
     * @return true if successful, false otherwise
     */
    bool write(QIODevice *io, QSharedPointer<Editor> editor);
    bool write(QUrl outFileName, QSharedPointer<Editor> editor);

    /**
     * @brief getNewDocumentName
     * @return Returns a QString with a fitting name for a new document tab.
     */
    QString getNewDocumentName() const;

private:
    TopEditorContainer *m_topEditorContainer;
    QFileSystemWatcher *m_fsWatcher;

    /**
     * @brief Read a file and puts the content into the provided Editor, clearing
     *        its history and marking it as clean. Tries to automatically
     *        detect the encoding.
     * @param file
     * @param editor
     * @return fulfilled if successful, rejected otherwise
     */
    QPromise<void> read(QFile *file, QSharedPointer<Editor> editor);
    QPromise<void> read(QFile *file, QSharedPointer<Editor> editor, QTextCodec *codec, bool bom);
    // FIXME Separate from reload

    /**
     * @brief loadDocuments Responsible for loading or reloading a number of text files.
     * @param docLoader Contains parameters for document loading. See DocumentLoader class for info.
     */
    QPromise<void> loadDocuments(const DocumentLoader& docLoader);

    /**
     * @brief Loads documents in background. Experimental API that needs to be integrated
     * into DocumentLoader.
     * @param docLoader
     * @return
     */
    QList<std::pair<QSharedPointer<Editor>, QPromise<QSharedPointer<Editor>>>> loadDocumentsInBackground(const DocumentLoader& docLoader);

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

    /**
     * @brief getAvailableSudoProgram Queries the system to find a supported graphical sudo tool.
     * @return Empty string if none found. Else either 'kdesu', 'gksu', or 'pkexec'.
     */
    QString getAvailableSudoProgram() const;

    /**
     * @brief Attempts to save the contents of editor to outFileName using a graphical sudo program.
     * @param sudoProgram Name of the sudo tool to use. Only 'kdesu', 'gksu' and 'pkexec' supported.
     * @param outFileName Target location of file
     * @param editor Editor to be saved
     * @return True if successful.
     */
    bool trySudoSave(QString sudoProgram, QUrl outFileName, QSharedPointer<Editor> editor);

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

    /**
     * @brief The document has been successfully loaded.
     * @param tabWidget The TabWidget that contains the loaded doc.
     * @param tab The tab index of the loaded doc.
     * @param wasAlreadyOpened True if the document was only reloaded.
     * @param updateRecentDocuments True if the document should be remembered
     *        as a recently opened file.
     */
    void documentLoaded(EditorTabWidget *tabWidget, int tab, bool wasAlreadyOpened, bool updateRecentDocuments);

private slots:
    void documentChanged(QString fileName);
};

#endif // DOCENGINE_H
