#include "docengine.h"
#include "mainwindow.h"
#include <QFile>
#include <QIODevice>
#include <QFileInfo>
#include <QTextCodec>
#include <QMessageBox>
docengine::docengine(QObject* parent) : QObject(parent)
{
    fw = new QFileSystemWatcher(this);
    connect(fw,SIGNAL(fileChanged(QString)),this,SLOT(fileChanged(QString)));
}

docengine::~docengine()
{
    delete fw;
}

void docengine::addDocument(const QString & fileName)
{
    if(fw&&fileName != ""){
        fw->addPath(fileName);
    }
}

void docengine::removeDocument(QString fileName)
{
    if(fw && fileName != "") {
        fw->removePath(fileName);
    }
}

bool docengine::saveDocument(QsciScintillaqq *sci, QString fileName)
{
    QTabWidgetqq* tabWidget = sci->getTabWidget();
    QFile file(fileName);

    removeDocument(fileName);
    while(!file.open(QIODevice::WriteOnly)){
        if(!errorSaveDocument(&file)) {
            return false;
        }
    }

    QTextCodec *codec = QTextCodec::codecForName(sci->encoding.toUtf8());
    QString textToSave = sci->text();
    if(sci->BOM) {
        textToSave = QChar(QChar::ByteOrderMark) + textToSave;
    }
    QByteArray string = codec->fromUnicode(textToSave);

    while(file.write(string) == -1){
        if(!errorSaveDocument(&file)) {
            return false;
        }
    }


    file.close();

     addDocument(fileName);

    //Update the file name if necessary.
    if((fileName != "") && (sci->fileName() != fileName)) {
        removeDocument(sci->fileName());
        addDocument(fileName);
        sci->setFileName(fileName);
        sci->autoSyntaxHighlight();
        tabWidget->setTabToolTip(sci->getTabIndex(), sci->fileName());
        tabWidget->setTabText(sci->getTabIndex(), sci->baseName());

    }

    sci->setModified(false);

    return true;
}

bool docengine::loadDocument(QsciScintillaqq *sci, QString fileName)
{

}

bool docengine::errorSaveDocument(QFile *file)
{
    QFileInfo fi(*file);
    QMessageBox msgBox;
    msgBox.setWindowTitle(MainWindow::instance()->windowTitle());
    msgBox.setText(tr("Error trying to write to \"%1\"").arg(fi.fileName()));
    msgBox.setDetailedText(file->errorString());
    msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry);
    msgBox.setDefaultButton(QMessageBox::Retry);
    msgBox.setIcon(QMessageBox::Critical);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Abort) {
        return false;
    }
    return true;
}

void docengine::fileChanged(QString fileName)
{
    QMessageBox mbx;
    mbx.setText(tr("File '%1' has been changed.").arg(fileName));
    mbx.exec();
}

void docengine::reloadDocument(QsciScintillaqq* sci, QString fileName)
{

}
