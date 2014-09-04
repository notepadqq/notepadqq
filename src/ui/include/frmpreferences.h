#ifndef FRMPREFERENCES_H
#define FRMPREFERENCES_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "QSettings"
#include "include/topeditorcontainer.h"

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

private:
    Ui::frmPreferences *ui;
    TopEditorContainer *m_topEditorContainer;
    QMap<QString, QVariant> *m_langsTempSettings;
    QList<QMap<QString, QString>> m_langs;
    QVariantMap *m_commonLanguageProperties;

    void loadLanguages(QSettings *s);
    void saveLanguages(QSettings *s);
    void setCurrentLanguageTempValue(QString key, QVariant value);
};

#endif // FRMPREFERENCES_H
