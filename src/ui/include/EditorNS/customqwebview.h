#ifndef CUSTOMQWEBVIEW_H
#define CUSTOMQWEBVIEW_H

#include <QWebView>
#include <QWheelEvent>

namespace EditorNS
{

    class CustomQWebView : public QWebView
    {
        Q_OBJECT
    public:
        explicit CustomQWebView(QWidget *parent = 0);

    signals:
        void mouseWheel(QWheelEvent *ev);
        void urlsDropped(QList<QUrl> urls);
    public slots:

    protected:
        void wheelEvent(QWheelEvent *ev);
        void keyPressEvent(QKeyEvent *ev);
        void dropEvent(QDropEvent *ev);
    };

}

#endif // CUSTOMQWEBVIEW_H
