#include "include/EditorNS/bannerfilechanged.h"

#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>

namespace EditorNS {

BannerFileChanged::BannerFileChanged(QWidget *parent) 
    : BannerBasicMessage(parent) 
{
    // Set the message in a single call to minimize UI updates
    setMessage(tr("This file has been changed outside of Notepadqq."));

    // Initialize buttons once and reuse their references.
    QPushButton *btnReload = addButton(tr("Reload"));
    QPushButton *btnIgnore = addButton(tr("Ignore"));

    // Connect signals to slots without redundancy or multiple steps
    connect(btnReload, &QPushButton::clicked, this, &BannerFileChanged::reload);
    connect(btnIgnore, &QPushButton::clicked, this, &BannerFileChanged::ignore);
}

} // namespace EditorNS
