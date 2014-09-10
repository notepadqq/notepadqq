#include "include/EditorNS/bannerfilechanged.h"
#include <QLabel>
#include <QPushButton>
#include <QStyleOption>
#include <QPainter>

namespace EditorNS
{

    BannerFileChanged::BannerFileChanged(QWidget *parent) :
        BannerBasicMessage(parent)
    {
        setMessage(tr("This file has been changed outside of Notepadqq."));

        QPushButton *btnReload = addButton(tr("Reload"));
        connect(btnReload, &QPushButton::clicked, this, &BannerFileChanged::reload);

        QPushButton *btnIgnore = addButton(tr("Ignore"));
        connect(btnIgnore, &QPushButton::clicked, this, &BannerFileChanged::ignore);
    }

}
