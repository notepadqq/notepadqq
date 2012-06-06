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

#include "qsciscintillaqq.h"
#include "constants.h"
#include <QFileSystemWatcher>
#include <QKeyEvent>
#include <QIODevice>
#include <QTextCodec>
#include <QTextStream>
#include <Qsci/qsciscintillabase.h>

QsciScintillaqq::QsciScintillaqq(QWidget *parent) :
    QsciScintilla(parent)
{

    _fileWatchEnabled = true;
    encoding = "UTF-8";
    BOM = false;
    this->setFileName("");
    fswatch = new QFileSystemWatcher(parent);
    isCtrlPressed = false;
    setIgnoreNextSignal(false);
    connect(fswatch, SIGNAL(fileChanged(QString)), SLOT(internFileChanged(QString)));
    connect(this,SIGNAL(SCN_UPDATEUI()), SLOT(handleUpdateUI_All()));

    this->initialize();
}

QsciScintillaqq::~QsciScintillaqq()
{
    delete fswatch;
}

QString QsciScintillaqq::fileName()
{
    return _fileName;
}

void QsciScintillaqq::setFileName(QString filename)
{
    _fileName = filename;
    fswatch->removePath(fileName());
    if(filename != "")
        fswatch->addPath(filename);
}

void QsciScintillaqq::setFileWatchEnabled(bool enable)
{
    _fileWatchEnabled = enable;
}

void QsciScintillaqq::setIgnoreNextSignal(bool ignore)
{
    _ignoreNextSignal = ignore;
}

bool QsciScintillaqq::ignoreNextSignal()
{
    return _ignoreNextSignal;
}

bool QsciScintillaqq::fileWatchEnabled()
{
    return _fileWatchEnabled;
}

void QsciScintillaqq::internFileChanged(const QString &path)
{
    if(fileWatchEnabled())
    {
        if(ignoreNextSignal()) {
            setIgnoreNextSignal(false);
        } else {
            emit fileChanged(path, this);
        }
    }
    // Starts the filesystemwatcher again
    setFileName(this->fileName());
}

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
    if(e->key() == Qt::Key_Control) {
         isCtrlPressed = true;
    }

    emit keyPressed(e);
    QsciScintilla::keyPressEvent(e);
}

void QsciScintillaqq::keyReleaseEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Control) {
         isCtrlPressed = false;
    }

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

void QsciScintillaqq::handleUpdateUI_All()
{
     emit updateUI();
}

bool QsciScintillaqq::highlightTextRecurrence(int searchFlags, QString text, long searchFrom, long searchTo, int selector)
{
    //if(sci->findFirst(sci->selectedText(), false, false, true, false, true, 0, 0, false))

    if (searchFrom == searchTo)
        return false;

    while(1)
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

        searchFrom++;

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
    if(isCtrlPressed)
    {
        e->accept();

        int d = e->delta() / 120;
        if(d>0)
        {
            while(d > 0)
            {
                d -= 120;
                this->zoomIn();
            }
        } else
        {
            while(d < 0)
            {
                d += 120;
                this->zoomOut();
            }
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
    this->setMarginWidth(1, 47);
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
