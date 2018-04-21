#include "include/EditorNS/customqwebview.h"
#include <QMimeData>
#include <QMenu>

namespace EditorNS
{

    CustomQWebView::CustomQWebView(QWidget *parent) :
        QWebView(parent)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &CustomQWebView::customContextMenuRequested,
                this, &CustomQWebView::onCustomContextMenuRequested);
    }

    void CustomQWebView::onCustomContextMenuRequested(const QPoint& pos)
    {
        QMenu menu;
        menu.addAction(page()->action(QWebPage::Cut));
        menu.addAction(page()->action(QWebPage::Copy));
        menu.addAction(page()->action(QWebPage::Paste));
        menu.addAction(page()->action(QWebPage::SelectAll));
        menu.exec(mapToGlobal(pos));
    }

    void CustomQWebView::wheelEvent(QWheelEvent *ev)
    {
        emit mouseWheel(ev);
        if (ev->modifiers() & Qt::ShiftModifier) {
            QWheelEvent hScroll (ev->pos(), ev->delta(), ev->buttons(), ev->modifiers(), Qt::Horizontal);
	    QWebView::wheelEvent(&hScroll);
        } else {
            QWebView::wheelEvent(ev);
        }
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

    void CustomQWebView::focusInEvent(QFocusEvent* event)
    {
        QWebView::focusInEvent(event);
        emit gotFocus();
    }
}
