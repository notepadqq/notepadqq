#include "include/frmpreferences.h"

#include "include/EditorNS/editor.h"
#include "include/Extensions/extensionsloader.h"
#include "include/Sessions/backupservice.h"
#include "include/keygrabber.h"
#include "include/mainwindow.h"
#include "include/notepadqq.h"
#include "include/stats.h"
#include "ui_frmpreferences.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QSortFilterProxyModel>
#include <QToolBar>

int frmPreferences::s_lastSelectedTab = 0;

frmPreferences::frmPreferences(TopEditorContainer *topEditorContainer, QWidget *parent) :
    QDialog(parent),
    m_settings(NqqSettings::getInstance()),
    ui(new Ui::frmPreferences),
    m_topEditorContainer(topEditorContainer)
{
    ui->setupUi(this);

    //setFixedSize(this->width(), this->height());
    //setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

    m_previewEditor = Editor::getNewEditorUnmanagedPtr(this);
    m_previewEditor->setLanguageFromFilePath("test.js");
    m_previewEditor->setValue(R"(var enabled = false;)" "\n"
                              R"()" "\n"
                              R"(function example(a, b) {)" "\n"
                              R"(    if (b == 0 && enabled) {)" "\n"
                              R"(        var ret = a > 3 ? "ok" : null;)" "\n"
                              R"(        return !ret;)" "\n"
                              R"(    })" "\n"
                              R"()" "\n"
                              R"(    return example(a + 1, 0);)" "\n"
                              R"(})" "\n"
                              );

    // Update the preview editor so that the editor's current settings are applied and shown properly
    // in the editor preview
    updatePreviewEditorFont();

    // Select first item in treeWidget
    ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(s_lastSelectedTab));

    ui->chkCollectStatistics->setChecked(m_settings.General.getCollectStatistics());
    ui->chkWarnForDifferentIndentation->setChecked(m_settings.General.getWarnForDifferentIndentation());
    ui->chkRememberSession->setChecked(m_settings.General.getRememberTabsOnExit());
    ui->chkExitOnLastTabClose->setChecked(m_settings.General.getExitOnLastTabClose());

    ui->chkAutosave->setChecked(m_settings.General.getAutosaveInterval() > 0);
    ui->sbAutosaveInterval->setValue(m_settings.General.getAutosaveInterval());

    loadLanguages();
    loadAppearanceTab();
    loadTranslations();
    loadShortcuts();
    loadToolbar();

    ui->chkSearch_SearchAsIType->setChecked(m_settings.Search.getSearchAsIType());
    ui->chkSearch_SaveHistory->setChecked(m_settings.Search.getSaveHistory());

    ui->txtNodejs->setText(m_settings.Extensions.getRuntimeNodeJS());
    ui->txtNpm->setText(m_settings.Extensions.getRuntimeNpm());
}

frmPreferences::~frmPreferences()
{
    delete ui;
}

void frmPreferences::resetAllShortcuts() {
    auto& bindings = m_keyGrabber->getAllBindings();

    for (auto& item : bindings) {
        const QString& objName = item.getAction()->objectName();
        const QKeySequence seq = m_settings.Shortcuts.getDefaultShortcut(objName);
        item.setText(seq.toString());
    }

    m_keyGrabber->checkForConflicts();
}

void frmPreferences::resetSelectedShortcut()
{
    auto& bindings = m_keyGrabber->getAllBindings();
    auto currItem = m_keyGrabber->currentItem();

    // Search for the selected item and set its key sequence to the default one.
    for (auto& item : bindings) {
        if (currItem != item.getTreeItem()) continue;

        const QString& objName = item.getAction()->objectName();
        const QKeySequence seq = m_settings.Shortcuts.getDefaultShortcut(objName);
        item.setText(seq.toString());
        break;
    }

    m_keyGrabber->checkForConflicts();
}

