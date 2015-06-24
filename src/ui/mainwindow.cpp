#include "include/mainwindow.h"
#include "ui_mainwindow.h"
#include "include/EditorNS/editor.h"
#include "include/editortabwidget.h"
#include "include/frmabout.h"
#include "include/frmpreferences.h"
#include "include/notepadqq.h"
#include "include/iconprovider.h"
#include "include/EditorNS/bannerfilechanged.h"
#include "include/EditorNS/bannerfileremoved.h"
#include "include/EditorNS/bannerindentationdetected.h"
#include "include/clickablelabel.h"
#include "include/frmencodingchooser.h"
#include "include/frmindentationmode.h"
#include "include/Extensions/extensionsloader.h"
#include "include/frmlinenumberchooser.h"
#include "include/Extensions/Stubs/windowstub.h"
#include "include/Extensions/installextension.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QUrl>
#include <QMimeData>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolButton>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QDesktopServices>
#include <QJsonArray>

QList<MainWindow*> MainWindow::m_instances = QList<MainWindow*>();

MainWindow::MainWindow(const QString &workingDirectory, const QStringList &arguments, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_topEditorContainer(new TopEditorContainer(this)),
    m_fileSearchResultsWidget(new FileSearchResultsWidget()),
    m_workingDirectory(workingDirectory)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    MainWindow::m_instances.append(this);

    // Gets company name from QCoreApplication::setOrganizationName(). Same for app name.
    m_settings = new QSettings(this);

    setCentralWidget(m_topEditorContainer);

    m_docEngine = new DocEngine(m_settings, m_topEditorContainer);
    connect(m_docEngine, &DocEngine::fileOnDiskChanged, this, &MainWindow::on_fileOnDiskChanged);
    connect(m_docEngine, &DocEngine::documentSaved, this, &MainWindow::on_documentSaved);
    connect(m_docEngine, &DocEngine::documentReloaded, this, &MainWindow::on_documentReloaded);
    connect(m_docEngine, &DocEngine::documentLoaded, this, &MainWindow::on_documentLoaded);

    loadIcons();

    // Context menu initialization
    m_tabContextMenu = new QMenu(this);
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    m_tabContextMenuActions.append(ui->actionClose);
    m_tabContextMenuActions.append(ui->actionClose_All_BUT_Current_Document);
    m_tabContextMenuActions.append(ui->actionSave);
    m_tabContextMenuActions.append(ui->actionSave_as);
    m_tabContextMenuActions.append(ui->actionRename);
    m_tabContextMenuActions.append(ui->actionPrint);
    m_tabContextMenuActions.append(separator);
    m_tabContextMenuActions.append(ui->actionMove_to_Other_View);
    m_tabContextMenuActions.append(ui->actionClone_to_Other_View);
    m_tabContextMenuActions.append(ui->actionMove_to_New_Window);
    m_tabContextMenuActions.append(ui->actionOpen_in_New_Window);
    m_tabContextMenu->addActions(m_tabContextMenuActions);

    fixKeyboardShortcuts();

    // Set popup for action_Open in toolbar
    QToolButton *btnActionOpen = static_cast<QToolButton *>(ui->mainToolBar->widgetForAction(ui->action_Open));
    btnActionOpen->setMenu(ui->menuRecent_Files);
    btnActionOpen->setPopupMode(QToolButton::MenuButtonPopup);

    // Action group for EOL modes
    QActionGroup *eolActionGroup = new QActionGroup(this);
    eolActionGroup->addAction(ui->actionWindows_Format);
    eolActionGroup->addAction(ui->actionUNIX_Format);
    eolActionGroup->addAction(ui->actionMac_Format);

    // Action group for indentation modes
    QActionGroup *indentationActionGroup = new QActionGroup(this);
    indentationActionGroup->addAction(ui->actionIndentation_Default_settings);
    indentationActionGroup->addAction(ui->actionIndentation_Custom);

    connect(m_topEditorContainer, &TopEditorContainer::customTabContextMenuRequested,
            this, &MainWindow::on_customTabContextMenuRequested);

    connect(m_topEditorContainer, &TopEditorContainer::tabCloseRequested,
            this, &MainWindow::on_tabCloseRequested);

    connect(m_topEditorContainer, &TopEditorContainer::currentEditorChanged,
            this, &MainWindow::on_currentEditorChanged);

    connect(m_topEditorContainer, &TopEditorContainer::editorAdded,
            this, &MainWindow::on_editorAdded);

    connect(m_topEditorContainer, &TopEditorContainer::editorMouseWheel,
            this, &MainWindow::on_editorMouseWheel);

    connect(m_topEditorContainer, &TopEditorContainer::tabBarDoubleClicked,
            this, &MainWindow::on_tabBarDoubleClicked);

    createStatusBar();

    updateRecentDocsInMenu();

    setAcceptDrops(true);

    ui->dockFileSearchResults->setWidget(m_fileSearchResultsWidget);
    connect(m_fileSearchResultsWidget, &FileSearchResultsWidget::resultMatchClicked,
            this, &MainWindow::on_resultMatchClicked);

    // Initialize UI from settings
    ui->actionWord_wrap->setChecked(m_settings->value("wordWrap", false).toBool());
    ui->actionShow_Tabs->setChecked(m_settings->value("tabsVisible", false).toBool());

    // Inserts at least an editor
    openCommandLineProvidedUrls(workingDirectory, arguments);
    // From now on, there is at least an Editor and at least
    // an EditorTabWidget within m_topEditorContainer.

    // Set zoom from settings
    qreal zoom = m_settings->value("zoom", 1).toReal();
    for (int i = 0; i < m_topEditorContainer->count(); i++) {
        m_topEditorContainer->tabWidget(i)->setZoomFactor(zoom);
    }

    restoreWindowSettings();

    ui->dockFileSearchResults->hide();

    // If there was another window already opened, move this window
    // slightly to the bottom-right, so that they won't completely overlap.
    if (!isMaximized() && m_instances.count() > 1) {
        QPoint curPos = pos();
        move(curPos.x() + 50, curPos.y() + 50);
    }

    setupLanguagesMenu();

    showExtensionsMenu(Extensions::ExtensionsLoader::extensionRuntimePresent());

    // DEBUG: Add a second tabWidget
    //this->topEditorContainer->addTabWidget()->addEditorTab(false, "test");

    emit Notepadqq::getInstance().newWindow(this);
}

MainWindow::MainWindow(const QStringList &arguments, QWidget *parent)
    : MainWindow(QDir::currentPath(), arguments, parent)
{ }

MainWindow::~MainWindow()
{
    MainWindow::m_instances.removeAll(this);

    delete ui;
    delete m_docEngine;
}

QList<MainWindow*> MainWindow::instances()
{
    return MainWindow::m_instances;
}

MainWindow *MainWindow::lastActiveInstance()
{
    if (m_instances.length() > 0) {
        return m_instances.last();
    } else {
        return nullptr;
    }
}

TopEditorContainer *MainWindow::topEditorContainer()
{
    return m_topEditorContainer;
}

void MainWindow::restoreWindowSettings()
{
    m_settings->beginGroup("MainWindow");
    restoreGeometry(m_settings->value("geometry").toByteArray());
    restoreState(m_settings->value("windowState").toByteArray());
    m_settings->endGroup();
}

