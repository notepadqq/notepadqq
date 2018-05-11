#include "include/EditorNS/customqwebview.h"
#include <QMimeData>
#include <QMenu>

namespace EditorNS
{

    CustomQWebView::CustomQWebView(QWidget *parent) :
        QWebEngineView(parent)
    {
    }

    void CustomQWebView::wheelEvent(QWheelEvent *ev)
    {
        emit mouseWheel(ev);

        if (ev->modifiers() & Qt::ShiftModifier) {
            QWheelEvent hScroll (ev->pos(), ev->delta(), ev->buttons(), ev->modifiers(), Qt::Horizontal);
            QWebEngineView::wheelEvent(&hScroll);
        } else {
            QWebEngineView::wheelEvent(ev);
        }
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

    void CustomQWebView::focusInEvent(QFocusEvent* event)
    {
        QWebEngineView::focusInEvent(event);
        emit gotFocus();
    }

    void CustomQWebView::contextMenuEvent(QContextMenuEvent* event)
    {
        QMenu *menu = new QMenu(this);

        menu->insertAction(nullptr, page()->action(QWebEnginePage::Cut));
        menu->insertAction(nullptr, page()->action(QWebEnginePage::Copy));
        menu->insertAction(nullptr, page()->action(QWebEnginePage::Paste));
        menu->insertAction(nullptr, page()->action(QWebEnginePage::SelectAll));

        menu->popup(event->globalPos());
    }
}