void frmPreferences::updatePreviewEditorFont()
{
    const QString font = ui->cmbFontFamilies->isEnabled() ? ui->cmbFontFamilies->currentFont().family() : "";
    const int size = ui->spnFontSize->isEnabled() ? ui->spnFontSize->value() : 0;
    const double lineHeight = ui->spnLineHeight->isEnabled() ? ui->spnLineHeight->value() : 0;
    const bool lineNumbersVisible = ui->chkShowLineNumbers->isChecked();

    m_previewEditor->setFont(font, size, lineHeight);

    m_previewEditor->setLineNumbersVisible(lineNumbersVisible);

    // Re-setting language also updates the position of text selection. If not done, selected text
    // would often glitch out when changing the font causes the position of text characters to change.
    m_previewEditor->setLanguage(m_previewEditor->getLanguage());
}

void frmPreferences::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem * /*previous*/)
{
    int index = ui->treeWidget->indexOfTopLevelItem(current);

    if (index != -1) {
        ui->stackedWidget->setCurrentIndex(index);
        s_lastSelectedTab = index;
    }
}


void frmPreferences::on_buttonBox_clicked(QAbstractButton *button)
{
    // Accept and Reject buttons are handled separately. Other buttons need to use this generic clicked() event.

    if (button == ui->buttonBox->button(QDialogButtonBox::Apply))
        applySettings();
}

void frmPreferences::on_buttonBox_accepted()
{
    if (applySettings())
        accept();
}

void frmPreferences::loadLanguages()
{
    auto &ls = m_settings.Languages;
    //"Default" language
    ui->cmbLanguages->addItem("Default", "default");
    LanguageSettings lang = {
        "default",
        ls.getTabSize("default"),
        ls.getIndentWithSpaces("default"),
        ls.getUseDefaultSettings("default")
    };
    m_tempLangSettings.push_back(lang);

    for (const auto& l : LanguageService::getInstance().languages()) {
        ui->cmbLanguages->addItem(l.name.isEmpty() ? "?" : l.name, l.id);
        LanguageSettings lang = {
            l.id,
            ls.getTabSize(l.id),
            ls.getIndentWithSpaces(l.id),
            ls.getUseDefaultSettings(l.id)
        };

        m_tempLangSettings.push_back(lang);
    }

    ui->cmbLanguages->setCurrentIndex(0);
    ui->cmbLanguages->currentIndexChanged(0);
}

void frmPreferences::saveLanguages()
{
    // Write all temporary language settings back into the settings file.
    for (auto&& lang : m_tempLangSettings) {
        m_settings.Languages.setTabSize(lang.langId, lang.tabSize);
        m_settings.Languages.setIndentWithSpaces(lang.langId, lang.indentWithSpaces);
        m_settings.Languages.setUseDefaultSettings(lang.langId, lang.useDefaultSettings);
    }
}

void frmPreferences::loadAppearanceTab()
{
    QList<Editor::Theme> themes = m_topEditorContainer->currentTabWidget()->currentEditor()->themes();

    ui->cmbColorScheme->addItem("Default", "default");

    QString themeSetting = m_settings.Appearance.getColorScheme();

    for (const auto& theme : themes) {
        ui->cmbColorScheme->addItem(theme.name, theme.name); // First is display text, second is item data.

        if (themeSetting == theme.name) {
            ui->cmbColorScheme->setCurrentIndex(ui->cmbColorScheme->count() - 1);
        }
    }

    ui->colorSchemePreviewFrame->layout()->addWidget(m_previewEditor);

    const QString fontFamily = m_settings.Appearance.getOverrideFontFamily();
    if (!fontFamily.isEmpty()) {
        ui->chkOverrideFontFamily->setChecked(true);
        ui->cmbFontFamilies->setCurrentFont(fontFamily);
    }

    const int fontSize = m_settings.Appearance.getOverrideFontSize();
    if (fontSize != 0) {
        ui->chkOverrideFontSize->setChecked(true);
        ui->spnFontSize->setValue(fontSize);
    }

    const double lineHeight = m_settings.Appearance.getOverrideLineHeight();
    if (lineHeight != 0) {
        ui->chkOverrideLineHeight->setChecked(true);
        ui->spnLineHeight->setValue(lineHeight);
    }

    const bool showLineNumbers = m_settings.Appearance.getShowLineNumbers();
    if (showLineNumbers) {
        ui->chkShowLineNumbers->setChecked(true);
    }
}

