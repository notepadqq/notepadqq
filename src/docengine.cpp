#include "docengine.h"
#include "mainwindow.h"
#include <QFile>
#include <QIODevice>
#include <QFileInfo>
#include <QTextCodec>
#include <QMessageBox>
#include "generalfunctions.h"


docengine::docengine(QObject* parent) : QObject(parent)
{
    //We need a setting check here to see if we even need to initialize the file watcher.
    fw = new QFileSystemWatcher(this);
    connect(fw,SIGNAL(fileChanged(QString)),this,SLOT(documentChanged(QString)));
}

docengine::~docengine()
{
    delete fw;
}

void docengine::addDocument(const QString & fileName)
{
    if(fw&&fileName != ""&&(!fw->files().contains(fileName))){
        fw->addPath(fileName);
    }
}

void docengine::removeDocument(QString fileName)
{
    if(fw && fileName != "") {
        fw->removePath(fileName);
    }
}

bool docengine::write(QIODevice *io, QsciScintillaqq* sci)
{
    if(!io->open(QIODevice::WriteOnly))
        return false;
    QTextStream stream(io);
    //Support for saving in all supported formats....
    int   _docLength = sci->length();
    char *_docBuffer = (char*)sci->SendScintilla(QsciScintilla::SCI_GETCHARACTERPOINTER);
    QTextCodec *codec = QTextCodec::codecForName(sci->encoding.toUtf8());
    QByteArray string = codec->fromUnicode(QString::fromUtf8(_docBuffer,_docLength));

    if(sci->BOM)
    {
        stream.setGenerateByteOrderMark(true);
    }
    stream.setCodec(codec);

    if(io->write(string) == -1)
        return false;
    io->close();

    return true;
}

int docengine::saveDocument(QsciScintillaqq *sci, QString fileName, bool copy)
{
    QTabWidgetqq* tabWidget = sci->getTabWidget();
    QFile file(fileName);
    QFileInfo fi(file);
    bool retry = true;
    do {
        retry = false;
        removeDocument(sci->fileName());

        if(!write(&file,sci)) {
            // Manage error
            QMessageBox msgBox;
            msgBox.setWindowTitle(MainWindow::instance()->windowTitle());
            msgBox.setText(tr("Error trying to write to \"%1\"").arg(file.fileName()));
            msgBox.setDetailedText(file.errorString());
            msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry);
            msgBox.setDefaultButton(QMessageBox::Retry);
            msgBox.setIcon(QMessageBox::Critical);
            int ret = msgBox.exec();
            if(ret == QMessageBox::Abort) {
                return MainWindow::saveFileResult_Canceled;
                break;
            } else if(ret == QMessageBox::Retry) {
                retry = true;
                continue;
            }
        }

    //Update the file name if necessary.
        if(!copy){
            if((fileName != "") && (sci->fileName() != fileName)) {
                removeDocument(sci->fileName());
                sci->setFileName(fileName);
                sci->autoSyntaxHighlight();
                tabWidget->setTabToolTip(sci->getTabIndex(), sci->fileName());
                tabWidget->setTabText(sci->getTabIndex(), fi.fileName());

            }
            sci->setModified(false);
        }
        addDocument(fileName);

        file.close();

    }while(retry);

    return MainWindow::saveFileResult_Saved;
}

bool docengine::read(QIODevice *io, QsciScintillaqq* sci, QString encoding)
{
    if( !sci )                          return false;
    if(!io->open(QIODevice::ReadOnly))  return false;
    QFileInfo fi(static_cast<QFile>(io));
    QString readEncodedAs = generalFunctions::getFileEncoding(fi.absoluteFilePath());

    QTextStream stream ( io );
    QString txt;

    stream.setCodec((encoding != "") ? encoding.toUtf8() : readEncodedAs.toUtf8());

    txt = stream.readAll();
    io->close();

    sci->clear();
    sci->append(txt);

    return true;
}

