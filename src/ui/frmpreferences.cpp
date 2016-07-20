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
    loadColorSchemes(&s);
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

void frmPreferences::loadShortcuts(QSettings* s)
{
    //TODO: Anyone know how to do this without making a laundry list?
    //
    m_shortcuts = new QMap<QString,QString>;
    m_shortcuts->insert("action_New","New");
    m_shortcuts->insert("action_Open","Open");
    m_shortcuts->insert("actionOpen_Folder","Open Folder");
    m_shortcuts->insert("actionReload_from_Disk","Reload from Disk");
    m_shortcuts->insert("actionSave","Save");
    m_shortcuts->insert("actionSave_as","Save as");
    m_shortcuts->insert("actionSave_a_Copy_As","Save a Copy As");
    m_shortcuts->insert("actionSave_All","Save All");
    m_shortcuts->insert("actionRename","Rename");
    m_shortcuts->insert("actionClose","Close");
    m_shortcuts->insert("actionC_lose_All","Close All");
    m_shortcuts->insert("actionClose_All_BUT_Current_Document","Close All BUT Current Document");
    m_shortcuts->insert("actionLoad_Session","Load Session");
    m_shortcuts->insert("actionSave_Session","Save Session");
    m_shortcuts->insert("actionPrint","Print");
    m_shortcuts->insert("actionPrint_Now","Print Now");
    m_shortcuts->insert("actionE_xit","Exit");
    m_shortcuts->insert("action_Undo","Undo");
    m_shortcuts->insert("action_Redo","Redo");
    m_shortcuts->insert("actionCu_t","Cut");
    m_shortcuts->insert("action_Copy","Copy");
    m_shortcuts->insert("action_Paste","Paste");
    m_shortcuts->insert("action_Delete","Delete");
    m_shortcuts->insert("actionSelect_All","Select All");
    m_shortcuts->insert("actionWindows_Format","Windows Format");
    m_shortcuts->insert("actionUNIX_Format","UNIX Format");
    m_shortcuts->insert("actionMac_Format","Mac Format");
    m_shortcuts->insert("actionCurrent_Full_File_path_to_Clipboard","Current Full File Path To Clipboard");
    m_shortcuts->insert("actionCurrent_Filename_to_Clipboard","Current Filename To Clipboard");
    m_shortcuts->insert("actionCurrent_Directory_Path_to_Clipboard","Current Directory Path To Clipboard");
    m_shortcuts->insert("actionUPPERCASE","UPPERCASE");
    m_shortcuts->insert("actionLowercase","Lowercase");
    m_shortcuts->insert("actionIndentation_Default_settings","Indentation_Default_settings");
    m_shortcuts->insert("actionIndentation_Custom","Indentation Custom");
    m_shortcuts->insert("actionDuplicate_Line","Duplicate Line");
    m_shortcuts->insert("actionDelete_Line","Delete Line");
    m_shortcuts->insert("actionMove_Line_Up","Move Line Up");
    m_shortcuts->insert("actionMove_Line_Down","Move Line Down");
    m_shortcuts->insert("actionTrim_Trailing_Space","Trim Trailing Space");
    m_shortcuts->insert("actionTrim_Leading_Space","Trim Leading Space");
    m_shortcuts->insert("actionTrim_Leading_and_Trailing_Space","Trim Leading and Trailing Space");
    m_shortcuts->insert("actionEOL_to_Space","EOL To Space");
    m_shortcuts->insert("actionTAB_to_Space","TAB To Space");
    m_shortcuts->insert("actionSpace_to_TAB_All","Space To TAB All");
    m_shortcuts->insert("actionSpace_to_TAB_Leading","Space to TAB Leading");
    m_shortcuts->insert("actionSearch","Search");
    m_shortcuts->insert("actionFind_in_Files","Find In Files");
    m_shortcuts->insert("actionFind_Next","Find Next");
    m_shortcuts->insert("actionFind_Previous","Find Previous");
    m_shortcuts->insert("actionReplace","Replace");
    m_shortcuts->insert("actionGo_to_line","Go To Line");
    m_shortcuts->insert("actionWord_wrap","Word Wrap");
    m_shortcuts->insert("actionText_Direction_RTL","Text Direction RTL");
    m_shortcuts->insert("actionFull_Screen","Full Screen");
    m_shortcuts->insert("actionShow_Tabs","Show Tabs");
    m_shortcuts->insert("actionShow_End_of_Line","Show End of Line");
    m_shortcuts->insert("actionShow_All_Characters","Show All Characters");
    m_shortcuts->insert("actionShow_Indent_Guide","Show Indent Guide");
    m_shortcuts->insert("actionShow_Wrap_Symbol","Show Wrap Symbol");
    m_shortcuts->insert("actionZoom_In","Zoom In");
    m_shortcuts->insert("actionZoom_Out","Zoom Out");
    m_shortcuts->insert("actionRestore_Default_Zoom","Restore Default Zoom");
    m_shortcuts->insert("actionMove_to_Other_View","Move To Other View");
    m_shortcuts->insert("actionClone_to_Other_View","Clone To Other View");
    m_shortcuts->insert("actionMove_to_New_Window","Move To New Window");
    m_shortcuts->insert("actionOpen_in_New_Window","Open In New Window");
    m_shortcuts->insert("actionInterpret_as_UTF_8","Interpret As UTF-8");
    m_shortcuts->insert("actionInterpret_as_UTF_8_without_BOM","Interpret As UTF-8 Without BOM");
    m_shortcuts->insert("actionInterpret_as_UTF_16BE_UCS_2_Big_Endian","Interpret as UTF-16BE UCS 2 Big Endian");
    m_shortcuts->insert("actionInterpret_as_UTF_16LE_UCS_2_Little_Endian","Interpret as UTF-16LE UCS 2 Little Endian");
    m_shortcuts->insert("actionInterpret_as","Interpret As");
    m_shortcuts->insert("actionReload_file_interpreted_as","Reload File Interpreted As");
    m_shortcuts->insert("actionUTF_8","UTF-8");
    m_shortcuts->insert("actionUTF_8_without_BOM","UTF-8 Without BOM");
    m_shortcuts->insert("actionUTF_16BE","UTF-16BE");
    m_shortcuts->insert("actionUTF_16LE","UTF16-LE");
    m_shortcuts->insert("actionConvert_to","Convert To");
    m_shortcuts->insert("actionPreferences","Preferences");
    m_shortcuts->insert("actionAbout_Qt","About QT");
    m_shortcuts->insert("actionAbout_Notepadqq","About NotepadQQ");
    m_shortcuts->insert("action_Run","Run");
    m_shortcuts->insert("actionLaunch_in_Firefox","Launch In Firefox");
    m_shortcuts->insert("actionLaunch_in_Chrome","Launch In Chrome");
    m_shortcuts->insert("actionLaunch_in_Chromium","Launch In Chromium");
    m_shortcuts->insert("actionGet_php_help","Get PHP Help");
    m_shortcuts->insert("actionGoogle_Search","Google Search");
    m_shortcuts->insert("actionWikipedia_Search","Wikipedia Search");
    m_shortcuts->insert("actionOpen_file","Open File");
    m_shortcuts->insert("actionOpen_in_another_window","Open In Another Window");
    m_shortcuts->insert("actionModify_Shortcut_Delete_Command","Modify Shortcut Delete Command");
    m_shortcuts->insert("actionOpen_a_New_Window","Open a New Window");
    m_shortcuts->insert("actionInstall_Extension","Install Extension");

    kg = new keyGrabber();

    ui->stackedWidget->insertWidget(4,kg);
    kg->setRowCount(m_shortcuts->size());

    QMap<QString,QString>::iterator it;
    int i = 0;
    for(it = m_shortcuts->begin();it != m_shortcuts->end();it++) {
        kg->setItem(i,0,new QTableWidgetItem(it.value()));
        kg->setItem(i,1,new QTableWidgetItem(s->value(it.key()).toString()));
        i++;
    }

}

void frmPreferences::saveShortcuts(QSettings* s)
{
    int rows = kg->rowCount();
    for(int i=0;i<rows;i++) {
//        qDebug() << m_shortcuts->key(kg->item(i,0)->text()) << " : " << kg->item(i,1)->text();
        s->setValue(m_shortcuts->key(kg->item(i,0)->text()),kg->item(i,1)->text());
    } 
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
    saveTranslation(&s);
    saveShortcuts(&s);
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

void frmPreferences::saveColorScheme(QSettings *s)
{
    QMap<QString, QVariant> selected = ui->cmbColorScheme->currentData().toMap();
    s->setValue("Appearance/ColorScheme", selected.value("name").toString());
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
    QMap<QString, QVariant> selected = ui->cmbColorScheme->currentData().toMap();
    QString name = selected.value("name").toString();
    m_previewEditor->setTheme(Editor::themeFromName(name));
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