void frmPreferences::saveAppearanceTab()
{
    m_settings.Appearance.setColorScheme(ui->cmbColorScheme->currentData().toString());

    const QString fontFamily = ui->cmbFontFamilies->isEnabled() ? ui->cmbFontFamilies->currentFont().family() : "";
    const int fontSize = ui->spnFontSize->isEnabled() ? ui->spnFontSize->value() : 0;
    const double lineHeight = ui->spnLineHeight->isEnabled() ? ui->spnLineHeight->value() : 0;
    const bool showLineNumbers = ui->chkShowLineNumbers->isChecked();

    m_settings.Appearance.setOverrideFontFamily(fontFamily);
    m_settings.Appearance.setOverrideFontSize(fontSize);
    m_settings.Appearance.setOverrideLineHeight(lineHeight);
    m_settings.Appearance.setShowLineNumbers(showLineNumbers);
}

void frmPreferences::loadTranslations()
{
    QList<QString> translations = Notepadqq::translations();

    QString localizationSetting = m_settings.General.getLocalization();

    for (const auto& langCode : translations) {
        QString langName = QLocale::languageToString(QLocale(langCode).language());
        ui->localizationComboBox->addItem(langName, langCode);
    }

    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(ui->localizationComboBox);
    proxy->setSourceModel(ui->localizationComboBox->model());
    ui->localizationComboBox->model()->setParent(proxy);
    ui->localizationComboBox->setModel(proxy);
    ui->localizationComboBox->model()->sort(0);

    ui->localizationComboBox->setCurrentIndex(
                ui->localizationComboBox->findData(
                    QLocale::languageToString(QLocale(localizationSetting).language()), Qt::DisplayRole));
}

void frmPreferences::saveTranslation()
{
    const auto selected = ui->localizationComboBox->currentData().toString();
    m_settings.General.setLocalization(selected);
}

void frmPreferences::loadShortcuts()
{
    MainWindow* mw = qobject_cast<MainWindow*>(parent());

    m_keyGrabber = new KeyGrabber();

    const auto& menus = mw->getMenus();

    //addMenus() intentionally skips the Language menu since it would just clutter up everything.
    m_keyGrabber->addMenus(menus);
    m_keyGrabber->expandAll();
    m_keyGrabber->checkForConflicts();

    // Build the interface
    QWidget *container = ui->pageShortcuts;
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *btnLayout = new QHBoxLayout();

    QPushButton *resetSelected = new QPushButton(tr("Reset Selected"));
    QPushButton *resetAll = new QPushButton(tr("Reset All"));

    QObject::connect(resetSelected, &QPushButton::clicked, this, &frmPreferences::resetSelectedShortcut);
    QObject::connect(resetAll, &QPushButton::clicked, this, &frmPreferences::resetAllShortcuts);

    resetSelected->setFixedWidth(144);
    resetAll->setFixedWidth(128);

    btnLayout->addWidget(resetSelected);
    btnLayout->addWidget(resetAll);
    btnLayout->addStretch(1);

    layout->addWidget(m_keyGrabber);
    layout->addLayout(btnLayout);

    container->setLayout(layout);
}

void frmPreferences::saveShortcuts()
{
    auto& bindings = m_keyGrabber->getAllBindings();

    for (auto& item : bindings) {
        const QString& objName = item.getAction()->objectName();
        QKeySequence seq = item.text();

        m_settings.Shortcuts.setShortcut(objName, seq);
        item.getAction()->setShortcut(seq);
    }
}

void frmPreferences::loadToolbar()
{
    auto* wnd = MainWindow::lastActiveInstance();

    auto actions = wnd->getActions();

    auto* widgetItem = new QListWidgetItem("-- Separator --");
    widgetItem->setData(Qt::UserRole, "Separator");
    ui->listToolbarAll->addItem(widgetItem);

    for (auto item : actions) {
        if (item->objectName().isEmpty() || !item->isVisible())
            continue;

        QString text = item->text().replace("&", "");
        auto* widgetItem = new QListWidgetItem(item->icon(), text);
        widgetItem->setData(Qt::UserRole, item->objectName());
        ui->listToolbarAll->addItem(widgetItem);
    }

    auto* toolbar = wnd->getToolBar();
    for (auto item : toolbar->actions()) {
        if (item->isSeparator()) {
            auto* widgetItem = new QListWidgetItem("-- Separator --");
            widgetItem->setData(Qt::UserRole, "Separator");
            ui->listToolbarCurrent->addItem(widgetItem);
        }
        else {
            QString text = item->text().replace("&", "");
            auto* widgetItem = new QListWidgetItem(item->icon(), text);
            widgetItem->setData(Qt::UserRole, item->objectName());
            ui->listToolbarCurrent->addItem(widgetItem);
        }
    }
}

