#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QPushButton>
#include <QPainter>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include "include/runpreferences.h"
#include "include/iconprovider.h"

RunPreferences::RunPreferences(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    m_settings(NqqSettings::getInstance())
{
    QVBoxLayout *v1 = new QVBoxLayout;
    QHBoxLayout *h1 = new QHBoxLayout;
    QHBoxLayout *h2 = new QHBoxLayout;
    QHBoxLayout *h3 = new QHBoxLayout;
    m_commands = new QTableWidget(1, 2);

    QStringList headers = (QStringList() << tr("Text") << tr("Command"));

    QHeaderView *vh = m_commands->verticalHeader();
    vh->sectionResizeMode(QHeaderView::Fixed);
    vh->setDefaultSectionSize(16);
    vh->setMaximumSectionSize(16);

    QHeaderView *hh = m_commands->horizontalHeader();
    hh->setStretchLastSection(true);

    setMinimumSize(500, 200);

    v1->addWidget(m_commands);
    v1->addItem(h3);
    
    QPushButton *btnRm   = new QPushButton(tr("Remove"));
    QPushButton *btnOk   = new QPushButton(tr("Okay"));
    QPushButton *btnCancel = new QPushButton(tr("Cancel"));
    h1->addWidget(btnRm);
    h1->setAlignment(Qt::AlignLeft); 
    h2->addWidget(btnOk);
    h2->addWidget(btnCancel);
    h2->setAlignment(Qt::AlignRight);
    h3->addItem(h1);
    h3->addItem(h2);

    setLayout(v1);

    connect(btnRm, SIGNAL(clicked()), this, SLOT(slotRemove()));
    connect(btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    connect(m_commands, SIGNAL(cellChanged(int, int)), this, SLOT(slotInitCell(int, int)));

    QMap <QString, QString> cmdData = m_settings.Run.getCommands();
    QSortFilterProxyModel *pModel = new QSortFilterProxyModel(this);
    pModel->setSourceModel(m_commands->model());
    m_commands->setAlternatingRowColors(true);
    m_commands->setSortingEnabled(false);
    m_commands->setHorizontalHeaderLabels(headers);
    m_commands->setSelectionMode(QAbstractItemView::SingleSelection);
    m_commands->setItemDelegate(new RunDelegate(this));
    m_commands->setRowCount(cmdData.size()+1);

    int workRow = 0;
    QMapIterator<QString, QString> it(cmdData);
    while(it.hasNext())
    {
        it.next();
        QTableWidgetItem* item = new QTableWidgetItem(it.key());
        m_commands->setItem(workRow, 0, item);
        item = new QTableWidgetItem(it.value());
        m_commands->setItem(workRow, 1, item);
        workRow++;
    }
}

RunPreferences::~RunPreferences()
{
}

void RunPreferences::slotOk()
{
    hide();
    m_settings.Run.resetCommands();
    const int totalCommands = m_commands->rowCount();
    for (int i = 0; i < totalCommands; ++i) {
        if (!m_commands->item(i, 0) || !m_commands->item(i, 1)) {
            continue;
        }
        const QString &cmdName = m_commands->item(i, 0)->text();
        const QString &cmdData = m_commands->item(i, 1)->text();
        if(cmdName.size() && cmdData.size()) {
            m_settings.Run.setCommand(cmdName, cmdData);
        }
    }
    accept();
}

void RunPreferences::slotInitCell(int row, int)
{
    if (m_commands->rowCount() - 1 == row) {
        if (!m_commands->item(row, 0) || !m_commands->item(row, 1)) {
            return;
        }
        m_commands->setRowCount(row + 2);
    }
}

void RunPreferences::slotRemove()
{
    int row = m_commands->currentRow();
    if (m_commands->rowCount() > 1) {
        m_commands->removeRow(row);
    }
}

RunDelegate::RunDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      openIcon(IconProvider::fromTheme("document-open"))
{
}

void RunDelegate::paint(QPainter *painter, 
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const
{   
    if (index.column() == 1) {
        painter->save();
        QStyleOptionButton btn;
        QRect r = option.rect;
        int x, y;
        x = r.left() + r.width() - 16;
        y = r.top();
        btn.rect = QRect(x, y, 16, 16);
        btn.icon = openIcon;
        btn.iconSize = QSize(14, 14);
        btn.state = QStyle::State_Enabled;
        painter->drawText(r, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::EditRole).toString());
        option.widget->style()->drawControl (QStyle::CE_PushButtonLabel, &btn, painter);
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool RunDelegate::editorEvent(QEvent *event,
        QAbstractItemModel *model,
        const QStyleOptionViewItem &option,
        const QModelIndex &index)
{
    if (index.column() == 1) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            int clickX = e->x();
            int clickY = e->y();
            int x, y;
            QRect r = option.rect;
            x = r.left() + r.width() - 16;
            y = r.top();
            if (clickX > x && clickX < x + 16) {
                if (clickY > y && clickY < y + 16) {
                    QString f = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(parent()),
                            tr("Open File"));
                    QString oldData = model->data(index, Qt::EditRole).toString();
                    oldData.prepend(f);
                    model->setData(index, oldData, Qt::EditRole);
                    return true;
                }
            }
        }
    }
    return false;
}

