#ifndef BANNERBASICMESSAGE_H
#define BANNERBASICMESSAGE_H

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

namespace EditorNS
{

    class BannerBasicMessage : public QWidget
    {
        Q_OBJECT
    public:
        explicit BannerBasicMessage(QWidget *parent = nullptr);

        enum class Importance {
            Warning,
            Question
        };

        void setMessage(QString text);
        QPushButton *addButton(QString text);
        void setImportance(Importance importance);

    signals:
        void bannerRemoved();

    public slots:

    protected:
        void paintEvent(QPaintEvent *ev);
    private:
        QHBoxLayout *m_layout;
        QLabel *m_message;
        QWidget *m_topWidget;
    };

}
#endif // BANNERBASICMESSAGE_H