void frmPreferences::saveToolbar()
{
    QStringList list;
    for (int i=0; i<ui->listToolbarCurrent->count(); ++i) {
        auto* item = ui->listToolbarCurrent->item(i);
        list << item->data(Qt::UserRole).toString();
    }

    auto string = list.join('|');

    // Only update if there's actually a change
    if (string == m_settings.MainWindow.getToolBarItems())
        return;

    m_settings.MainWindow.setToolBarItems(string);

    for (auto* wnd : MainWindow::instances())
        wnd->loadToolBar();
}

bool frmPreferences::applySettings()
{
    if (m_keyGrabber->hasConflicts()) {
        //Try our best to show the error to the user immediately.
        ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(ui->stackedWidget->indexOf(ui->pageShortcuts)));
        m_keyGrabber->scrollToConflict();

        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("<h3>" + tr("Keyboard shortcut conflict") + "</h3>");
        msgBox.setInformativeText(tr("Two or more actions share the same shortcut. These conflicts must be resolved before your changes can be saved."));
        msgBox.exec();
        return false;
    }


    m_settings.General.setCollectStatistics(ui->chkCollectStatistics->isChecked());
    m_settings.General.setWarnForDifferentIndentation(ui->chkWarnForDifferentIndentation->isChecked());
    m_settings.General.setRememberTabsOnExit(ui->chkRememberSession->isChecked());
    m_settings.General.setExitOnLastTabClose(ui->chkExitOnLastTabClose->isChecked());

    const int autosaveInSeconds = ui->chkAutosave->isChecked() ?
                                     ui->sbAutosaveInterval->value() : 0;
    m_settings.General.setAutosaveInterval(autosaveInSeconds);

    saveLanguages();
    saveAppearanceTab();
    saveTranslation();
    saveShortcuts();
    saveToolbar();

    m_settings.Search.setSearchAsIType(ui->chkSearch_SearchAsIType->isChecked());
    m_settings.Search.setSaveHistory(ui->chkSearch_SaveHistory->isChecked());

    m_settings.Extensions.setRuntimeNodeJS(ui->txtNodejs->text());
    m_settings.Extensions.setRuntimeNpm(ui->txtNpm->text());

    const Editor::Theme& newTheme = Editor::themeFromName(ui->cmbColorScheme->currentData().toString());
    const QString fontFamily = ui->cmbFontFamilies->isEnabled() ? ui->cmbFontFamilies->currentFont().family() : "";
    const int fontSize = ui->spnFontSize->isEnabled() ? ui->spnFontSize->value() : 0;
    const double lineHeight = ui->spnLineHeight->isEnabled() ? ui->spnLineHeight->value() : 0;
    const bool lineNumbersVisible = ui->chkShowLineNumbers->isChecked();

    // Apply changes to currently opened editors
    for (MainWindow *w : MainWindow::instances()) {
        w->showExtensionsMenu(Extensions::ExtensionsLoader::extensionRuntimePresent());

        w->topEditorContainer()->forEachEditor([&](const int, const int, EditorTabWidget *, Editor *editor) {

            // Set new theme
            editor->setTheme(newTheme);

            // Set font override
            editor->setFont(fontFamily, fontSize, lineHeight);

            // Set line numbers visibility
            editor->setLineNumbersVisible(lineNumbersVisible);

            // Reset language-dependent settings (e.g. tab settings)
            editor->setLanguage(editor->getLanguage());

            return true;
        });
    }

    // Invalidate already initialized editors in the buffer and add a single new
    // Editor to the buffer so we won't have an empty queue.
    Editor::invalidateEditorBuffer();
    Editor::addEditorToBuffer(1);

    // Check if we need to send stats
    Stats::init();


    if (autosaveInSeconds > 0)
        BackupService::enableAutosave(autosaveInSeconds);
    else
        BackupService::disableAutosave();

    return true;
}

