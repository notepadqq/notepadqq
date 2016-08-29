#include "include/frmpreferences.h"
#include "include/EditorNS/editor.h"
#include "ui_frmpreferences.h"
#include "include/EditorNS/editor.h"
#include "include/mainwindow.h"
#include "include/Extensions/extensionsloader.h"
#include "include/notepadqq.h"
#include "include/keygrabber.h"
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include <QInputDialog>
#include <QTableWidgetItem>
#include <QSharedPointer>

frmPreferences::frmPreferences(TopEditorContainer *topEditorContainer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmPreferences),
    m_topEditorContainer(topEditorContainer),
    m_langsTempSettings(new QMap<QString, QVariant>())
{
    ui->setupUi(this);

    //setFixedSize(this->width(), this->height());
    //setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

    m_previewEditor = Editor::getNewEditorUnmanagedPtr(this);
    m_previewEditor->setLanguageFromFileName("test.js");
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

    // Select first item in treeWidget
    ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));

    QSettings s;

    ui->chkCheckQtVersionAtStartup->setChecked(s.value("checkQtVersionAtStartup", true).toBool());
    ui->chkWarnForDifferentIndentation->setChecked(s.value("warnForDifferentIndentation", true).toBool());

    loadLanguages(&s);
    loadAppearanceTab(&s);
    loadTranslations(&s);
    loadShortcuts(&s);

    ui->chkSearch_SearchAsIType->setChecked(s.value("Search/SearchAsIType", true).toBool());

    ui->txtNodejs->setText(s.value("Extensions/Runtime_Nodejs", "").toString());
    ui->txtNpm->setText(s.value("Extensions/Runtime_Npm", "").toString());
}

frmPreferences::~frmPreferences()
{
    delete ui;
    delete m_langsTempSettings;
    delete m_commonLanguageProperties;
    delete m_shortcuts;
}

void frmPreferences::resetShortcuts()
{
    MainWindow* mw = qobject_cast<MainWindow*>(parent());
    int i = 0;
    QMap<QString,QString>::iterator it;
    for(it = m_shortcuts->begin();it != m_shortcuts->end();it++) {
        kg->item(i,1)->setText(mw->getDefaultShortcut(m_shortcuts->key(kg->item(i,0)->text())));
        i++;
    }
}

void frmPreferences::loadShortcuts(QSettings* s)
{
    MainWindow* mw = qobject_cast<MainWindow*>(parent());
    m_shortcuts = new QMap<QString,QString>;

    foreach(QAction* a, mw->getActions())
    {
        if(a->objectName().isEmpty()) continue;
        m_shortcuts->insert(a->objectName(), a->iconText());
    }

    kg = new KeyGrabber();

    //Build the interface
    QWidget *container = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    QPushButton *resetDefaults = new QPushButton("Restore Defaults");
    QObject::connect(resetDefaults,SIGNAL(clicked()),this,SLOT(resetShortcuts()));
    resetDefaults->setFixedWidth(128);
    layout->addWidget(kg);
    layout->addWidget(resetDefaults);
    container->setLayout(layout);
    ui->stackedWidget->insertWidget(4,container);

    QMapIterator<QString,QString> it(*m_shortcuts);
    s->beginGroup("Shortcuts");
    while(it.hasNext()) {
        it.next();
        kg->insertRow(0);
        kg->setItem(0,0,new QTableWidgetItem(it.value()));
        kg->setItem(0,1,new QTableWidgetItem(s->value(it.key()).toString()));
    }
    s->endGroup();

    kg->sortItems(0);
    kg->selectRow(0);
    kg->checkConflicts();
}

void frmPreferences::saveShortcuts(QSettings* s)
{
    MainWindow* mw = qobject_cast<MainWindow*>(parent());
    int rows = kg->rowCount();
    s->beginGroup("Shortcuts");
    for(int i=0;i<rows;i++) {
        s->setValue(m_shortcuts->key(kg->item(i,0)->text()),kg->item(i,1)->text());
    }
    s->endGroup();
    mw->updateShortcuts();
}

void frmPreferences::updatePreviewEditorFont()
{
    QString font = ui->cmbFontFamilies->isEnabled() ? ui->cmbFontFamilies->currentFont().family() : "";
    int size = ui->spnFontSize->isEnabled() ? ui->spnFontSize->value() : 0;

    m_previewEditor->setFont(font, size);
}

void frmPreferences::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem * /*previous*/)
{
    int index = ui->treeWidget->indexOfTopLevelItem(current);

    if (index != -1) {
        ui->stackedWidget->setCurrentIndex(index);
    }
}

