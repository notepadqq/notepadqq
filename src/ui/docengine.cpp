#include "include/docengine.h"
#include "include/notepadqq.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QTextCodec>
#include <QTextStream>
#include <QCoreApplication>
#include <QPushButton>

#include "include/iconprovider.h"
#include "include/mainwindow.h"
#include "include/nqqsettings.h"
#include "include/Sessions/persistentcache.h"

DocEngine::DocEngine(TopEditorContainer *topEditorContainer, QObject *parent) :
    QObject(parent),
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

QString DocEngine::getNewDocumentName() const
{
    static int num = 1; // FIXME maybe find a smarter way
    return tr("new %1").arg(num++);
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

int showFileSizeDialog(const QString docName, long long fileSize, bool multipleFiles) {
    QMessageBox msgBox;

    msgBox.setWindowTitle(QCoreApplication::applicationName());
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    auto buttons = QMessageBox::Yes | QMessageBox::No;
    if (multipleFiles)
        buttons |= QMessageBox::YesToAll | QMessageBox::NoToAll;
    msgBox.setStandardButtons(buttons);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);

    msgBox.setText(QObject::tr("The file \"%1\" you are trying to open is %2 MiB in size. Do you want to continue?")
                   .arg(docName)
                   .arg(QString::number(fileSize / 1024.0 / 1024.0, 'f', 2)));

    return msgBox.exec();
}

int showReloadDialog(const QString docName) {
    QMessageBox msgBox;

    msgBox.setWindowTitle(QCoreApplication::applicationName());
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    msgBox.setText("<h3>" + QObject::tr("Do you want to reload «%1»?").arg(docName) + "</h3>");
    msgBox.setInformativeText(QObject::tr("Any changes made by you to this document will be lost."));

    QPixmap img = IconProvider::fromTheme("view-refresh")
                  .pixmap(64,64)
                  .scaled(64,64,Qt::KeepAspectRatio, Qt::SmoothTransformation);
    msgBox.setIconPixmap(img);

    return msgBox.exec();
}

