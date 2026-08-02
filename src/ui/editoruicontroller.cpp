#include "include/editoruicontroller.h"

#include "include/EditorNS/bannerindentationdetected.h"
#include "include/EditorNS/editor.h"
#include "include/EditorNS/languageservice.h"
#include "include/docengine.h"
#include "include/documentcontroller.h"
#include "include/editortabwidget.h"
#include "include/frmencodingchooser.h"
#include "include/frmindentationmode.h"
#include "include/frmlinenumberchooser.h"
#include "include/mainwindow.h"
#include "include/notepadqq.h"
#include "include/nqqsettings.h"
#include "include/topeditorcontainer.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextCodec>
#include <QWheelEvent>

#include <map>

using EditorNS::Editor;
using EditorNS::LanguageService;

EditorUiController::EditorUiController(MainWindow& window,
    Ui::MainWindow& ui,
    DocEngine& docEngine,
    TopEditorContainer& editorContainer,
    NqqSettings& settings)
    : QObject(&window)
    , m_window(window)
    , m_ui(ui)
    , m_docEngine(docEngine)
    , m_editorContainer(editorContainer)
    , m_settings(settings)
{
    connect(
        &m_editorContainer, &TopEditorContainer::currentEditorChanged, this, &EditorUiController::currentEditorChanged);
    connect(&m_editorContainer, &TopEditorContainer::editorAdded, this, &EditorUiController::connectEditor);
    connect(&m_editorContainer, &TopEditorContainer::editorMouseWheel, this, &EditorUiController::handleMouseWheel);
}

void EditorUiController::configureUiFromSettings()
{
    const bool showAll = m_settings.General.getShowAllSymbols();
    m_ui.actionWord_wrap->setChecked(m_settings.General.getWordWrap());
    m_ui.actionShow_All_Characters->setChecked(showAll);
    setSymbols(showAll);

    m_ui.actionMath_Rendering->setChecked(m_settings.General.getMathRendering());

    m_ui.actionToggle_Smart_Indent->setChecked(m_settings.General.getSmartIndentation());
    setSmartIndent(m_settings.General.getSmartIndentation());

    restoreSavedZoom();
}

void EditorUiController::setupLanguagesMenu()
{
    std::map<QChar, QMenu*> menuInitials;
    for (const auto& language : LanguageService::getInstance().languages()) {
        const QString id = language.id;
        const QChar letter = language.name.isEmpty() ? '?' : language.name.at(0).toUpper();
        QMenu* letterMenu;
        if (menuInitials.count(letter) != 0) {
            letterMenu = menuInitials[letter];
        } else {
            letterMenu = new QMenu(letter, &m_window);
            menuInitials.emplace(letter, letterMenu);
            m_ui.menu_Language->insertMenu(0, letterMenu);
        }

        QAction* action = new QAction(language.name, &m_window);
        connect(action, &QAction::triggered, this, [this, id](bool = false) { currentEditor()->setLanguage(id); });
        letterMenu->insertAction(0, action);
    }
}

Editor* EditorUiController::currentEditor() const
{ return m_editorContainer.currentTabWidget()->currentEditor(); }

void EditorUiController::connectEditor(EditorTabWidget* tabWidget, int tab)
{
    Editor* editor = tabWidget->editor(tab);

    // A transferred editor may still have the connection established in its old window.
    disconnect(editor, &Editor::bannerRemoved, nullptr, nullptr);

    connect(editor, &Editor::cursorActivity, this, [this, editor](const QMap<QString, QVariant>& data) {
        cursorActivity(editor, data);
    });
    connect(editor, &Editor::documentInfoRequested, this, &EditorUiController::refreshCursorInfo);
    connect(editor, &Editor::currentLanguageChanged, this, [this, editor](const QString&, const QString&) {
        currentLanguageChanged(editor);
    });
    connect(editor, &Editor::bannerRemoved, this, &EditorUiController::removeBanner);
    connect(editor, &Editor::cleanChanged, this, [this, editor] {
        if (currentEditor() == editor)
            refreshCurrentEditor(editor);
    });
    connect(editor, &Editor::urlsDropped, this, [this, editor](const QList<QUrl>& urls) {
        m_window.m_documentController->openDroppedUrls(urls, editor);
    });

    editor->setLineWrap(m_ui.actionWord_wrap->isChecked());
    editor->setTabsVisible(m_ui.actionShow_Tabs->isChecked());
    editor->setEOLVisible(m_ui.actionShow_End_of_Line->isChecked());
    editor->setWhitespaceVisible(m_ui.actionShow_Spaces->isChecked());
    editor->setOverwrite(m_overwrite);
    editor->setFont(m_settings.Appearance.getOverrideFontFamily(),
        m_settings.Appearance.getOverrideFontSize(),
        m_settings.Appearance.getOverrideLineHeight());
    editor->setLineNumbersVisible(m_settings.Appearance.getShowLineNumbers());
    editor->setSmartIndent(m_settings.General.getSmartIndentation());
    editor->setMathEnabled(m_ui.actionMath_Rendering->isChecked());
}

