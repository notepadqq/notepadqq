#ifndef BANNERFILEREMOVED_H
#define BANNERFILEREMOVED_H

#include "include/EditorNS/bannerbasicmessage.h"

namespace EditorNS
{

    class BannerFileRemoved : public BannerBasicMessage
    {
        Q_OBJECT
    public:
        explicit BannerFileRemoved(QWidget *parent = nullptr);

    signals:
        void save();
        void ignore();

    public slots:

    };

}

#endif // BANNERFILEREMOVED_H