void MainWindow::loadIcons()
{
    // To test fallback icons:
    // QIcon::setThemeSearchPaths(QStringList(""));

    // Assign (where possible) system theme icons to our actions.
    // If a system icon doesn't exist, fallback on the already assigned icon.
    ui->action_New->setIcon(IconProvider::fromTheme("document-new"));
    ui->action_Open->setIcon(IconProvider::fromTheme("document-open"));
    ui->actionSave->setIcon(IconProvider::fromTheme("document-save"));
    ui->actionSave_as->setIcon(IconProvider::fromTheme("document-save-as"));
    ui->actionSave_All->setIcon(IconProvider::fromTheme("document-save-all"));
    ui->actionClose->setIcon(IconProvider::fromTheme("document-close"));
    ui->actionC_lose_All->setIcon(IconProvider::fromTheme("document-close-all"));
    ui->actionPrint_Now->setIcon(IconProvider::fromTheme("document-print"));
    ui->actionCu_t->setIcon(IconProvider::fromTheme("edit-cut"));
    ui->action_Copy->setIcon(IconProvider::fromTheme("edit-copy"));
    ui->action_Paste->setIcon(IconProvider::fromTheme("edit-paste"));
    ui->action_Undo->setIcon(IconProvider::fromTheme("edit-undo"));
    ui->action_Redo->setIcon(IconProvider::fromTheme("edit-redo"));
    ui->actionZoom_In->setIcon(IconProvider::fromTheme("zoom-in"));
    ui->actionZoom_Out->setIcon(IconProvider::fromTheme("zoom-out"));
    ui->actionRestore_Default_Zoom->setIcon(IconProvider::fromTheme("zoom-original"));
    ui->action_Start_Recording->setIcon(IconProvider::fromTheme("media-record"));
    ui->action_Stop_Recording->setIcon(IconProvider::fromTheme("media-playback-stop"));
    ui->action_Playback->setIcon(IconProvider::fromTheme("media-playback-start"));
    ui->actionRun_a_Macro_Multiple_Times->setIcon(IconProvider::fromTheme("media-seek-forward"));
    ui->actionSave_Currently_Recorded_Macro->setIcon(IconProvider::fromTheme("document-save-as"));
    ui->actionPreferences->setIcon(IconProvider::fromTheme("preferences-other"));
    ui->actionSearch->setIcon(IconProvider::fromTheme("edit-find"));
    ui->actionReplace->setIcon(IconProvider::fromTheme("edit-find-replace"));
    ui->actionShow_All_Characters->setIcon(IconProvider::fromTheme("show-special-chars"));
    ui->actionWord_wrap->setIcon(IconProvider::fromTheme("word-wrap"));
    ui->actionFind_Next->setIcon(IconProvider::fromTheme("go-next"));
    ui->actionFind_Previous->setIcon(IconProvider::fromTheme("go-previous"));
}

void MainWindow::createStatusBar()
{
    QStatusBar *status = statusBar();
    status->setStyleSheet("QStatusBar::item { border: none; }; ");
    status->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setFrameStyle(QScrollArea::NoFrame);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::NoFrame);
    frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);

    scrollArea->setWidget(frame);
    scrollArea->setWidgetResizable(true);
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar {height:0px;}");
    scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");


    QLabel *label;
    QMargins tmpMargins;

    label = new QLabel("File Format", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(150);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_fileFormat = label;

    label = new QLabel("Ln 0, col 0", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(120);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_curPos = label;

    label = new QLabel("Sel 0, 0", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(120);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_selection = label;

    label = new QLabel("0 chars, 0 lines", this);
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_length_lines = label;

    label = new QLabel("EOL", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(118);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_EOLstyle = label;

    label = new QLabel("Encoding", this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(118);
    tmpMargins = label->contentsMargins();
    label->setContentsMargins(tmpMargins.left(), tmpMargins.top(), tmpMargins.right() + 10, tmpMargins.bottom());
    layout->addWidget(label);
    m_statusBar_textFormat = label;

    label = new QLabel(tr("INS"), this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setMinimumWidth(40);
    layout->addWidget(label);
    m_statusBar_overtypeNotify = label;

    if (Notepadqq::oldQt()) {
        ClickableLabel *cklabel = new ClickableLabel(QString("Qt ") + qVersion(), this);
        cklabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        cklabel->setStyleSheet("QLabel { background-color: #FF4136; color: white; }");
        cklabel->setCursor(Qt::PointingHandCursor);
        connect(cklabel, &ClickableLabel::clicked, this, [&]() {
            Notepadqq::showQtVersionWarning(false, this);
        });
        layout->addWidget(cklabel);
    }


    status->addWidget(scrollArea, 1);
    scrollArea->setFixedHeight(frame->height());
}

void MainWindow::setupLanguagesMenu()
{
    Editor *editor = currentEditor();
    if (editor == 0) {
        qDebug() << "currentEditor is null";
        throw;
    }

    QList<QMap<QString, QString>> langs = editor->languages();
    std::sort(langs.begin(), langs.end(), Editor::LanguageGreater());

    //ui->menu_Language->setStyleSheet("* { menu-scrollable: 1 }");
    QMap<QChar, QMenu*> menuInitials;
    for (int i = 0; i < langs.length(); i++) {
        const QMap<QString, QString> &map = langs.at(i);

        QString name = map.value("name", "?");
        if (name.length() == 0) name = "?";
        QChar letter = name.at(0).toUpper();

        QMenu *letterMenu;
        if (menuInitials.contains(letter)) {
            letterMenu = menuInitials.value(letter, 0);
        } else {
            letterMenu = new QMenu(letter, this);
            menuInitials.insert(letter, letterMenu);
            ui->menu_Language->insertMenu(0, letterMenu);
        }

        QString langId = map.value("id", "");
        QAction *action = new QAction(map.value("name"), this);
        connect(action, &QAction::triggered, this, [=](bool /*checked*/ = false) {
            currentEditor()->setLanguage(langId);
        });
        letterMenu->insertAction(0, action);
    }
}

void MainWindow::fixKeyboardShortcuts()
{
    QList<QMenu*> lst;
    lst = ui->menuBar->findChildren<QMenu*>();

    foreach (QMenu* m, lst)
    {
        addAction(m->menuAction());
        addActions(m->actions());
    }
}

QUrl MainWindow::stringToUrl(QString fileName, QString workingDirectory)
{
    if (workingDirectory.isEmpty())
        workingDirectory = m_workingDirectory;

    QUrl f = QUrl(fileName);
    if (f.isRelative()) { // No schema
        QFileInfo fi(fileName);
        if (fi.isRelative()) { // Relative local path
            QString absolute = QDir::cleanPath(workingDirectory + QDir::separator() + fileName);
            return QUrl::fromLocalFile(absolute);
        } else {
            return QUrl::fromLocalFile(fileName);
        }
    } else {
        return f;
    }
}

void MainWindow::openCommandLineProvidedUrls(const QString &workingDirectory, const QStringList &arguments)
{
    if (arguments.count() == 0) {
        ui->action_New->trigger();
        return;
    }

    QSharedPointer<QCommandLineParser> parser = Notepadqq::getCommandLineArgumentsParser(arguments);

    QStringList rawUrls = parser->positionalArguments();

    if (rawUrls.count() < 1)
    {
        // Open a new empty document
        ui->action_New->trigger();
    }
    else
    {
        // Open selected files
        QList<QUrl> files;
        for(int i = 0; i < rawUrls.count(); i++)
        {
            files.append(stringToUrl(rawUrls.at(i), workingDirectory));
        }

        EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
        // Make sure we have a tabWidget: if not, create it.
        if(tabW == 0) {
            tabW = m_topEditorContainer->addTabWidget();
        }

        m_docEngine->loadDocuments(files, tabW);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    QMainWindow::dragEnterEvent(e);

    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QMainWindow::dropEvent(e);

    QList<QUrl> fileNames = e->mimeData()->urls();
    if (!fileNames.empty()) {
        m_docEngine->loadDocuments(fileNames,
                                   m_topEditorContainer->currentTabWidget());
    }
}

void MainWindow::on_editorUrlsDropped(QList<QUrl> urls)
{
    EditorTabWidget *tabWidget;
    Editor *editor = dynamic_cast<Editor *>(sender());

    if (editor) {
        tabWidget = m_topEditorContainer->tabWidgetFromEditor(editor);
    } else {
        tabWidget = m_topEditorContainer->currentTabWidget();
    }

    if (!urls.empty()) {
        m_docEngine->loadDocuments(urls,
                                   tabWidget);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Insert) {
        toggleOverwrite();
    } else {
        QMainWindow::keyPressEvent(ev);
    }
}

void MainWindow::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            if (m_instances.length() > 0 && m_instances.last() != this) {
                int pos = m_instances.indexOf(this);
                if (pos > -1) {
                    // Move this instance at the end of the list
                    m_instances.move(pos, m_instances.length() - 1);
                }
            }
        }
    }
}

void MainWindow::toggleOverwrite()
{
    m_overwrite = !m_overwrite;

    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setOverwrite(m_overwrite);
        return true;
    });

    if (m_overwrite) {
        m_statusBar_overtypeNotify->setText(tr("OVR"));
    } else {
        m_statusBar_overtypeNotify->setText(tr("INS"));
    }
}

QString MainWindow::getNewDocumentName()
{
    static int num = 1; // FIXME maybe find a smarter way
    return tr("new %1").arg(num++);
}

void MainWindow::on_action_New_triggered()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();

    // Make sure we have a tabWidget: if not, create it.
    if(tabW == 0) {
        tabW = m_topEditorContainer->addTabWidget();
    }

    m_docEngine->addNewDocument(getNewDocumentName(), true, tabW);
}

