#include "include/EditorNS/bannerfileremoved.h"
#include <QLabel>
#include <QPushButton>
#include <QStyleOption>
#include <QPainter>

namespace EditorNS
{

    BannerFileRemoved::BannerFileRemoved(QWidget *parent) :
        BannerBasicMessage(parent)
    {
        setMessage(tr("This file has been deleted from the file system."));

        QPushButton *btnReload = addButton(tr("Save"));
        connect(btnReload, &QPushButton::clicked, this, &BannerFileRemoved::save);

        QPushButton *btnIgnore = addButton(tr("Ignore"));
        connect(btnIgnore, &QPushButton::clicked, this, &BannerFileRemoved::ignore);
    }

}