void EditorUiController::currentEditorChanged(EditorTabWidget* tabWidget, int tab)
{
    if (tab == -1)
        return;

    Editor* editor = tabWidget->editor(tab);
    refreshCurrentEditor(editor);
    editor->requestDocumentInfo();
    editor->setFocus();
}

void EditorUiController::refreshCurrentEditor(Editor* editor)
{
    m_window.m_sbFileFormatBtn->setText(editor->getLanguage()->name);

    QString newTitle;
    if (editor->filePath().isEmpty()) {
        EditorTabWidget* tabWidget = m_editorContainer.tabWidgetFromEditor(editor);
        if (tabWidget != nullptr) {
            const int tab = tabWidget->indexOf(editor);
            if (tab != -1)
                newTitle = QString("%1 - %2").arg(tabWidget->tabText(tab)).arg(QApplication::applicationName());
        }
    } else {
        const QUrl url = editor->filePath();
        const QString path =
            url.toDisplayString(QUrl::RemovePassword | QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::RemoveAuthority |
                                QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::PreferLocalFile |
                                QUrl::RemoveFilename | QUrl::NormalizePathSegments | QUrl::StripTrailingSlash);
        newTitle = QString("%1%2 (%3) - %4")
                       .arg(Notepadqq::fileNameFromUrl(editor->filePath()))
                       .arg(editor->isClean() ? "" : "*")
                       .arg(path)
                       .arg(QApplication::applicationName());
    }

    if (newTitle != m_window.windowTitle())
        m_window.setWindowTitle(newTitle.isNull() ? QApplication::applicationName() : newTitle);

    QPointer<Editor> guardedEditor = editor;
    QPointer<EditorUiController> guardedController = this;
    editor->isCleanP().then([guardedController, guardedEditor](bool isClean) {
        if (!guardedController || !guardedEditor || guardedController->currentEditor() != guardedEditor)
            return;
        const QUrl fileName = guardedEditor->filePath();
        guardedController->m_ui.actionRename->setEnabled(!fileName.isEmpty());
        guardedController->m_ui.actionMove_to_New_Window->setEnabled(isClean);
        guardedController->m_ui.actionOpen_in_New_Window->setEnabled(isClean);
    });

    const bool allowReloading = !editor->filePath().isEmpty();
    m_ui.actionReload_File_Interpreted_As->setEnabled(allowReloading);
    m_ui.actionReload_from_Disk->setEnabled(allowReloading);

    const QString eol = editor->endOfLineSequence();
    if (eol == "\r\n") {
        m_ui.actionWindows_Format->setChecked(true);
        m_window.m_sbEOLFormatBtn->setText(m_window.tr("Windows"));
    } else if (eol == "\n") {
        m_ui.actionUNIX_Format->setChecked(true);
        m_window.m_sbEOLFormatBtn->setText(m_window.tr("UNIX / OS X"));
    } else if (eol == "\r") {
        m_ui.actionMac_Format->setChecked(true);
        m_window.m_sbEOLFormatBtn->setText(m_window.tr("Old Mac"));
    }

    QString encoding;
    if (editor->codec()->mibEnum() == MIB_UTF_8 && !editor->bom()) {
        encoding = m_window.tr("%1 w/o BOM").arg(QString::fromUtf8(editor->codec()->name()));
    } else {
        encoding = QString::fromUtf8(editor->codec()->name());
    }
    m_window.m_sbTextFormatBtn->setText(encoding);

    if (editor->isUsingCustomIndentationMode())
        m_ui.actionIndentation_Custom->setChecked(true);
    else
        m_ui.actionIndentation_Default_Settings->setChecked(true);
}

