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

QTEST_MAIN(NotepadqqTest)

#include "tst_notepadqqtest.moc"
