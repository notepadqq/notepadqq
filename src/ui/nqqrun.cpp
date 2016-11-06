#include <QApplication>
#include <QEasingCurve>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include "include/nqqrun.h"
#include "include/iconprovider.h"

using namespace NqqRun;

RunPreferences::RunPreferences(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    m_settings(NqqSettings::getInstance())
{ 
    QVBoxLayout *v1 = new QVBoxLayout;
    QHBoxLayout *h1 = new QHBoxLayout;
    QHBoxLayout *h2 = new QHBoxLayout;
    QHBoxLayout *h3 = new QHBoxLayout;
    QPushButton *btnOk   = new QPushButton(tr("OK"));
    QPushButton *btnCancel = new QPushButton(tr("Cancel"));
    RunDelegate *delegate = new RunDelegate(this);
    QShortcut *keyDelete = new QShortcut(QKeySequence("Delete"), this);

    QLabel *info = new QLabel(tr("\
    <h3>Special placeholders</h3><ul>\
    <li><em>\%fullpath\%</em> - Full path of the currently active file.</li>\
    <li><em>\%directory\%</em> - Directory of the currently active file.</li>\
    <li><em>\%filename\%</em> - Name of the currently active file.</li>\
    <li><em>\%selection\%</em> - Currently selected text.</li>\
    </ul>"));

    m_commands = new QTableWidget(1, 2);

    QHeaderView *vh = m_commands->verticalHeader();
    QHeaderView *hh = m_commands->horizontalHeader();
    vh->sectionResizeMode(QHeaderView::QHeaderView::Fixed);
    vh->setDefaultSectionSize(20);
    hh->setStretchLastSection(true);


    v1->addWidget(info);
    v1->addWidget(m_commands);
    v1->addItem(h3);
    
    h2->addWidget(btnOk);
    h2->addWidget(btnCancel);
    h2->setAlignment(Qt::AlignRight);
    h3->addItem(h1);
    h3->addItem(h2);

    setLayout(v1);
    setMinimumSize(500, 200);
    
    QMap <QString, QString> cmdData = m_settings.Run.getCommands();
    QSortFilterProxyModel *pModel = new QSortFilterProxyModel(this);
    pModel->setSourceModel(m_commands->model());

    m_commands->setAlternatingRowColors(true);
    m_commands->setSortingEnabled(false);
    m_commands->setHorizontalHeaderLabels((QStringList() << tr("Text") << tr("Command")));
    m_commands->setSelectionMode(QAbstractItemView::SingleSelection);
    m_commands->setItemDelegate(delegate);
    m_commands->setRowCount(cmdData.size() + 1);

    int workRow = 0;
    QMapIterator<QString, QString> it(cmdData);
    while(it.hasNext())
    {
        it.next();
        QTableWidgetItem *item = new QTableWidgetItem(it.key());
        m_commands->setItem(workRow, 0, item);
        item = new QTableWidgetItem(it.value());
        m_commands->setItem(workRow, 1, item);
        workRow++;
    }

    connect(keyDelete, &QShortcut::activated, this, &RunPreferences::slotRemove);
    connect(btnOk, &QPushButton::clicked, this, &RunPreferences::slotOk);
    connect(btnCancel, &QPushButton::clicked, this, &RunPreferences::reject);
    connect(delegate, &RunDelegate::needsRemoval, this, &RunPreferences::slotRemove);
    connect(m_commands, &QTableWidget::cellChanged, this, &RunPreferences::slotInitCell);
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
        if (cmdName.size() && cmdData.size()) {
            m_settings.Run.setCommand(cmdName, cmdData);
        }
    }
    accept();
}