void MainWindow::setCurrentEditorLanguage(QString language)
{
    currentEditor()->setLanguage(language);
}

void MainWindow::on_customTabContextMenuRequested(QPoint point, EditorTabWidget * /*tabWidget*/, int /*tabIndex*/)
{
    m_tabContextMenu->exec(point);
}

bool MainWindow::reloadWithWarning(EditorTabWidget *tabWidget, int tab, QTextCodec *codec, bool bom)
{
    if (!tabWidget->editor(tab)->isClean()) {
        QMessageBox msgBox(this);
        QString name = tabWidget->tabText(tab).toHtmlEscaped();

        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

        msgBox.setText("<h3>" + tr("Your changes to «%1» will be discarded.").arg(name) + "</h3>");
        msgBox.setButtonText(QMessageBox::Ok, tr("Reload"));

        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setEscapeButton(QMessageBox::Cancel);

        msgBox.exec();

        if (msgBox.standardButton(msgBox.clickedButton()) == QMessageBox::Cancel)
            return false;
    }

    return m_docEngine->reloadDocument(tabWidget, tab, codec, bom);
}

void MainWindow::on_actionMove_to_Other_View_triggered()
{
    EditorTabWidget *curTabWidget = m_topEditorContainer->currentTabWidget();
    EditorTabWidget *destTabWidget;

    if(m_topEditorContainer->count() >= 2) {
        int viewId = 1;
        if(m_topEditorContainer->widget(1) == curTabWidget) {
            viewId = 0;
        }

        destTabWidget = m_topEditorContainer->tabWidget(viewId);

    } else {
        destTabWidget = m_topEditorContainer->addTabWidget();
    }

    destTabWidget->transferEditorTab(true, curTabWidget, curTabWidget->currentIndex());

    removeTabWidgetIfEmpty(curTabWidget);
}

void MainWindow::removeTabWidgetIfEmpty(EditorTabWidget *tabWidget) {
    if(tabWidget->count() == 0) {
        delete tabWidget;
    }
}

void MainWindow::on_action_Open_triggered()
{
    QUrl defaultUrl = currentEditor()->fileName();
    if (defaultUrl.isEmpty())
            defaultUrl = QUrl::fromLocalFile(m_settings->value("lastSelectedDir", ".").toString());

    QList<QUrl> fileNames = QFileDialog::getOpenFileUrls(
                this,
                tr("Open"),
                defaultUrl,
                tr("All files (*)"),
                0, 0);

    if (!fileNames.empty()) {
        m_docEngine->loadDocuments(fileNames,
                                   m_topEditorContainer->currentTabWidget());

        m_settings->setValue("lastSelectedDir",
                             QFileInfo(fileNames[0].toLocalFile()).absolutePath());
    }
}

void MainWindow::on_actionOpen_Folder_triggered()
{
    QUrl defaultUrl = currentEditor()->fileName();
    if (defaultUrl.isEmpty())
            defaultUrl = QUrl::fromLocalFile(m_settings->value("lastSelectedDir", ".").toString());

    // Select directory
    QString folder = QFileDialog::getExistingDirectory(this, tr("Open Folder"), defaultUrl.toLocalFile(), 0);
    if (!folder.isEmpty()) {

        // Get files within directory
        QDir dir(folder);
        QStringList files = dir.entryList(QStringList(), QDir::Files);

        // Convert file names to urls
        QList<QUrl> fileNames;
        for (QString file : files) {
            // Exclude hidden and backup files
            if (!file.startsWith(".") && !file.endsWith("~")) {
                fileNames.append(stringToUrl(file, folder));
            }
        }

        if (!fileNames.isEmpty()) {

            m_docEngine->loadDocuments(fileNames,
                                       m_topEditorContainer->currentTabWidget());

            m_settings->setValue("lastSelectedDir", folder);

        }

    }
}

int MainWindow::askIfWantToSave(EditorTabWidget *tabWidget, int tab, int reason)
{
    QMessageBox msgBox(this);
    QString name = tabWidget->tabText(tab).toHtmlEscaped();

    msgBox.setWindowTitle(QCoreApplication::applicationName());

    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch(reason)
    {
    case askToSaveChangesReason_generic:
        msgBox.setText("<h3>" + tr("Do you want to save changes to «%1»?").arg(name) + "</h3>");
        msgBox.setButtonText(QMessageBox::Discard, tr("Don't Save"));
        break;
    case askToSaveChangesReason_tabClosing:
        msgBox.setText("<h3>" + tr("Do you want to save changes to «%1» before closing?").arg(name) + "</h3>");
        break;
    }

    msgBox.setInformativeText(tr("If you don't save the changes you made, you'll lose them forever."));
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setEscapeButton(QMessageBox::Cancel);

    QPixmap img = IconProvider::fromTheme("document-save").pixmap(64,64).scaled(64,64,Qt::KeepAspectRatio, Qt::SmoothTransformation);
    msgBox.setIconPixmap(img);

    msgBox.exec();

    return msgBox.standardButton(msgBox.clickedButton());
}

int MainWindow::closeTab(EditorTabWidget *tabWidget, int tab, bool remove, bool force)
{
    int result = MainWindow::tabCloseResult_AlreadySaved;
    Editor *editor = tabWidget->editor(tab);

    // Don't remove the tab if it's the last tab, it's empty, in an unmodified state and it's not associated with a file name.
    // Else, continue.
    if (! (m_topEditorContainer->count() == 1 && tabWidget->count() == 1
         && editor->fileName().isEmpty() && editor->isClean())) {

        if(!force && !editor->isClean()) {
            tabWidget->setCurrentIndex(tab);
            int ret = askIfWantToSave(tabWidget, tab, askToSaveChangesReason_tabClosing);
            if(ret == QMessageBox::Save) {
                // Save
                int saveResult = save(tabWidget, tab);
                if(saveResult == MainWindow::saveFileResult_Canceled)
                {
                    // The user canceled the "save dialog". Let's ignore the close event.
                    result = MainWindow::tabCloseResult_Canceled;
                } else if(saveResult == MainWindow::saveFileResult_Saved)
                {
                    if (remove) m_docEngine->closeDocument(tabWidget, tab);
                    result = MainWindow::tabCloseResult_Saved;
                }
            } else if(ret == QMessageBox::Discard) {
                // Don't save and close
                if (remove) m_docEngine->closeDocument(tabWidget, tab);
                result = MainWindow::tabCloseResult_NotSaved;
            } else if(ret == QMessageBox::Cancel) {
                // Don't save and cancel closing
                result = MainWindow::tabCloseResult_Canceled;
            }
        } else {
            // The tab is already saved: we can remove it safely.
            if (remove) m_docEngine->closeDocument(tabWidget, tab);
            result = MainWindow::tabCloseResult_AlreadySaved;
        }

        // Ensure the focus is still on this tabWidget
        if (tabWidget->count() > 0) {
            tabWidget->currentEditor()->setFocus();
        }
    }

    if(tabWidget->count() == 0) {
        /* Not so good... 0 tabs opened is a bad idea. So, if there are more
         * than one TabWidgets opened (split-screen) then we completely
         * remove this one. Otherwise, we add a new empty tab.
        */
        if(m_topEditorContainer->count() > 1) {
            delete tabWidget;
            m_topEditorContainer->tabWidget(0)->currentEditor()->setFocus();
        } else {
            ui->action_New->trigger();
        }
    }


    return result;
}