void EditorUiController::refreshCursorInfo(const QMap<QString, QVariant>& data)
{
    const auto cursorData = data["cursor"].toList();
    const auto selectionData = data["selections"].toList();
    const auto contentData = data["content"].toList();
    QString message = m_window.tr("Ln %1, Col %2").arg(cursorData[0].toInt() + 1).arg(cursorData[1].toInt() + 1);
    message += m_window.tr("    Sel %1 (%2)").arg(selectionData[1].toInt()).arg(selectionData[0].toInt());
    message += m_window.tr("    %1 chars, %2 lines").arg(contentData[1].toInt()).arg(contentData[0].toInt());
    m_window.m_sbDocumentInfoLabel->setText(message);
}

void EditorUiController::cursorActivity(Editor* editor, const QMap<QString, QVariant>& data)
{
    if (editor != nullptr && currentEditor() == editor)
        refreshCursorInfo(data);
}

void EditorUiController::currentLanguageChanged(Editor* editor)
{
    if (currentEditor() == editor)
        refreshCurrentEditor(editor);
}

void EditorUiController::handleMouseWheel(EditorTabWidget* tabWidget, int tab, QWheelEvent* event)
{
    if (!(QApplication::keyboardModifiers() & Qt::ControlModifier))
        return;

    const qreal currentZoom = tabWidget->editor(tab)->zoomFactor();
    qreal delta = event->angleDelta().y() / 120;
    delta /= 10;
    const qreal newZoom = currentZoom + delta;
    tabWidget->setZoomFactor(newZoom);
    m_settings.General.setZoom(newZoom);
}

void EditorUiController::toggleOverwrite()
{
    m_overwrite = !m_overwrite;
    m_editorContainer.forEachEditor([this](const int, const int, EditorTabWidget*, Editor* editor) {
        editor->setOverwrite(m_overwrite);
        return true;
    });
    m_window.m_sbOvertypeBtn->setText(m_overwrite ? m_window.tr("OVR") : m_window.tr("INS"));
}

void EditorUiController::setCurrentEditorLanguage(const QString& language)
{ currentEditor()->setLanguage(language); }

bool EditorUiController::updateSymbols(bool visible)
{
    if (!visible && m_ui.actionShow_All_Characters->isChecked()) {
        m_settings.General.setTabsVisible(m_ui.actionShow_Tabs->isChecked());
        m_settings.General.setSpacesVisisble(m_ui.actionShow_Spaces->isChecked());
        m_settings.General.setShowEOL(m_ui.actionShow_End_of_Line->isChecked());
        m_ui.actionShow_All_Characters->blockSignals(true);
        m_ui.actionShow_All_Characters->setChecked(false);
        m_ui.actionShow_All_Characters->blockSignals(false);
        m_settings.General.setShowAllSymbols(false);
        return true;
    }

    if (visible && !m_ui.actionShow_All_Characters->isChecked()) {
        const bool showEol = m_ui.actionShow_End_of_Line->isChecked();
        const bool showTabs = m_ui.actionShow_Tabs->isChecked();
        const bool showSpaces = m_ui.actionShow_Spaces->isChecked();
        if (showEol && showTabs && showSpaces)
            m_ui.actionShow_All_Characters->setChecked(true);
    }
    return false;
}

void EditorUiController::setTabsVisible(bool visible)
{
    m_editorContainer.forEachEditorConcurrent(
        [visible](const int, const int, EditorTabWidget*, Editor* editor, std::function<void()> done) {
            editor->setTabsVisible(visible);
            done();
        });
    if (!updateSymbols(visible))
        m_settings.General.setTabsVisible(visible);
}

