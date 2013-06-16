#ifndef APPWIDESETTINGS_H
#define APPWIDESETTINGS_H

#include "qsciscintillaqq.h"
class QSettings;

namespace widesettings {

    extern const char * SETTING_WRAP_MODE;
    extern const char * SETTING_SHOW_ALL_CHARS;
    extern const char * SETTING_EOL_MODE;
    extern const char * SETTING_ZOOM_LEVEL;

    bool apply_wrap_mode (QsciScintilla::WrapMode m, QsciScintillaqq * w);
    bool apply_invisible_chars(bool v, QsciScintillaqq* w);
    bool apply_eol_mode(QsciScintilla::EolMode m, QsciScintillaqq * w);
    bool apply_zoom_level(double m, QsciScintillaqq* w);

    bool set_eol_mode(QsciScintilla::EolMode m, QsciScintillaqq * w);
    bool apply_monospace_font(QString family, int size, QsciScintillaqq* w);
    bool toggle_word_wrap(QsciScintillaqq * w);
    bool toggle_invisible_chars(QsciScintillaqq * w);    

    void apply_settings(QsciScintillaqq * w);
    void apply_single_document_settings(QsciScintillaqq * w );
}

#endif // APPWIDESETTINGS_H
