#ifndef BANNERBASICMESSAGE_H
#define BANNERBASICMESSAGE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace EditorNS
{

    class BannerBasicMessage : public QWidget
    {
        Q_OBJECT
    public:
        explicit BannerBasicMessage(QWidget *parent = 0);

        void setMessage(QString text);
        QPushButton *addButton(QString text);
    signals:
        void bannerRemoved();

    public slots:

    protected:
        void paintEvent(QPaintEvent *ev);
    private:
        QHBoxLayout *m_layout;
        QLabel *m_message;

    };

}
#endif // BANNERBASICMESSAGE_H
