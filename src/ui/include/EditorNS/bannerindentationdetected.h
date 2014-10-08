#ifndef BANNERINDENTATIONDETECTED_H
#define BANNERINDENTATIONDETECTED_H

#include "include/EditorNS/bannerbasicmessage.h"
#include "include/EditorNS/editor.h"

namespace EditorNS
{

    class BannerIndentationDetected : public BannerBasicMessage
    {
        Q_OBJECT
    public:
        BannerIndentationDetected(bool mode, Editor::IndentationMode detected, Editor::IndentationMode current, QWidget *parent = 0);

    signals:
        void useDocumentSettings();
        void useApplicationSettings();
    };

}

#endif // BANNERINDENTATIONDETECTED_H
