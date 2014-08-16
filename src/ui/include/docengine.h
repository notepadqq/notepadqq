#ifndef DOCENGINE_H
#define DOCENGINE_H

#include <QObject>
#include <QSettings>
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
    bool loadDocuments(QStringList fileNames, EditorTabWidget *tabWidget, bool reload);

    static QString getFileMimeType(QString file);
    static QString getFileMimeEncoding(QString file);
    static QString getFileType(QString file);
    static QString getFileInformation(QString file, int flags);

    /**
     * @brief Saves a document to the file system.
     * @param tabWidget
     * @param tab
     * @param outFileName Where to save the file. If it's an empty string,
     *                    then the file name is the same as the current
     *                    file name of the document.
     * @param copy If true, do not change the file name of the document to the
     *             new path. Just save a copy.
     * @return A MainWindow::saveFileResult.
     */
    int saveDocument(EditorTabWidget *tabWidget, int tab, QString outFileName = "", bool copy = false);
private:
    QSettings *settings;
    TopEditorContainer *topEditorContainer;
    bool read(QIODevice *io, Editor *editor, QString encoding);
    QPair<int, int> findOpenEditorByFileName(QString filename);

    bool write(QIODevice *io, Editor *editor);
signals:

public slots:

};

#endif // DOCENGINE_H
