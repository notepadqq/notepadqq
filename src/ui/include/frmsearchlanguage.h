#ifndef FRMSEARCHLANGUAGE_H
#define FRMSEARCHLANGUAGE_H

#include <QDialog>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include "editor.h"

namespace Ui {
class frmSearchLanguage;
}

class frmSearchLanguage : public QDialog
{
    Q_OBJECT

public:
    explicit frmSearchLanguage(Editor *editor, QWidget *parent = 0);
    ~frmSearchLanguage();

private slots:
    void on_txtFilter_textChanged(const QString &arg1);

private:
    Ui::frmSearchLanguage *ui;

    class LanguagesModel: public QAbstractTableModel
    {
    public:
        LanguagesModel(QMap<QString, QList<QString> > languages, QObject *parent = 0);
        ~LanguagesModel();
        int rowCount(const QModelIndex & parent = QModelIndex() ) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    private:
        //QMap<QString, QList<QString> > m_languages;
        QList<QPair<QString, QString> > m_languagesList;
    };

    LanguagesModel *m_languagesModel;
    QSortFilterProxyModel *m_filter;
};

#endif // FRMSEARCHLANGUAGE_H
