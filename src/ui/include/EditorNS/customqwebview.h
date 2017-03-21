#ifndef CUSTOMQWEBVIEW_H
#define CUSTOMQWEBVIEW_H

#include <QWheelEvent>
#include <QWebEngineView>

namespace EditorNS
{

    class CustomQWebView : public QWebEngineView
    {
        Q_OBJECT
    public:
        explicit CustomQWebView(QWidget *parent = 0);
        void connectJavaScriptObject(QString name, QObject *obj);

    signals:
        void mouseWheel(QWheelEvent *ev);
        void urlsDropped(QList<QUrl> urls);

    public slots:

    protected:
        void wheelEvent(QWheelEvent *ev);
        void keyPressEvent(QKeyEvent *ev);
        void dropEvent(QDropEvent *ev);
    };

    class CustomQWebViewPage : public QWebEnginePage {
        Q_OBJECT;
    protected:
        void javaScriptConsoleMessage(
                QWebEnginePage::JavaScriptConsoleMessageLevel level, 
                const QString &message, 
                int lineNumber, 
                const QString &sourceID);
    };

}

#endif // CUSTOMQWEBVIEW_H
