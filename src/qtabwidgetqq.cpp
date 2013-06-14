/*
 *
 * This file is part of the Notepadqq text editor.
 *
 * Copyright(c) 2010 Notepadqq team.
 * http://notepadqq.sourceforge.net/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "qtabwidgetqq.h"
#include "qsciscintillaqq.h"
#include "qtabwidgetscontainer.h"
#include "constants.h"
#include "mainwindow.h"
#include <QTabBar>
#include <QVBoxLayout>

int QTabWidgetqq::_newTabCount = 0;

QTabWidgetqq::QTabWidgetqq(QWidget *parent) :
    QTabWidget(parent)
{
    this->setObjectName("tabWidget");
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setDocumentMode(true);
    this->setTabsClosable(true);
    this->setMovable(true);
    //tabWidget1->setTabIcon(0, QIcon());
    this->setStyleSheet("QTabBar::tab { height: 24px; }");
    //tabWidget1->setIconSize(QSize(12,12));
}

int QTabWidgetqq::getTabIndexAt(const QPoint &pos)
{
    return this->tabBar()->tabAt(pos);
}

int QTabWidgetqq::addNewDocument()
{
    return addEditorTab(true, tr("new") + " " + QString::number(++_newTabCount));
}

/**
 * Adds a tab to the tabWidget
 *
 * @param   setFocus    If true, the new tab will receive focus
 * @param   title       The title of the new tab
 */
int QTabWidgetqq::addEditorTab(bool setFocus, QString title)
{
    this->setUpdatesEnabled(false);

    // Let's add a new tab...
    QWidget *widget = new QWidget(this);
    widget->setObjectName("singleTabWidget");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    // Create textbox
    QsciScintillaqq* sci = new QsciScintillaqq(widget);
    sci->setObjectName("editorWidget");

    connect(sci, SIGNAL(modificationChanged(bool)), this, SLOT(on_modification_changed(bool)));

    /* TODO
    connect(sci, SIGNAL(fileChanged(QString, QsciScintillaqq*)), SLOT(fileChanged(QString, QsciScintillaqq*)));
    connect(sci, SIGNAL(textChanged()), SLOT(on_scintillaTextChanged()));
    connect(sci, SIGNAL(selectionChanged()), SLOT(on_scintillaSelectionChanged()));
    connect(sci, SIGNAL(cursorPositionChanged(int,int)), SLOT(on_scintillaCursorPositionChanged(int,int)));
    connect(sci, SIGNAL(updateUI()), SLOT(on_scintillaUpdateUI()));
    */

    layout->addWidget(sci);
    widget->setLayout(layout);
    //this->setDocumentMode(true);


    /* TODO
    bool _showallchars = ui->actionShow_All_Characters->isChecked();
    updateScintillaPropertiesForAllTabs();
    if(_showallchars) {
        ui->actionShow_All_Characters->setChecked(true);
        on_actionShow_All_Characters_triggered();
    }
    */

    // Add the tab as last thing so we can use QSciScintillaqqAt method on currentTabChanged signal
    int index = this->addTab(widget, title);
    if(setFocus) {
        this->setCurrentIndex(index);
    }
    this->setTabIcon(index, QIcon(":/icons/icons/saved.png"));


    this->getTabWidgetsContainer()->_on_newQsciScintillaqqWidget(sci);

    sci->setFocus();
    // sci->SendScintilla(QsciScintilla::SCI_SETFOCUS, true);
    // sci->SendScintilla(QsciScintilla::SCI_GRABFOCUS);

    // updateGui(index, tabWidget1); TODO

    // Emettere segnale QsciScintillaqq_Created(QsciScintillaqq *sci)

    this->setUpdatesEnabled(true);

    return index;
}

QsciScintillaqq *QTabWidgetqq::focusQSciScintillaqq()
{
    return QSciScintillaqqAt(this->currentIndex());
}

QsciScintillaqq *QTabWidgetqq::QSciScintillaqqAt(int index)
{
    QWidget *widget = this->widget(index);

    if(widget != 0) {
        return widget->findChild<QsciScintillaqq *>("editorWidget");
    } else {
        return 0;
    }
}

QTabWidgetsContainer *QTabWidgetqq::getTabWidgetsContainer()
{
    QTabWidgetsContainer *container;
    do {
         container = qobject_cast<QTabWidgetsContainer *>(this->parentWidget());
    } while(!container);

    return container;
}

void QTabWidgetqq::on_text_changed()
{

}

void QTabWidgetqq::on_modification_changed(bool m)
{
    QsciScintillaqq *send = static_cast<QsciScintillaqq *>(sender());
    if(m) {
        this->setTabIcon(send->getTabIndex(), QIcon(":/icons/icons/unsaved.png"));
    } else {
        this->setTabIcon(send->getTabIndex(), QIcon(":/icons/icons/saved.png"));
    }
}
