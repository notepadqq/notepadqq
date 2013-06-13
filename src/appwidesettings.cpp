#include "appwidesettings.h"
#include "mainwindow.h"
#include <QSettings>

namespace widesettings {
    const char * SETTING_WRAP_MODE      = "wrap_mode";
    const char * SETTING_SHOW_ALL_CHARS = "show_all_chars";
    const char * SETTING_EOL_MODE  = "eol_mode";

    bool apply_wrap_mode(QsciScintilla::WrapMode m, QsciScintillaqq* w)
    {
        if ( !w )                 return false;
        if ( m == w->wrapMode() ) return false;
        w->setWrapMode(m);
        MainWindow::instance()->getSettings()->setValue(SETTING_WRAP_MODE, m);
        return true;
    }

    bool apply_invisible_chars(bool v, QsciScintillaqq* w)
    {
        if ( !w )                 return false;
        w->setEolVisibility(v);
        w->setWhitespaceVisibility( v ? QsciScintillaqq::WsVisible : QsciScintillaqq::WsInvisible );
        MainWindow::instance()->getSettings()->setValue(SETTING_SHOW_ALL_CHARS, v);
        return true;
    }

    //New and current document
    bool apply_eol_mode(QsciScintilla::EolMode m, QsciScintillaqq* w)
    {
        if( !w )               return false;
        if( m == w->eolMode()) return false;
        w->setEolMode(m);
        w->convertEols(m);
        MainWindow::instance()->getSettings()->setValue(SETTING_EOL_MODE, m);
        return true;
    }

    bool toggle_word_wrap(QsciScintillaqq* w)
    {
        if ( !w ) return false;
        return apply_wrap_mode(
                    (w->wrapMode() == QsciScintilla::WrapNone ?
                     QsciScintilla::WrapWord : QsciScintilla::WrapNone), w );
    }

    bool toggle_invisible_chars(QsciScintillaqq* w)
    {
        if ( !w ) return false;
        bool visible = w->eolVisibility() || w->whitespaceVisibility();
        return apply_invisible_chars(!visible, w);
    }

    bool set_eol_mode(QsciScintilla::EolMode m, QsciScintillaqq* w)
    {
        if ( !w ) return false;
        return apply_eol_mode(m, w);
    }

    void apply_settings(QsciScintillaqq * w )
    {
        if ( !w ) return;

        // APPLY WORD WRAP
        QsciScintilla::WrapMode m = static_cast<QsciScintilla::WrapMode>(
                    MainWindow::instance()->getSettings()->value(SETTING_WRAP_MODE).toInt() );

        bool show_all_chars = MainWindow::instance()->getSettings()->value(SETTING_SHOW_ALL_CHARS).toBool();

        apply_wrap_mode(m, w);
        apply_invisible_chars(show_all_chars, w);
    }

    //These are applied under special cases(ex. new documents and currently active document only) to avoid nerfing other documents accidentally
    void apply_single_document_settings(QsciScintillaqq * w )
    {
        if ( !w ) return;

        // APPLY WORD WRAP
        QsciScintilla::EolMode eol = static_cast<QsciScintilla::EolMode>(
                    MainWindow::instance()->getSettings()->value(SETTING_EOL_MODE).toInt() );
        apply_eol_mode(eol, w);
    }
}
