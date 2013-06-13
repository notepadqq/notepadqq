#include "appwidesettings.h"
#include "mainwindow.h"
#include <QSettings>

namespace widesettings {

    const char * SETTING_WRAP_MODE = "wrap_mode";

    bool apply_wrap_mode(QsciScintilla::WrapMode m, QsciScintillaqq* w)
    {
        if ( !w )                 return false;
        if ( m == w->wrapMode() ) return false;
        w->setWrapMode(m);
        MainWindow::instance()->getSettings()->setValue(SETTING_WRAP_MODE, m);
        return true;
    }

    bool toggle_word_wrap(QsciScintillaqq* w)
    {
        if ( !w ) return false;
        return apply_wrap_mode(
                    (w->wrapMode() == QsciScintilla::WrapNone ?
                     QsciScintilla::WrapWord : QsciScintilla::WrapNone), w );
    }

    void apply_settings(QsciScintillaqq * w )
    {
        if ( !w ) return;

        // APPLY WORD WRAP
        QsciScintilla::WrapMode m = static_cast<QsciScintilla::WrapMode>(
                    MainWindow::instance()->getSettings()->value(SETTING_WRAP_MODE).toInt() );
        apply_wrap_mode(m, w);
    }
}
