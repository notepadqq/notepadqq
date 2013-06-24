#include "appwidesettings.h"
#include "mainwindow.h"
#include <QSettings>
#include <QDebug>

namespace widesettings {
    const char * SETTING_WRAP_MODE         = "wrap_mode";
    const char * SETTING_WRAP_SYMBOL       = "wrap_symbol";
    const char * SETTING_SHOW_ALL_CHARS    = "show_all_chars";
    const char * SETTING_SHOW_END_OF_LINE  = "show_eol";
    const char * SETTING_SHOW_WHITE_SPACE  = "show_white_space";
    const char * SETTING_SHOW_INDENT_GUIDE = "show_indent_guide";
    const char * SETTING_EOL_MODE          = "eol_mode";
    const char * SETTING_ZOOM_LEVEL        = "zoom_level";
    const char * SETTING_MONO_FONT_NAME    = "mono_font_name";
    const char * SETTING_MONO_FONT_SIZE    = "mono_font_size";

    const char * SETTING_TABBAR_HIDE       = "tabbar/hide";
    const char * SETTING_TABBAR_VERTICAL   = "tabbar/vertical";
    const char * SETTING_TABBAR_MOVABLE    = "tabbar/movable";
    const char * SETTING_TABBAR_REDUCE     = "tabbar/reduce";
    const char * SETTING_TABBAR_HIGHLIGHT  = "tabbar/highlight";

    bool apply_tabbar_highlight(bool on)
    {
        MainWindow* mwin = MainWindow::instance();
        mwin->getSettings()->setValue(SETTING_TABBAR_HIGHLIGHT,on);
        mwin->update_appwide_ui(SETTING_TABBAR_HIGHLIGHT);
        return true;
    }


    bool apply_tabbar_hide(bool on)
    {
        MainWindow* mwin = MainWindow::instance();
        mwin->getSettings()->setValue(SETTING_TABBAR_HIDE,on);
        mwin->update_appwide_ui(SETTING_TABBAR_HIDE);
        return true;
    }

    bool apply_tabbar_vertical(bool on)
    {
        MainWindow* mwin = MainWindow::instance();
        mwin->getSettings()->setValue(SETTING_TABBAR_VERTICAL,on);
        mwin->update_appwide_ui(SETTING_TABBAR_VERTICAL);
        return true;
    }

    bool apply_tabbar_movable(bool on)
    {
        MainWindow* mwin = MainWindow::instance();
        mwin->getSettings()->setValue(SETTING_TABBAR_MOVABLE,on);
        mwin->update_appwide_ui(SETTING_TABBAR_MOVABLE);
        return true;
    }

    bool apply_tabbar_reduce(bool on)
    {
        MainWindow* mwin = MainWindow::instance();
        mwin->getSettings()->setValue(SETTING_TABBAR_REDUCE,on);
        //This is actually updated in the same area when the stylesheet is set.
        mwin->update_appwide_ui(SETTING_TABBAR_VERTICAL);
        return true;
    }

    bool apply_wrap_mode(QsciScintilla::WrapMode m, QsciScintillaqq* w)
    {
        MainWindow::instance()->getSettings()->setValue(SETTING_WRAP_MODE, m);

        if ( !w )                 return false;
        if ( m == w->wrapMode() ) return false;
        w->setWrapMode(m);
        return true;
    }

    bool apply_invisible_chars(bool v, QsciScintillaqq* w)
    {
        MainWindow::instance()->getSettings()->setValue(SETTING_SHOW_ALL_CHARS, v);
        MainWindow::instance()->getSettings()->setValue(SETTING_SHOW_END_OF_LINE, v);
        MainWindow::instance()->getSettings()->setValue(SETTING_SHOW_WHITE_SPACE, v);

        if ( !w )                 return false;
        w->setEolVisibility(v);
        w->setWhitespaceVisibility( v ? QsciScintillaqq::WsVisible : QsciScintillaqq::WsInvisible );
        return true;
    }

    bool apply_end_of_line(bool v, QsciScintillaqq* w)
    {
        MainWindow::instance()->getSettings()->setValue(SETTING_SHOW_END_OF_LINE, v);

        if ( !w )                 return false;
        w->setEolVisibility(v);
        MainWindow::instance()->getSettings()->setValue(SETTING_SHOW_ALL_CHARS,
                                                       (w->eolVisibility() && w->whitespaceVisibility()) ? true : false);
        return true;
    }

