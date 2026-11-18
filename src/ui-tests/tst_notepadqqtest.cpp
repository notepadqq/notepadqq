#include "include/EditorNS/editor.h"
#include "include/editoruicontroller.h"
#include "include/mainwindow.h"
#include "include/notepadqq.h"

#include <QCloseEvent>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QTemporaryDir>
#include <QtTest>

class NotepadqqTest : public QObject {
    Q_OBJECT

public:
    NotepadqqTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void editorPathIsHtml();
    void acceptedCloseDisconnectsEditorUiController();
    void directWindowDeletionDestroysEditorUiControllerFirst();
    void openingFileReplacesUntouchedNewTab();
    void openingFileKeepsTabThatIsStillLoading();

private:
    QTemporaryDir m_settingsDirectory;
};

NotepadqqTest::NotepadqqTest() {}

void NotepadqqTest::initTestCase()
{
    QVERIFY(m_settingsDirectory.isValid());
    QCoreApplication::setOrganizationName("Notepadqq-tests");
    QCoreApplication::setApplicationName("Notepadqq-tests");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDirectory.path());
}

void NotepadqqTest::cleanupTestCase()
{
    EditorNS::Editor::invalidateEditorBuffer();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

void NotepadqqTest::editorPathIsHtml()
{ QVERIFY(Notepadqq::editorPath().endsWith(".html")); }

// Returns the real editor UI controller owned directly by the window.
static EditorUiController* editorUiController(MainWindow* window)
{
    for (QObject* child : window->children()) {
        if (auto* controller = dynamic_cast<EditorUiController*>(child))
            return controller;
    }
    return nullptr;
}

// Ensures accepted close disconnects container and direct-editor callbacks before deferred deletion.
void NotepadqqTest::acceptedCloseDisconnectsEditorUiController()
{
    auto* window = new MainWindow(QStringList());
    window->setAttribute(Qt::WA_DeleteOnClose, false);

    QPointer<EditorUiController> controller = editorUiController(window);
    QVERIFY(controller);
    EditorNS::Editor* editor = window->currentEditor();
    EditorTabWidget* tabWidget = window->topEditorContainer()->currentTabWidget();
    const int tab = tabWidget->currentIndex();
    QCloseEvent closeEvent;
    QCoreApplication::sendEvent(window, &closeEvent);
    QVERIFY(closeEvent.isAccepted());
    QVERIFY(controller);

    const QString sentinelTitle("closed-window-sentinel");
    window->setWindowTitle(sentinelTitle);
    QVERIFY(QMetaObject::invokeMethod(window->topEditorContainer(),
        "currentEditorChanged",
        Qt::DirectConnection,
        Q_ARG(EditorTabWidget*, tabWidget),
        Q_ARG(int, tab)));
    QCOMPARE(window->windowTitle(), sentinelTitle);

    QVERIFY(QMetaObject::invokeMethod(editor,
        "currentLanguageChanged",
        Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("plaintext")),
        Q_ARG(QString, QStringLiteral("Plain Text"))));
    QCOMPARE(window->windowTitle(), sentinelTitle);

    delete window;
}

// Ensures direct deletion tears down the controller before MainWindow's QObject destruction phase.
void NotepadqqTest::directWindowDeletionDestroysEditorUiControllerFirst()
{
    auto* window = new MainWindow(QStringList());
    EditorUiController* controller = editorUiController(window);
    QVERIFY(controller);

    QStringList destructionOrder;
    connect(controller, &QObject::destroyed, this, [&destructionOrder] { destructionOrder.append("controller"); });
    connect(window, &QObject::destroyed, this, [&destructionOrder] { destructionOrder.append("window"); });

    delete window;

    QCOMPARE(destructionOrder, QStringList({"controller", "window"}));
}

// Creates an empty file in the given directory and returns its URL.
static QUrl createEmptyFile(const QTemporaryDir& directory, const QString& fileName)
{
    QFile file(directory.filePath(fileName));
    if (!file.open(QIODevice::WriteOnly))
        return QUrl();
    return QUrl::fromLocalFile(file.fileName());
}

// Guards the intended behaviour: the untouched "new 1" tab of a fresh window gives way to the opened file.
void NotepadqqTest::openingFileReplacesUntouchedNewTab()
{
    QTemporaryDir files;
    QVERIFY(files.isValid());
    const QUrl openedUrl = createEmptyFile(files, "opened.txt");
    QVERIFY(!openedUrl.isEmpty());

    auto* window = new MainWindow(QStringList());
    EditorTabWidget* tabWidget = window->topEditorContainer()->currentTabWidget();
    QCOMPARE(tabWidget->count(), 1);

    window->getDocEngine()->getDocumentLoader().setUrl(openedUrl).setTabWidget(tabWidget).execute().wait();

    QCOMPARE(tabWidget->count(), 1);
    QCOMPARE(tabWidget->editor(0)->filePath(), openedUrl);

    delete window;
}

// Ensures opening a file does not close a tab whose background load has not finished yet.
//
// On startup the session restores tabs with DocEngine::loadDocumentsInBackground(), which marks each editor as
// isLoading until its content has been read. An unsaved tab restored from an empty cache file looks exactly like
// the untouched "new 1" tab meanwhile: no file path and clean. When a file passed on the command line was opened
// in that window, the foreground loader closed the restored tab, and the pending session callback then crashed on
// a tab index of -1 (Editor::tabName() called on a null editor).
void NotepadqqTest::openingFileKeepsTabThatIsStillLoading()
{
    QTemporaryDir files;
    QVERIFY(files.isValid());
    const QUrl openedUrl = createEmptyFile(files, "opened.txt");
    QVERIFY(!openedUrl.isEmpty());

    auto* window = new MainWindow(QStringList());
    EditorTabWidget* tabWidget = window->topEditorContainer()->currentTabWidget();
    QCOMPARE(tabWidget->count(), 1);

    // Put the only tab into the state of a restored tab whose background load is still running. Setting the flag
    // directly keeps the test deterministic instead of depending on the timing of a real background load.
    QPointer<EditorNS::Editor> loadingEditor = tabWidget->editor(0);
    QVERIFY(loadingEditor->filePath().isEmpty());
    loadingEditor->isLoading = true;

    window->getDocEngine()->getDocumentLoader().setUrl(openedUrl).setTabWidget(tabWidget).execute().wait();

    QVERIFY(loadingEditor);
    QCOMPARE(tabWidget->count(), 2);
    QCOMPARE(tabWidget->indexOf(loadingEditor), 0);
    QCOMPARE(tabWidget->editor(1)->filePath(), openedUrl);

    loadingEditor->isLoading = false;
    delete window;
}

QTEST_MAIN(NotepadqqTest)

#include "tst_notepadqqtest.moc"
