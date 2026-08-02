#include "include/documentcontroller.h"

#include "include/EditorNS/bannerfilechanged.h"
#include "include/EditorNS/bannerfileremoved.h"
#include "include/EditorNS/defer.h"
#include "include/EditorNS/editor.h"
#include "include/Sessions/backupservice.h"
#include "include/Sessions/persistentcache.h"
#include "include/Sessions/sessions.h"
#include "include/commandlineopenruntime.h"
#include "include/docengine.h"
#include "include/editortabwidget.h"
#include "include/iconprovider.h"
#include "include/mainwindow.h"
#include "include/notepadqq.h"
#include "include/nqqsettings.h"
#include "include/topeditorcontainer.h"
#include "ui_mainwindow.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QPointer>

DocumentController::DocumentController(
    MainWindow& window, DocEngine& docEngine, TopEditorContainer& editorContainer, NqqSettings& settings)
    : QObject(&window)
    , m_window(window)
    , m_docEngine(docEngine)
    , m_editorContainer(editorContainer)
    , m_settings(settings)
{
    connect(&m_docEngine, &DocEngine::fileOnDiskChanged, this, &DocumentController::fileOnDiskChanged);
    connect(&m_docEngine, &DocEngine::documentSaved, this, &DocumentController::documentSaved);
    connect(&m_docEngine, &DocEngine::documentReloaded, this, &DocumentController::documentReloaded);
    connect(&m_docEngine, &DocEngine::documentLoaded, this, &DocumentController::documentLoaded);
}

QUrl DocumentController::stringToUrl(const QString& fileName, const QString& workingDirectory) const
{
    const QString baseDirectory = workingDirectory.isEmpty() ? m_window.m_workingDirectory : workingDirectory;
    const QUrl url(fileName);
    if (!url.isRelative())
        return url;

    const QFileInfo fileInfo(fileName);
    if (!fileInfo.isRelative())
        return QUrl::fromLocalFile(fileName);

    return QUrl::fromLocalFile(QDir::cleanPath(baseDirectory + QDir::separator() + fileName));
}

void DocumentController::openCommandLineProvidedUrls(const QString& workingDirectory, const QStringList& arguments)
{
    const int currentlyOpenTabs = m_editorContainer.currentTabWidget()->count();

    if (arguments.isEmpty()) {
        if (currentlyOpenTabs == 0)
            m_window.ui->actionNew->trigger();
        return;
    }

    const QSharedPointer<QCommandLineParser> parser = Notepadqq::getCommandLineArgumentsParser(arguments);
    const QStringList rawUrls = parser->positionalArguments();

    if (rawUrls.isEmpty() && currentlyOpenTabs == 0) {
        m_window.ui->actionNew->trigger();
        return;
    }

    QList<QUrl> files;
    for (const QString& rawUrl : rawUrls)
        files.append(stringToUrl(rawUrl, workingDirectory));

    const auto loading =
        m_docEngine.getDocumentLoader().setUrls(files).setTabWidget(m_editorContainer.currentTabWidget()).execute();

    if (!parser->isSet("line") && !parser->isSet("column"))
        return;

    int line = 0;
    if (parser->isSet("line")) {
        bool okay;
        line = parser->value("line").toInt(&okay);
        if (!okay)
            qWarning() << m_window.tr("Invalid value for '--line' argument: %1").arg(parser->value("line"));
    }

    int column = 0;
    if (parser->isSet("column")) {
        bool okay;
        column = parser->value("column").toInt(&okay);
        if (!okay)
            qWarning() << m_window.tr("Invalid value for '--column' argument: %1").arg(parser->value("column"));
    }

    CommandLineOpenRuntime::continueAfterLoading(loading, this, [this, rawUrls, line, column] {
        if (rawUrls.size() > 1) {
            qWarning() << m_window.tr(
                "The '--line' and '--column' arguments will be ignored since more than one file is opened.");
            return;
        }

        QPointer<EditorNS::Editor> editor = m_editorContainer.currentTabWidget()->currentEditor();
        EditorNS::deferToObject(this, [editor, line, column] {
            if (editor)
                editor->setCursorPosition(line - 1, column - 1);
        });
    });
}

