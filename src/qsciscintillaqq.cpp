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
#include "mainwindow.h"
#include "qsciscintillaqq.h"
#include "qtabwidgetqq.h"
#include "constants.h"
#include "generalfunctions.h"
#include <QFileSystemWatcher>
#include <QKeyEvent>
#include <QIODevice>
#include <QTextCodec>
#include <QTextStream>
#include <Qsci/qsciscintillabase.h>
#include <QtXml/qdom.h>
#include <QHash>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QChar>

/*#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexerbatch.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercsharp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerd.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexerfortran.h>
#include <Qsci/qscilexerfortran77.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexeridl.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexermakefile.h>
#include <Qsci/qscilexerpascal.h>
#include <Qsci/qscilexerperl.h>
#include <Qsci/qscilexerpostscript.h>
#include <Qsci/qscilexerpov.h>
#include <Qsci/qscilexerproperties.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexerspice.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertcl.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexerverilog.h>
#include <Qsci/qscilexervhdl.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexeryaml.h>

#include <Qsci/qscilexercustom.h>*/
#include "userlexer.h"

QsciScintillaqq::QsciScintillaqq(QWidget *parent) :
    QsciScintilla(parent)
{
    encoding = "UTF-8";
    BOM = false;
    this->setFileName("");
    connect(this, SIGNAL(SCN_UPDATEUI(int)), this, SIGNAL(updateUI()));
    connect(this, SIGNAL(linesChanged()), this, SLOT(updateLineMargin()) );
    this->initialize();
}

QsciScintillaqq::~QsciScintillaqq()
{
    //delete fswatch;
}

QString QsciScintillaqq::fileName()
{
    return _fileName;
}

void QsciScintillaqq::setFileName(QString filename)
{
    _fileName = filename;
}

QsciScintilla::EolMode QsciScintillaqq::guessEolMode()
{
    int   _docLength = this->length();
    char *_docBuffer = (char*)this->SendScintilla(QsciScintilla::SCI_GETCHARACTERPOINTER);
    QTextCodec *codec = QTextCodec::codecForName(this->encoding.toUtf8());
    QByteArray a = codec->fromUnicode(QString::fromUtf8(_docBuffer,_docLength));

    int _win = a.count("\r\n");
    int _mac = a.count('\r');
    int _unix= a.count('\n');

    if( (_win+_mac+_unix) == 0) {
        return static_cast<QsciScintilla::EolMode>(-1);
    }
    if(_win >= _mac && _win >= _unix)
    {
        return QsciScintilla::EolWindows;
    } else if(_mac > _win && _mac > _unix) {
        return QsciScintilla::EolMac;
    } else if(_unix > _win && _unix > _unix) {
        return QsciScintilla::EolUnix;
    }
    return QsciScintilla::EolUnix;
}

bool QsciScintillaqq::overType()
{
    int ovr = SendScintilla(QsciScintillaBase::SCI_GETOVERTYPE);
    if(ovr == 1)
        return true;
    else
        return false;
}

void QsciScintillaqq::safeCopy()
{
    QClipboard *clipboard = QApplication::clipboard();
    int  contentLength = this->length()-1;
    char stringData[contentLength+1];
    this->SendScintilla(SCI_GETSELTEXT,0,(void*)&stringData);
    //Replace NUL byte characters with a space so it can be pasted into other places.
    for(int i=0;i<contentLength-1;i++) {
        if(stringData[i] == '\0') {
            stringData[i] = ' ';
        }
    }
    stringData[contentLength] = '\0';
    clipboard->setText(stringData);
}

void QsciScintillaqq::keyPressEvent(QKeyEvent *e)
{
    emit keyPressed(e);
    QsciScintilla::keyPressEvent(e);
}

void QsciScintillaqq::keyReleaseEvent(QKeyEvent *e)
{
    if(e->modifiers() & Qt::ControlModifier) {
        switch(e->key()) {
        case Qt::Key_C:
            safeCopy();
            break;
        }
    }
    emit keyReleased(e);
    QsciScintilla::keyReleaseEvent(e);
}

bool QsciScintillaqq::highlightTextRecurrence(int searchFlags, QString text, long searchFrom, long searchTo, int selector)
{
    //if(sci->findFirst(sci->selectedText(), false, false, true, false, true, 0, 0, false))

    if (searchFrom == searchTo)
        return false;

    while(searchFrom < searchTo)
    {
        SendScintilla(SCI_SETSEARCHFLAGS, searchFlags);
        SendScintilla(SCI_SETTARGETSTART, searchFrom);
        SendScintilla(SCI_SETTARGETEND, searchTo);
        ScintillaString s = convertTextQ2S(text);
        if(SendScintilla(SCI_SEARCHINTARGET, ScintillaStringLength(s), ScintillaStringData(s)) == -1)
        {
            return false;
        }

        // It was found.
        long targstart = SendScintilla(SCI_GETTARGETSTART);
        long targend = SendScintilla(SCI_GETTARGETEND);

        this->SendScintilla(QsciScintilla::SCI_SETINDICATORCURRENT, selector);
        this->SendScintilla(QsciScintilla::SCI_INDICATORFILLRANGE,
                           targstart,
                           targend - targstart);

        searchFrom = targend + 1;
    }
    return true;
}

