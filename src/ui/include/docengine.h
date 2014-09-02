#ifndef DOCENGINE_H
#define DOCENGINE_H

#include <QObject>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QFile>
#include "editortabwidget.h"
#include "topeditorcontainer.h"

/**
 * @brief Provides methods for managing documents
 *
 * This class is responsible for reading files, monitoring
 * file changes, etc.
 */
class DocEngine : public QObject
{
    Q_OBJECT
public:
    explicit DocEngine(QSettings *settings, TopEditorContainer *topEditorContainer, QObject *parent = 0);
    ~DocEngine();

    // FIXME Separate from reload
    bool loadDocuments(QStringList fileNames, EditorTabWidget *tabWidget, bool reload);

    static QString getFileMimeEncoding(QString file);
    static QString getFileInformation(QString file, int flags);

    /**
     * @brief Saves a document to the file system.
     * @param tabWidget tabWidget where the document is
     * @param tab tab of the tabWidget that identifies the document
     * @param outFileName Where to save the file. If it's an empty string,
     *                    then the file name is the same as the current
     *                    file name of the document.
     * @param copy If true, do not change the file name of the document to the
     *             new path. Just save a copy.
     * @return A MainWindow::saveFileResult.
     */
    int saveDocument(EditorTabWidget *tabWidget, int tab, QString outFileName = "", bool copy = false);

    void closeDocument(EditorTabWidget *tabWidget, int tab);

private:
    QSettings *m_settings;
    TopEditorContainer *m_topEditorContainer;
    QFileSystemWatcher *m_fsWatcher;
    bool read(QFile *file, Editor *editor, QString encoding);
    QPair<int, int> findOpenEditorByFileName(QString filename);
    bool write(QIODevice *io, Editor *editor);
    void monitorDocument(const QString &fileName);
    void unmonitorDocument(const QString &fileName);
signals:
    void fileOnDiskChanged(EditorTabWidget *tabWidget, int tab, bool removed);

public slots:

private slots:
    void documentChanged(QString fileName);
};

#endif // DOCENGINE_H
