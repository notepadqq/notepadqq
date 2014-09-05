#include "include/customqwebview.h"

CustomQWebView::CustomQWebView(QWidget *parent) :
    QWebView(parent)
{
}

void CustomQWebView::wheelEvent(QWheelEvent *ev)
{
    emit mouseWheel(ev);
    QWebView::wheelEvent(ev);
}