void frmPreferences::on_buttonBox_accepted()
{
    QSettings s;
    s.setValue("checkQtVersionAtStartup", ui->chkCheckQtVersionAtStartup->isChecked());
    s.setValue("warnForDifferentIndentation", ui->chkWarnForDifferentIndentation->isChecked());

    saveLanguages(&s);
    saveAppearanceTab(&s);
    saveTranslation(&s);
    saveShortcuts(&s);
    s.setValue("Search/SearchAsIType", ui->chkSearch_SearchAsIType->isChecked());

    s.setValue("Extensions/Runtime_Nodejs", ui->txtNodejs->text());
    s.setValue("Extensions/Runtime_Npm", ui->txtNpm->text());

    const Editor::Theme& newTheme = Editor::themeFromName(ui->cmbColorScheme->currentData().toString());
    const QString fontFamily = ui->cmbFontFamilies->isEnabled() ? ui->cmbFontFamilies->currentFont().family() : "";
    const int fontSize = ui->spnFontSize->isEnabled() ? ui->spnFontSize->value() : 0;

    // Apply changes to currently opened editors
    for (MainWindow *w : MainWindow::instances()) {
        w->showExtensionsMenu(Extensions::ExtensionsLoader::extensionRuntimePresent());

        w->topEditorContainer()->forEachEditor([&](const int, const int, EditorTabWidget *, Editor *editor) {

            // Set new theme
            editor->setTheme(newTheme);

            // Set font override
            editor->setFont(fontFamily, fontSize);

            // Reset language-dependent settings (e.g. tab settings)
            editor->setLanguage(editor->language());

            return true;
        });
    }

    // Invalidate already initialized editors in the buffer and add a single new
    // Editor to the buffer so we won't have an empty queue.
    Editor::invalidateEditorBuffer();
    Editor::addEditorToBuffer(1);

    accept();
}

void frmPreferences::loadLanguages(QSettings *s)
{
    m_commonLanguageProperties = new QVariantMap();
    m_commonLanguageProperties->insert("tabSize", 4);
    m_commonLanguageProperties->insert("indentWithSpaces", false);


    m_langs = m_topEditorContainer->currentTabWidget()->currentEditor()->languages();

    std::sort(m_langs.begin(), m_langs.end(), Editor::LanguageGreater());

    ui->cmbLanguages->addItem("Default", QVariant(QString()));
    QString keyPrefix = "Languages/";

    for (QVariantMap::iterator prop = m_commonLanguageProperties->begin(); prop != m_commonLanguageProperties->end(); ++prop) {
        m_langsTempSettings->insert(keyPrefix + prop.key(), s->value(keyPrefix + prop.key(), prop.value()));
    }

    for (int i = 0; i < m_langs.length(); i++) {
        const QMap<QString, QString> &map = m_langs.at(i);

        QString langId = map.value("id", "");
        ui->cmbLanguages->addItem(map.value("name", "?"), langId);

        QString keyPrefix = "Languages/" + langId + "/";

        m_langsTempSettings->insert(keyPrefix + "useDefaultSettings", s->value(keyPrefix + "useDefaultSettings", true));

        for (QVariantMap::iterator prop = m_commonLanguageProperties->begin(); prop != m_commonLanguageProperties->end(); ++prop) {
            m_langsTempSettings->insert(keyPrefix + prop.key(), s->value(keyPrefix + prop.key(), prop.value()));
        }
    }

    ui->cmbLanguages->setCurrentIndex(0);
    ui->cmbLanguages->currentIndexChanged(0);
}

void frmPreferences::loadAppearanceTab(QSettings *s)
{
    QList<Editor::Theme> themes = m_topEditorContainer->currentTabWidget()->currentEditor()->themes();

    ui->cmbColorScheme->addItem("Default", "default");

    QString themeSetting = s->value("Appearance/ColorScheme", "").toString();

    for (Editor::Theme theme : themes) {
        ui->cmbColorScheme->addItem(theme.name, theme.name); // First is display text, second is item data.

        if (themeSetting == theme.name) {
            ui->cmbColorScheme->setCurrentIndex(ui->cmbColorScheme->count() - 1);
        }
    }

    ui->colorSchemePreviewFrame->layout()->addWidget(m_previewEditor);

    // Avoid glitch where scrollbars are appearing for a moment
    QSize renderSize = ui->colorSchemePreviewFrame->size();
    m_previewEditor->forceRender(renderSize);


    QString fontFamily = s->value("Appearance/OverrideFontFamily").toString();
    if (!fontFamily.isEmpty()) {
        ui->chkOverrideFontFamily->setChecked(true);
        ui->cmbFontFamilies->setCurrentFont(fontFamily);
    }

    int fontSize = s->value("Appearance/OverrideFontSize").toInt();
    if (fontSize != 0) {
        ui->chkOverrideFontSize->setChecked(true);
        ui->spnFontSize->setValue(fontSize);
    }
}

