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
        explicit CustomQWebView(QWidget *parent = nullptr);

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

        /*
         * QWebEngineView eats various types of events. Since we still need them
         * we'll have to install a custom event filter on the WebView's child delegate
         * QOpenGLWidget.
         */
        bool event(QEvent* evt) override;
        bool eventFilter(QObject *obj, QEvent *ev) override;

    private:
        QObject *childObj = nullptr;
    };

}

#endif // CUSTOMQWEBVIEW_H