void EditorUiController::setSpacesVisible(bool visible)
{
    m_editorContainer.forEachEditorConcurrent(
        [visible](const int, const int, EditorTabWidget*, Editor* editor, std::function<void()> done) {
            editor->setWhitespaceVisible(visible);
            done();
        });
    if (!updateSymbols(visible))
        m_settings.General.setSpacesVisisble(visible);
}

void EditorUiController::setEndOfLineVisible(bool visible)
{
    m_editorContainer.forEachEditorConcurrent(
        [visible](const int, const int, EditorTabWidget*, Editor* editor, std::function<void()> done) {
            editor->setEOLVisible(visible);
            done();
        });
    if (!updateSymbols(visible))
        m_settings.General.setShowEOL(visible);
}

void EditorUiController::setSymbols(bool visible)
{
    if (visible) {
        m_ui.actionShow_End_of_Line->setChecked(true);
        m_ui.actionShow_Tabs->setChecked(true);
        m_ui.actionShow_Spaces->setChecked(true);
    } else {
        bool showEol = m_settings.General.getShowEOL();
        bool showTabs = m_settings.General.getTabsVisible();
        bool showSpaces = m_settings.General.getSpacesVisisble();
        if (showEol && showTabs && showSpaces) {
            showEol = !showEol;
            showTabs = !showTabs;
            showSpaces = !showSpaces;
        }
        m_ui.actionShow_End_of_Line->setChecked(showEol);
        m_ui.actionShow_Tabs->setChecked(showTabs);
        m_ui.actionShow_Spaces->setChecked(showSpaces);
    }

    m_editorContainer.forEachEditorConcurrent(
        [this, visible](const int, const int, EditorTabWidget*, Editor* editor, std::function<void()> done) {
            editor->setEOLVisible(m_ui.actionShow_End_of_Line->isChecked());
            editor->setTabsVisible(m_ui.actionShow_Tabs->isChecked());
            editor->setWhitespaceVisible(visible);
            done();
        });
    m_settings.General.setShowAllSymbols(visible);
}

void EditorUiController::setMathRendering(bool enabled)
{
    m_editorContainer.forEachEditorConcurrent(
        [enabled](const int, const int, EditorTabWidget*, Editor* editor, std::function<void()> done) {
            editor->setMathEnabled(enabled);
            done();
        });
    m_settings.General.setMathRendering(enabled);
}

void EditorUiController::setWordWrap(bool enabled)
{
    m_editorContainer.forEachEditor([enabled](const int, const int, EditorTabWidget*, Editor* editor) {
        editor->setLineWrap(enabled);
        return true;
    });
    m_settings.General.setWordWrap(enabled);
}

void EditorUiController::setSmartIndent(bool enabled)
{
    m_editorContainer.forEachEditor([enabled](const int, const int, EditorTabWidget*, Editor* editor) {
        editor->setSmartIndent(enabled);
        return true;
    });
    m_settings.General.setSmartIndentation(enabled);
}

void EditorUiController::setZoom(qreal zoom)
{
    m_editorContainer.currentTabWidget()->setZoomFactor(zoom);
    m_settings.General.setZoom(zoom);
}

void EditorUiController::restoreDefaultZoom()
{
    const qreal zoom = m_settings.General.resetZoom();
    m_editorContainer.currentTabWidget()->setZoomFactor(zoom);
}

void EditorUiController::restoreSavedZoom()
{
    const qreal zoom = m_settings.General.getZoom();
    for (int i = 0; i < m_editorContainer.count(); ++i)
        m_editorContainer.tabWidget(i)->setZoomFactor(zoom);
}

void EditorUiController::adjustZoom(qreal delta)
{ setZoom(currentEditor()->zoomFactor() + delta); }

void EditorUiController::copySelections()
{
    currentEditor()->selectedTexts().then(
        [](const QStringList& selections) { QApplication::clipboard()->setText(selections.join("\n")); });
}

void EditorUiController::pasteSelections()
{
    const QString text = QApplication::clipboard()->text().replace(QRegularExpression("\n|\r\n|\r"), "\n");
    currentEditor()->setSelectionsText(text.split("\n"));
}

