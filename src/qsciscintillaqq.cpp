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

#include <Qsci/qscilexerbash.h>
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
#include <Qsci/qscilexerproperties.h> /**/
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

#include <Qsci/qscilexercustom.h>
#include <userlexer.h>

QsciScintillaqq::QsciScintillaqq(QWidget *parent) :
    QsciScintilla(parent)
{
//    _fileWatchEnabled = true;
    encoding = "UTF-8";
    BOM = false;
    this->setFileName("");
//    fswatch = new QFileSystemWatcher(parent);
//    isCtrlPressed = false;
//    setIgnoreNextSignal(false);
//    connect(fswatch, SIGNAL(fileChanged(QString)), SLOT(internFileChanged(QString)));
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
//    if(filename != "") {
//        fswatch->removePath(fileName());
//        fswatch->addPath(filename);
//    }
}

//void QsciScintillaqq::setFileWatchEnabled(bool enable)
//{
//    _fileWatchEnabled = enable;
//}

//void QsciScintillaqq::setIgnoreNextSignal(bool ignore)
//{
//    _ignoreNextSignal = ignore;
//}

//bool QsciScintillaqq::ignoreNextSignal()
//{
//    return _ignoreNextSignal;
//}

//bool QsciScintillaqq::fileWatchEnabled()
//{
//    return _fileWatchEnabled;
//}

//void QsciScintillaqq::internFileChanged(const QString &path)
//{
//    if(fileWatchEnabled())
//    {
//        if(ignoreNextSignal()) {
//            setIgnoreNextSignal(false);
//        } else {
//            emit fileChanged(path, this);
//        }
//    }
//    // Starts the filesystemwatcher again
//    setFileName(this->fileName());
//}

