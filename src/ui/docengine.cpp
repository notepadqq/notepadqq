#include "include/docengine.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QTextCodec>
#include "include/mainwindow.h"
#include "include/languages.h"
#include <magic.h>

DocEngine::DocEngine(QSettings *settings, TopEditorContainer *topEditorContainer, QObject *parent) :
    QObject(parent), settings(settings), topEditorContainer(topEditorContainer)
{

}



bool DocEngine::read(QIODevice *io, Editor* editor, QString encoding)
{
    if(!editor)
        return false;

    if(!io->open(QIODevice::ReadOnly))
        return false;

    QFile *file = static_cast<QFile*>(io);
    QFileInfo fi(*file);

    QString readEncodedAs = this->getFileMimeEncoding(fi.absoluteFilePath());
    QTextStream stream ( io );
    QString txt;

    stream.setCodec((encoding != "") ? encoding.toUtf8() : readEncodedAs.toUtf8());
    stream.setCodec(readEncodedAs.toUtf8());

    txt = stream.readAll();
    io->close();

    editor->sendMessage("C_CMD_SET_VALUE", txt);
    editor->sendMessage("C_CMD_MARK_CLEAN", 0);

    return true;
}

bool DocEngine::loadDocuments(QStringList fileNames, EditorTabWidget *tabWidget, bool reload)
{
    if(!fileNames.empty()) {
        this->settings->setValue("lastSelectedDir", QFileInfo(fileNames[0]).absolutePath());

        for (int i = 0; i < fileNames.count(); i++) {
            QFile file(fileNames[i]);
            QFileInfo fi(fileNames[i]);

            QPair<int, int> openPos = this->findOpenEditorByFileName(fi.absoluteFilePath());
            if(!reload) {
                if (openPos.first > -1 ) {
                    if(fileNames.count() == 1) {
                        EditorTabWidget *tabW =
                                (EditorTabWidget *)topEditorContainer->widget(openPos.first);

                        tabW->setCurrentIndex(openPos.second);
                    }
                    continue;
                }
            }

            int tabIndex = 0;
            if (reload) {
                tabWidget = (EditorTabWidget *)topEditorContainer->widget(openPos.first);
                tabIndex = openPos.second;
            } else {
                tabIndex = tabWidget->addEditorTab(true, fi.fileName());
            }

            Editor* editor = (Editor *)tabWidget->widget(tabIndex);

            if (!read(&file, editor, "UTF-8")) {
                // Manage error
                QMessageBox msgBox;
                //msgBox.setWindowTitle(mwin->windowTitle());
                msgBox.setText(tr("Error trying to open \"%1\"").arg(fi.fileName()));
                msgBox.setDetailedText(file.errorString());
                msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
                msgBox.setDefaultButton(QMessageBox::Retry);
                msgBox.setIcon(QMessageBox::Critical);
                int ret = msgBox.exec();
                if(ret == QMessageBox::Abort) {
                    tabWidget->removeTab(tabIndex);
                    break;
                } else if(ret == QMessageBox::Retry) {
                    tabWidget->removeTab(tabIndex);
                    i--;
                    continue;
                } else if(ret == QMessageBox::Ignore) {
                    tabWidget->removeTab(tabIndex);
                    continue;
                }
            }

            // If there was only a new empty tab opened, remove it
            if(tabWidget->count() == 2) {
                Editor * victim = (Editor *)tabWidget->widget(0);
                if (victim->fileName() == "" && victim->isClean()) {
                    tabWidget->removeTab(0);
                    tabIndex--;
                }
            }

            file.close();
            if(!reload) {
                editor->setFileName(fi.absoluteFilePath());
                //sci->setEolMode(sci->guessEolMode());
                tabWidget->setTabToolTip(tabIndex, fi.absoluteFilePath());
                editor->sendMessage(
                            "C_CMD_SET_LANGUAGE",
                            Languages::detectLanguage(editor->fileName()));

                //addDocument(fi.absoluteFilePath());
            } else {
                //sci->scrollCursorToCenter(pos);
            }

            editor->setFocus();
        }
    }
    return true;
}

QPair<int, int> DocEngine::findOpenEditorByFileName(QString filename)
{
    for (int i = 0; i < this->topEditorContainer->count(); i++) {
        EditorTabWidget *tabW = (EditorTabWidget *)this->topEditorContainer->widget(i);
        int id = tabW->findOpenEditorByFileName(filename);
        if (id > -1)
            return QPair<int, int>(i, id);
    }

    return QPair<int, int>(-1, -1);
}

bool DocEngine::write(QIODevice *io, Editor *editor)
{
    if(!io->open(QIODevice::WriteOnly))
        return false;

    QTextStream stream(io);

    //Support for saving in all supported formats....
    QString string = editor->sendMessageWithResult("C_FUN_GET_VALUE", 0).toString();
    QTextCodec *codec = QTextCodec::codecForName("utf8");//(sci->encoding().toUtf8()); //FIXME
    QByteArray data = codec->fromUnicode(string);

    /*if(sci->BOM())
    {
        stream.setGenerateByteOrderMark(true);
    }*/ // FIXME
    stream.setCodec(codec);

    if(io->write(data) == -1)
        return false;
    io->close();

    return true;
}

int DocEngine::saveDocument(EditorTabWidget *tabWidget, int tab, QString outFileName, bool copy)
{
    Editor *editor = tabWidget->editor(tab);
    QFile file(outFileName);
    QFileInfo fi(file);

    do
    {
        //removeDocument(sci->fileName()); //FIXME

        if (write(&file, editor)) {
            break;
        } else {
            // Handle error
            QMessageBox msgBox;
            //msgBox.setWindowTitle(MainWindow::instance()->windowTitle()); // FIXME
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
                continue;
            }
        }

    } while (1);

    // Update the file name if necessary.
    if (!copy) {
        if ((outFileName != "") && (editor->fileName() != outFileName)) {
            //removeDocument(sci->fileName());
            editor->setFileName(outFileName);
            editor->sendMessage(
                        "C_CMD_SET_LANGUAGE",
                        Languages::detectLanguage(editor->fileName()));

            tabWidget->setTabToolTip(tab, editor->fileName());
            tabWidget->setTabText(tab, fi.fileName());

        }
        editor->sendMessage("C_CMD_MARK_CLEAN", 0);
    }
    //addDocument(fileName);

    file.close();

    return MainWindow::saveFileResult_Saved;
}

QString DocEngine::getFileMimeType(QString file)
{
    return getFileInformation(file, (MAGIC_ERROR|MAGIC_MIME_TYPE));
}

QString DocEngine::getFileMimeEncoding(QString file)
{
    return getFileInformation(file, (MAGIC_ERROR|MAGIC_MIME_ENCODING));
}

QString DocEngine::getFileType(QString file)
{
    return getFileInformation(file,(MAGIC_ERROR|MAGIC_RAW));
}

QString DocEngine::getFileInformation(QString file, int flags)
{
    if((!(QFile(file).exists())) && (file == "")) return "";

    magic_t myt = magic_open(flags);
    magic_load(myt,NULL);
    QString finfo = magic_file(myt,file.toStdString().c_str());
    magic_close(myt);

    //We go a different route for checking encoding
    if ((flags & MAGIC_MIME_ENCODING)) {
        //Don't ever return a codec we don't support, will cause crashes.
        foreach(QByteArray codec, QTextCodec::availableCodecs()){
            if(codec.toUpper() == finfo.toUpper()) {
                return codec;
            }
        }

        return "UTF-8";
    } else if ((flags & MAGIC_RAW)) {
        return finfo.section(',', 0, 0);
    } else {
        return finfo;
    }
}
