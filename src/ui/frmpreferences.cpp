#include "include/frmpreferences.h"
#include "include/EditorNS/editor.h"
#include "ui_frmpreferences.h"
#include "include/EditorNS/editor.h"
#include "include/mainwindow.h"
#include "include/Extensions/extensionsloader.h"
#include <QFileDialog>

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
    loadColorSchemes(&s);

    ui->chkSearch_SearchAsIType->setChecked(s.value("Search/SearchAsIType", true).toBool());

    ui->txtNodejs->setText(s.value("Extensions/Runtime_Nodejs", "").toString());
    ui->txtNpm->setText(s.value("Extensions/Runtime_Npm", "").toString());
}

frmPreferences::~frmPreferences()
{
    delete ui;
    delete m_langsTempSettings;
    delete m_commonLanguageProperties;
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
    saveColorScheme(&s);

    s.setValue("Search/SearchAsIType", ui->chkSearch_SearchAsIType->isChecked());

    s.setValue("Extensions/Runtime_Nodejs", ui->txtNodejs->text());
    s.setValue("Extensions/Runtime_Npm", ui->txtNpm->text());

    // Apply changes to currently opened editors
    for (MainWindow *w : MainWindow::instances()) {
        w->showExtensionsMenu(Extensions::ExtensionsLoader::extensionRuntimePresent());

        w->topEditorContainer()->forEachEditor([&](const int, const int, EditorTabWidget *, Editor *editor) {

            // Reset language-dependent settings (e.g. tab settings)
            editor->setLanguage(editor->language());

            // Set theme
            QMap<QString, QVariant> theme_map = ui->cmbColorScheme->currentData().toMap();
            Editor::Theme theme;
            theme.name = theme_map.value("name").toString();
            theme.path = theme_map.value("path").toString();
            editor->setTheme(theme);

            // Invalidate already initialized editors in the buffer
            editor->invalidateEditorBuffer();
            editor->addEditorToBuffer();

            return true;
        });
    }

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

void frmPreferences::loadColorSchemes(QSettings *s)
{
    QList<Editor::Theme> themes = m_topEditorContainer->currentTabWidget()->currentEditor()->themes();

    QMap<QString, QVariant> defaultTheme;
    defaultTheme.insert("name", "default");
    defaultTheme.insert("path", "");
    ui->cmbColorScheme->addItem("Default", defaultTheme);
    ui->cmbColorScheme->setCurrentIndex(0);

    QString themeSetting = s->value("Appearance/ColorScheme", "").toString();

    for (Editor::Theme theme : themes) {
        QMap<QString, QVariant> tmap;
        tmap.insert("name", theme.name);
        tmap.insert("path", theme.path);
        ui->cmbColorScheme->addItem(theme.name, tmap);

        if (themeSetting == theme.name) {
            ui->cmbColorScheme->setCurrentIndex(ui->cmbColorScheme->count() - 1);
        }
    }

    ui->colorSchemePreviewFrame->layout()->addWidget(m_previewEditor);
    m_previewEditor->setTheme(Editor::themeFromName(themeSetting));

    // Avoid glitch where scrollbars are appearing for a moment
    QSize renderSize = ui->colorSchemePreviewFrame->size();
    m_previewEditor->forceRender(renderSize);
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

void frmPreferences::saveColorScheme(QSettings *s)
{
    QMap<QString, QVariant> selected = ui->cmbColorScheme->currentData().toMap();
    s->setValue("Appearance/ColorScheme", selected.value("name").toString());
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
    QMap<QString, QVariant> selected = ui->cmbColorScheme->currentData().toMap();
    QString name = selected.value("name").toString();
    m_previewEditor->setTheme(Editor::themeFromName(name));
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
