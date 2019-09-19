#ifndef FRMPREFERENCES_H
#define FRMPREFERENCES_H

#include "include/keygrabber.h"
#include "include/nqqsettings.h"
#include "include/topeditorcontainer.h"

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui {
class frmPreferences;
}

class QAbstractButton;

class frmPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit frmPreferences(TopEditorContainer *topEditorContainer, QWidget *parent = nullptr);
    ~frmPreferences();

private slots:
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_cmbLanguages_currentIndexChanged(int index);
    void on_chkLanguages_useDefaultSettings_toggled(bool checked);
    void on_txtLanguages_TabSize_valueChanged(int value);
    void on_chkLanguages_IndentWithSpaces_toggled(bool checked);
    void on_cmbColorScheme_currentIndexChanged(int index);
    void on_localizationComboBox_activated(int index);
    void on_btnNodejsBrowse_clicked();
    void on_btnNpmBrowse_clicked();
    void on_txtNodejs_textChanged(const QString &);
    void on_txtNpm_textChanged(const QString &);
    void resetSelectedShortcut();
    void resetAllShortcuts();
    void on_chkOverrideFontFamily_toggled(bool checked);
    void on_chkOverrideFontSize_toggled(bool checked);
    void on_spnFontSize_valueChanged(int arg1);
    void on_cmbFontFamilies_currentFontChanged(const QFont &f);
    void on_chkOverrideLineHeight_toggled(bool checked);
    void on_spnLineHeight_valueChanged(double arg1);
    void on_chkShowLineNumbers_toggled(bool checked);

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_chkSearch_SaveHistory_toggled(bool checked);

    void on_btnToolbarAdd_clicked();

    void on_btnToolbarRemove_clicked();

    void on_btnToolbarUp_clicked();

    void on_btnToolbarDown_clicked();

    void on_btnToolbarReset_clicked();

    void on_chkAutosave_toggled(bool checked);

private:
    /**
     * @brief s_lastSelectedTab Contains the index of the last selected preferences tab. Default is 0.
     */
    static int s_lastSelectedTab;

    struct LanguageSettings {
        QString langId;
        int tabSize;
        bool indentWithSpaces;
        bool useDefaultSettings;
    };
    QList<LanguageSettings> m_tempLangSettings;

    KeyGrabber *m_keyGrabber;

    NqqSettings& m_settings;
    Ui::frmPreferences *ui;
    TopEditorContainer *m_topEditorContainer;
    QSharedPointer<Editor> m_previewEditor;

    void loadLanguages();
    void saveLanguages();
    void loadAppearanceTab();
    void saveAppearanceTab();
    void loadTranslations();
    void saveTranslation();
    void loadShortcuts();
    void saveShortcuts();
    void loadToolbar();
    void saveToolbar();

    /**
     * @brief applySettings Applies all user-set settings.
     * @return True if settings were successfully changed
     */
    bool applySettings();

    bool extensionBrowseRuntime(QLineEdit *lineEdit);
    void checkExecutableExists(QLineEdit *path);
    void updatePreviewEditorFont();
};

#endif // FRMPREFERENCES_H