void DocEngine::loadDocuments(const DocEngine::DocumentLoader& docLoader)
{
    const auto& fileNames = docLoader.urls;
    const auto& rememberLastSelectedDir = docLoader.rememberLastDir;
    const auto& reloadAction = docLoader.reloadAction;
    auto* tabWidget = docLoader.tabWidget;
    const auto& codec = docLoader.textCodec;
    const auto& bom = docLoader.bom;
    auto fileSizeAction = docLoader.fileSizeAction;

    if (fileNames.empty())
        return;

    if (rememberLastSelectedDir)
        NqqSettings::getInstance().General.setLastSelectedDir(QFileInfo(fileNames[0].toLocalFile()).absolutePath());

    // Used to know if the document that we're loading is
    // the first one in the list.
    bool isFirstDocument = true;

    for (int i = 0; i < fileNames.count(); i++) {
        const QUrl& url = fileNames[i];

        if (url.isEmpty())
            continue;

        if (!url.isLocalFile()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(QCoreApplication::applicationName());
            msgBox.setText(tr("Protocol not supported for file \"%1\".").arg(url.toDisplayString()));
            msgBox.exec();
            continue;
        }

        QString localFileName = url.toLocalFile();
        QFileInfo fi(localFileName);

        const QPair<int, int> openPos = findOpenEditorByUrl(url);
        const bool isAlreadyOpen = openPos.first > -1; //'true' when we're reloading a tab

        if(isAlreadyOpen && reloadAction == ReloadActionDont) {
            EditorTabWidget *tabW = static_cast<EditorTabWidget *>
                                    (m_topEditorContainer->widget(openPos.first));

            if (isFirstDocument) {
                isFirstDocument = false;
                tabW->setCurrentIndex(openPos.second);
            }

            emit documentLoaded(tabW, openPos.second, true, rememberLastSelectedDir);
            continue;
        }

        const int warnAtSize = NqqSettings::getInstance().General.getWarnIfFileLargerThan() * 1024 * 1024;
        const auto fileSize = fi.size();

        // Only warn if warnAtSize is at least 1. Otherwise the warning is disabled.
        const bool fileTooLarge = warnAtSize > 0 && fileSize > warnAtSize;
        if (fileSizeAction!=FileSizeActionYesToAll && fileTooLarge) {
            if (fileSizeAction==FileSizeActionNoToAll)
                continue;

            int ret = showFileSizeDialog(fi.fileName(), fileSize, fileNames.size() > 1);

            switch(ret) {
            case QMessageBox::YesToAll:
                fileSizeAction = FileSizeActionYesToAll;
                break;
            case QMessageBox::Yes:
                break;
            case QMessageBox::NoToAll:
                fileSizeAction = FileSizeActionNoToAll;
                continue;
            case QMessageBox::No:
                continue;
            }
        }

        int tabIndex;
        if (isAlreadyOpen) {
            tabWidget = m_topEditorContainer->tabWidget(openPos.first);
            tabIndex = openPos.second;
        } else {
            tabIndex = tabWidget->addEditorTab(false, fi.fileName());
        }

        Editor* editor = tabWidget->editor(tabIndex);

        // In case of a reload, save cursor and scroll position
        QPair<int, int> scrollPosition;
        QPair<int, int> cursorPosition;
        if (isAlreadyOpen) {
            scrollPosition = editor->scrollPosition();
            cursorPosition = editor->cursorPosition();
        }

        if (isAlreadyOpen && reloadAction == DocEngine::ReloadActionAsk && !editor->isClean()) {
            EditorTabWidget *tabW = static_cast<EditorTabWidget *>
                                    (m_topEditorContainer->widget(openPos.first));
            tabW->setCurrentIndex(openPos.second);

            int retVal = showReloadDialog(fi.fileName());
            if (retVal == QMessageBox::Cancel)
                continue;
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
        if (isAlreadyOpen) {
            editor->setScrollPosition(scrollPosition);
            editor->setCursorPosition(cursorPosition);
        }

        if (!file.exists()) {
            // If it's a file that doesn't exists,
            // set it as if it has changed. This way, if someone
            // creates that file from outside of notepadqq,
            // when the user tries to save over it he gets a warning.
            editor->setFileOnDiskChanged(true);
            editor->markDirty();
        }

        // If there was only a new empty tab opened, remove it
        if (tabWidget->count() == 2) {
            Editor *victim = tabWidget->editor(0);
            if (victim->filePath().isEmpty() && victim->isClean()) {
                tabWidget->removeTab(0);
                tabIndex--;
            }
        }

        file.close();
        if (isAlreadyOpen) {
            editor->setFileOnDiskChanged(false);
        } else {
            editor->setFilePath(url);
            tabWidget->setTabToolTip(tabIndex, fi.absoluteFilePath());
            editor->setLanguageFromFileName();
        }

        monitorDocument(editor);

        if (isFirstDocument) {
            isFirstDocument = false;
            tabWidget->setCurrentIndex(tabIndex);
            tabWidget->editor(tabIndex)->setFocus();
        }

        if (isAlreadyOpen) {
            emit documentReloaded(tabWidget, tabIndex);
        } else {
            emit documentLoaded(tabWidget, tabIndex, false, rememberLastSelectedDir);
        }
    }
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


QString DocEngine::getAvailableSudoProgram() const
{
    QProcess p;

    p.start("which kdesu");
    p.waitForFinished(10);
    if (p.exitCode() == 0) return "kdesu";

    p.start("which gksu");
    p.waitForFinished(10);
    if (p.exitCode() == 0) return "gksu";

    return "";
}

bool DocEngine::trySudoSave(QString sudoProgram, QUrl outFileName, Editor* editor) {
    if(sudoProgram.isEmpty())
        return false;

    QString filePath = PersistentCache::createValidCacheName(
                PersistentCache::cacheDirPath(),
                outFileName.fileName() )
            .toLocalFile();

    QFile file(filePath);

    if (!write(&file, editor))
        return false;

    QProcess p;

    if (sudoProgram == "kdesu")
        p.start("kdesu", QStringList()
                << "--noignorebutton"
                << "-n"
                << "-c" << "cp" << filePath << outFileName.toLocalFile());
    else if (sudoProgram == "gksu")
        p.start("gksu", QStringList()
                << "-S" << "-m" << tr("Notepadqq asks permission to overwrite the following file:\n\n%1")
                .arg(outFileName.toLocalFile())
                << "cp" << filePath << outFileName.toLocalFile());
    else
        return false;


    p.waitForFinished(-1);
    file.remove();

    return p.exitCode() == 0;
}

int DocEngine::saveDocument(EditorTabWidget *tabWidget, int tab, QUrl outFileName, bool copy)
{
    Editor *editor = tabWidget->editor(tab);

    if (!copy)
        unmonitorDocument(editor);

    if (outFileName.isEmpty())
        outFileName = editor->filePath();

    if (outFileName.isLocalFile()) {
        QFile file(outFileName.toLocalFile());

        do
        {
            if (write(&file, editor)) {
                break;
            } else {
                static QString sudoProgram = getAvailableSudoProgram();

                // Handle error
                QMessageBox msgBox;
                msgBox.setWindowTitle(QCoreApplication::applicationName());
                msgBox.setText(tr("Error trying to write to \"%1\"").arg(file.fileName()));
                msgBox.setDetailedText(file.errorString());
                auto abort = msgBox.addButton(tr("Abort"), QMessageBox::RejectRole);
                auto retry = msgBox.addButton(tr("Retry"), QMessageBox::AcceptRole);
                auto retryRoot = sudoProgram.isEmpty() ?
                            nullptr : msgBox.addButton(tr("Retry as Root"), QMessageBox::AcceptRole);

                msgBox.exec();
                auto clicked = msgBox.clickedButton();

                if (clicked == abort) {
                   monitorDocument(editor);
                   return DocEngine::saveFileResult_Canceled;
                } else if (clicked == retry) {
                    continue;
                } else if (clicked == retryRoot) {
                    if (trySudoSave(sudoProgram, outFileName, editor))
                        break;
                    else {
                        continue;
                    }
                }
            }

        } while (1);

        // Update the file name if necessary.
        if (!copy) {
            if (editor->filePath() != outFileName) {
                editor->setFilePath(outFileName);
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

        return DocEngine::saveFileResult_Saved;

    } else {
        // FIXME ERROR
        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setText(tr("Protocol not supported for file \"%1\".").arg(outFileName.toDisplayString()));
        msgBox.exec();

        return DocEngine::saveFileResult_Canceled;
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
    monitorDocument(editor->filePath().toLocalFile());
}

void DocEngine::unmonitorDocument(Editor *editor)
{
    unmonitorDocument(editor->filePath().toLocalFile());
}

bool DocEngine::isMonitored(Editor *editor)
{
    return m_fsWatcher->files().contains(editor->filePath().toLocalFile());
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
        if (!codec)
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
        if (!codec)
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