int MainWindow::closeTab(EditorTabWidget *tabWidget, int tab)
{
    return closeTab(tabWidget, tab, true, false);
}

int MainWindow::save(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);

    if (editor->fileName().isEmpty())
    {
        // Call "save as"
        return saveAs(tabWidget, tab, false);

    } else {
        // If the file has changed outside the editor, ask
        // the user if he want to save it.
        bool fileOverwrite = false;
        if (editor->fileName().isLocalFile())
            fileOverwrite = QFile(editor->fileName().toLocalFile()).exists();

        if (editor->fileOnDiskChanged() && fileOverwrite) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(QCoreApplication::applicationName());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("<h3>" +
                           tr("The file on disk has changed since the last "
                              "read.\nDo you want to save it anyway?") +
                           "</h3>");
            msgBox.setInformativeText(tr("Saving the file might cause "
                                         "loss of external data."));
            msgBox.setStandardButtons(QMessageBox::Save |
                                      QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Cancel)
                return MainWindow::saveFileResult_Canceled;
        }

        return m_docEngine->saveDocument(tabWidget, tab, editor->fileName());
    }
}

int MainWindow::saveAs(EditorTabWidget *tabWidget, int tab, bool copy)
{
    // Ask for a file name
    QString filename = QFileDialog::getSaveFileName(
                this,
                tr("Save as"),
                getSaveDialogDefaultFileName(tabWidget, tab).toLocalFile(),
                tr("Any file (*)"),
                0, 0);

    if (filename != "") {
        m_settings->setValue("lastSelectedDir",
                           QFileInfo(filename).absolutePath());
        // Write
        return m_docEngine->saveDocument(tabWidget, tab, QUrl::fromLocalFile(filename), copy);
    } else {
        return MainWindow::saveFileResult_Canceled;
    }
}

QUrl MainWindow::getSaveDialogDefaultFileName(EditorTabWidget *tabWidget, int tab)
{
    QUrl docFileName = tabWidget->editor(tab)->fileName();

    if (docFileName.isEmpty()) {
        return QUrl::fromLocalFile(m_settings->value("lastSelectedDir", ".").toString()
                + "/" + tabWidget->tabText(tab));
    } else {
        return docFileName;
    }
}

Editor *MainWindow::currentEditor()
{
    return m_topEditorContainer->currentTabWidget()->currentEditor();
}

QSharedPointer<Editor> MainWindow::currentEditorSharedPtr()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    return tabW->editorSharedPtr(tabW->currentIndex());
}

QAction * MainWindow::addExtensionMenuItem(QString extensionId, QString text)
{
    QMap<QString, QSharedPointer<Extensions::Extension>> extensions = Extensions::ExtensionsLoader::loadedExtensions();

    if (extensions.contains(extensionId)) {
        QSharedPointer<Extensions::Extension> extension = extensions.value(extensionId);

        // Create the menu for the extension if it doesn't exist yet.
        if (!m_extensionMenus.contains(extension)) {
            QMenu *menu = new QMenu(extension->name(), this);
            ui->menuExtensions->addMenu(menu);
            m_extensionMenus.insert(extension, menu);
        }

        // Create the menu item
        QAction *action = new QAction(text, this);
        m_extensionMenus[extension]->addAction(action);

        return action;
    } else {
        // Invalid extension id
        return NULL;
    }
}

void MainWindow::on_tabCloseRequested(EditorTabWidget *tabWidget, int tab)
{
    closeTab(tabWidget, tab);
}

void MainWindow::on_actionSave_triggered()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    save(tabW, tabW->currentIndex());
}

void MainWindow::on_actionSave_as_triggered()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    saveAs(tabW, tabW->currentIndex(), false);
}

void MainWindow::on_actionSave_a_Copy_As_triggered()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    saveAs(tabW, tabW->currentIndex(), true);
}

void MainWindow::on_action_Copy_triggered()
{
    QStringList sel = currentEditor()->selectedTexts();
    QApplication::clipboard()->setText(sel.join("\n"));
}

void MainWindow::on_action_Paste_triggered()
{
    // Normalize foreign text format
    QString text = QApplication::clipboard()->text()
            .replace(QRegularExpression("\n|\r\n|\r"), "\n");

    currentEditor()->setSelectionsText(text.split("\n"));
}

void MainWindow::on_actionCu_t_triggered()
{
    ui->action_Copy->trigger();
    currentEditor()->setSelectionsText(QStringList(""));
}

void MainWindow::on_currentEditorChanged(EditorTabWidget *tabWidget, int tab)
{
    if (tab != -1) {
        Editor *editor = tabWidget->editor(tab);
        refreshEditorUiInfo(editor);
        refreshEditorUiCursorInfo(editor);
    }
}

void MainWindow::on_editorAdded(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    connect(editor, &Editor::cursorActivity, this, &MainWindow::on_cursorActivity);
    connect(editor, &Editor::currentLanguageChanged, this, &MainWindow::on_currentLanguageChanged);
    connect(editor, &Editor::bannerRemoved, this, &MainWindow::on_bannerRemoved);
    connect(editor, &Editor::cleanChanged, this, [=]() {
        if (currentEditor() == editor)
            refreshEditorUiInfo(editor);
    });
    connect(editor, &Editor::urlsDropped, this, &MainWindow::on_editorUrlsDropped);

    // Initialize editor with UI settings
    editor->setLineWrap(ui->actionWord_wrap->isChecked());
    editor->setTabsVisible(ui->actionShow_Tabs->isChecked());
    editor->setOverwrite(m_overwrite);
}

void MainWindow::on_cursorActivity()
{
    Editor *editor = dynamic_cast<Editor *>(sender());
    if (!editor)
        return;

    if (currentEditor() == editor) {
        refreshEditorUiCursorInfo(editor);
    }
}

void MainWindow::on_currentLanguageChanged(QString /*id*/, QString /*name*/)
{
    Editor *editor = dynamic_cast<Editor *>(sender());
    if (!editor)
        return;

    if (currentEditor() == editor) {
        refreshEditorUiInfo(editor);
    }
}

void MainWindow::refreshEditorUiCursorInfo(Editor *editor)
{
    if (editor != 0) {
        // Update status bar
        int len = editor->sendMessageWithResult("C_FUN_GET_TEXT_LENGTH").toInt();
        int lines = editor->lineCount();
        m_statusBar_length_lines->setText(tr("%1 chars, %2 lines").arg(len).arg(lines));

        QPair<int, int> cursor = editor->cursorPosition();
        int selectedChars = 0;
        int selectedPieces = 0;
        QStringList selections = editor->selectedTexts();
        for (QString sel : selections) {
            selectedChars += sel.length();
            selectedPieces += sel.split("\n").count();
        }

        m_statusBar_curPos->setText(tr("Ln %1, col %2")
                                     .arg(cursor.first + 1)
                                     .arg(cursor.second + 1));

        m_statusBar_selection->setText(tr("Sel %1 (%2)").arg(selectedChars).arg(selectedPieces));
    }
}

