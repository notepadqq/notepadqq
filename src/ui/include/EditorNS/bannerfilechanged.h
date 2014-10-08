#ifndef BANNERFILECHANGED_H
#define BANNERFILECHANGED_H

#include "include/EditorNS/bannerbasicmessage.h"

namespace EditorNS
{

    class BannerFileChanged : public BannerBasicMessage
    {
        Q_OBJECT
    public:
        explicit BannerFileChanged(QWidget *parent = 0);

    signals:
        void reload();
        void ignore();

    public slots:

    };

}

#endif // BANNERFILECHANGED_H
