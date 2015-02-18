#include "include/docengine.h"
#include "include/notepadqq.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QTextCodec>
#include <QCoreApplication>
#include "include/mainwindow.h"

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

DocEngine::DecodedText DocEngine::readToString(QFile *file)
{
    return readToString(file, nullptr, false);
}

DocEngine::DecodedText DocEngine::readToString(QFile *file, QTextCodec *codec, bool bom)
{
    DecodedText decoded;

    if(!file->open(QFile::ReadOnly)) {
        decoded.error = true;
        return decoded;
    }

    if (codec == nullptr) {
        decoded = decodeText(file->readAll());
    } else {
        decoded = decodeText(file->readAll(), codec, bom);
    }

    file->close();

    return decoded;
}

bool DocEngine::read(QFile *file, Editor *editor)
{
    return read(file, editor, nullptr, false);
}

bool DocEngine::read(QFile *file, Editor* editor, QTextCodec *codec, bool bom)
{
    if(!editor)
        return false;

    DecodedText decoded = readToString(file, codec, bom);

    if (decoded.error)
        return false;

    editor->setCodec(decoded.codec);
    editor->setBom(decoded.bom);

    if (decoded.text.indexOf("\r\n") != -1)
        editor->setEndOfLineSequence("\r\n");
    else if (decoded.text.indexOf("\n") != -1)
        editor->setEndOfLineSequence("\n");
    else if (decoded.text.indexOf("\r") != -1)
        editor->setEndOfLineSequence("\r");

    editor->setValue(decoded.text);
    editor->sendMessage("C_CMD_CLEAR_HISTORY");
    editor->markClean();

    return true;
}

bool DocEngine::loadDocuments(const QList<QUrl> &fileNames, EditorTabWidget *tabWidget)
{
    return loadDocuments(fileNames, tabWidget, false, nullptr, false);
}

bool DocEngine::loadDocument(const QUrl &fileName, EditorTabWidget *tabWidget)
{
    QList<QUrl> files;
    files.append(fileName);
    return loadDocuments(files, tabWidget);
}

bool DocEngine::reloadDocument(EditorTabWidget *tabWidget, int tab)
{
    return reloadDocument(tabWidget, tab, nullptr, false);
}

bool DocEngine::reloadDocument(EditorTabWidget *tabWidget, int tab, QTextCodec *codec, bool bom)
{
    Editor *editor = tabWidget->editor(tab);
    QList<QUrl> files;
    files.append(editor->fileName());
    return loadDocuments(files, tabWidget, true, codec, bom);
}

