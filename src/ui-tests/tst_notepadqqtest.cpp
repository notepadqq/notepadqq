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
    void readToStringDetectsEncoding_data();
    void readToStringDetectsEncoding();

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

// Encodes text with the named codec, as an editor saving in that encoding would write it to disk.
static QByteArray encodeAs(const char* codecName, const QString& text)
{ return QTextCodec::codecForName(codecName)->fromUnicode(text); }

// Every row is a file content and the text a user expects to see when opening it. The expected codec is only
// checked where it is unambiguous: for single-byte encodings uchardet may pick any codec that decodes the
// sample correctly, e.g. ISO-8859-3 instead of ISO-8859-1 for German text.
void NotepadqqTest::readToStringDetectsEncoding_data()
{
    QTest::addColumn<QByteArray>("contents");
    QTest::addColumn<QString>("expectedText");
    QTest::addColumn<QByteArray>("expectedCodec");

    // A long ASCII part pushes the non-ASCII bytes beyond the 64 KiB uchardet used to look at.
    const QString asciiFiller = QStringLiteral("This line is plain ASCII text used as filler.\n").repeated(1600);
    // Source code with a single umlaut in a comment: uchardet took this for ISO-8859-1.
    const QString code = QStringLiteral("int add(int a, int b)\n{\n    return a + b;\n}\n").repeated(3);
    const QString codeWithUmlaut = code + QStringLiteral("// Prüfung\n") + code;

    // UTF-8 that uchardet guessed wrong, shown as "Ã„rger", "StraÃŸe" or "PrÃ¼fung" before.
    QTest::newRow("utf8-capital-umlaut") << QStringLiteral("Ärger\n").toUtf8() << QStringLiteral("Ärger\n")
                                         << QByteArray("UTF-8");
    QTest::newRow("utf8-sharp-s") << QStringLiteral("Straße\n").toUtf8() << QStringLiteral("Straße\n")
                                  << QByteArray("UTF-8");
    QTest::newRow("utf8-single-umlaut-in-code") << codeWithUmlaut.toUtf8() << codeWithUmlaut << QByteArray("UTF-8");

    // Behaviour that must not change.
    QTest::newRow("utf8-sentence") << QStringLiteral("Die Größe der Straße ändert sich.\n").toUtf8()
                                   << QStringLiteral("Die Größe der Straße ändert sich.\n") << QByteArray("UTF-8");
    QTest::newRow("ascii-is-utf8") << QByteArray("Just ASCII.\n") << QStringLiteral("Just ASCII.\n")
                                   << QByteArray("UTF-8");
    QTest::newRow("empty-is-utf8") << QByteArray() << QString() << QByteArray("UTF-8");
    QTest::newRow("latin1-sentence") << encodeAs("ISO-8859-1", QStringLiteral("Die Größe der Straße ändert sich.\n"))
                                     << QStringLiteral("Die Größe der Straße ändert sich.\n") << QByteArray();
    QTest::newRow("windows1251-russian") << encodeAs("windows-1251",
                                                QStringLiteral("Привет, это тестовый текст на русском языке.\n"))
                                         << QStringLiteral("Привет, это тестовый текст на русском языке.\n")
                                         << QByteArray("windows-1251");

    // Latin-1 after a long ASCII part was decoded as UTF-8, turning every umlaut into U+FFFD.
    QTest::newRow("latin1-after-64k") << encodeAs("ISO-8859-1", asciiFiller + QStringLiteral("Grüße\n"))
                                      << asciiFiller + QStringLiteral("Grüße\n") << QByteArray();
}

// Ensures files are decoded with the encoding they were written in when no BOM tells it.
void NotepadqqTest::readToStringDetectsEncoding()
{
    QFETCH(QByteArray, contents);
    QFETCH(QString, expectedText);
    QFETCH(QByteArray, expectedCodec);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QFile file(directory.filePath("sample.txt"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write(contents), contents.size());
    file.close();

    const DocEngine::DecodedText decoded = DocEngine::readToString(&file);

    QVERIFY(!decoded.error);
    QVERIFY(decoded.codec != nullptr);
    QVERIFY(!decoded.bom);
    QCOMPARE(decoded.text, expectedText);
    if (!expectedCodec.isEmpty()) {
        QCOMPARE(decoded.codec->name(), expectedCodec);
    }
}

QTEST_MAIN(NotepadqqTest)

#include "tst_notepadqqtest.moc"
