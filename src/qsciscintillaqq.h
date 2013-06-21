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

#ifndef QSCISCINTILLAQQ_H
#define QSCISCINTILLAQQ_H

#include <QWidget>
#include <Qsci/qsciscintilla.h>
#include <QFileSystemWatcher>

#define ScintillaStringData(s)      (s).constData()
#define ScintillaStringLength(s)    (s).size()

class QTabWidgetqq;

class QsciScintillaqq : public QsciScintilla
{
    Q_OBJECT
    Q_PROPERTY(QString _fileName READ fileName WRITE setFileName)

public:
    ~QsciScintillaqq();
    explicit QsciScintillaqq(QWidget *parent = 0);

    struct CharacterRange {
        long cpMin;
        long cpMax;
    };

    struct Sci_TextRange {
        struct CharacterRange chrg;
        char *lpstrText;
    };

    struct Sci_TextToFind {
        struct CharacterRange chrg;     // range to search
        char *lpstrText;                // the search pattern (zero terminated)
        struct CharacterRange chrgText; // returned as position of matching text
    };

    int            getTabIndex();
    bool           isNewEmptyDocument();
    void           autoSyntaxHighlight();
    void           forceUIUpdate();
    void           safeCopy();


    void           scrollCursorToCenter(int pos);
    CharacterRange getSelectionRange();

    QString       fileName();
    QString       encoding();
    QString       forcedLanguage();
    QTabWidgetqq* tabWidget();
    bool          BOM();

    int                             getSelectedTextCount();

    void          setFileName(QString filename);
    void          setEncoding(QString enc="UTF-8");
    void          setForcedLanguage(QString language);
    void          setBOM(bool yes=true);

private:
    typedef QByteArray ScintillaString;
    QString _fileName;
    QString _encoding;
    QString _forcedLanguage;
    bool    _BOM;

    int     oldSelectionLineFrom, oldSelectionIndexFrom, oldSelectionLineTo, oldSelectionIndexTo;

    void    keyPressEvent(QKeyEvent *e);
    void    keyReleaseEvent(QKeyEvent *e);
    void    initialize();

    void    applyGlobalStyles();

private slots:
    void    wheelEvent(QWheelEvent * e);

signals:
    void    keyPressed(QKeyEvent *e);
    void    keyReleased(QKeyEvent *e);
    void    updateUI();
    void    overtypeChanged(bool yes);

public slots:
    void                   updateLineMargin();
    bool                   overType();
    bool                   highlightTextRecurrence(int searchFlags, QString text, long searchFrom, long searchTo, int selector);
    QsciScintilla::EolMode guessEolMode();
    ScintillaString        convertTextQ2S(const QString &q) const;
};

#endif // QSCISCINTILLAQQ_H