void DocumentController::handleDragEnter(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void DocumentController::handleDrop(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    m_docEngine.getDocumentLoader().setUrls(urls).setTabWidget(m_editorContainer.currentTabWidget()).execute();
}

void DocumentController::openDroppedUrls(QList<QUrl> urls, EditorNS::Editor* sourceEditor)
{
    EditorTabWidget* tabWidget =
        sourceEditor ? m_editorContainer.tabWidgetFromEditor(sourceEditor) : m_editorContainer.currentTabWidget();
    if (urls.isEmpty())
        return;

    if (urls.size() == 1) {
        const QString path = urls.front().toLocalFile();
        const QFileInfo fileInfo(path);
        if (fileInfo.isDir()) {
            urls.clear();
            for (const QFileInfo& entry : QDir(path).entryInfoList(QDir::Files))
                urls.push_back(QUrl::fromLocalFile(entry.filePath()));
        }
    }

    m_docEngine.getDocumentLoader().setUrls(urls).setTabWidget(tabWidget).execute();
}

void DocumentController::openFiles()
{
    QUrl defaultUrl = m_window.currentEditor()->filePath();
    if (defaultUrl.isEmpty())
        defaultUrl = QUrl::fromLocalFile(m_settings.General.getLastSelectedDir());

    BackupServicePauser backupServicePauser;
    backupServicePauser.pause();

    const auto dialogOption =
        m_settings.General.getUseNativeFilePicker() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog;
    const QList<QUrl> fileNames = QFileDialog::getOpenFileUrls(
        &m_window, m_window.tr("Open"), defaultUrl, m_window.tr("All files (*)"), nullptr, dialogOption);
    if (fileNames.isEmpty())
        return;

    m_docEngine.getDocumentLoader().setUrls(fileNames).setTabWidget(m_editorContainer.currentTabWidget()).execute();
}

void DocumentController::openFolder()
{
    QUrl defaultUrl = m_window.currentEditor()->filePath();
    if (defaultUrl.isEmpty())
        defaultUrl = QUrl::fromLocalFile(m_settings.General.getLastSelectedDir());

    BackupServicePauser backupServicePauser;
    backupServicePauser.pause();

    const auto dialogOption =
        m_settings.General.getUseNativeFilePicker() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog;
    const QString folder = QFileDialog::getExistingDirectory(
        &m_window, m_window.tr("Open Folder"), defaultUrl.toLocalFile(), dialogOption);
    if (folder.isEmpty())
        return;

    QList<QUrl> fileNames;
    for (const QString& file : QDir(folder).entryList(QStringList(), QDir::Files)) {
        if (!file.startsWith('.') && !file.endsWith('~'))
            fileNames.append(stringToUrl(file, folder));
    }
    if (fileNames.isEmpty())
        return;

    m_docEngine.getDocumentLoader().setUrls(fileNames).setTabWidget(m_editorContainer.currentTabWidget()).execute();
}

bool DocumentController::saveTabsToCache()
{
    while (!Sessions::saveSession(
        &m_docEngine, &m_editorContainer, PersistentCache::cacheSessionPath(), PersistentCache::cacheDirPath())) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setText(m_window.tr("Error while trying to save this session. Please ensure the following directory is "
                                   "accessible:\n\n") +
                       PersistentCache::cacheDirPath() + "\n\n" +
                       m_window.tr("By choosing \"ignore\" your session won't be saved."));
        msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
        msgBox.setDefaultButton(QMessageBox::Retry);
        msgBox.setIcon(QMessageBox::Critical);

        const int result = msgBox.exec();
        if (result == QMessageBox::Abort)
            return false;
        if (result == QMessageBox::Ignore)
            return true;
    }
    return true;
}

bool DocumentController::finalizeAllTabs()
{
    const int tabWidgetsCount = m_editorContainer.count();
    for (int i = 0; i < tabWidgetsCount; ++i) {
        EditorTabWidget* tabWidget = m_editorContainer.tabWidget(i);
        const int tabCount = tabWidget->count();
        for (int j = 0; j < tabCount; ++j) {
            if (closeTab(tabWidget, j, false, false) == MainWindow::tabCloseResult_Canceled)
                return false;
        }
    }
    return true;
}

bool DocumentController::prepareToClose()
{
    return MainWindow::m_instances.size() == 1 && m_settings.General.getRememberTabsOnExit() ? saveTabsToCache()
                                                                                             : finalizeAllTabs();
}

