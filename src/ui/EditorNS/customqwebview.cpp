#include "include/EditorNS/customqwebview.h"
#include <QMimeData>
#include <QWebChannel>

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

    void CustomQWebView::connectJavaScriptObject(QString name, QObject *obj)
    {
        page()->webChannel()->registerObject(name, obj);
    }
}