void MainWindow::refreshEditorUiInfo(Editor *editor)
{
    // Update current language in statusbar
    QVariantMap data = editor->sendMessageWithResult("C_FUN_GET_CURRENT_LANGUAGE").toMap();
    QString name = data.value("lang").toMap().value("name").toString();
    m_statusBar_fileFormat->setText(name);


    // Update MainWindow title
    QString newTitle;
    if (editor->fileName().isEmpty()) {

        EditorTabWidget *tabWidget = m_topEditorContainer->tabWidgetFromEditor(editor);
        if (tabWidget != 0) {
            int tab = tabWidget->indexOf(editor);
            if (tab != -1) {
                newTitle = QString("%1 - %2")
                               .arg(tabWidget->tabText(tab))
                               .arg(QApplication::applicationName());
            }
        }

    } else {
        QUrl url = editor->fileName();

        QString path = url.toDisplayString(QUrl::RemovePassword |
                                           QUrl::RemoveUserInfo |
                                           QUrl::RemovePort |
                                           QUrl::RemoveAuthority |
                                           QUrl::RemoveQuery |
                                           QUrl::RemoveFragment |
                                           QUrl::PreferLocalFile |
                                           QUrl::RemoveFilename |
                                           QUrl::NormalizePathSegments |
                                           QUrl::StripTrailingSlash
                                           );

        newTitle = QString("%1 (%2) - %3")
                       .arg(Notepadqq::fileNameFromUrl(editor->fileName()))
                       .arg(path)
                       .arg(QApplication::applicationName());

    }

    if (newTitle != windowTitle()) {
        setWindowTitle(newTitle.isNull() ? QApplication::applicationName() : newTitle);
    }


    // Enable / disable menus
    bool isClean = editor->isClean();
    QUrl fileName = editor->fileName();
    ui->actionRename->setEnabled(!fileName.isEmpty());
    ui->actionMove_to_New_Window->setEnabled(isClean);
    ui->actionOpen_in_New_Window->setEnabled(isClean);

    // EOL
    QString eol = editor->endOfLineSequence();
    if (eol == "\r\n") {
        ui->actionWindows_Format->setChecked(true);
        m_statusBar_EOLstyle->setText(tr("Windows"));
    } else if (eol == "\n") {
        ui->actionUNIX_Format->setChecked(true);
        m_statusBar_EOLstyle->setText(tr("UNIX / OS X"));
    } else if (eol == "\r") {
        ui->actionMac_Format->setChecked(true);
        m_statusBar_EOLstyle->setText(tr("Old Mac"));
    }

    // Encoding
    QString encoding;
    if (editor->codec()->mibEnum() == MIB_UTF_8 && !editor->bom()) {
        // Is UTF-8 without BOM
        encoding = tr("%1 w/o BOM").arg(QString::fromUtf8(editor->codec()->name()));
    } else {
        encoding = QString::fromUtf8(editor->codec()->name());
    }
    m_statusBar_textFormat->setText(encoding);

    // Indentation
    if (editor->isUsingCustomIndentationMode()) {
        ui->actionIndentation_Custom->setChecked(true);
    } else {
        ui->actionIndentation_Default_settings->setChecked(true);
    }
}

void MainWindow::on_action_Delete_triggered()
{
    currentEditor()->setSelectionsText(QStringList(""));
}

void MainWindow::on_actionSelect_All_triggered()
{
    currentEditor()->sendMessage("C_CMD_SELECT_ALL");
}

void MainWindow::on_actionAbout_Notepadqq_triggered()
{
    frmAbout *_about;
    _about = new frmAbout(this);
    _about->exec();

    _about->deleteLater();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_action_Undo_triggered()
{
    currentEditor()->sendMessage("C_CMD_UNDO");
}

void MainWindow::on_action_Redo_triggered()
{
    currentEditor()->sendMessage("C_CMD_REDO");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);

    int tabWidgetsCount = m_topEditorContainer->count();
    for(int i = 0; i < tabWidgetsCount; i++) {
        EditorTabWidget *tabWidget = m_topEditorContainer->tabWidget(i);
        int tabCount = tabWidget->count();

        for(int j = 0; j < tabCount; j++) {
            int closeResult = closeTab(tabWidget, j, false, false);
            if (closeResult == MainWindow::tabCloseResult_Canceled) {
                // Cancel all
                event->ignore();
                return;
            }
        }
    }

    m_settings->beginGroup("MainWindow");
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("windowState", saveState());
    m_settings->endGroup();

    // Disconnect signals to avoid handling events while
    // the UI is being destroyed.
    disconnect(m_topEditorContainer, 0, this, 0);
}

void MainWindow::on_actionE_xit_triggered()
{
    close();
}

void MainWindow::instantiateFrmSearchReplace()
{
    if (!m_frmSearchReplace) {
        m_frmSearchReplace = new frmSearchReplace(
                            m_topEditorContainer,
                            this);

        connect(m_frmSearchReplace, &frmSearchReplace::fileSearchResultFinished,
                this, &MainWindow::on_fileSearchResultFinished);
    }
}

void MainWindow::on_fileSearchResultFinished(FileSearchResult::SearchResult result)
{
    m_fileSearchResultsWidget->addSearchResult(result);
    ui->dockFileSearchResults->show();
}

void MainWindow::on_actionSearch_triggered()
{
    if (!m_frmSearchReplace) {
        instantiateFrmSearchReplace();
    }

    QStringList sel = currentEditor()->selectedTexts();
    if (sel.length() > 0 && sel[0].length() > 0) {
        m_frmSearchReplace->setSearchText(sel[0]);
    }

    m_frmSearchReplace->show(frmSearchReplace::TabSearch);
    m_frmSearchReplace->activateWindow();
}

void MainWindow::on_actionCurrent_Full_File_path_to_Clipboard_triggered()
{
    Editor *editor = currentEditor();
    if (currentEditor()->fileName().isEmpty())
    {
        EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
        QApplication::clipboard()->setText(tabWidget->tabText(tabWidget->indexOf(editor)));
    } else {
        QApplication::clipboard()->setText(
                    editor->fileName().toDisplayString(QUrl::PreferLocalFile |
                                                       QUrl::RemovePassword));
    }
}

void MainWindow::on_actionCurrent_Filename_to_Clipboard_triggered()
{
    Editor *editor = currentEditor();
    if (currentEditor()->fileName().isEmpty())
    {
        EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
        QApplication::clipboard()->setText(tabWidget->tabText(tabWidget->indexOf(editor)));
    } else {
        QApplication::clipboard()->setText(Notepadqq::fileNameFromUrl(editor->fileName()));
    }
}

void MainWindow::on_actionCurrent_Directory_Path_to_Clipboard_triggered()
{
    Editor *editor = currentEditor();
    if(currentEditor()->fileName().isEmpty())
    {
        QApplication::clipboard()->setText("");
    } else {
        QApplication::clipboard()->setText(
                    editor->fileName().toDisplayString(QUrl::RemovePassword |
                                                       QUrl::RemoveUserInfo |
                                                       QUrl::RemovePort |
                                                       QUrl::RemoveAuthority |
                                                       QUrl::RemoveQuery |
                                                       QUrl::RemoveFragment |
                                                       QUrl::PreferLocalFile |
                                                       QUrl::RemoveFilename |
                                                       QUrl::NormalizePathSegments
                                                       ));
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    frmPreferences *_pref;
    _pref = new frmPreferences(m_topEditorContainer, this);
    _pref->exec();

    _pref->deleteLater();
}

void MainWindow::on_actionClose_triggered()
{
    closeTab(m_topEditorContainer->currentTabWidget(),
             m_topEditorContainer->currentTabWidget()->currentIndex());
}

void MainWindow::on_actionC_lose_All_triggered()
{
    bool canceled = false;

    // Save what needs to be saved, check if user wants to cancel the closing
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor */*editor*/) {
        int closeResult = closeTab(tabWidget, editorId, false, false);
        if (closeResult == MainWindow::tabCloseResult_Canceled) {
            canceled = true;
            return false; // Cancel all
        } else {
            return true;
        }
    });

    if (!canceled) {
        m_topEditorContainer->forEachEditor(true, [&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor */*editor*/) {
            closeTab(tabWidget, editorId, true, true);
            return true;
        });
    }
}

