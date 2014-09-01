#include "include/docengine.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QTextCodec>
#include <QCoreApplication>
#include "include/mainwindow.h"
#include "include/languages.h"
#include <magic.h>

DocEngine::DocEngine(QSettings *settings, TopEditorContainer *topEditorContainer, QObject *parent) :
    QObject(parent),
    m_settings(settings),
    m_topEditorContainer(topEditorContainer),
    m_fsWatcher(new QFileSystemWatcher(this))
{
    connect(m_fsWatcher, &QFileSystemWatcher::fileChanged, this, &DocEngine::documentChanged);
}

DocEngine::~DocEngine()
{
    delete m_fsWatcher;
}

bool DocEngine::read(QFile *file, Editor* editor, QString encoding)
{
    if(!editor)
        return false;

    if(!file->open(QFile::ReadOnly))
        return false;

    QFileInfo fi(*file);

    QString readEncodedAs = getFileMimeEncoding(fi.absoluteFilePath());
    QTextStream stream(file);
    QString txt;

    stream.setCodec((encoding != "") ? encoding.toUtf8() : readEncodedAs.toUtf8());
    stream.setCodec(readEncodedAs.toUtf8());

    txt = stream.readAll();
    file->close();

    editor->sendMessage("C_CMD_SET_VALUE", txt);
    editor->sendMessage("C_CMD_MARK_CLEAN");

    return true;
}

bool DocEngine::loadDocuments(QStringList fileNames, EditorTabWidget *tabWidget, bool reload)
{
    if(!fileNames.empty()) {
        m_settings->setValue("lastSelectedDir", QFileInfo(fileNames[0]).absolutePath());

        for (int i = 0; i < fileNames.count(); i++) {
            QFile file(fileNames[i]);
            QFileInfo fi(fileNames[i]);

            QPair<int, int> openPos = findOpenEditorByFileName(fi.absoluteFilePath());
            if(!reload) {
                if (openPos.first > -1 ) {
                    if(fileNames.count() == 1) {
                        EditorTabWidget *tabW =
                                (EditorTabWidget *)m_topEditorContainer->widget(openPos.first);

                        tabW->setCurrentIndex(openPos.second);
                    }
                    continue;
                }
            }

            int tabIndex = 0;
            if (reload) {
                tabWidget = (EditorTabWidget *)m_topEditorContainer->widget(openPos.first);
                tabIndex = openPos.second;
            } else {
                tabIndex = tabWidget->addEditorTab(true, fi.fileName());
            }

            Editor* editor = (Editor *)tabWidget->widget(tabIndex);

            if (!read(&file, editor, "UTF-8")) {
                // Manage error
                QMessageBox msgBox;
                msgBox.setWindowTitle(QCoreApplication::applicationName());
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
            if (tabWidget->count() == 2) {
                Editor * victim = (Editor *)tabWidget->widget(0);
                if (victim->fileName() == "" && victim->isClean()) {
                    tabWidget->removeTab(0);
                    tabIndex--;
                }
            }

            file.close();
            if (!reload) {
                editor->setFileName(fi.absoluteFilePath());
                //sci->setEolMode(sci->guessEolMode());
                tabWidget->setTabToolTip(tabIndex, fi.absoluteFilePath());
                editor->setLanguage(Languages::detectLanguage(editor->fileName()));
            } else {
                //sci->scrollCursorToCenter(pos);
                editor->setFileOnDiskChanged(false);
            }

            monitorDocument(editor->fileName());

            editor->setFocus();
        }
    }
    return true;
}

QPair<int, int> DocEngine::findOpenEditorByFileName(QString filename)
{
    for (int i = 0; i < m_topEditorContainer->count(); i++) {
        EditorTabWidget *tabW = (EditorTabWidget *)m_topEditorContainer->widget(i);
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
    QString string = editor->value();
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

void DocEngine::monitorDocument(const QString &fileName)
{
    if(m_fsWatcher &&
            fileName != "" &&
            !m_fsWatcher->files().contains(fileName)) {

        m_fsWatcher->addPath(fileName);
    }
}

void DocEngine::unmonitorDocument(const QString &fileName)
{
    if(m_fsWatcher && fileName != "") {
        m_fsWatcher->removePath(fileName);
    }
}

int DocEngine::saveDocument(EditorTabWidget *tabWidget, int tab, QString outFileName, bool copy)
{
    Editor *editor = tabWidget->editor(tab);

    if (!copy)
        unmonitorDocument(editor->fileName());

    if (outFileName.isEmpty() || outFileName.isNull())
        outFileName = editor->fileName();

    QFile file(outFileName);
    QFileInfo fi(file);

    do
    {
        if (write(&file, editor)) {
            break;
        } else {
            // Handle error
            QMessageBox msgBox;
            msgBox.setWindowTitle(QCoreApplication::applicationName());
            msgBox.setText(tr("Error trying to write to \"%1\"").arg(file.fileName()));
            msgBox.setDetailedText(file.errorString());
            msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry);
            msgBox.setDefaultButton(QMessageBox::Retry);
            msgBox.setIcon(QMessageBox::Critical);
            int ret = msgBox.exec();
            if(ret == QMessageBox::Abort) {
                monitorDocument(editor->fileName());
                return MainWindow::saveFileResult_Canceled;
            } else if(ret == QMessageBox::Retry) {
                continue;
            }
        }

    } while (1);

    // Update the file name if necessary.
    if (!copy) {
        if (editor->fileName() != outFileName) {
            editor->setFileName(outFileName);
            editor->setLanguage(Languages::detectLanguage(editor->fileName()));

            tabWidget->setTabToolTip(tab, editor->fileName());
            tabWidget->setTabText(tab, fi.fileName());

        }
        editor->sendMessage("C_CMD_MARK_CLEAN", 0);
        editor->setFileOnDiskChanged(false);
    }

    file.close();

    monitorDocument(editor->fileName());

    return MainWindow::saveFileResult_Saved;
}

void DocEngine::documentChanged(QString fileName)
{
    unmonitorDocument(fileName);

    QPair<int, int> pos = findOpenEditorByFileName(fileName);
    if (pos.first != -1) {
        QFile file(fileName);
        EditorTabWidget *tabWidget = m_topEditorContainer->tabWidget(pos.first);

        // FIXME Set editor as dirty
        tabWidget->editor(pos.second)->setFileOnDiskChanged(true);
        emit fileOnDiskChanged(tabWidget, pos.second, !file.exists());
    }
}

void DocEngine::closeDocument(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    unmonitorDocument(editor->fileName());
    tabWidget->removeTab(tab);
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

    // We go a different route for checking encoding
    if ((flags & MAGIC_MIME_ENCODING)) {
        // Don't ever return a codec we don't support, will cause crashes.
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