void frmPreferences::loadTranslations(QSettings *s)
{
    QList<QString> translations = Notepadqq::translations();

    QString localizationSetting = s->value("Localization", "en").toString();

    for (QString langCode : translations) {
        QString langName = QLocale::languageToString(QLocale(langCode).language());

        QMap<QString, QVariant> tmap;
        tmap.insert("langName", langName);
        tmap.insert("langCode", langCode);

        ui->localizationComboBox->addItem(langName, tmap);
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

void frmPreferences::saveLanguages(QSettings *s)
{
    QString keyPrefix = "Languages/";

    for (QVariantMap::iterator prop = m_commonLanguageProperties->begin(); prop != m_commonLanguageProperties->end(); ++prop) {
        s->setValue(keyPrefix + prop.key(), m_langsTempSettings->value(keyPrefix + prop.key()));
    }

    for (int i = 0; i < m_langs.length(); i++) {
        const QMap<QString, QString> &map = m_langs.at(i);

        QString langId = map.value("id", "");
        QString keyPrefix = "Languages/" + langId + "/";

        s->setValue(keyPrefix + "useDefaultSettings", m_langsTempSettings->value(keyPrefix + "useDefaultSettings"));

        for (QVariantMap::iterator prop = m_commonLanguageProperties->begin(); prop != m_commonLanguageProperties->end(); ++prop) {
            s->setValue(keyPrefix + prop.key(), m_langsTempSettings->value(keyPrefix + prop.key()));
        }
    }
}

void frmPreferences::saveAppearanceTab(QSettings *s)
{
    s->setValue("Appearance/ColorScheme", ui->cmbColorScheme->currentData().toString());

    QString fontFamily = ui->cmbFontFamilies->isEnabled() ? ui->cmbFontFamilies->currentFont().family() : "";
    int fontSize = ui->spnFontSize->isEnabled() ? ui->spnFontSize->value() : 0;
    s->setValue("Appearance/OverrideFontFamily", fontFamily);
    s->setValue("Appearance/OverrideFontSize", fontSize);
}

void frmPreferences::saveTranslation(QSettings *s)
{
    QMap<QString, QVariant> selected = ui->localizationComboBox->currentData().toMap();
    s->setValue("Localization", selected.value("langCode").toString());
}

void frmPreferences::on_buttonBox_rejected()
{
    reject();
}

void frmPreferences::on_cmbLanguages_currentIndexChanged(int index)
{
    QVariant data = ui->cmbLanguages->itemData(index);

    QString keyPrefix;

    if (data.isNull()) {
        // General

        // Hide "use default settings" checkbox, and enable the other stuff
        ui->chkLanguages_useDefaultSettings->setVisible(false);
        ui->frameLanguages->setEnabled(true);

        keyPrefix = "Languages/";
    } else {
        QString langId = data.toString();
        keyPrefix = "Languages/" + langId + "/";

        // Show "use default settings" checkbox
        ui->chkLanguages_useDefaultSettings->setVisible(true);

        // Load "use default settings" value
        bool usingDefault = m_langsTempSettings->value(keyPrefix + "useDefaultSettings").toBool();
        ui->chkLanguages_useDefaultSettings->setChecked(usingDefault);
        ui->chkLanguages_useDefaultSettings->toggled(usingDefault);
    }

    ui->txtLanguages_TabSize->setValue(m_langsTempSettings->value(keyPrefix + "tabSize").toInt());
    ui->chkLanguages_IndentWithSpaces->setChecked(m_langsTempSettings->value(keyPrefix + "indentWithSpaces").toBool());
}

void frmPreferences::on_chkLanguages_useDefaultSettings_toggled(bool checked)
{
    ui->frameLanguages->setEnabled(!checked);

    QVariant langId = ui->cmbLanguages->currentData();
    if (langId.isNull() == false) {
        QString keyPrefix = "Languages/" + langId.toString() + "/";
        m_langsTempSettings->insert(keyPrefix + "useDefaultSettings", checked);
    }
}

void frmPreferences::setCurrentLanguageTempValue(QString key, QVariant value)
{
    QVariant langId = ui->cmbLanguages->currentData();

    QString keyPrefix;
    if (langId.isNull())
        keyPrefix = "Languages/";
    else
        keyPrefix = "Languages/" + langId.toString() + "/";

    m_langsTempSettings->insert(keyPrefix + key, value);
}

void frmPreferences::on_txtLanguages_TabSize_valueChanged(int value)
{
    setCurrentLanguageTempValue("tabSize", value);
}

void frmPreferences::on_chkLanguages_IndentWithSpaces_toggled(bool checked)
{
    setCurrentLanguageTempValue("indentWithSpaces", checked);
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
    if (fn.isNull()) {
        return false;
    } else {
        lineEdit->setText(fn);
        return true;
    }
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
