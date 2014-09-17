#include "include/docengine.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QTextCodec>
#include <QCoreApplication>
#include "include/mainwindow.h"
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

int DocEngine::addNewDocument(QString name, bool setFocus, EditorTabWidget *tabWidget)
{
    int tab = tabWidget->addEditorTab(setFocus, name);
    tabWidget->editor(tab)->setLanguage("plaintext");
    return tab;
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
    editor->sendMessage("C_CMD_CLEAR_HISTORY");
    editor->sendMessage("C_CMD_MARK_CLEAN");

    return true;
}

bool DocEngine::loadDocuments(const QList<QUrl> &fileNames, EditorTabWidget *tabWidget)
{
    return loadDocuments(fileNames, tabWidget, false);
}

bool DocEngine::loadDocument(const QUrl &fileName, EditorTabWidget *tabWidget)
{
    QList<QUrl> files;
    files.append(fileName);
    return loadDocuments(files, tabWidget);
}

bool DocEngine::reloadDocument(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    QList<QUrl> files;
    files.append(editor->fileName());
    return loadDocuments(files, tabWidget, true);
}

bool DocEngine::loadDocuments(const QList<QUrl> &fileNames, EditorTabWidget *tabWidget, const bool reload)
{
    if(!fileNames.empty()) {
        m_settings->setValue("lastSelectedDir", QFileInfo(fileNames[0].toLocalFile()).absolutePath());

        // Used to know if the document that we're loading is
        // the first one in the list.
        bool isFirstDocument = true;

        for (int i = 0; i < fileNames.count(); i++)
        {
            if (fileNames[i].isLocalFile()) {
                QString localFileName = fileNames[i].toLocalFile();
                QFileInfo fi(localFileName);

                QPair<int, int> openPos = findOpenEditorByUrl(fileNames[i]);
                if(!reload) {
                    if (openPos.first > -1 ) {
                        EditorTabWidget *tabW = static_cast<EditorTabWidget *>
                                (m_topEditorContainer->widget(openPos.first));

                        if (isFirstDocument) {
                            isFirstDocument = false;
                            tabW->setCurrentIndex(openPos.second);
                        }

                        emit documentLoaded(tabW, openPos.second, true);
                        continue;
                    }
                }

                int tabIndex;
                if (reload) {
                    tabWidget = m_topEditorContainer->tabWidget(openPos.first);
                    tabIndex = openPos.second;
                } else {
                    tabIndex = tabWidget->addEditorTab(false, fi.fileName());
                }

                Editor* editor = tabWidget->editor(tabIndex);

                // In case of a reload, save cursor and scroll position
                QPair<int, int> scrollPosition;
                QPair<int, int> cursorPosition;
                if (reload) {
                    scrollPosition = editor->scrollPosition();
                    cursorPosition = editor->cursorPosition();
                }

                QFile file(localFileName);
                if (file.exists()) {
                    if (!read(&file, editor, "UTF-8")) {
                        // Handle error
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
                }

                // In case of reload, restore cursor and scroll position
                if (reload) {
                    editor->setScrollPosition(scrollPosition);
                    editor->setCursorPosition(cursorPosition);
                }

                if (!file.exists()) {
                    // If it's a file that doesn't exists,
                    // set it as if it has changed. This way, if someone
                    // creates that file from outside of notepadqq,
                    // when the user tries to save over it he gets a warning.
                    editor->setFileOnDiskChanged(true);
                }

                // If there was only a new empty tab opened, remove it
                if (tabWidget->count() == 2) {
                    Editor *victim = tabWidget->editor(0);
                    if (victim->fileName().isEmpty() && victim->isClean()) {
                        tabWidget->removeTab(0);
                        tabIndex--;
                    }
                }

                file.close();
                if (!reload) {
                    editor->setFileName(fileNames[i]);
                    //sci->setEolMode(sci->guessEolMode());
                    tabWidget->setTabToolTip(tabIndex, fi.absoluteFilePath());
                    editor->setLanguageFromFileName();
                } else {
                    //sci->scrollCursorToCenter(pos);
                    editor->setFileOnDiskChanged(false);
                }

                monitorDocument(editor);

                if (isFirstDocument) {
                    isFirstDocument = false;
                    tabWidget->setCurrentIndex(tabIndex);
                    tabWidget->editor(tabIndex)->setFocus();
                }

                if (reload) {
                    emit documentReloaded(tabWidget, tabIndex);
                } else {
                    emit documentLoaded(tabWidget, tabIndex, false);
                }

            } else if (fileNames[i].isEmpty()) {
                // Do nothing

            } else {
                // TODO Better looking msgbox
                QMessageBox msgBox;
                msgBox.setWindowTitle(QCoreApplication::applicationName());
                msgBox.setText(tr("Protocol not supported for file \"%1\".").arg(fileNames[i].toDisplayString()));
                msgBox.exec();
            }
        }
    }

    return true;
}

QPair<int, int> DocEngine::findOpenEditorByUrl(QUrl filename)
{
    for (int i = 0; i < m_topEditorContainer->count(); i++) {
        EditorTabWidget *tabW = m_topEditorContainer->tabWidget(i);
        int id = tabW->findOpenEditorByUrl(filename);
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
            !fileName.isEmpty() &&
            !m_fsWatcher->files().contains(fileName)) {

        m_fsWatcher->addPath(fileName);
    }
}

void DocEngine::unmonitorDocument(const QString &fileName)
{
    if(m_fsWatcher && !fileName.isEmpty()) {
        m_fsWatcher->removePath(fileName);
    }
}

int DocEngine::saveDocument(EditorTabWidget *tabWidget, int tab, QUrl outFileName, bool copy)
{
    Editor *editor = tabWidget->editor(tab);

    if (!copy)
        unmonitorDocument(editor);

    if (outFileName.isEmpty())
        outFileName = editor->fileName();

    if (outFileName.isLocalFile()) {
        QFile file(outFileName.toLocalFile());

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
                    monitorDocument(editor);
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
                editor->setLanguageFromFileName();
            }
            editor->sendMessage("C_CMD_MARK_CLEAN", 0);
            editor->setFileOnDiskChanged(false);
        }

        file.close();

        monitorDocument(editor);

        if (!copy) {
            emit documentSaved(tabWidget, tab);
        }

        return MainWindow::saveFileResult_Saved;

    } else {
        // FIXME ERROR
        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setText(tr("Protocol not supported for file \"%1\".").arg(outFileName.toDisplayString()));
        msgBox.exec();

        return MainWindow::saveFileResult_Canceled;
    }
}

void DocEngine::documentChanged(QString fileName)
{
    unmonitorDocument(fileName);

    QPair<int, int> pos = findOpenEditorByUrl(QUrl::fromLocalFile(fileName));
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
    unmonitorDocument(editor);
    tabWidget->removeTab(tab);
}

void DocEngine::monitorDocument(Editor *editor)
{
    monitorDocument(editor->fileName().toLocalFile());
}

void DocEngine::unmonitorDocument(Editor *editor)
{
    unmonitorDocument(editor->fileName().toLocalFile());
}

bool DocEngine::isMonitored(Editor *editor)
{
    return m_fsWatcher->files().contains(editor->fileName().toLocalFile());
}

QPair<QString, QTextCodec *> DocEngine::decodeText(QByteArray contents)
{
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    const QString text = codec->toUnicode(contents.constData(), contents.size(), &state);
    if (state.invalidChars > 0) {
        qDebug() << "Not a valid UTF-8 sequence.";
    }

    return QPair<QString, QTextCodec *>(text, codec);
}

QString DocEngine::getFileMimeEncoding(const QString &file)
{
    return getFileInformation(file, (MAGIC_ERROR|MAGIC_MIME_ENCODING));
}

QString DocEngine::getFileInformation(const QString &file, const int flags)
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
