#include "include/EditorNS/bannerbasicmessage.h"
#include <QStyleOption>
#include <QPainter>

namespace EditorNS
{

    BannerBasicMessage::BannerBasicMessage(QWidget *parent) :
        QWidget(parent)
    {
        setContentsMargins(0, 0, 0, 0);

        QWidget *topWidget = new QWidget(this);
        QVBoxLayout *topLayout = new QVBoxLayout(this);
        topLayout->setContentsMargins(0, 0, 0, 0);
        topLayout->addWidget(topWidget);

        m_layout = new QHBoxLayout(topWidget);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setMargin(12);

        topWidget->setObjectName("BannerBasicMessage_base");
        topWidget->setStyleSheet("#BannerBasicMessage_base {"
                                 "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0 stop:0 rgba(241, 218, 54, 255), stop:1 rgba(249, 239, 166, 255));"
                                 "}");


        m_message = new QLabel(this);

        m_message->setStyleSheet(".QLabel { margin-left: 5px; margin-right: 10px; } ");
        m_message->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        m_layout->addWidget(m_message);

        m_layout->addStretch(1);

        topWidget->setLayout(m_layout);
        setLayout(topLayout);
    }

    void BannerBasicMessage::setMessage(QString text)
    {
        m_message->setText(text);
    }

    QPushButton * BannerBasicMessage::addButton(QString text)
    {
        QPushButton *button = new QPushButton(this);
        button->setText(text);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_layout->insertWidget(m_layout->count() - 1, button);
        return button;
    }

    void BannerBasicMessage::paintEvent(QPaintEvent * /*ev*/)
    {
        // Needed for setStyleSheet to work, see here:
        // http://stackoverflow.com/questions/18344135/why-do-stylesheets-not-work-when-subclassing-qwidget-and-using-q-object
        QStyleOption o;
        o.initFrom(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
    }

}