    bool apply_white_space(bool v, QsciScintillaqq* w)
    {
        MainWindow::instance()->getSettings()->setValue(SETTING_SHOW_WHITE_SPACE, v);

        if ( !w )                 return false;
        w->setWhitespaceVisibility( v ? QsciScintillaqq::WsVisible : QsciScintillaqq::WsInvisible );
        MainWindow::instance()->getSettings()->setValue(SETTING_SHOW_ALL_CHARS,
                                                       (w->eolVisibility() && w->whitespaceVisibility()) ? true : false);
        return true;
    }

    bool apply_indent_guide(bool v, QsciScintillaqq* w)
    {
        MainWindow::instance()->getSettings()->setValue(SETTING_SHOW_INDENT_GUIDE, v);

        if ( !w )                 return false;
        w->setIndentationGuides(v);
        return true;
    }

    bool apply_wrap_symbol(bool v, QsciScintillaqq* w)
    {
        MainWindow::instance()->getSettings()->setValue(SETTING_WRAP_SYMBOL, v);

        if ( !w )                 return false;
        w->setWrapVisualFlags(v ? QsciScintilla::WrapFlagByBorder: QsciScintilla::WrapFlagNone);
        return true;
    }

    bool apply_monospace_font(QString family, int size, QsciScintillaqq* w)
    {
        MainWindow::instance()->getSettings()->setValue(SETTING_MONO_FONT_NAME, family);
        MainWindow::instance()->getSettings()->setValue(SETTING_MONO_FONT_SIZE, size);

        if ( !w )                 return false;
        QFont f(family, size);
        qDebug() << family << " size: " << size;
        w->setFont(f);
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

    bool apply_zoom_level(double m, QsciScintillaqq* w)
    {
        if( !w )               return false;
        w->zoomTo(m);
        w->updateLineMargin();
        MainWindow::instance()->getSettings()->setValue(SETTING_ZOOM_LEVEL, m);
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
        bool visible = (w->eolVisibility() && w->whitespaceVisibility());
        return apply_invisible_chars(!visible, w);
    }

    bool toggle_end_of_line(QsciScintillaqq* w)
    {
        if ( !w ) return false;
        bool visible = w->eolVisibility();
        return apply_end_of_line(!visible, w);
    }

    bool toggle_white_space(QsciScintillaqq* w)
    {
        if( !w ) return false;
        bool visible = (w->whitespaceVisibility() == QsciScintilla::WsVisible) ? true : false;
        return apply_white_space(!visible, w);
    }

    bool toggle_indent_guide(QsciScintillaqq* w)
    {
        if ( !w ) return false;
        bool visible = w->indentationGuides();
        return apply_indent_guide(!visible, w);
    }

    bool toggle_wrap_symbol(QsciScintillaqq* w)
    {
        if ( !w ) return false;
        bool visible = MainWindow::instance()->getSettings()->value(SETTING_WRAP_SYMBOL,false).toBool();
        return apply_wrap_symbol(!visible, w);
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

        bool show_all_chars             = MainWindow::instance()->getSettings()->value(SETTING_SHOW_ALL_CHARS).toBool();
        bool show_end_of_line           = MainWindow::instance()->getSettings()->value(SETTING_SHOW_END_OF_LINE).toBool();
        bool show_white_space           = MainWindow::instance()->getSettings()->value(SETTING_SHOW_WHITE_SPACE).toBool();
        bool show_wrap_symbol           = MainWindow::instance()->getSettings()->value(SETTING_WRAP_SYMBOL).toBool();
        bool show_indent_guide          = MainWindow::instance()->getSettings()->value(SETTING_SHOW_INDENT_GUIDE).toBool();

        double zoom_level               = MainWindow::instance()->getSettings()->value(SETTING_ZOOM_LEVEL).toDouble();

        apply_wrap_mode(m, w);
        apply_invisible_chars(show_all_chars, w);
        apply_end_of_line(show_end_of_line, w);
        apply_white_space(show_white_space, w);
        apply_wrap_symbol(show_wrap_symbol, w);
        apply_indent_guide(show_indent_guide, w);
        apply_zoom_level(zoom_level, w);
    }

    //These are applied under special cases(ex. new documents and currently active document only) to avoid nerfing other documents accidentally
    void apply_single_document_settings(QsciScintillaqq * w )
    {
        if ( !w ) return;

        // APPLY EOL CONVERSION
        QsciScintilla::EolMode eol = static_cast<QsciScintilla::EolMode>(
                    MainWindow::instance()->getSettings()->value(SETTING_EOL_MODE).toInt() );
        apply_eol_mode(eol, w);

        QString mono_font_name = MainWindow::instance()->getSettings()->value(SETTING_MONO_FONT_NAME).toString();
        int     mono_font_size = MainWindow::instance()->getSettings()->value(SETTING_MONO_FONT_SIZE).toInt();
        apply_monospace_font(mono_font_name, mono_font_size, w);
    }
}
