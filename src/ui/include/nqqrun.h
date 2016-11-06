#ifndef _NQQRUN_H_
#define _NQQRUN_H_
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include "include/nqqsettings.h"

namespace NqqRun {
class RunPreferences : public QDialog
{
    Q_OBJECT

private:
    NqqSettings &m_settings;
    QTableWidget *m_commands;
public:
    RunPreferences(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~RunPreferences();

private slots:
    void slotRemove();
    void slotOk();
    void slotInitCell(int row, int column);
};

class RunDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    RunDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
            const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    QIcon openIcon;
    QIcon rmIcon;

signals:
    void needsRemoval();
};

class RunDialog : public QDialog
{
    Q_OBJECT
private:
    NqqSettings &m_settings;
    QLabel *m_status;
    QLineEdit *m_command;
    bool m_saved;

public:
    RunDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~RunDialog();
    QString getCommandInput();
    bool saved();
    static QStringList parseCommandString(QString cmd);

private slots:
    void slotSave();
    void slotHideStatus();
};

};
#endif