bool DocEngine::loadDocuments(const QList<QUrl> &fileNames, EditorTabWidget *tabWidget, const bool reload, QTextCodec *codec, bool bom)
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
                    if (!read(&file, editor, codec, bom)) {
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

QPair<int, int> DocEngine::findOpenEditorByUrl(const QUrl &filename) const
{
    for (int i = 0; i < m_topEditorContainer->count(); i++) {
        EditorTabWidget *tabW = m_topEditorContainer->tabWidget(i);
        int id = tabW->findOpenEditorByUrl(filename);
        if (id > -1)
            return QPair<int, int>(i, id);
    }

    return QPair<int, int>(-1, -1);
}

QByteArray DocEngine::getBomForCodec(QTextCodec *codec)
{
    QByteArray bom;
    int tmpSize;
    int aSize; // Size of the "a" character

    QTextStream stream(&bom);
    stream.setCodec(codec);
    stream.setGenerateByteOrderMark(true);

    // Write an 'a' so that the BOM gets written.
    stream << "a";
    stream.flush();
    tmpSize = bom.size();

    // Write another 'a' so that we can see how much
    // the byte array grows and then get the size of an 'a'
    stream << "a";
    stream.flush();

    // Get the size of the 'a' character
    aSize = bom.size() - tmpSize;

    // Resize the byte array to remove the two 'a' chars
    bom.resize(bom.size() - 2 * aSize);

    return bom;
}

bool DocEngine::writeFromString(QIODevice *io, const DecodedText &write)
{
    if (!io->open(QIODevice::WriteOnly))
        return false;

    QByteArray data = write.codec->fromUnicode(write.text);

    // Some codecs always put the BOM (e.g. UTF-16BE).
    // Others don't (e.g. UTF-8) so we have to manually
    // write it, if the BOM is required.
    QByteArray manualBom;
    if (write.bom) {
        // We can't write the BOM using QTextStream.setGenerateByteOrderMark(),
        // because we would need to open the QIODevice as Text (QIODevice::Text),
        // but if we do, QTextStream will replace any newline character with
        // the OS representation (and we want to be free to use *whatever*
        // line ending we want).
        // So we generate the BOM here, and then
        // we prepend it to the output of our QIODevice.

        if (write.codec->mibEnum() == MIB_UTF_8) { // UTF-8
            manualBom = getBomForCodec(write.codec);
        }
    }

    if (!manualBom.isEmpty() && io->write(manualBom) == -1) {
        io->close();
        return false;
    }

    if (io->write(data) == -1) {
        io->close();
        return false;
    }

    io->close();

    return true;
}

bool DocEngine::write(QIODevice *io, Editor *editor)
{
    DecodedText info;
    info.text = editor->value()
            .replace("\n", editor->endOfLineSequence());

    info.codec = editor->codec();
    info.bom = editor->bom();

    return writeFromString(io, info);
}

void DocEngine::reinterpretEncoding(Editor *editor, QTextCodec *codec, bool bom)
{
    QPair<int, int> scrollPosition = editor->scrollPosition();
    QPair<int, int> cursorPosition = editor->cursorPosition();

    QTextCodec *oldCodec = editor->codec();
    QByteArray data = oldCodec->fromUnicode(editor->value());
    editor->setValue(codec->toUnicode(data));
    editor->setCodec(codec);
    editor->setBom(bom);

    editor->setScrollPosition(scrollPosition);
    editor->setCursorPosition(cursorPosition);
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
            editor->markClean();
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

        Editor *editor = tabWidget->editor(pos.second);
        editor->markDirty();
        editor->setFileOnDiskChanged(true);
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

DocEngine::DecodedText DocEngine::decodeText(const QByteArray &contents)
{
    // Search for a BOM mark
    QTextCodec *bomCodec = QTextCodec::codecForUtfText(contents, nullptr);
    if (bomCodec != nullptr) {
        return decodeText(contents, bomCodec, true);
    }


    // FIXME Could potentially be slow on large files!!
    //       We should try checking only the first few KB.

    int bestInvalidChars = -1;
    DecodedText bestDecodedText;

    QList<int> alreadyTriedMibs;

    // First try with these known codecs, in order.
    // The first one without invalid characters is good.
    QList<QByteArray> codecStrings = QList<QByteArray>
            ({"UTF-8", "ISO-8859-1", "Windows-1251",
              "Shift-JIS", "Windows-1252",
              QTextCodec::codecForLocale()->name() });

    for (QByteArray codecString : codecStrings) {
        QTextCodec::ConverterState state;
        QTextCodec *codec = QTextCodec::codecForName(codecString);
        if (codec == 0)
            continue;

        const QString text = codec->toUnicode(contents.constData(), contents.size(), &state);

        if (state.invalidChars == 0) {
            bestDecodedText.codec = codec;
            bestDecodedText.text = text;
            bestDecodedText.bom = false;

            return bestDecodedText;

        } else {
            alreadyTriedMibs.append(codec->mibEnum());

            if (bestInvalidChars == -1 || state.invalidChars < bestInvalidChars) {
                bestInvalidChars = state.invalidChars;
                bestDecodedText.codec = codec;
                bestDecodedText.text = text;
                bestDecodedText.bom = false;
            }
        }
    }

    // If we're here, none of the codecs in codecStrings worked
    // (and variables bestCodec & co. *are* set).
    // We try the other codecs hoping to find the best one.
    QList<int> mibs = QTextCodec::availableMibs();
    for (int mib : mibs) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);
        if (codec == 0)
            continue;

        if (alreadyTriedMibs.contains(codec->mibEnum()))
            continue;

        QTextCodec::ConverterState state;
        const QString text = codec->toUnicode(contents.constData(), contents.size(), &state);

        if (state.invalidChars < bestInvalidChars) {
            bestInvalidChars = state.invalidChars;
            bestDecodedText.codec = codec;
            bestDecodedText.text = text;
            bestDecodedText.bom = false;
        }
    }

    return bestDecodedText;
}

DocEngine::DecodedText DocEngine::decodeText(const QByteArray &contents, QTextCodec *codec, bool contentHasBOM)
{
    QTextCodec::ConverterState state;
    const QString text = codec->toUnicode(contents.constData(), contents.size(), &state);

    DecodedText ret;
    ret.bom = contentHasBOM;
    ret.codec = codec;
    ret.text = text;

    return ret;
}
