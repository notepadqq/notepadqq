#include "include/EditorNS/jsproxy.h"

namespace EditorNS {

JsProxy::JsProxy(QObject* parent) : QObject(parent)
{
    m_tempQueue = new QQueue<QPair<QString, QVariant>>();
}

void JsProxy::pushQueuedMessages()
{
    while(!m_tempQueue->isEmpty()) {
        QPair<QString, QVariant> msg = m_tempQueue->dequeue();
        emit sendMsgInternal(msg.first, msg.second);
    }
    delete m_tempQueue;
}

void JsProxy::sendMsg(const QString& msg, const QVariant& data)
{
    if(!m_ready) {
        m_tempQueue->enqueue(qMakePair(msg, data));
    }else {
        emit sendMsgInternal(msg, data);
    }
}

void JsProxy::sendEditorEvent(QString msg, QVariant data)
{
    if (msg == "J_EVT_READY" && m_ready == false) {
        m_ready = true;
        pushQueuedMessages();
    }
    emit editorEvent(msg, data);
}

QVariant JsProxy::getResult()
{
    return m_result;
}

void JsProxy::setResult(QVariant data)
{
    m_result = data;
    emit replyReady();
}

}