void MainWindow::on_fileOnDiskChanged(EditorTabWidget *tabWidget, int tab, bool removed)
{
    Editor *editor = tabWidget->editor(tab);

    if (removed) {
        BannerFileRemoved *banner = new BannerFileRemoved(this);
        banner->setObjectName("fileremoved");
        editor->insertBanner(banner);

        connect(banner, &BannerFileRemoved::ignore, this, [=]() {
            editor->removeBanner(banner);
            editor->setFocus();
        });

        connect(banner, &BannerFileRemoved::save, this, [=]() {
            save(tabWidget, tab);
        });

    } else {
        BannerFileChanged *banner = new BannerFileChanged(this);
        banner->setObjectName("filechanged");
        editor->insertBanner(banner);

        connect(banner, &BannerFileChanged::ignore, this, [=]() {
            editor->removeBanner(banner);
            editor->setFocus();
            // FIXME Set editor as clean
        });

        connect(banner, &BannerFileChanged::reload, this, [=]() {
            editor->removeBanner(banner);
            editor->setFocus();

            m_docEngine->reloadDocument(tabWidget, tab);
        });
    }
}

void MainWindow::on_actionReplace_triggered()
{
    if (!m_frmSearchReplace) {
        instantiateFrmSearchReplace();
    }

    QStringList sel = currentEditor()->selectedTexts();
    if (sel.length() > 0 && sel[0].length() > 0) {
        m_frmSearchReplace->setSearchText(sel[0]);
    }

    m_frmSearchReplace->show(frmSearchReplace::TabReplace);
    m_frmSearchReplace->activateWindow();
}

void MainWindow::on_actionPlain_text_triggered()
{
    currentEditor()->setLanguage("plaintext");
}

void MainWindow::on_actionRestore_Default_Zoom_triggered()
{
    m_topEditorContainer->currentTabWidget()->setZoomFactor(1);
    m_settings->setValue("zoom", 1);
}

void MainWindow::on_actionZoom_In_triggered()
{
    qreal curZoom = currentEditor()->zoomFactor();
    qreal newZoom = curZoom + 0.25;
    m_topEditorContainer->currentTabWidget()->setZoomFactor(newZoom);
    m_settings->setValue("zoom", newZoom);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    qreal curZoom = currentEditor()->zoomFactor();
    qreal newZoom = curZoom - 0.25;
    m_topEditorContainer->currentTabWidget()->setZoomFactor(newZoom);
    m_settings->setValue("zoom", newZoom);
}

void MainWindow::on_editorMouseWheel(EditorTabWidget *tabWidget, int tab, QWheelEvent *ev)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        qreal curZoom = tabWidget->editor(tab)->zoomFactor();
        qreal diff = ev->delta() / 120;
        diff /= 10;

        // Increment/Decrement zoom factor by 0.1 at each step.
        qreal newZoom = curZoom + diff;
        tabWidget->setZoomFactor(newZoom);
        m_settings->setValue("zoom", newZoom);
    }
}

void MainWindow::transformSelectedText(std::function<QString (const QString &)> func)
{
    Editor *editor = currentEditor();
    QStringList sel = editor->selectedTexts();

    for (int i = 0; i < sel.length(); i++) {
        sel.replace(i, func(sel.at(i)));
    }

    editor->setSelectionsText(sel, Editor::selectMode_selected);
}

void MainWindow::on_actionUPPERCASE_triggered()
{
    transformSelectedText([](const QString &str) {
        return str.toUpper();
    });
}

void MainWindow::on_actionLowercase_triggered()
{
    transformSelectedText([](const QString &str) {
        return str.toLower();
    });
}

void MainWindow::on_actionClose_All_BUT_Current_Document_triggered()
{
    Editor *keepOpen = currentEditor();
    bool canceled = false;

    // Save what needs to be saved, check if user wants to cancel the closing
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor *editor) {
        if (keepOpen == editor)
            return true;

        int closeResult = closeTab(tabWidget, editorId, false, false);
        if (closeResult == MainWindow::tabCloseResult_Canceled) {
            canceled = true;
            return false; // Cancel all
        } else {
            return true;
        }
    });

    if (!canceled) {
        m_topEditorContainer->forEachEditor(true, [&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor *editor) {
            if (keepOpen == editor)
                return true;

            closeTab(tabWidget, editorId, true, true);
            return true;
        });
    }

}

void MainWindow::on_actionSave_All_triggered()
{
    // No tab must get closed (or added) while we're iterating!!
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int editorId, EditorTabWidget *tabWidget, Editor *editor) {
        if (editor->isClean()) {
            return true;
        } else {
            tabWidget->setCurrentIndex(editorId);
            int result = save(tabWidget, editorId);
            return (result != saveFileResult_Canceled);
        }
    });
}

void MainWindow::on_bannerRemoved(QWidget *banner)
{
    delete banner;
}

void MainWindow::on_documentSaved(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    editor->removeBanner("filechanged");
    editor->removeBanner("fileremoved");

    if (editor == currentEditor()) {
        ui->actionRename->setEnabled(true);
    }
}

void MainWindow::on_documentReloaded(EditorTabWidget *tabWidget, int tab)
{
    Editor *editor = tabWidget->editor(tab);
    editor->removeBanner("filechanged");
    editor->removeBanner("fileremoved");

    if (currentEditor() == editor) {
        refreshEditorUiInfo(editor);
        refreshEditorUiCursorInfo(editor);
    }
}

void MainWindow::on_documentLoaded(EditorTabWidget *tabWidget, int tab, bool wasAlreadyOpened)
{
    Editor *editor = tabWidget->editor(tab);

    const int MAX_RECENT_ENTRIES = 10;

    QUrl newUrl = editor->fileName();
    QList<QVariant> recentDocs = m_settings->value("recentDocuments", QList<QVariant>()).toList();
    recentDocs.insert(0, QVariant(newUrl));

    // Remove duplicates
    for (int i = recentDocs.count() - 1; i >= 1; i--) {
        if (newUrl == recentDocs[i].toUrl())
            recentDocs.removeAt(i);
    }

    while (recentDocs.count() > MAX_RECENT_ENTRIES)
        recentDocs.removeLast();

    m_settings->setValue("recentDocuments", QVariant(recentDocs));

    updateRecentDocsInMenu();

    if (!wasAlreadyOpened) {
        if (m_settings->value("warnForDifferentIndentation", true).toBool()) {
            checkIndentationMode(editor);
        }
    }
}

void MainWindow::checkIndentationMode(Editor *editor)
{
    bool found = false;
    Editor::IndentationMode detected = editor->detectDocumentIndentation(&found);
    if (found) {
        Editor::IndentationMode curr = editor->indentationMode();
        bool differentTabSpaces = detected.useTabs != curr.useTabs;
        bool differentSpaceSize = detected.useTabs == false && curr.useTabs == false && detected.size != curr.size;

        if (differentTabSpaces || differentSpaceSize) {
            // Show msg
            BannerIndentationDetected *banner = new BannerIndentationDetected(
                        differentSpaceSize,
                        detected,
                        curr,
                        this);
            banner->setObjectName("indentationdetected");

            editor->insertBanner(banner);

            connect(banner, &BannerIndentationDetected::useApplicationSettings, this, [=]() {
                editor->removeBanner(banner);
                editor->setFocus();
            });

            connect(banner, &BannerIndentationDetected::useDocumentSettings, this, [=]() {
                editor->removeBanner(banner);
                if (detected.useTabs) {
                    editor->setCustomIndentationMode(true);
                } else {
                    editor->setCustomIndentationMode(detected.useTabs, detected.size);
                }
                ui->actionIndentation_Custom->setChecked(true);
                editor->setFocus();
            });
        }
    }
}

