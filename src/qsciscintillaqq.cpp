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

#include "userlexer.h"

QsciScintillaqq::QsciScintillaqq(QWidget *parent) :
    QsciScintilla(parent)
{
    _encoding   = "UTF-8";
    _BOM        = false;
    _forcedLanguage = "";
    _fileName   = "";

    connect(this, SIGNAL(SCN_UPDATEUI(int)), this, SIGNAL(updateUI()));
    connect(this, SIGNAL(linesChanged()), this, SLOT(updateLineMargin()) );

    this->initialize();
}

QsciScintillaqq::~QsciScintillaqq()
{
}

QString QsciScintillaqq::fileName()
{
    return _fileName;
}

void QsciScintillaqq::setFileName(QString filename)
{
    _fileName = filename;
}

QString QsciScintillaqq::encoding()
{
    return _encoding;
}

void QsciScintillaqq::setEncoding(QString enc)
{
    _encoding = enc;
}

bool QsciScintillaqq::BOM()
{
    return _BOM;
}

void QsciScintillaqq::setBOM(bool yes)
{
    _BOM = yes;
}

//Get the number of characters in the current selection.
int QsciScintillaqq::getSelectedTextCount()
{
    CharacterRange range = getSelectionRange();
    return (range.cpMax - range.cpMin);
}

void QsciScintillaqq::scrollCursorToCenter(int pos)
{
    SendScintilla(SCI_GOTOPOS,pos);
    int line                     = SendScintilla(SCI_LINEFROMPOSITION,pos);
    int firstVisibleDisplayLine  = SendScintilla(SCI_GETFIRSTVISIBLELINE);
    int firstVisibleDocumentLine = SendScintilla(SCI_DOCLINEFROMVISIBLE, firstVisibleDisplayLine);
    int nbLine                   = SendScintilla(SCI_LINESONSCREEN, firstVisibleDisplayLine);
    int lastVisibleDocumentLine  = SendScintilla(SCI_DOCLINEFROMVISIBLE, firstVisibleDisplayLine + nbLine);

    int middleLine;
    if( line - firstVisibleDocumentLine < lastVisibleDocumentLine - line)
        middleLine = firstVisibleDocumentLine + nbLine/2;
    else
        middleLine = lastVisibleDocumentLine - nbLine/2;

    int nbLinesToScroll = line - middleLine;
    scroll(0,nbLinesToScroll);
}

QsciScintilla::EolMode QsciScintillaqq::guessEolMode()
{
    int             _docLength = this->length();
    char*           _docBuffer = (char*)this->SendScintilla(QsciScintilla::SCI_GETCHARACTERPOINTER);
    QTextCodec*     codec      = QTextCodec::codecForName(this->encoding().toUtf8());
    QByteArray      a          = codec->fromUnicode(QString::fromUtf8(_docBuffer,_docLength));

    int _win = a.count("\r\n");
    int _mac = a.count('\r');
    int _unix= a.count('\n');

    if( (_win+_mac+_unix) == 0) {
        return static_cast<QsciScintilla::EolMode>(-1);
    }
    if(_win >= _mac && _win >= _unix)
        return QsciScintilla::EolWindows;
    else if(_mac > _win && _mac > _unix)
        return QsciScintilla::EolMac;
    else if(_unix > _win && _unix > _unix)
        return QsciScintilla::EolUnix;
    return QsciScintilla::EolUnix;
}

bool QsciScintillaqq::overType()
{
    int ovr = SendScintilla(QsciScintillaBase::SCI_GETOVERTYPE);
    return (ovr==1);
}