int DocumentController::askIfWantToSave(EditorTabWidget* tabWidget, int tab, int reason)
{
    QMessageBox msgBox(&m_window);
    const QString name = tabWidget->tabText(tab).toHtmlEscaped();
    msgBox.setWindowTitle(QCoreApplication::applicationName());
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (reason) {
    case MainWindow::askToSaveChangesReason_generic:
        msgBox.setText("<h3>" + m_window.tr("Do you want to save changes to «%1»?").arg(name) + "</h3>");
        if (QAbstractButton* discardButton = msgBox.button(QMessageBox::Discard))
            discardButton->setText(m_window.tr("Don't Save"));
        break;
    case MainWindow::askToSaveChangesReason_tabClosing:
        msgBox.setText("<h3>" + m_window.tr("Do you want to save changes to «%1» before closing?").arg(name) + "</h3>");
        break;
    }

    msgBox.setInformativeText(m_window.tr("If you don't save the changes you made, you'll lose them forever."));
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setEscapeButton(QMessageBox::Cancel);
    msgBox.setIconPixmap(IconProvider::fromTheme("document-save")
            .pixmap(64, 64)
            .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.exec();
    return msgBox.standardButton(msgBox.clickedButton());
}

int DocumentController::closeTab(EditorTabWidget* tabWidget, int tab, bool remove, bool force)
{
    int result = MainWindow::tabCloseResult_AlreadySaved;
    EditorNS::Editor* editor = tabWidget->editor(tab);

    if (m_editorContainer.count() == 1 && tabWidget->count() == 1 && editor->filePath().isEmpty() &&
        editor->value().isEmpty()) {
        if (m_settings.General.getExitOnLastTabClose())
            m_window.close();
        goto cleanup;
    }

    if (force || editor->isClean() || (editor->filePath().isEmpty() && editor->value().isEmpty())) {
        if (remove)
            m_docEngine.closeDocument(tabWidget, tab);
        goto cleanup;
    }

    tabWidget->setCurrentIndex(tab);
    switch (askIfWantToSave(tabWidget, tab, MainWindow::askToSaveChangesReason_tabClosing)) {
    case QMessageBox::Save:
        switch (save(tabWidget, tab)) {
        case DocEngine::saveFileResult_Canceled:
            result = MainWindow::tabCloseResult_Canceled;
            break;
        case DocEngine::saveFileResult_Saved:
            if (remove)
                m_docEngine.closeDocument(tabWidget, tab);
            result = MainWindow::tabCloseResult_Saved;
            break;
        }
        break;
    case QMessageBox::Discard:
        if (remove)
            m_docEngine.closeDocument(tabWidget, tab);
        result = MainWindow::tabCloseResult_NotSaved;
        break;
    case QMessageBox::Cancel:
        result = MainWindow::tabCloseResult_Canceled;
        break;
    }

    if (tabWidget->count() > 0)
        tabWidget->currentEditor()->setFocus();

cleanup:
    if (tabWidget->count() > 0)
        return result;

    if (m_editorContainer.count() > 1) {
        delete tabWidget;
        m_editorContainer.tabWidget(0)->currentEditor()->setFocus();
    } else if (m_settings.General.getExitOnLastTabClose()) {
        m_window.close();
    } else {
        m_window.ui->actionNew->trigger();
    }
    return result;
}

int DocumentController::closeTab(EditorTabWidget* tabWidget, int tab)
{ return closeTab(tabWidget, tab, true, false); }

int DocumentController::save(EditorTabWidget* tabWidget, int tab)
{
    EditorNS::Editor* editor = tabWidget->editor(tab);
    if (editor->filePath().isEmpty())
        return saveAs(tabWidget, tab, false);

    bool fileOverwrite = false;
    if (editor->filePath().isLocalFile())
        fileOverwrite = QFile(editor->filePath().toLocalFile()).exists();

    if (editor->fileOnDiskChanged() && fileOverwrite) {
        QMessageBox msgBox(&m_window);
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(
            "<h3>" + m_window.tr("The file on disk has changed since the last read.\nDo you want to save it anyway?") +
            "</h3>");
        msgBox.setInformativeText(m_window.tr("Saving the file might cause loss of external data."));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        if (msgBox.exec() == QMessageBox::Cancel)
            return DocEngine::saveFileResult_Canceled;
    }

    return m_docEngine.saveDocument(tabWidget, tab, editor->filePath());
}

QUrl DocumentController::saveDialogDefaultFileName(EditorTabWidget* tabWidget, int tab) const
{
    const QUrl documentFileName = tabWidget->editor(tab)->filePath();
    if (!documentFileName.isEmpty())
        return documentFileName;

    const auto& extensions = tabWidget->editor(tab)->getLanguage()->fileExtensions;
    const QString extension = extensions.isEmpty() ? QString() : "." + extensions.first();
    return QUrl::fromLocalFile(m_settings.General.getLastSelectedDir() + "/" + tabWidget->tabText(tab) + extension);
}

int DocumentController::saveAs(EditorTabWidget* tabWidget, int tab, bool copy)
{
    BackupServicePauser backupServicePauser;
    backupServicePauser.pause();

    const auto dialogOption =
        m_settings.General.getUseNativeFilePicker() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog;
    const QString filename = QFileDialog::getSaveFileName(&m_window,
        m_window.tr("Save as"),
        saveDialogDefaultFileName(tabWidget, tab).toLocalFile(),
        m_window.tr("Any file (*)"),
        nullptr,
        dialogOption);
    if (filename.isEmpty())
        return DocEngine::saveFileResult_Canceled;

    m_settings.General.setLastSelectedDir(QFileInfo(filename).absolutePath());
    return m_docEngine.saveDocument(tabWidget, tab, QUrl::fromLocalFile(filename), copy);
}

void DocumentController::closeAll()
{
    bool canceled = false;
    m_editorContainer.forEachEditor([&](const int, const int editorId, EditorTabWidget* tabWidget, EditorNS::Editor*) {
        if (closeTab(tabWidget, editorId, false, false) == MainWindow::tabCloseResult_Canceled) {
            canceled = true;
            return false;
        }
        return true;
    });

    if (!canceled) {
        m_editorContainer.forEachEditor(
            true, [&](const int, const int editorId, EditorTabWidget* tabWidget, EditorNS::Editor*) {
                closeTab(tabWidget, editorId, true, true);
                return true;
            });
    }
}

void DocumentController::closeAllExceptCurrent()
{
    EditorNS::Editor* keepOpen = m_window.currentEditor();
    bool canceled = false;
    m_editorContainer.forEachEditor(
        [&](const int, const int editorId, EditorTabWidget* tabWidget, EditorNS::Editor* editor) {
            if (keepOpen == editor)
                return true;
            if (closeTab(tabWidget, editorId, false, false) == MainWindow::tabCloseResult_Canceled) {
                canceled = true;
                return false;
            }
            return true;
        });

    if (!canceled) {
        m_editorContainer.forEachEditor(
            true, [&](const int, const int editorId, EditorTabWidget* tabWidget, EditorNS::Editor* editor) {
                if (keepOpen != editor)
                    closeTab(tabWidget, editorId, true, true);
                return true;
            });
    }
}

void DocumentController::closeLeft()
{
    EditorTabWidget* tabWidget = m_editorContainer.currentTabWidget();
    const int currentEditorId = tabWidget->currentIndex();
    for (int i = currentEditorId - 1; i >= 0; --i) {
        if (closeTab(tabWidget, i, false, false) == MainWindow::tabCloseResult_Canceled)
            return;
    }
    for (int i = currentEditorId - 1; i >= 0; --i)
        closeTab(tabWidget, i, true, true);
}

void DocumentController::closeRight()
{
    EditorTabWidget* tabWidget = m_editorContainer.currentTabWidget();
    const int currentEditorId = tabWidget->currentIndex();
    for (int i = currentEditorId + 1; i < tabWidget->count(); ++i) {
        if (closeTab(tabWidget, i, false, false) == MainWindow::tabCloseResult_Canceled)
            return;
    }
    for (int i = tabWidget->count() - 1; i > currentEditorId; --i)
        closeTab(tabWidget, i, true, true);
}

void DocumentController::saveAll()
{
    m_editorContainer.forEachEditor(
        [&](const int, const int editorId, EditorTabWidget* tabWidget, EditorNS::Editor* editor) {
            if (editor->isClean())
                return true;
            tabWidget->setCurrentIndex(editorId);
            return save(tabWidget, editorId) != DocEngine::saveFileResult_Canceled;
        });
}

void DocumentController::fileOnDiskChanged(EditorTabWidget* tabWidget, int tab, bool removed)
{
    EditorNS::Editor* editor = tabWidget->editor(tab);
    if (removed) {
        auto* banner = new BannerFileRemoved(&m_window);
        banner->setObjectName("fileremoved");
        editor->insertBanner(banner);
        connect(banner, &BannerFileRemoved::ignore, this, [editor, banner] {
            editor->removeBanner(banner);
            editor->setFocus();
        });
        connect(banner, &BannerFileRemoved::save, this, [this, tabWidget, tab] { save(tabWidget, tab); });
        return;
    }

    auto* banner = new BannerFileChanged(&m_window);
    banner->setObjectName("filechanged");
    editor->insertBanner(banner);
    connect(banner, &BannerFileChanged::ignore, this, [editor, banner] {
        editor->removeBanner(banner);
        editor->setFocus();
    });
    connect(banner, &BannerFileChanged::reload, this, [this, editor, tabWidget, banner] {
        editor->removeBanner(banner);
        editor->setFocus();
        m_docEngine.getDocumentLoader()
            .setUrl(editor->filePath())
            .setTabWidget(tabWidget)
            .setReloadAction(DocEngine::ReloadActionDo)
            .execute();
    });
}

void DocumentController::documentSaved(EditorTabWidget* tabWidget, int tab)
{
    EditorNS::Editor* editor = tabWidget->editor(tab);
    editor->removeBanner("filechanged");
    editor->removeBanner("fileremoved");
    if (editor == m_window.currentEditor())
        m_window.ui->actionRename->setEnabled(true);
}

void DocumentController::documentReloaded(EditorTabWidget* tabWidget, int tab)
{
    EditorNS::Editor* editor = tabWidget->editor(tab);
    editor->removeBanner("filechanged");
    editor->removeBanner("fileremoved");
    if (editor == m_window.currentEditor()) {
        m_window.refreshEditorUiInfo(editor);
        editor->requestDocumentInfo();
    }
}

void DocumentController::documentLoaded(
    EditorTabWidget* tabWidget, int tab, bool wasAlreadyOpened, bool updateRecentDocs)
{
    EditorNS::Editor* editor = tabWidget->editor(tab);
    constexpr int maxRecentEntries = 10;
    if (updateRecentDocs) {
        const QUrl newUrl = editor->filePath();
        QList<QVariant> recentDocs = m_settings.General.getRecentDocuments();
        recentDocs.insert(0, QVariant(newUrl));
        for (int i = recentDocs.count() - 1; i >= 1; --i) {
            if (newUrl == recentDocs[i].toUrl())
                recentDocs.removeAt(i);
        }
        while (recentDocs.count() > maxRecentEntries)
            recentDocs.removeLast();
        m_settings.General.setRecentDocuments(recentDocs);
        m_window.updateRecentDocsInMenu();
    }

    if (!wasAlreadyOpened && m_settings.General.getWarnForDifferentIndentation())
        m_window.checkIndentationMode(editor);
}

void DocumentController::reloadCurrentDocument()
{
    EditorTabWidget* tabWidget = m_editorContainer.currentTabWidget();
    EditorNS::Editor* editor = tabWidget->currentEditor();
    if (editor->filePath().isEmpty())
        return;

    m_docEngine.getDocumentLoader()
        .setUrl(editor->filePath())
        .setTabWidget(tabWidget)
        .setTextCodec(editor->codec())
        .setBOM(editor->bom())
        .execute();
}

void DocumentController::renameCurrentDocument()
{
    EditorTabWidget* tabWidget = m_editorContainer.currentTabWidget();
    const QUrl oldFilename = tabWidget->currentEditor()->filePath();
    const int result = saveAs(tabWidget, tabWidget->currentIndex(), false);
    if (result != DocEngine::saveFileResult_Saved || oldFilename.isEmpty() ||
        QFileInfo(oldFilename.toLocalFile()) == QFileInfo(tabWidget->currentEditor()->filePath().toLocalFile())) {
        return;
    }

    const QString filename = oldFilename.toLocalFile();
    if (QFile::exists(filename) && !QFile::remove(filename)) {
        QMessageBox::warning(
            &m_window, QApplication::applicationName(), QString("Error: unable to remove file %1").arg(filename));
    }
}

void DocumentController::clearRecentFiles()
{
    m_settings.General.resetRecentDocuments();
    m_window.updateRecentDocsInMenu();
}

void DocumentController::openAllRecentFiles()
{
    QList<QVariant> allRecentUrlVariants = m_settings.General.getRecentDocuments();
    QList<QUrl> urlsToOpen;
    QList<QUrl> urlsOfMissingFiles;
    for (const QVariant& document : allRecentUrlVariants) {
        const QUrl url = document.toUrl();
        if (QFileInfo::exists(url.toLocalFile()))
            urlsToOpen.push_back(url);
        else
            urlsOfMissingFiles.push_back(url);
    }

    if (!urlsOfMissingFiles.isEmpty()) {
        QString message = m_window.tr("The following files do not exist anymore. Do you want to open them anyway?\n");
        for (const QUrl& url : urlsOfMissingFiles)
            message += '\n' + url.toLocalFile();

        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(message);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        if (msgBox.exec() == QMessageBox::Yes) {
            urlsToOpen.clear();
            for (const QVariant& url : allRecentUrlVariants)
                urlsToOpen.push_back(url.toUrl());
        } else {
            for (const QUrl& url : urlsOfMissingFiles)
                allRecentUrlVariants.removeOne(QVariant::fromValue(url));
            m_settings.General.setRecentDocuments(allRecentUrlVariants);
            m_window.updateRecentDocsInMenu();
        }
    }

    m_docEngine.getDocumentLoader().setUrls(urlsToOpen).setTabWidget(m_editorContainer.currentTabWidget()).execute();
}

void DocumentController::openRecentFileEntry(const QUrl& url)
{
    const QString filePath = url.toLocalFile();
    if (!QFileInfo::exists(filePath)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(m_window.tr("The file \"%1\" does not exist. Do you want to re-create it?").arg(filePath));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) {
            QList<QVariant> recentDocs = m_settings.General.getRecentDocuments();
            recentDocs.removeOne(QVariant::fromValue(url));
            m_settings.General.setRecentDocuments(recentDocs);
            m_window.updateRecentDocsInMenu();
            return;
        }
    }

    m_docEngine.getDocumentLoader().setUrl(url).setTabWidget(m_editorContainer.currentTabWidget()).execute();
}