void MainWindow::updateRecentDocsInMenu()
{
    QList<QVariant> recentDocs = m_settings->value("recentDocuments", QList<QVariant>()).toList();

    ui->menuRecent_Files->clear();

    QList<QAction *> actions;
    for (QVariant recentDoc : recentDocs) {
        QUrl url = recentDoc.toUrl();
        QAction *action = new QAction(Notepadqq::fileNameFromUrl(url), this);
        connect(action, &QAction::triggered, this, [=]() {
            m_docEngine->loadDocument(url, m_topEditorContainer->currentTabWidget());
        });

        actions.append(action);
    }

    // If there are no recent files, show a placeholder
    bool anyRecentDoc = (actions.count() != 0);
    if (!anyRecentDoc) {
        QAction *action = new QAction(tr("No recent files"), this);
        action->setEnabled(false);
        actions.append(action);
    }

    ui->menuRecent_Files->addActions(actions);

    if (anyRecentDoc) {
        ui->menuRecent_Files->addSeparator();
        ui->menuRecent_Files->addActions({ui->actionOpen_All_Recent_Files,
                                          ui->actionEmpty_Recent_Files_List});
    }
}

void MainWindow::on_actionReload_from_Disk_triggered()
{
    EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
    Editor *editor = tabWidget->currentEditor();

    reloadWithWarning(tabWidget,
                      tabWidget->currentIndex(),
                      editor->codec(),
                      editor->bom());
}

void MainWindow::on_actionFind_Next_triggered()
{
    if (m_frmSearchReplace)
        m_frmSearchReplace->findFromUI(true);
}

void MainWindow::on_actionFind_Previous_triggered()
{
    if (m_frmSearchReplace)
        m_frmSearchReplace->findFromUI(false);
}

void MainWindow::on_actionRename_triggered()
{
    EditorTabWidget *tabW = m_topEditorContainer->currentTabWidget();
    QUrl oldFilename = tabW->currentEditor()->fileName();
    int result = saveAs(tabW, tabW->currentIndex(), false);

    if (result == saveFileResult_Saved && !oldFilename.isEmpty()) {

        if (QFileInfo(oldFilename.toLocalFile()) != QFileInfo(tabW->currentEditor()->fileName().toLocalFile())) {

            // Remove the old file
            QString filename = oldFilename.toLocalFile();
            if (QFile::exists(filename)) {
                if(!QFile::remove(filename)) {
                    QMessageBox::warning(this, QApplication::applicationName(),
                                         QString("Error: unable to remove file %1")
                                         .arg(filename));
                }
            }
        }

    }
}

void MainWindow::on_actionWord_wrap_toggled(bool on)
{
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setLineWrap(on);
        return true;
    });
    m_settings->setValue("wordWrap", on);
}

void MainWindow::on_actionEmpty_Recent_Files_List_triggered()
{
    m_settings->remove("recentDocuments");
    updateRecentDocsInMenu();
}

void MainWindow::on_actionOpen_All_Recent_Files_triggered()
{
    QList<QVariant> recentDocs = m_settings->value("recentDocuments", QList<QVariant>()).toList();

    QList<QUrl> convertedList;
    for (QVariant doc : recentDocs) {
        convertedList.append(doc.toUrl());
    }

    m_docEngine->loadDocuments(convertedList, m_topEditorContainer->currentTabWidget());
}

void MainWindow::on_actionUNIX_Format_triggered()
{
    Editor *editor = currentEditor();
    editor->setEndOfLineSequence("\n");
    editor->markDirty();
}

void MainWindow::on_actionWindows_Format_triggered()
{
    Editor *editor = currentEditor();
    editor->setEndOfLineSequence("\r\n");
    editor->markDirty();
}

void MainWindow::on_actionMac_Format_triggered()
{
    Editor *editor = currentEditor();
    editor->setEndOfLineSequence("\r");
    editor->markDirty();
}

void MainWindow::convertEditorEncoding(Editor *editor, QTextCodec *codec, bool bom)
{
    editor->setCodec(codec);
    editor->setBom(bom);
    editor->markDirty();

    if (editor == currentEditor())
        refreshEditorUiInfo(editor);
}

void MainWindow::on_actionUTF_8_triggered()
{
    convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), true);
}

void MainWindow::on_actionUTF_8_without_BOM_triggered()
{
    convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), false);
}

void MainWindow::on_actionUTF_16BE_triggered()
{
    convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-16BE"), true);
}

void MainWindow::on_actionUTF_16LE_triggered()
{
    convertEditorEncoding(currentEditor(), QTextCodec::codecForName("UTF-16LE"), true);
}

void MainWindow::on_actionInterpret_as_UTF_8_triggered()
{
    m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), true);
}

void MainWindow::on_actionInterpret_as_UTF_8_without_BOM_triggered()
{
    m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-8"), false);
}

void MainWindow::on_actionInterpret_as_UTF_16BE_UCS_2_Big_Endian_triggered()
{
    m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-16BE"), true);
}

void MainWindow::on_actionInterpret_as_UTF_16LE_UCS_2_Little_Endian_triggered()
{
    m_docEngine->reinterpretEncoding(currentEditor(), QTextCodec::codecForName("UTF-16LE"), true);
}

void MainWindow::on_actionShow_Tabs_toggled(bool on)
{
    m_topEditorContainer->forEachEditor([&](const int /*tabWidgetId*/, const int /*editorId*/, EditorTabWidget */*tabWidget*/, Editor *editor) {
        editor->setTabsVisible(on);
        return true;
    });
    m_settings->setValue("tabsVisible", on);
}

void MainWindow::on_actionConvert_to_triggered()
{
    Editor *editor = currentEditor();
    frmEncodingChooser *dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Convert to:"));

    if (dialog->exec() == QDialog::Accepted) {
        convertEditorEncoding(editor, dialog->selectedCodec(), false);
    }

    dialog->deleteLater();
}

void MainWindow::on_actionReload_file_interpreted_as_triggered()
{
    Editor *editor = currentEditor();
    frmEncodingChooser *dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Reload as:"));

    if (dialog->exec() == QDialog::Accepted) {
        EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
        reloadWithWarning(tabWidget, tabWidget->currentIndex(), dialog->selectedCodec(), false);
    }

    dialog->deleteLater();
}

void MainWindow::on_actionIndentation_Default_settings_triggered()
{
    currentEditor()->clearCustomIndentationMode();
}

void MainWindow::on_actionIndentation_Custom_triggered()
{
    Editor *editor = currentEditor();

    frmIndentationMode *dialog = new frmIndentationMode(this);
    dialog->populateWidgets(editor->indentationMode());

    if (dialog->exec() == QDialog::Accepted) {
        Editor::IndentationMode indent = dialog->indentationMode();
        editor->setCustomIndentationMode(indent.useTabs, indent.size);
    }

    // Make sure the UI is consistent even if the user canceled the dialog.
    if (editor->isUsingCustomIndentationMode()) {
        ui->actionIndentation_Custom->setChecked(true);
    } else {
        ui->actionIndentation_Default_settings->setChecked(true);
    }

    dialog->deleteLater();
}

void MainWindow::on_actionInterpret_as_triggered()
{
    Editor *editor = currentEditor();
    frmEncodingChooser *dialog = new frmEncodingChooser(this);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(tr("Interpret as:"));

    if (dialog->exec() == QDialog::Accepted) {
        m_docEngine->reinterpretEncoding(editor, dialog->selectedCodec(), false);
    }

    dialog->deleteLater();
}

