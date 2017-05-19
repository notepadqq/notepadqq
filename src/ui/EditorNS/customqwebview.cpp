#include "include/EditorNS/customqwebview.h"
#include <QMimeData>

namespace EditorNS
{

    CustomQWebView::CustomQWebView(QWidget *parent) :
        QWebView(parent)
    {
        setFocusPolicy(Qt::StrongFocus); // Mouse wheel won't grant focus
    }

    void CustomQWebView::wheelEvent(QWheelEvent *ev)
    {
        emit mouseWheel(ev);
        QWebView::wheelEvent(ev);
    }

    void CustomQWebView::keyPressEvent(QKeyEvent *ev)
    {
        switch (ev->key()) {
        case Qt::Key_Insert:
            ev->ignore();
            break;
        default:
            QWebView::keyPressEvent(ev);
        }
    }

    void CustomQWebView::dropEvent(QDropEvent *ev)
    {
        if (ev->mimeData()->hasUrls()) {
            ev->ignore();
            emit urlsDropped(ev->mimeData()->urls());
        } else {
            QWebView::dropEvent(ev);
        }
    }

    void CustomQWebView::focusInEvent(QFocusEvent* evt)
    {
        QWebView::focusInEvent(evt);
        emit gotFocus();
    }

}