QsciScintillaqq::ScintillaString QsciScintillaqq::convertTextQ2S(const QString &q) const
{
    if (isUtf8())
        return q.toUtf8();

    return q.toLatin1();
}

void QsciScintillaqq::wheelEvent(QWheelEvent * e)
{
    if(e->modifiers() & Qt::ControlModifier)
    {
        if( e->delta() < 0) {
            MainWindow::instance()->on_actionZoom_Out_triggered();
        }else if( e->delta() > 0) {
            MainWindow::instance()->on_actionZoom_In_triggered();
        }
    } else
    {
        QsciScintilla::wheelEvent(e);
    }
}

/**
 * Initialize Scintilla settings
 */
void QsciScintillaqq::initialize()
{
    // Set font
    QFont *f = new QFont("Courier New", 10, -1, false);
    this->setFont(*f);
    QColor *c = new QColor("#000000"); // DB8B0B
    this->setColor(*c);
    this->setCaretForegroundColor(QColor("#5E5E5E"));


    this->setMarginLineNumbers(1, true);
    this->setFolding(QsciScintillaqq::BoxedTreeFoldStyle);
    this->setAutoIndent(true);
    this->setAutoCompletionThreshold(2);
    this->setUtf8(true);
    //sci->SendScintilla(QsciScintilla::SCI_SETCODEPAGE, 950);
    //sci->SendScintilla(QsciScintilla::SCI_SETFOLDFLAGS, QsciScintilla::SC_FOLDFLAG_LINEBEFORE_CONTRACTED + QsciScintilla::SC_FOLDFLAG_LINEAFTER_CONTRACTED);

    /*QsciAPIs apis(&lex);
    apis.add("test");
    apis.add("test123");
    apis.add("foobar");
    apis.prepare();
    lex.setAPIs(&apis);*/

    // autoLexer(sci->fileName(), sci); TODO

    this->setBraceMatching(QsciScintillaqq::SloppyBraceMatch);
    this->setCaretLineVisible(true);
    this->setCaretLineBackgroundColor(QColor("#E6EBF5"));
    this->setIndentationGuidesForegroundColor(QColor("#C0C0C0"));

    this->SendScintilla(QsciScintilla::SCI_INDICSETSTYLE, SELECTOR_DefaultSelectionHighlight, QsciScintilla::INDIC_ROUNDBOX);
    this->SendScintilla(QsciScintilla::SCI_INDICSETFORE, SELECTOR_DefaultSelectionHighlight, 0x00FF24);
    this->SendScintilla(QsciScintilla::SCI_INDICSETALPHA, SELECTOR_DefaultSelectionHighlight, 100);
    this->SendScintilla(QsciScintilla::SCI_INDICSETUNDER, SELECTOR_DefaultSelectionHighlight, true);

    delete f;
    delete c;
}

int QsciScintillaqq::getTabIndex()
{
    QWidget *widget = this->parentWidget();
    while(widget->objectName() != "singleTabWidget")
    {
        widget = widget->parentWidget();
    }

    QWidget *tabwidget = widget->parentWidget();
    while(tabwidget->objectName() != "tabWidget")
    {
        tabwidget = tabwidget->parentWidget();
    }

    QTabWidgetqq *tabwidget_cast = static_cast<QTabWidgetqq *>(tabwidget);

    return tabwidget_cast->indexOf(widget);
}

QTabWidgetqq *QsciScintillaqq::getTabWidget()
{
    QWidget *tabwidget = this->parentWidget();
    while(tabwidget->objectName() != "tabWidget")
    {
        tabwidget = tabwidget->parentWidget();
    }

    return static_cast<QTabWidgetqq *>(tabwidget);
}

/**
* Detects if the document is a new empty document
*
* @return true or false
*/
bool QsciScintillaqq::isNewEmptyDocument()
{
    if(this->text() == ""
       && this->isModified() == false
       && this->fileName() == "" ) {

        return true;
    } else {
        return false;
    }
}

void QsciScintillaqq::forceUIUpdate()
{
    emit updateUI();
}

void QsciScintillaqq::autoSyntaxHighlight()
{
    QFont *f = MainWindow::instance()->systemMonospace();
    QFileInfo info(fileName());

    // TODO. WHERE SHOULD I DELETE THE CREATED LEXER???
    QsciLexer* lex = MainWindow::instance()->getLexerFactory()->createLexer(info, this);
    if ( lex ) {
        lex->setDefaultFont(*f);
        lex->setFont(*f);
        setLexer(lex);
    }
}

//Keeps the line margin readable at all times
void QsciScintillaqq::updateLineMargin() {
    setMarginWidth(1,QString("00%1").arg(lines()));
}