void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer);
    if (dialog.exec() == QDialog::Accepted)
          currentEditor()->print(&printer);
}

void MainWindow::on_actionPrint_Now_triggered()
{
    QPrinter printer(QPrinter::HighResolution);
    currentEditor()->print(&printer);
}

void MainWindow::on_actionLaunch_in_Firefox_triggered()
{
    QUrl fileName = currentEditor()->fileName();
    if (!fileName.isEmpty()) {
        QStringList args;
        args << fileName.toString(QUrl::None);
        QProcess::startDetached("firefox", args);
    }
}

void MainWindow::on_actionLaunch_in_Chromium_triggered()
{
    QUrl fileName = currentEditor()->fileName();
    if (!fileName.isEmpty()) {
        QStringList args;
        args << fileName.toString(QUrl::None);
        QProcess::startDetached("chromium-browser", args);
    }
}

void MainWindow::on_actionLaunch_in_Chrome_triggered()
{
    QUrl fileName = currentEditor()->fileName();
    if (!fileName.isEmpty()) {
        QStringList args;
        args << fileName.toString(QUrl::None);
        QProcess::startDetached("google-chrome", args);
    }
}

QStringList MainWindow::currentWordOrSelections()
{
    Editor *editor = currentEditor();
    QStringList selection = editor->selectedTexts();

    if (selection.isEmpty() || selection.first().isEmpty()) {
        return QStringList(editor->getCurrentWord());
    } else {
        return selection;
    }
}

QString MainWindow::currentWordOrSelection()
{
    QStringList terms = currentWordOrSelections();
    if (terms.isEmpty()) {
        return QString();
    } else {
        return terms.first();
    }
}

void MainWindow::currentWordOnlineSearch(const QString &searchUrl)
{
    QString term = currentWordOrSelection();

    if (!term.isNull() && !term.isEmpty()) {
        QUrl phpHelp = QUrl(searchUrl.arg(QString(QUrl::toPercentEncoding(term))));
        QDesktopServices::openUrl(phpHelp);
    }
}

void MainWindow::on_actionGet_php_help_triggered()
{
    currentWordOnlineSearch("https://php.net/%1");
}

void MainWindow::on_actionGoogle_Search_triggered()
{
    currentWordOnlineSearch("https://www.google.com/?#q=%1");
}

void MainWindow::on_actionWikipedia_Search_triggered()
{
    currentWordOnlineSearch("https://en.wikipedia.org/?search=%1");
}

void MainWindow::on_actionOpen_a_New_Window_triggered()
{
    MainWindow *b = new MainWindow(QStringList(), 0);
    b->show();
}

void MainWindow::on_actionOpen_in_New_Window_triggered()
{
    QStringList args;
    args.append(QApplication::arguments().first());
    if (!currentEditor()->fileName().isEmpty()) {
        args.append(currentEditor()->fileName().toString(QUrl::None));
    }

    MainWindow *b = new MainWindow(args, 0);
    b->show();
}

void MainWindow::on_actionMove_to_New_Window_triggered()
{
    QStringList args;
    args.append(QApplication::arguments().first());
    if (!currentEditor()->fileName().isEmpty()) {
        args.append(currentEditor()->fileName().toString(QUrl::None));
    }

    EditorTabWidget *tabWidget = m_topEditorContainer->currentTabWidget();
    int tab = tabWidget->currentIndex();
    if (closeTab(tabWidget, tab) != tabCloseResult_Canceled) {
        MainWindow *b = new MainWindow(args, 0);
        b->show();
    }
}

void MainWindow::on_actionOpen_file_triggered()
{
    QStringList terms = currentWordOrSelections();
    if (!terms.isEmpty()) {
        QList<QUrl> urls;
        for (QString term : terms) {
            urls.append(QUrl::fromLocalFile(term));
        }

        m_docEngine->loadDocuments(urls,
                               m_topEditorContainer->currentTabWidget());
    }
}

void MainWindow::on_actionOpen_in_another_window_triggered()
{
    QStringList terms = currentWordOrSelections();
    if (!terms.isEmpty()) {
        terms.prepend(QApplication::arguments().first());

        MainWindow *b = new MainWindow(terms, 0);
        b->show();
    }
}

void MainWindow::on_tabBarDoubleClicked(EditorTabWidget *tabWidget, int tab)
{
    if (tab == -1) {
        m_docEngine->addNewDocument(getNewDocumentName(), true, tabWidget);
    }
}

void MainWindow::on_actionFind_in_Files_triggered()
{
    if (!m_frmSearchReplace) {
        instantiateFrmSearchReplace();
    }

    QStringList sel = currentEditor()->selectedTexts();
    if (sel.length() > 0 && sel[0].length() > 0) {
        m_frmSearchReplace->setSearchText(sel[0]);
    }

    m_frmSearchReplace->show(frmSearchReplace::TabSearchInFiles);
    m_frmSearchReplace->activateWindow();
}

void MainWindow::on_actionDelete_Line_triggered()
{
    currentEditor()->sendMessage("C_CMD_DELETE_LINE");
}

void MainWindow::on_actionDuplicate_Line_triggered()
{
    currentEditor()->sendMessage("C_CMD_DUPLICATE_LINE");
}

void MainWindow::on_resultMatchClicked(const FileSearchResult::FileResult &file, const FileSearchResult::Result &match)
{
    QUrl url = stringToUrl(file.fileName);
    m_docEngine->loadDocument(url,
                              m_topEditorContainer->currentTabWidget());

    QPair<int, int> pos = m_docEngine->findOpenEditorByUrl(url);

    if (pos.first == -1 || pos.second == -1)
        return;

    EditorTabWidget *tabW = m_topEditorContainer->tabWidget(pos.first);
    Editor *editor = tabW->editor(pos.second);

    editor->setSelection(match.matchStartLine, match.matchStartCol, match.matchEndLine, match.matchEndCol);

    editor->setFocus();
}

void MainWindow::on_actionTrim_Trailing_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_TRIM_TRAILING_SPACE");
}

void MainWindow::on_actionTrim_Leading_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_TRIM_LEADING_SPACE");
}

void MainWindow::on_actionTrim_Leading_and_Trailing_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_TRIM_LEADING_TRAILING_SPACE");
}

void MainWindow::on_actionEOL_to_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_EOL_TO_SPACE");
}

void MainWindow::on_actionTAB_to_Space_triggered()
{
    currentEditor()->sendMessage("C_CMD_TAB_TO_SPACE");
}

void MainWindow::on_actionSpace_to_TAB_All_triggered()
{
    currentEditor()->sendMessage("C_CMD_SPACE_TO_TAB_ALL");
}

void MainWindow::on_actionSpace_to_TAB_Leading_triggered()
{
    currentEditor()->sendMessage("C_CMD_SPACE_TO_TAB_LEADING");
}

void MainWindow::on_actionGo_to_line_triggered()
{
    Editor *editor = currentEditor();
    int currentLine = editor->cursorPosition().first;
    int lines = editor->lineCount();
    frmLineNumberChooser *frm = new frmLineNumberChooser(1, lines, currentLine + 1, this);
    if (frm->exec() == QDialog::Accepted) {
        int line = frm->value();
        editor->setSelection(line - 1, 0, line - 1, 0);
    }
}

void MainWindow::on_actionInstall_Extension_triggered()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Extension"), QString(), "Notepadqq extensions (*.nqqext)");
    if (!file.isNull()) {
        Extensions::InstallExtension *installExt = new Extensions::InstallExtension(file, this);
        installExt->exec();
        installExt->deleteLater();
    }
}

void MainWindow::showExtensionsMenu(bool show)
{
    ui->menuExtensions->menuAction()->setVisible(show);
}
