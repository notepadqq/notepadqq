#ifndef FRMPREFERENCES_H
#define FRMPREFERENCES_H

#include <QDialog>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include "QSettings"
#include "include/topeditorcontainer.h"
#include "include/keygrabber.h"

namespace Ui {
class frmPreferences;
}

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
    void resetShortcuts();
    void on_chkOverrideFontFamily_toggled(bool checked);
    void on_chkOverrideFontSize_toggled(bool checked);
    void on_spnFontSize_valueChanged(int arg1);
    void on_cmbFontFamilies_currentFontChanged(const QFont &f);

private:
    Ui::frmPreferences *ui;
    TopEditorContainer *m_topEditorContainer;
    QMap<QString, QVariant> *m_langsTempSettings;
    QMap<QString,QString> *m_shortcuts;
    QList<QMap<QString, QString>> m_langs;
    QVariantMap *m_commonLanguageProperties;
    Editor *m_previewEditor;
    KeyGrabber *kg;

    void loadLanguages(QSettings *s);
    void saveLanguages(QSettings *s);
    void setCurrentLanguageTempValue(QString key, QVariant value);
    void loadAppearanceTab(QSettings *s);
    void saveAppearanceTab(QSettings *s);
    bool extensionBrowseRuntime(QLineEdit *lineEdit);
    void checkExecutableExists(QLineEdit *path);
    void loadTranslations(QSettings *s);
    void saveTranslation(QSettings *s);
    void loadShortcuts(QSettings *s);
    void saveShortcuts(QSettings *s);
    void updatePreviewEditorFont();
};

#endif // FRMPREFERENCES_H
