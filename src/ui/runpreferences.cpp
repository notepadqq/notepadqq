#include "include/runpreferences.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QPushButton>
#include <QApplication>
RunPreferences::RunPreferences(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
{
    QVBoxLayout *v1 = new QVBoxLayout;
    QHBoxLayout *h1 = new QHBoxLayout;
    m_commands = new QTableWidget(1, 2);

    QStringList headers = (QStringList() << tr("Text") << tr("Command"));

    QHeaderView *vh = m_commands->verticalHeader();
    vh->sectionResizeMode(QHeaderView::Fixed);
    vh->setDefaultSectionSize(16);

    QHeaderView *hh = m_commands->horizontalHeader();
    hh->setStretchLastSection(true);

    m_commands->setAlternatingRowColors(true);
    m_commands->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_commands->setSortingEnabled(false);
    m_commands->setHorizontalHeaderLabels(headers);
    m_commands->setSelectionBehavior(QAbstractItemView::SelectRows);

    setMinimumSize(500, 200);

    v1->addWidget(m_commands);
    v1->addItem(h1);
    
    QFontMetrics fm(QApplication::font());
    QPushButton *btnAdd  = new QPushButton(tr("Add..."));
    QPushButton *btnEdit = new QPushButton(tr("Modify..."));
    QPushButton *btnRm   = new QPushButton(tr("Remove..."));
    h1->addWidget(btnAdd);
    h1->addWidget(btnEdit);
    h1->addWidget(btnRm);
    h1->setAlignment(Qt::AlignLeft);
    setLayout(v1);
}

RunPreferences::~RunPreferences()
{
}

void RunPreferences::slotAdd()
{

}

void RunPreferences::slotModify()
{

}

void RunPreferences::slotRemove()
{

}

