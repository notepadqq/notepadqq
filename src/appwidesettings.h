#ifndef APPWIDESETTINGS_H
#define APPWIDESETTINGS_H

#include "qsciscintillaqq.h"
class QSettings;

namespace widesettings {

    extern const char * SETTING_WRAP_MODE;

    bool apply_wrap_mode (QsciScintilla::WrapMode m, QsciScintillaqq * w);
    bool toggle_word_wrap(QsciScintillaqq * w);

    void apply_settings(QsciScintillaqq * w);
}

#endif // APPWIDESETTINGS_H
