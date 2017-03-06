#include "include/EditorNS/customqwebview.h"
#include <QEventLoop>
#include <QBuffer>
#include <QMimeData>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <QWebChannel>
#include <QUuid>

namespace EditorNS
{

    CustomQWebView::CustomQWebView(QWidget *parent) :
        QWebEngineView(parent)
    {
        QWebChannel *channel = new QWebChannel(page());
        page()->setWebChannel(channel);
    }

    void CustomQWebView::wheelEvent(QWheelEvent *ev)
    {
        emit mouseWheel(ev);
        QWebEngineView::wheelEvent(ev);
    }

    void CustomQWebView::keyPressEvent(QKeyEvent *ev)
    {
        switch (ev->key()) {
        case Qt::Key_Insert:
            ev->ignore();
            break;
        default:
            QWebEngineView::keyPressEvent(ev);
        }
    }

    void CustomQWebView::dropEvent(QDropEvent *ev)
    {
        if (ev->mimeData()->hasUrls()) {
            ev->ignore();
            emit urlsDropped(ev->mimeData()->urls());
        } else {
            QWebEngineView::dropEvent(ev);
        }
    }

    QVariant CustomQWebView::evaluateJavaScript(const QString &expr)
    {
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
        page()->webChannel()->registerObject(name, obj);
    }
}
