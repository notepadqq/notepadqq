#ifndef DOCENGINE_H
#define DOCENGINE_H

#include <QFileSystemWatcher>
#include <QFile>
#include <QObject>
#include "qsciscintillaqq.h"
class docengine : public QObject
{
    Q_OBJECT
public:
    explicit docengine(QObject *parent = 0);
    ~docengine();

    int saveDocument(QsciScintillaqq* sci,QString fileName="",bool copy=false);
    bool loadDocuments(QStringList fileNames, QTabWidgetqq *tabWidget,bool reload=false);
    void addDocument(const QString & fileName);
    void removeDocument(QString fileName);
    int  isDocumentOpen(const QString & filePath);
    //This one is going to be used for close all but X tab.
    void verifyDocuments();

    void setFileWatcherEnabled(bool yes);
    bool isFileWatcherEnabled();

private:
    QFileSystemWatcher* fw;



private slots:
    void documentChanged(QString fileName);
    bool errorSaveDocument(QFile *file);

};

#endif // DOCENGINE_H