void frmPreferences::on_buttonBox_rejected()
{
    reject();
}

void frmPreferences::on_cmbLanguages_currentIndexChanged(int index)
{
    if (m_tempLangSettings.size() <= index)
        return;

    const LanguageSettings& ls = m_tempLangSettings[index];

    if (ls.langId == "default") {
        // Hide "use default settings" checkbox, and enable the other stuff
        ui->chkLanguages_useDefaultSettings->setVisible(false);
        ui->frameLanguages->setEnabled(true);
    } else {
        // Show "use default settings" checkbox
        ui->chkLanguages_useDefaultSettings->setVisible(true);
        ui->chkLanguages_useDefaultSettings->setChecked(ls.useDefaultSettings);
    }

    ui->txtLanguages_TabSize->setValue(ls.tabSize);
    ui->chkLanguages_IndentWithSpaces->setChecked(ls.indentWithSpaces);
}

void frmPreferences::on_chkLanguages_useDefaultSettings_toggled(bool checked)
{
    ui->frameLanguages->setEnabled(!checked);

    m_tempLangSettings[ui->cmbLanguages->currentIndex()].useDefaultSettings = checked;
}

void frmPreferences::on_txtLanguages_TabSize_valueChanged(int value)
{
    m_tempLangSettings[ui->cmbLanguages->currentIndex()].tabSize = value;
}

void frmPreferences::on_chkLanguages_IndentWithSpaces_toggled(bool checked)
{
    m_tempLangSettings[ui->cmbLanguages->currentIndex()].indentWithSpaces = checked;
}

void frmPreferences::on_cmbColorScheme_currentIndexChanged(int /*index*/)
{
    m_previewEditor->setTheme(Editor::themeFromName(ui->cmbColorScheme->currentData().toString()));
}

void frmPreferences::on_localizationComboBox_activated(int /*index*/)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(QCoreApplication::applicationName());
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("<h3>" + QObject::tr("Restart required") + "</h3>");
    msgBox.setInformativeText(QObject::tr("You need to restart Notepadqq for the localization changes to take effect."));
    msgBox.exec();
}

bool frmPreferences::extensionBrowseRuntime(QLineEdit *lineEdit)
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Browse"), lineEdit->text());
    if (fn.isNull())
        return false;
    lineEdit->setText(fn);
    return true;
}

void frmPreferences::checkExecutableExists(QLineEdit *path)
{
    QPalette palette;
    QFileInfo fi(path->text());

    if (!(fi.isFile() && fi.isExecutable())) {
        palette.setColor(QPalette::ColorRole::Text, Qt::GlobalColor::red);
    }
    path->setPalette(palette);
}

void frmPreferences::on_btnNodejsBrowse_clicked()
{
    extensionBrowseRuntime(ui->txtNodejs);
}

void frmPreferences::on_btnNpmBrowse_clicked()
{
    extensionBrowseRuntime(ui->txtNpm);
}

void frmPreferences::on_txtNodejs_textChanged(const QString &)
{
    checkExecutableExists(ui->txtNodejs);
}

void frmPreferences::on_txtNpm_textChanged(const QString &)
{
    checkExecutableExists(ui->txtNpm);
}

void frmPreferences::on_chkOverrideFontFamily_toggled(bool checked)
{
    ui->cmbFontFamilies->setEnabled(checked);
    updatePreviewEditorFont();
}

void frmPreferences::on_chkOverrideFontSize_toggled(bool checked)
{
    ui->spnFontSize->setEnabled(checked);
    updatePreviewEditorFont();
}

void frmPreferences::on_spnFontSize_valueChanged(int /*arg1*/)
{
    updatePreviewEditorFont();
}

void frmPreferences::on_cmbFontFamilies_currentFontChanged(const QFont& /*f*/)
{
    updatePreviewEditorFont();
}

