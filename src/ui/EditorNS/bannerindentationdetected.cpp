#include "include/EditorNS/bannerindentationdetected.h"
#include <QLabel>
#include <QPushButton>
#include <QStyleOption>
#include <QPainter>

namespace EditorNS
{

    BannerIndentationDetected::BannerIndentationDetected(bool mode, Editor::IndentationMode detected, Editor::IndentationMode current, QWidget *parent) :
        BannerBasicMessage(parent)
    {
        setImportance(BannerIndentationDetected::Importance::Question);

        QString message;
        QPushButton *tmp;
        if (mode == false) {
            message = "This file is indented with %1, but your current settings specify to use %2.";
            if (current.useTabs) {
                message = message.arg(tr("spaces")).arg(tr("tabs"));

                tmp = addButton(tr("Use spaces"));
                connect(tmp, &QPushButton::clicked, this, &BannerIndentationDetected::useDocumentSettings);
            } else {
                message = message.arg(tr("tabs")).arg(tr("spaces"));

                tmp = addButton(tr("Use tabs"));
                connect(tmp, &QPushButton::clicked, this, &BannerIndentationDetected::useDocumentSettings);
            }
        } else {
            message = "This file is indented with %1 spaces, but your current settings specify to use %2 spaces.";
            message = message.arg(detected.size).arg(current.size);

            tmp = addButton(tr("Use %1 spaces").arg(detected.size));
            connect(tmp, &QPushButton::clicked, this, &BannerIndentationDetected::useDocumentSettings);
        }

        tmp = addButton(tr("Ignore"));
        connect(tmp, &QPushButton::clicked, this, &BannerIndentationDetected::useApplicationSettings);

        setMessage(message);
    }

}
