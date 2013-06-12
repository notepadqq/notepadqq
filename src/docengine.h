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

    bool saveDocument(QsciScintillaqq* sci,QString fileName="");
    bool loadDocument(QsciScintillaqq* sci,QString fileName="");
    void verifyDocuments();

private:
    QFileSystemWatcher* fw;
    void addDocument(const QString & fileName);
    void removeDocument(QString fileName);


private slots:
    void reloadDocument(QsciScintillaqq* sci, QString fileName);
    void fileChanged(QString fileName);
    bool errorSaveDocument(QFile *file);

};

#endif // DOCENGINE_H
