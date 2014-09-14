#ifndef FRMSEARCHREPLACE_H
#define FRMSEARCHREPLACE_H

#include "topeditorcontainer.h"
#include <QDialog>
#include <QMainWindow>

namespace Ui {
class frmSearchReplace;
}

class frmSearchReplace : public QMainWindow
{
    Q_OBJECT

public:
    enum Tabs { TabSearch, TabReplace };
    explicit frmSearchReplace(TopEditorContainer *topEditorContainer, QWidget *parent = 0);
    ~frmSearchReplace();
    void show(Tabs defaultTab);
private slots:
    void on_btnFindNext_clicked();

    void on_btnFindPrev_clicked();

    void on_btnReplaceNext_clicked();

    void on_btnReplacePrev_clicked();

    void on_btnReplaceAll_clicked();

    void on_btnSelectAll_clicked();

    void on_actionReplace_toggled(bool on);

    void on_actionFind_toggled(bool on);

    void on_chkShowAdvanced_toggled(bool checked);

private:
    Ui::frmSearchReplace*  ui;
    TopEditorContainer*    m_topEditorContainer;
    QString                m_lastSearch;
    Editor*                currentEditor();

    enum class SearchMode {
        PlainText,
        SpecialChars,
        Regex
    };

    void search(QString string, SearchMode searchMode, bool forward);
    QString plainTextToRegex(QString text);
    void replace(QString string, QString replacement, SearchMode searchMode, bool forward);
    int replaceAll(QString string, QString replacement, SearchMode searchMode);
    int selectAll(QString string, bool isRegex);
    void setCurrentTab(Tabs tab);
    void manualSizeAdjust();
    SearchMode searchModeFromUI();
};

#endif // FRMSEARCHREPLACE_H
