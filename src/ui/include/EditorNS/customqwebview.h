#ifndef CUSTOMQWEBVIEW_H
#define CUSTOMQWEBVIEW_H

#include <QWebEngineView>
#include <QWheelEvent>

namespace EditorNS
{

    class CustomQWebView : public QWebEngineView
    {
        Q_OBJECT
    public:
        explicit CustomQWebView(QWidget *parent = 0);

    signals:
        void mouseWheel(QWheelEvent *ev);
        void urlsDropped(QList<QUrl> urls);
        void gotFocus();

    protected:
        void wheelEvent(QWheelEvent *ev) override;
        void keyPressEvent(QKeyEvent *ev) override;
        void dropEvent(QDropEvent *ev) override;
        void focusInEvent(QFocusEvent* event) override;
        void contextMenuEvent(QContextMenuEvent* ev) override;
    };

}

#endif // CUSTOMQWEBVIEW_H