QsciScintilla::EolMode QsciScintillaqq::guessEolMode()
{
    QString a = this->text();
    int _win = a.count(QRegExp("\r\n"));
    int _mac = a.count(QRegExp("\r"));
    int _unix= a.count(QRegExp("\n"));

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

void QsciScintillaqq::keyPressEvent(QKeyEvent *e)
{
    //if(e->key() == Qt::Key_Control) {
    //     isCtrlPressed = true;
    //}

    emit keyPressed(e);
    QsciScintilla::keyPressEvent(e);
}

void QsciScintillaqq::keyReleaseEvent(QKeyEvent *e)
{
    //if(e->key() == Qt::Key_Control) {
    //     isCtrlPressed = false;
    //}

    emit keyReleased(e);
    QsciScintilla::keyReleaseEvent(e);
}

bool QsciScintillaqq::write(QIODevice *io)
{
    if(!io->open(QIODevice::WriteOnly))
        return false;

    QTextCodec *codec = QTextCodec::codecForName(encoding.toUtf8());
    QString textToSave = this->text();
    if(BOM)
    {
        textToSave = QChar(QChar::ByteOrderMark) + textToSave;
    }
    QByteArray string = codec->fromUnicode(textToSave);

    if(io->write(string) == -1)
        return false;
    io->close();

    return true;
}

bool QsciScintillaqq::read(QIODevice *io)
{
    return this->read(io, "UTF-8");
}

bool QsciScintillaqq::read(QIODevice *io, QString readEncodedAs)
{
    if(!io->open(QIODevice::ReadOnly))
        return false;

    QTextStream stream ( io );
    QString txt;

    // stream.setCodec("Windows-1252");
    stream.setCodec(readEncodedAs.toUtf8());
    // stream.setCodec("UTF-16BE");
    // stream.setCodec("UTF-16LE");


    txt = stream.readAll();
    io->close();

    this->setText(txt);

    return true;
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
    //if(isCtrlPressed)
    if(e->modifiers() & Qt::ControlModifier)
    {
//        e->accept();

//        int d = e->delta() / 120;
//        if(d>0)
//        {
//            while(d > 0)
//            {
//                d -= 120;
//                this->zoomIn();
//            }
//        } else
//        {
//            while(d < 0)
//            {
//                d += 120;
//                this->zoomOut();
//            }
//        }
        if( e->delta() < 0) {
            this->zoomOut();
        }else if( e->delta() > 0) {
            this->zoomIn();
        }
        this->updateLineMargin();
        this->syncZoom();
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
    this->updateLineMargin();
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
    /* We'll parse the example.xml */
    QFile *file = new QFile("/home/daniele/.wine/drive_c/Program Files/Notepad++/langs.model.xml");

    QDomDocument doc( "myDocument" );
    doc.setContent( file );                        // myFile is a QFile

    QDomElement docElement = doc.documentElement();   // docElement now refers to the node "xml"
    QDomNode node;

    node = docElement.firstChildElement("Languages");
    if(!node.isNull()) {
        node = node.firstChildElement("Language");
        while(!node.isNull()) {
            QString languageName = node.attributes().namedItem("name").nodeValue();

            if(languageName == "cpp") {
                QStringList extensions = node.attributes().namedItem("ext").nodeValue().split(" ", QString::SkipEmptyParts);
                QString commentLine = node.attributes().namedItem("commentLine").nodeValue();
                QString commentStart = node.attributes().namedItem("commentStart").nodeValue();
                QString commentEnd = node.attributes().namedItem("commentEnd").nodeValue();
                QHash<QString, QStringList> kwclass;

                QDomNode keywords_node = node.firstChildElement("Keywords");
                while(!keywords_node.isNull()) {
                    QString keyword_class_name = keywords_node.attributes().namedItem("name").nodeValue();

                    QStringList keywords = keywords_node.toElement().text().split(" ", QString::SkipEmptyParts);
                    kwclass.insert(keyword_class_name, keywords);

                    keywords_node = keywords_node.nextSiblingElement("Keywords");
                }

                //this->lexer()->setDefaultColor(QColor("#000000"));

                break;
            }

            node = node.nextSiblingElement("Language");
        }
    }

    return;



    QFont *f;
    f = new QFont("Courier New", 10, -1, false);

    if(this->fileName() != "")
    {

        QString ext = QFileInfo(this->fileName()).suffix().toLower();
        if(ext=="cmake")
        {
            QsciLexerCMake lex(this);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            this->setLexer(&lex);
        } else if(ext=="cs")
        {
            QsciLexerCSharp lex(this);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            this->setLexer(&lex);
        } else if(ext=="css")
        {
            QsciLexerCSS lex(this);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            this->setLexer(&lex);
        } else if(ext=="d")
        {
            QsciLexerD lex(this);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            this->setLexer(&lex);
        } else if(ext=="diff" || ext=="patch")
        {
            QsciLexerDiff lex(this);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            this->setLexer(&lex);
        } else if(ext=="f" || ext=="for" || ext=="f90" || ext=="f95")
        {
            QsciLexerFortran lex(this);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            this->setLexer(&lex);
        } else if(ext=="f77")
        {
            QsciLexerFortran77 lex(this);

            lex.setDefaultFont(*f);
            lex.setFont(*f);
            this->setLexer(&lex);
        }
        else
        {
            // Let's try with mime-types!

            QString fileMime = generalFunctions::getFileMime(this->fileName());
            if(fileMime == "text/html" ||
               fileMime == "text/x-php")
            {
                    QsciLexerHTML lex(this);

                    lex.setDefaultFont(*f);
                    //lex.setDefaultColor(QColor("#000000"));
                    //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                    lex.setFont(*f);
                    this->setLexer(&lex); // ** TODO SEGFAULT? **
            }
            else if( fileMime == "text/x-c")
            {
                     QsciLexerCPP lex(this);

                     lex.setDefaultFont(*f);
                     //lex.setDefaultColor(QColor("#000000"));
                     //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                     lex.setFont(*f);
                     this->setLexer(&lex);
            }
            else if( fileMime == "text/x-shellscript" )
            {
                     QsciLexerBash lex(this);

                     lex.setDefaultFont(*f);
                     //lex.setDefaultColor(QColor("#000000"));
                     //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                     lex.setFont(*f);
                     this->setLexer(&lex);
            }
            else if( fileMime == "application/xml" )
            {
                     QsciLexerXML lex(this);

                     lex.setDefaultFont(*f);
                     //lex.setDefaultColor(QColor("#000000"));
                     //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                     lex.setFont(*f);
                     this->setLexer(&lex);
            }
            else if( fileMime == "text/x-msdos-batch" )
            {
                     QsciLexerBatch lex(this);

                     lex.setDefaultFont(*f);
                     //lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
                     lex.setFont(*f);
                     this->setLexer(&lex);
            }
            /* else if( ext == "test")
{
userLexer lex(this);
f = new QFont("Courier New", 10, -1, false);
lex.setDefaultFont(*f);
lex.setDefaultColor(QColor("#000000"));
//lex.setColor(QColor("#ff3300"), QsciLexerHTML::PHPKeyword);
lex.setFont(*f);
this->setLexer(&lex);
} */
            else
            {
                    // Plain text
                    this->setLexer(0);
            }

        }

    } else
    {
        this->setLexer(0);
    }

    if(this->lexer() != 0)
    {
        //
    }

}

//Keeps the line margin readable at all times
void QsciScintillaqq::updateLineMargin() {
    setMarginWidth(1,QString("00%1").arg(lines()));
}

//Ensures all QSciScintillaqq widgets within the same tab group keep the same zoom level.
void QsciScintillaqq::syncZoom() {
    QTabWidgetqq* tabWidget = this->getTabWidget();
    int maxindex = tabWidget->count();
    double currentZoom = this->SendScintilla(QsciScintillaBase::SCI_GETZOOM);
    for(int i=0;i<maxindex;i++){
        tabWidget->QSciScintillaqqAt(i)->zoomTo(currentZoom);
        tabWidget->QSciScintillaqqAt(i)->updateLineMargin();
    }
}

QString QsciScintillaqq::baseName()
{
    QFile file(this->fileName());
    if(!file.exists()) {
        return "";
    }
    QFileInfo fi(file);

    return fi.fileName();
}