void EditorUiController::cutSelections()
{
    m_ui.actionCopy->trigger();
    currentEditor()->setSelectionsText(QStringList(""));
}

void EditorUiController::beginEndSelect()
{
    if (!m_beginSelectPositionSet) {
        m_beginSelectPosition = currentEditor()->cursorPosition();
        m_beginSelectPositionSet = true;
        return;
    }

    const QPair<int, int> endSelectPosition = currentEditor()->cursorPosition();
    currentEditor()->setSelection(
        m_beginSelectPosition.first, m_beginSelectPosition.second, endSelectPosition.first, endSelectPosition.second);
    m_beginSelectPositionSet = false;
}

void EditorUiController::deleteSelections()
{ currentEditor()->setSelectionsText(QStringList("")); }

void EditorUiController::selectAll()
{ sendEditorCommand("C_CMD_SELECT_ALL"); }

void EditorUiController::setRightToLeft()
{ sendEditorCommand("C_CMD_SET_RTL"); }

void EditorUiController::setLeftToRight()
{ sendEditorCommand("C_CMD_SET_LTR"); }

void EditorUiController::undo()
{ sendEditorCommand("C_CMD_UNDO"); }

void EditorUiController::redo()
{ sendEditorCommand("C_CMD_REDO"); }

void EditorUiController::setPlainText()
{ currentEditor()->setLanguage("plaintext"); }

void EditorUiController::transformSelectedText(const std::function<QString(const QString&)>& transform)
{
    QPointer<Editor> editor = currentEditor();
    editor->selectedTexts().then([editor, transform](QStringList selections) {
        if (!editor)
            return;
        for (int i = 0; i < selections.length(); ++i)
            selections.replace(i, transform(selections.at(i)));
        editor->setSelectionsText(selections, Editor::SelectMode::Selected);
    });
}

void EditorUiController::removeBanner(QWidget* banner)
{ delete banner; }

void EditorUiController::checkIndentationMode(Editor* editor)
{
    QPointer<Editor> guardedEditor = editor;
    QPointer<EditorUiController> guardedController = this;
    editor->detectDocumentIndentation().then([guardedController, guardedEditor](
                                                 const std::pair<IndentationMode, bool>& result) {
        if (!guardedController || !guardedEditor || !result.second)
            return;

        const IndentationMode detected = result.first;
        guardedEditor->indentationModeP().then([guardedController, guardedEditor, detected](IndentationMode current) {
            if (!guardedController || !guardedEditor)
                return;
            const bool differentTabSpaces = detected.useTabs != current.useTabs;
            const bool differentSpaceSize = !detected.useTabs && !current.useTabs && detected.size != current.size;
            if (!differentTabSpaces && !differentSpaceSize)
                return;

            auto* banner =
                new BannerIndentationDetected(differentSpaceSize, detected, current, &guardedController->m_window);
            banner->setObjectName("indentationdetected");
            guardedEditor->insertBanner(banner);

            connect(
                banner, &BannerIndentationDetected::useApplicationSettings, guardedController, [guardedEditor, banner] {
                    if (!guardedEditor)
                        return;
                    guardedEditor->removeBanner(banner);
                    guardedEditor->setFocus();
                });
            connect(banner,
                &BannerIndentationDetected::useDocumentSettings,
                guardedController,
                [guardedController, guardedEditor, banner, detected] {
                    if (!guardedController || !guardedEditor)
                        return;
                    guardedEditor->removeBanner(banner);
                    if (detected.useTabs)
                        guardedEditor->setCustomIndentationMode(true);
                    else
                        guardedEditor->setCustomIndentationMode(detected.useTabs, detected.size);
                    guardedController->m_ui.actionIndentation_Custom->setChecked(true);
                    guardedEditor->setFocus();
                });
        });
    });
}

void EditorUiController::setEndOfLineSequence(const QString& sequence)
{
    Editor* editor = currentEditor();
    editor->setEndOfLineSequence(sequence);
    editor->markDirty();
}

