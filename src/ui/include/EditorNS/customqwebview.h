#ifndef CUSTOMQWEBVIEW_H
#define CUSTOMQWEBVIEW_H

#include <QWheelEvent>
#include <QWebEngineView>
#include <QQueue>
#include <QUuid>

namespace EditorNS
{

    class CustomQWebView : public QWebEngineView
    {
        Q_OBJECT
        QString jsStringEscape(QString str) const;
        QQueue<int> m_jsRequests;
    public:
        explicit CustomQWebView(QWidget *parent = 0);

        QVariant evaluateJavaScript(const QString &expr);
        void connectJavaScriptObject(QString name, QObject *obj);

    signals:
        void mouseWheel(QWheelEvent *ev);
        void JavascriptEvaluated(QUuid requestId);
        void urlsDropped(QList<QUrl> urls);

    public slots:

    protected:
        void wheelEvent(QWheelEvent *ev);
        void keyPressEvent(QKeyEvent *ev);
        void dropEvent(QDropEvent *ev);
    };

}

#endif // CUSTOMQWEBVIEW_H