void RunPreferences::slotInitCell(int row, int)
{
    QTableWidgetItem *iText = m_commands->item(row, 0);
    QTableWidgetItem *iCmd = m_commands->item(row, 1);
    if (!iText || !iCmd) {
        return;
    }

    if (m_commands->rowCount() - 1 == row) {
        if (iText->text().length() && iCmd->text().length()) {
            m_commands->setRowCount(row + 2);
        } else if (row == m_commands->rowCount() - 2) {
            m_commands->setRowCount(row + 1);
        }
    } else if (m_commands->rowCount() - 2 == row) {
        // Check to see if we can remove the last row safely.
        if (!iText->text().length() || !iCmd->text().length()) {
            int rmLast = 0;
            QTableWidgetItem *iLastText = m_commands->item(row + 1, 0);
            QTableWidgetItem *iLastCmd = m_commands->item(row + 1, 1);
            if (!iLastText || !iLastText->text().length()) {
                rmLast++;
            }
            if (!iLastCmd || !iLastCmd->text().length()) {
                rmLast++;
            }
            if (rmLast == 2) {
                m_commands->setRowCount(row + 1);
            }
        }
    }
}

void RunPreferences::slotRemove()
{
    int row = m_commands->currentRow();
    if (m_commands->rowCount() > 1 && row != m_commands->rowCount() - 1) {
        m_commands->removeRow(row);
    } else {
        if (m_commands->item(row, 0)) {
            m_commands->item(row, 0)->setText("");
        }
        if (m_commands->item(row, 1)) {
            m_commands->item(row, 1)->setText("");
        }
    }
}

RunDelegate::RunDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      openIcon(IconProvider::fromTheme("document-open")),
      rmIcon(IconProvider::fromTheme("edit-delete"))
{
}

void RunDelegate::paint(QPainter *painter, 
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{   
        
    if (index.column() == 1) {
        painter->save();
        QStyleOptionButton btnOpen;
        QRect r = option.rect;
        int x, y;
        x = r.left() + r.width() - 32;
        y = r.top() + 2;
        btnOpen.rect = QRect(x, y, 16, 16);
        btnOpen.icon = openIcon;
        btnOpen.iconSize = QSize(14, 14);
        btnOpen.state = QStyle::State_Enabled;
        r.setWidth(r.width() - 32);
        
        // Elide Text
        QFontMetrics fm(option.font);
        QString editText = fm.elidedText(index.data(Qt::EditRole).toString(),
                Qt::ElideRight,
                r.width());

        QPen oldPen = painter->pen();
        if (option.state & QStyle::State_Selected) {
            painter->setPen(QPen(QColor(option.palette.highlightedText().color())));
        }
        painter->drawText(r, Qt::AlignLeft | Qt::AlignVCenter, editText);
        painter->setPen(oldPen);
        option.widget->style()->drawControl (QStyle::CE_PushButtonLabel, &btnOpen, painter);

        QStyleOptionButton btnRm;
        x += 16;
        btnRm.rect = QRect(x, y, 16, 16);
        btnRm.icon = rmIcon;
        btnRm.iconSize = QSize(14, 14);
        btnRm.state = QStyle::State_Enabled;
        option.widget->style()->drawControl (QStyle::CE_PushButtonLabel, &btnRm, painter);

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
            x = r.left() + r.width() - 32;
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

            x = r.left() + r.width() - 16;
            y = r.top();
            if (clickX > x && clickX < x + 16) {
                if (clickY > y && clickY < y + 16) {
                    emit needsRemoval();
                    return true;
                }
            }
 

        }
    }
    return false;
}

