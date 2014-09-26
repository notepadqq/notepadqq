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
    public slots:

    protected:
        void wheelEvent(QWheelEvent *ev);
        void keyPressEvent(QKeyEvent *ev);
    };

}

#endif // CUSTOMQWEBVIEW_H