void QsciScintillaqq::safeCopy()
{
    const int contentLength = this->getSelectedTextCount();
    char*     stringData    = new char[contentLength+1];

    this->SendScintilla(SCI_GETSELTEXT,0,(void*)stringData);
    for(int i=0;i<contentLength;i++) {
        if(stringData[i] == '\0')
            stringData[i] = ' ';
    }
    stringData[contentLength] = '\0';
    QApplication::clipboard()->setText(stringData);
    delete [] stringData;
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
    }else {
        switch(e->key())
        {
        case Qt::Key_Insert:
            emit overtypeChanged(overType());
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
    QFont* system_font = MainWindow::instance()->systemMonospace();
    qDebug() << "system font: " << system_font->family() << " " << system_font->pointSize();


    this->setMarginLineNumbers(1, true);
    this->setFolding(QsciScintillaqq::BoxedTreeFoldStyle);
    this->setAutoIndent(true);
    this->setAutoCompletionThreshold(2);
    this->setUtf8(true);

    // GLOBALS
    applyGlobalStyles();

    this->setBraceMatching(QsciScintillaqq::SloppyBraceMatch);
    this->setCaretLineVisible(true);

    this->SendScintilla(QsciScintilla::SCI_INDICSETSTYLE, SELECTOR_DefaultSelectionHighlight, QsciScintilla::INDIC_ROUNDBOX);
    this->SendScintilla(QsciScintilla::SCI_INDICSETALPHA, SELECTOR_DefaultSelectionHighlight, 100);
    this->SendScintilla(QsciScintilla::SCI_INDICSETUNDER, SELECTOR_DefaultSelectionHighlight, true);
    this->SendScintilla(SCI_SETYCARETPOLICY,QsciScintilla::CARET_SLOP);
}


void QsciScintillaqq::applyGlobalStyles()
{
    ShrPtrStylerDefinition glob_style = MainWindow::instance()->getLexerFactory()->getGlobalStyler();
    ShrPtrWordsStyle       def_style  = glob_style->words_stylers_by_name.value(stylename::DEFAULT);
    this->setColor(def_style->fg_color);
    this->setPaper(def_style->bg_color);

    ShrPtrWordsStyle caret_style       = glob_style->words_stylers_by_name.value(stylename::CARET);
    ShrPtrWordsStyle indent_style      = glob_style->words_stylers_by_name.value(stylename::INDENT_GUIDELINE);
    ShrPtrWordsStyle select_style      = glob_style->words_stylers_by_name.value(stylename::SELECTED_TEXT);
    ShrPtrWordsStyle fold_margin_style = glob_style->words_stylers_by_name.value(stylename::FOLD_MARGIN);
    ShrPtrWordsStyle margins_style     = glob_style->words_stylers_by_name.value(stylename::LINE_NUMBER_MARGIN);

    this->setSelectionBackgroundColor(select_style->bg_color);
    this->setSelectionForegroundColor(select_style->fg_color);

    this->setFoldMarginColors(fold_margin_style->fg_color, fold_margin_style->bg_color);
    this->setMarginsBackgroundColor(margins_style->bg_color);
    this->setMarginsForegroundColor(margins_style->fg_color);

    this->setCaretForegroundColor(caret_style->fg_color);
    this->setCaretLineBackgroundColor(indent_style->fg_color);

    this->setIndentationGuidesForegroundColor(indent_style->fg_color);

    this->SendScintilla(QsciScintilla::SCI_INDICSETFORE, SELECTOR_DefaultSelectionHighlight, indent_style->fg_color.value());
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

QTabWidgetqq *QsciScintillaqq::tabWidget()
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
    if(this->length()==0
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
    // DELETE THE OLD LEXER
    if ( lexer() ) lexer()->deleteLater();
    QsciLexer* lex;

    if(_forcedLanguage.isEmpty()) {
        QFileInfo info(fileName());
        lex = MainWindow::instance()->getLexerFactory()->createLexer( info, this);
    }else {
        lex = MainWindow::instance()->getLexerFactory()->createLexerByName( _forcedLanguage, this);
    }
    if ( lex ) {
        setLexer(lex);
    }
}

QString QsciScintillaqq::forcedLanguage()
{
    return _forcedLanguage;
}

void QsciScintillaqq::setForcedLanguage(QString language)
{
    _forcedLanguage = language.toLower();
    autoSyntaxHighlight();
}

//Keeps the line margin readable at all times
void QsciScintillaqq::updateLineMargin() {
    setMarginWidth(1,QString("00%1").arg(lines()));
}

QsciScintillaqq::CharacterRange QsciScintillaqq::getSelectionRange()
{
    CharacterRange crange;
    crange.cpMin = SendScintilla(SCI_GETSELECTIONSTART);
    crange.cpMax = SendScintilla(SCI_GETSELECTIONEND);
    return crange;
}