void DocumentController::loadSession()
{
    BackupServicePauser backupServicePauser;
    backupServicePauser.pause();

    const QString recentFolder = QUrl::fromLocalFile(m_settings.General.getLastSelectedSessionDir()).toLocalFile();
    const auto dialogOption =
        m_settings.General.getUseNativeFilePicker() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog;
    const QString filePath = QFileDialog::getOpenFileName(&m_window,
        m_window.tr("Open Session..."),
        recentFolder,
        m_window.tr("Session file (*.xml);;Any file (*)"),
        nullptr,
        dialogOption);
    if (filePath.isEmpty())
        return;

    m_settings.General.setLastSelectedSessionDir(QFileInfo(filePath).dir().absolutePath());
    Sessions::loadSession(&m_docEngine, &m_editorContainer, filePath);
}

void DocumentController::saveSession()
{
    BackupServicePauser backupServicePauser;
    backupServicePauser.pause();

    const QString recentFolder = QUrl::fromLocalFile(m_settings.General.getLastSelectedSessionDir()).toLocalFile();
    QFileDialog dialog(
        &m_window, m_window.tr("Save Session as..."), recentFolder, m_window.tr("Session file (*.xml);;Any file (*)"));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix("xml");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::DontUseNativeDialog, !m_settings.General.getUseNativeFilePicker());
    if (!dialog.exec() || dialog.selectedFiles().isEmpty())
        return;

    const QString filePath = dialog.selectedFiles().first();
    if (filePath.isEmpty())
        return;

    m_settings.General.setLastSelectedSessionDir(QFileInfo(filePath).dir().absolutePath());
    if (Sessions::saveSession(&m_docEngine, &m_editorContainer, filePath)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setText(m_window.tr("Error while trying to save this session. Please try a different file name."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
    }
}