RunDialog::RunDialog(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    m_settings(NqqSettings::getInstance()),
    m_saved(false)
{    
    QVBoxLayout *v1 = new QVBoxLayout;
    QHBoxLayout *h1 = new QHBoxLayout;
    QHBoxLayout *h2 = new QHBoxLayout;
    QHBoxLayout *h3 = new QHBoxLayout;
    QPushButton *btnOK = new QPushButton(tr("OK"));
    QPushButton *btnCancel = new QPushButton(tr("Cancel"));
    QPushButton *btnSave = new QPushButton(tr("Save..."));

    QLabel *info = new QLabel(tr("\
    <h3>Special placeholders</h3><ul>\
    <li><em>\%fullpath\%</em> - Full path of the currently active file.</li>\
    <li><em>\%directory\%</em> - Directory of the currently active file.</li>\
    <li><em>\%filename\%</em> - Name of the currently active file.</li>\
    <li><em>\%selection\%</em> - Currently selected text.</li>\
    </ul>"));

    m_command = new QLineEdit(this);
    m_status = new QLabel;

    QPalette palette = m_status->palette();
    palette.setColor(m_status->foregroundRole(), Qt::darkGreen);
    m_status->setPalette(palette);

    v1->addWidget(info);
    v1->addWidget(m_command);
    v1->addLayout(h3);

    h1->addWidget(btnSave);
    h1->addWidget(m_status);
    h1->setAlignment(Qt::AlignLeft);
    h2->addWidget(btnCancel);
    h2->addWidget(btnOK);
    h2->setAlignment(Qt::AlignRight);
    h3->addLayout(h1);
    h3->addLayout(h2);

    setLayout(v1);
    setMinimumSize(300, 100);

    connect(btnOK, &QPushButton::clicked, this, &RunDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &RunDialog::reject);
    connect(btnSave, &QPushButton::clicked, this, &RunDialog::slotSave);
    connect(m_command, &QLineEdit::returnPressed, this, &RunDialog::accept);
}

RunDialog::~RunDialog()
{
}

void RunDialog::slotSave()
{
    bool ok;
    QString name = QInputDialog::getText(this, 
            tr("Choose the name to be displayed in the run menu."),
            tr("Command Name:"),
            QLineEdit::Normal,
            m_command->text(), 
            &ok);
    if (ok && !name.isEmpty() && !m_command->text().isEmpty()) {
        m_settings.Run.setCommand(name, m_command->text());
        m_saved = 1;

        //Graphical fade for save state
        m_status->setText(tr("Command saved..."));
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(m_status);
        m_status->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
        a->setDuration(10000);
        a->setStartValue(5);
        a->setEndValue(0);
        a->setEasingCurve(QEasingCurve::Linear);
        a->start(QPropertyAnimation::DeleteWhenStopped);
        connect(a, &QPropertyAnimation::finished, this, &RunDialog::slotHideStatus);
    }
}

void RunDialog::slotHideStatus()
{
    m_status->setText("");
}

bool RunDialog::saved()
{
    return m_saved;
}

QString RunDialog::getCommandInput()
{
    return m_command->text();
}

QStringList RunDialog::parseCommandString(QString cmd) {
    QStringList parts;
    const char OUT = '\0';
    char quote = OUT;
    QString curr = "";
    for (int i = 0; i < cmd.length(); i++) {
        if (cmd[i] == '"') {
            if (quote == '"') {
                quote = OUT;
                parts.append(curr);
                curr = "";
            } else if (quote == '\'') {
                curr += '"';
            } else {
                quote = '"';
            }

        } else if (cmd[i] == '\'') {
            if (quote == '\'') {
                quote = OUT;
                parts.append(curr);
                curr = "";
            } else if (quote == '"') {
                curr += '\'';
            } else {
                quote = '\'';
            }

        } else if (cmd[i] == '\\') {

            if (i+1 < cmd.length()) {
                i++;
                if (quote == '\'') {
                    if (cmd[i] == '\'') {
                        curr += '\'';
                    } else {
                        curr += '\\';
                        curr += cmd[i];
                    }
                } else if (quote == '"') {
                    if (cmd[i] == '"') {
                        curr += '"';
                    } else if (cmd[i] == '\\') {
                        curr += '\\';
                    } else {
                        curr += '\\';
                        curr += cmd[i];
                    }
                } else {
                    curr += cmd[i];
                }
            } else {
                curr += '\\';
            }

        } else if (cmd[i] == ' ') {
            if (quote == OUT) {
                if (curr.length() > 0) {
                    parts.append(curr);
                    curr = "";
                }
            } else {
                curr += ' ';
            }
        } else {
            curr += cmd[i];
        }
    }
    if (curr.length() > 0) {
        parts.append(curr);
    }

    return parts;
}