bool docengine::loadDocuments(QStringList fileNames, QTabWidgetqq *tabWidget, bool reload)
{
    MainWindow* mwin = MainWindow::instance();
    if(!fileNames.isEmpty())
    {
        mwin->getSettings()->setValue("lastSelectedDir", QFileInfo(fileNames[0]).absolutePath());

        // Ok, now open our files
        for (int i = 0; i < fileNames.count(); i++) {

            QFile file(fileNames[i]);
            QFileInfo fi(fileNames[i]);

            int x = isDocumentOpen(fi.absoluteFilePath());
            if(!reload){
                if (x > -1 ) {
                    if(fileNames.count() == 1){
                        tabWidget->setCurrentIndex(x);
                    }
                    continue;
                }
            }
            int index = 0;
            if(reload){
                index = x;
            }else {
                index = tabWidget->addEditorTab(true, fi.fileName());
            }
            QsciScintillaqq* sci = tabWidget->QSciScintillaqqAt(index);

            sci->encoding = generalFunctions::getFileEncoding(fi.absoluteFilePath());

            if (!read(&file, sci)) {
                // Manage error
                QMessageBox msgBox;
                msgBox.setWindowTitle(mwin->windowTitle());
                msgBox.setText(tr("Error trying to open \"%1\"").arg(fi.fileName()));
                msgBox.setDetailedText(file.errorString());
                msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
                msgBox.setDefaultButton(QMessageBox::Retry);
                msgBox.setIcon(QMessageBox::Critical);
                int ret = msgBox.exec();
                if(ret == QMessageBox::Abort) {
                    tabWidget->removeTab(index);
                    break;
                } else if(ret == QMessageBox::Retry) {
                    tabWidget->removeTab(index);
                    i--;
                    continue;
                } else if(ret == QMessageBox::Ignore) {
                    tabWidget->removeTab(index);
                    continue;
                }
            }

            // If there was only a new empty tab opened, remove it
            if(tabWidget->count() == 2 && tabWidget->QSciScintillaqqAt(0)->isNewEmptyDocument()) {
                tabWidget->removeTab(0);
                index--;
            }

            sci->setFileName(fi.absoluteFilePath());
            sci->setEolMode(sci->guessEolMode());
            sci->setModified(false);
            tabWidget->setTabToolTip(index, sci->fileName());
            sci->autoSyntaxHighlight();
            addDocument(fi.absoluteFilePath());

            // updateGui(index, tabWidget1);

            file.close();

            sci->setFocus(Qt::OtherFocusReason);
            //tabWidget1->setFocus();
            //tabWidget1->currentWidget()->setFocus();
        }
    }
    return true;
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

void docengine::documentChanged(QString fileName)
{
    removeDocument(fileName);
    QTabWidgetqq *tabWidget = MainWindow::instance()->container->focusQTabWidgetqq();
    int x = isDocumentOpen(fileName);

    //Don't bother continuing if we can't find the document in one of the open tabs.... though this should never happen
    if(x == -1) {
        removeDocument(fileName);
        return;
    }

    QFile file(fileName);
    bool  fileExisted = file.exists();

    QMessageBox msgBox;
    msgBox.setWindowTitle(MainWindow::instance()->windowTitle());
    msgBox.setIcon(QMessageBox::Critical);
    if(fileExisted) {
        msgBox.setText(tr("The file \"%1\" has been changed outside of the editor.  Would you like to reload it?").arg(fileName));
    }else {
        msgBox.setText(tr("The file \"%1\" has been removed from the file system.  Would you like to save it now?").arg(fileName));
    }
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Yes);

    int ret = msgBox.exec();
    if(ret == QMessageBox::No) {
        return;
    }else if(ret == QMessageBox::Close) {

        if(x != -1) {
            MainWindow::instance()->kindlyTabClose(tabWidget->QSciScintillaqqAt(x));
        }
        return;
    }

    //If the file exists, we try to reload it, otherwise save it back to disk.
    if(fileExisted) {
        loadDocuments(QStringList(fileName),MainWindow::instance()->container->focusQTabWidgetqq(),true);
    }else {
        saveDocument(tabWidget->QSciScintillaqqAt(x),fileName);
    }
    addDocument(fileName);
}

int docengine::isDocumentOpen(const QString & filePath)
{
    QTabWidgetqq* tabWidget = MainWindow::instance()->container->focusQTabWidgetqq();
    // visit all QScintilla instance to check if "filepath" is already opened
    for ( int i = 0; i < tabWidget->count(); ++i ) {
        QsciScintillaqq* sci = tabWidget->QSciScintillaqqAt(i);
        if ( sci && sci->fileName() == filePath ) {
            return i;
        }
    }
    return -1;
}


//These will be used later when we implement preferences.
void docengine::setFileWatcherEnabled(bool yes)
{
    if(yes) {
        if(!fw) {
            fw = new QFileSystemWatcher(this);
            connect(fw,SIGNAL(fileChanged(QString)),this,SLOT(documentChanged(QString)));
        }
    }else {
        delete fw;
        fw = 0;
    }
}

bool docengine::isFileWatcherEnabled()
{
    if(fw){
        return true;
    }
    return false;
}