void EditorUiController::convertEditorEncoding(Editor* editor, QTextCodec* codec, bool bom)
{
    editor->setCodec(codec);
    editor->setBom(bom);
    editor->markDirty();
    if (editor == currentEditor())
        refreshCurrentEditor(editor);
}

void EditorUiController::convertCurrentEditorEncoding(QTextCodec* codec, bool bom)
{ convertEditorEncoding(currentEditor(), codec, bom); }

void EditorUiController::reinterpretCurrentEditorEncoding(QTextCodec* codec, bool bom)
{
    m_docEngine.reinterpretEncoding(currentEditor(), codec, bom);
    refreshCurrentEditor(currentEditor());
}

void EditorUiController::chooseEncodingForConversion()
{
    Editor* editor = currentEditor();
    auto* dialog = new frmEncodingChooser(&m_window);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(m_window.tr("Convert to:"));
    if (dialog->exec() == QDialog::Accepted)
        convertEditorEncoding(editor, dialog->selectedCodec(), false);
    dialog->deleteLater();
}

void EditorUiController::chooseEncodingForReload()
{
    Editor* editor = currentEditor();
    if (editor->filePath().isEmpty())
        return;

    auto* dialog = new frmEncodingChooser(&m_window);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(m_window.tr("Reload as:"));
    if (dialog->exec() == QDialog::Accepted) {
        m_docEngine.getDocumentLoader()
            .setUrl(editor->filePath())
            .setTabWidget(m_editorContainer.currentTabWidget())
            .setTextCodec(dialog->selectedCodec())
            .execute();
    }
    dialog->deleteLater();
}

void EditorUiController::useDefaultIndentation()
{ currentEditor()->clearCustomIndentationMode(); }

void EditorUiController::chooseCustomIndentation()
{
    Editor* editor = currentEditor();
    auto* dialog = new frmIndentationMode(&m_window);
    dialog->populateWidgets(editor->indentationMode());
    if (dialog->exec() == QDialog::Accepted) {
        const IndentationMode indentation = dialog->indentationMode();
        editor->setCustomIndentationMode(indentation.useTabs, indentation.size);
    }

    if (editor->isUsingCustomIndentationMode())
        m_ui.actionIndentation_Custom->setChecked(true);
    else
        m_ui.actionIndentation_Default_Settings->setChecked(true);
    dialog->deleteLater();
}

void EditorUiController::chooseEncodingForInterpretation()
{
    Editor* editor = currentEditor();
    auto* dialog = new frmEncodingChooser(&m_window);
    dialog->setEncoding(editor->codec());
    dialog->setInfoText(m_window.tr("Interpret as:"));
    if (dialog->exec() == QDialog::Accepted)
        m_docEngine.reinterpretEncoding(editor, dialog->selectedCodec(), false);
    dialog->deleteLater();
}

QtPromise::QPromise<QStringList> EditorUiController::currentWordOrSelections()
{
    QPointer<Editor> editor = currentEditor();
    return editor->selectedTexts().then([editor](const QStringList& selections) {
        if (!editor)
            return QtPromise::QPromise<QStringList>::resolve({});
        if (selections.isEmpty() || selections.first().isEmpty())
            return editor->getCurrentWord().then([](const QString& word) { return QStringList(word); });
        return QtPromise::QPromise<QStringList>::resolve(selections);
    });
}

QtPromise::QPromise<QString> EditorUiController::currentWordOrSelection()
{
    return currentWordOrSelections().then(
        [](const QStringList& terms) { return terms.isEmpty() ? QString() : terms.first(); });
}

void EditorUiController::sendEditorCommand(const QString& command)
{ currentEditor()->sendMessage(command); }

void EditorUiController::goToLine()
{
    QPointer<Editor> editor = currentEditor();
    const int currentLine = editor->cursorPosition().first;
    QPointer<EditorUiController> guardedController = this;
    editor->lineCount().then([guardedController, editor, currentLine](int lines) {
        if (!guardedController || !editor)
            return;
        frmLineNumberChooser chooser(1, lines, currentLine + 1, &guardedController->m_window);
        if (chooser.exec() == QDialog::Accepted) {
            const int line = chooser.value();
            editor->setSelection(line - 1, 0, line - 1, 0);
        }
    });
}
