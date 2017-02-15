#include "include/EditorNS/customqwebview.h"
#include <QEventLoop>
#include <QBuffer>
#include <QMimeData>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#ifdef USE_QTWEBENGINE
#include <QWebChannel>
#include <QUuid>
#else
#include <QWebFrame>
#endif

namespace EditorNS
{

    CustomQWebView::CustomQWebView(QWidget *parent) :
        WEBVIEWNAME(parent)
    {
#ifdef USE_QTWEBENGINE
        QWebChannel *channel = new QWebChannel(page());
        page()->setWebChannel(channel);
#endif
    }

    void CustomQWebView::wheelEvent(QWheelEvent *ev)
    {
        emit mouseWheel(ev);
        WEBVIEWNAME::wheelEvent(ev);
    }

    void CustomQWebView::keyPressEvent(QKeyEvent *ev)
    {
        switch (ev->key()) {
        case Qt::Key_Insert:
            ev->ignore();
            break;
        default:
            WEBVIEWNAME::keyPressEvent(ev);
        }
    }

    void CustomQWebView::dropEvent(QDropEvent *ev)
    {
        if (ev->mimeData()->hasUrls()) {
            ev->ignore();
            emit urlsDropped(ev->mimeData()->urls());
        } else {
            WEBVIEWNAME::dropEvent(ev);
        }
    }

    QVariant CustomQWebView::evaluateJavaScript(const QString &expr)
    {
#ifdef USE_QTWEBENGINE
        QUuid currId = QUuid::createUuid();
        QEventLoop loop;
        connect(this, &CustomQWebView::JavascriptEvaluated, &loop, [currId, &loop](QUuid requestId) {
            if (requestId == currId) {
                loop.quit();
            }
        });

        QVariant result;
        page()->runJavaScript(expr, [&, currId](const QVariant &_result) {
            result = _result;
            emit JavascriptEvaluated(currId);
        });

        loop.exec();

        return result;
#else
        return page()->mainFrame()->evaluateJavaScript(expr);
#endif
    }

    QString CustomQWebView::jsStringEscape(QString str) const {
        return str.replace("\\", "\\\\")
            .replace("'", "\\'")
            .replace("\"", "\\\"")
            .replace("\n", "\\n")
            .replace("\r", "\\r")
            .replace("\t", "\\t")
            .replace("\b", "\\b");
    }

    void CustomQWebView::connectJavaScriptObject(QString name, QObject *obj)
    {
#ifdef USE_QTWEBENGINE
        page()->webChannel()->registerObject(name, obj);
#else
        page()->mainFrame()->addToJavaScriptWindowObject(name, obj);
#endif
    }
}
