#include "include/EditorNS/bannerbasicmessage.h"
#include <QStyleOption>
#include <QPainter>

namespace EditorNS
{

    BannerBasicMessage::BannerBasicMessage(QWidget *parent) :
        QWidget(parent)
    {
        setContentsMargins(0, 0, 0, 0);

        m_topWidget = new QWidget(this);
        QVBoxLayout *topLayout = new QVBoxLayout(this);
        topLayout->setContentsMargins(0, 0, 0, 0);
        topLayout->addWidget(m_topWidget);

        m_layout = new QHBoxLayout(m_topWidget);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setMargin(12);

        m_topWidget->setObjectName("BannerBasicMessage_base");
        setImportance(Importance::Warning);

        m_message = new QLabel(this);

        m_message->setStyleSheet(".QLabel { margin-left: 5px; margin-right: 10px; color: white } ");
        m_message->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        m_layout->addWidget(m_message);

        m_layout->addStretch(1);

        m_topWidget->setLayout(m_layout);
        setLayout(topLayout);
    }

    void BannerBasicMessage::setMessage(QString text)
    {
        m_message->setText(text);
    }

    void BannerBasicMessage::setImportance(Importance importance)
    {
        if (importance == Importance::Warning) {
            m_topWidget->setStyleSheet("#BannerBasicMessage_base {"
                                     "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0 stop:0 rgba(227, 115, 0, 255), stop:1 rgba(245, 144, 39, 255));"
                                     "}");
        } else if (importance == Importance::Question) {
            m_topWidget->setStyleSheet("#BannerBasicMessage_base {"
                                     "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0 stop:0 rgba(58, 146, 222, 255), stop:1 rgba(112, 189, 255, 255));"
                                     "}");
        }
    }

    QPushButton * BannerBasicMessage::addButton(QString text)
    {
        QPushButton *button = new QPushButton(this);
        button->setText(text);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_layout->insertWidget(m_layout->count() - 1, button);

        button->setStyleSheet("QPushButton { border: 1px solid white; background: transparent; padding: 5px; color: white }"
                              "QPushButton:hover { background: rgba(0, 0, 0, 60); }"
                              "QPushButton:pressed { background: rgba(0, 0, 0, 100); }"
                              "QPushButton:focus { outline: none; text-decoration: underline; }"
                              "QPushButton:focus:pressed { outline: none; text-decoration: none; }");

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
