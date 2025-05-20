#include "include/EditorNS/customqwebview.h"

#include <QMenu>
#include <QMimeData>
#include <QCoreApplication>

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
        // Criando evento de rolagem horizontal corretamente no Qt 5.15.13
            QWheelEvent hScroll(
                ev->position(),                   // Posição local (QPointF)
                ev->globalPosition(),             // Posição global (QPointF)
                ev->pixelDelta(),                 // Delta em pixels (QPoint)
                ev->angleDelta(),                 // Delta em ângulos (QPoint)
                ev->buttons(),                    // Botões do mouse pressionados
                ev->modifiers(),                  // Modificadores do teclado
                ev->phase(),                      // Fase do evento de rolagem
                ev->source()                      // Fonte do evento de mouse
          );

        // Propaga o evento corretamente para o Qt
            QCoreApplication::sendEvent(this, &hScroll);
        } else {
            QWebEngineView::wheelEvent(ev);
        }
    }


    /*
    void CustomQWebView::wheelEvent(QWheelEvent *ev)
    {
        emit mouseWheel(ev);

        if (ev->modifiers() & Qt::ShiftModifier) {
            QWheelEvent hScroll (ev->position(), ev->angleDelta(), ev->buttons(), ev->modifiers(), Qt::Horizontal);
            QWebEngineView::wheelEvent(&hScroll);
        } else {
            QWebEngineView::wheelEvent(ev);
        }
    }*/

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

    bool EditorNS::CustomQWebView::eventFilter(QObject* obj, QEvent* ev)
    {
        if (obj != childObj)
            return QWebEngineView::eventFilter(obj, ev);

        switch (ev->type()) {
        case QEvent::FocusIn:
            focusInEvent(static_cast<QFocusEvent*>(ev));
            break;
        case QEvent::KeyPress:
            keyPressEvent(static_cast<QKeyEvent*>(ev));
            break;
        default:
            break;
        }

        return QWebEngineView::eventFilter(obj, ev);
    }

    bool EditorNS::CustomQWebView::event(QEvent* evt)
    {
        if (evt->type() == QEvent::ChildPolished) {
            QChildEvent* child_ev = static_cast<QChildEvent*>(evt);
            childObj = child_ev->child();

            if (childObj) {
                childObj->installEventFilter(this);
            }
        }

        return QWebEngineView::event(evt);
    }
}
