#include "include/EditorNS/customqwebview.h"

namespace EditorNS
{

    CustomQWebView::CustomQWebView(QWidget *parent) :
        QWebView(parent)
    {
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

}