void frmPreferences::on_chkOverrideLineHeight_toggled(bool checked)
{
    ui->spnLineHeight->setEnabled(checked);
    updatePreviewEditorFont();
}

void frmPreferences::on_spnLineHeight_valueChanged(double /*arg1*/)
{
    updatePreviewEditorFont();
}

void frmPreferences::on_chkShowLineNumbers_toggled(bool checked)
{
    updatePreviewEditorFont();
}

void frmPreferences::on_chkSearch_SaveHistory_toggled(bool checked)
{
    if (checked)
        return;

    if (m_settings.Search.getSearchHistory().isEmpty() &&
        m_settings.Search.getReplaceHistory().isEmpty() &&
        m_settings.Search.getFileHistory().isEmpty() &&
        m_settings.Search.getFilterHistory().isEmpty())
        return;


    QMessageBox msgBox(qApp->activeWindow());
    msgBox.setWindowTitle(QCoreApplication::applicationName());
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("Would you like to clear the existing history now?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::No);

    auto result = msgBox.exec();

    if(result == QMessageBox::Cancel) {
        ui->chkSearch_SaveHistory->setChecked(true);
        return;
    }

    if (result == QMessageBox::Yes) {
        m_settings.Search.resetSearchHistory();
        m_settings.Search.resetReplaceHistory();
        m_settings.Search.resetFileHistory();
        m_settings.Search.resetFilterHistory();
    }
}

void frmPreferences::on_btnToolbarAdd_clicked()
{
    auto* item = ui->listToolbarAll->currentItem();

    if (!item) return;

    auto idx = ui->listToolbarCurrent->currentRow();

    QListWidgetItem* newItem = new QListWidgetItem(item->icon(), item->text());
    newItem->setData(Qt::UserRole, item->data(Qt::UserRole));

    ui->listToolbarCurrent->insertItem(idx+1, newItem);
    ui->listToolbarCurrent->setCurrentRow(idx+1);
    //ui->listToolbarCurrent->scrollToItem(ui->listToolbarCurrent->currentItem());
}

void frmPreferences::on_btnToolbarRemove_clicked()
{
    auto* item = ui->listToolbarCurrent->currentItem();

    if (item)
        delete item;
}

void frmPreferences::on_btnToolbarUp_clicked()
{
    auto idx = ui->listToolbarCurrent->currentRow();

    if (idx > 0) {
        ui->listToolbarCurrent->insertItem(idx-1, ui->listToolbarCurrent->takeItem(idx));
        ui->listToolbarCurrent->setCurrentRow(idx-1);
    }
}

void frmPreferences::on_btnToolbarDown_clicked()
{
    auto idx = ui->listToolbarCurrent->currentRow();
    auto max =ui->listToolbarCurrent->count();

    if (idx < max-1) {
        ui->listToolbarCurrent->insertItem(idx+1, ui->listToolbarCurrent->takeItem(idx));
        ui->listToolbarCurrent->setCurrentRow(idx+1);
    }
}

void frmPreferences::on_btnToolbarReset_clicked()
{
    ui->listToolbarCurrent->clear();

    QString toolbarItems = MainWindow::lastActiveInstance()->getDefaultToolBarString();
    auto actions = MainWindow::lastActiveInstance()->getActions();
    auto parts = toolbarItems.split('|', QString::SkipEmptyParts);

    for (const auto& part : parts) {
        if (part == "Separator") {
            auto* widgetItem = new QListWidgetItem("-- Separator --");
            widgetItem->setData(Qt::UserRole, "Separator");
            ui->listToolbarCurrent->addItem(widgetItem);
            continue;
        }

        auto it = std::find_if(actions.begin(), actions.end(), [&part](QAction* ac) {
            return ac->objectName() == part;
        });

        if (it != actions.end()) {
            auto* item = *it;
            QString text = item->text().replace("&", "");
            auto* widgetItem = new QListWidgetItem(item->icon(), text);
            widgetItem->setData(Qt::UserRole, item->objectName());
            ui->listToolbarCurrent->addItem(widgetItem);
        }
    }
}

void frmPreferences::on_chkAutosave_toggled(bool checked)
{
    ui->sbAutosaveInterval->setEnabled(checked);
}
