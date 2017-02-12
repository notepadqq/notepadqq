#ifndef FRMPREFERENCES_H
#define FRMPREFERENCES_H

#include <QDialog>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include "include/topeditorcontainer.h"
#include "include/keygrabber.h"
#include "include/nqqsettings.h"

namespace Ui {
class frmPreferences;
}

class QAbstractButton;

class frmPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit frmPreferences(TopEditorContainer *topEditorContainer, QWidget *parent = 0);
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

    void on_buttonBox_clicked(QAbstractButton *button);

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
    Editor *m_previewEditor;

    void loadLanguages();
    void saveLanguages();
    void loadAppearanceTab();
    void saveAppearanceTab();
    void loadTranslations();
    void saveTranslation();
    void loadShortcuts();
    void saveShortcuts();

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
