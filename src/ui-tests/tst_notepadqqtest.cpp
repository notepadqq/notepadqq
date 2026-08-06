#include "include/EditorNS/editor.h"
#include "include/editoruicontroller.h"
#include "include/mainwindow.h"
#include "include/notepadqq.h"
#include "include/nqqfiledialog.h"
#include "include/nqqsettings.h"

#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileSystemModel>
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
    void hiddenFilesSettingDrivesDialogFilter();
    void showingHiddenFilesGivesUpTheNativeDialog();
    void entryFiltersRevealDotFiles();
    void forcedQtDialogActuallyListsHiddenEntries();

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

// Restores the file dialog settings whatever the test does with them.
class FileDialogSettingsGuard {
public:
    FileDialogSettingsGuard()
        : m_native(NqqSettings::getInstance().General.getUseNativeFilePicker())
        , m_hidden(NqqSettings::getInstance().General.getShowHiddenFiles())
    {
    }
    ~FileDialogSettingsGuard()
    {
        NqqSettings::getInstance().General.setUseNativeFilePicker(m_native);
        NqqSettings::getInstance().General.setShowHiddenFiles(m_hidden);
    }

private:
    const bool m_native;
    const bool m_hidden;
};

// The dialog filter must gain QDir::Hidden only when the user asked for hidden files.
void NotepadqqTest::hiddenFilesSettingDrivesDialogFilter()
{
    FileDialogSettingsGuard guard;
    NqqSettings::getInstance().General.setUseNativeFilePicker(false);

    NqqSettings::getInstance().General.setShowHiddenFiles(false);
    QFileDialog withoutHidden;
    NqqFileDialog::applySettings(withoutHidden);
    QVERIFY(!withoutHidden.filter().testFlag(QDir::Hidden));

    NqqSettings::getInstance().General.setShowHiddenFiles(true);
    QFileDialog withHidden;
    NqqFileDialog::applySettings(withHidden);
    QVERIFY(withHidden.filter().testFlag(QDir::Hidden));
}

// A native dialog ignores QDir::Hidden, so asking for hidden files has to force Qt's own dialog.
void NotepadqqTest::showingHiddenFilesGivesUpTheNativeDialog()
{
    FileDialogSettingsGuard guard;
    NqqSettings::getInstance().General.setUseNativeFilePicker(true);

    NqqSettings::getInstance().General.setShowHiddenFiles(false);
    QFileDialog nativeDialog;
    NqqFileDialog::applySettings(nativeDialog);
    QVERIFY(!nativeDialog.testOption(QFileDialog::DontUseNativeDialog));

    NqqSettings::getInstance().General.setShowHiddenFiles(true);
    QFileDialog forcedQtDialog;
    NqqFileDialog::applySettings(forcedQtDialog);
    QVERIFY(forcedQtDialog.testOption(QFileDialog::DontUseNativeDialog));
    QVERIFY(forcedQtDialog.filter().testFlag(QDir::Hidden));
}

// "Open Folder" enumerates the directory itself: dot files must follow the same setting.
void NotepadqqTest::entryFiltersRevealDotFiles()
{
    FileDialogSettingsGuard guard;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    for (const QString& name : {QStringLiteral("visible.txt"), QStringLiteral(".hidden.txt")}) {
        QFile file(QDir(directory.path()).filePath(name));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }

    NqqSettings::getInstance().General.setShowHiddenFiles(false);
    QCOMPARE(QDir(directory.path()).entryList(QStringList(), NqqFileDialog::entryFilters(QDir::Files)),
        QStringList({"visible.txt"}));

    NqqSettings::getInstance().General.setShowHiddenFiles(true);
    QCOMPARE(QDir(directory.path()).entryList(QStringList(), NqqFileDialog::entryFilters(QDir::Files)),
        QStringList({".hidden.txt", "visible.txt"}));
}

// QFileDialog::setFilter() only reaches the file system model once the widgets exist, so assert
// on the model that actually populates the list rather than on the dialog options.
void NotepadqqTest::forcedQtDialogActuallyListsHiddenEntries()
{
    FileDialogSettingsGuard guard;
    NqqSettings::getInstance().General.setUseNativeFilePicker(true);
    NqqSettings::getInstance().General.setShowHiddenFiles(true);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QFile hidden(QDir(directory.path()).filePath(".hidden.txt"));
    QVERIFY(hidden.open(QIODevice::WriteOnly));
    hidden.close();

    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setDirectory(directory.path());
    NqqFileDialog::applySettings(dialog);

    // Builds the widget hierarchy without ever putting the dialog on screen.
    dialog.setAttribute(Qt::WA_DontShowOnScreen);
    dialog.show();

    auto* model = dialog.findChild<QFileSystemModel*>();
    QVERIFY(model);
    QVERIFY(model->filter().testFlag(QDir::Hidden));

    const QModelIndex root = model->index(directory.path());
    QTRY_COMPARE(model->rowCount(root), 1);
    QCOMPARE(model->index(0, 0, root).data(Qt::DisplayRole).toString(), QStringLiteral(".hidden.txt"));

    dialog.hide();
}

QTEST_MAIN(NotepadqqTest)

#include "tst_notepadqqtest.moc"
