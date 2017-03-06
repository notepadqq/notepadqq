#include "include/EditorNS/jsproxy.h"

namespace EditorNS {

QVariant JsToCppProxy::getMsgData()
{
    return m_msgData;
}

QVariant JsToCppProxy::getResult()
{
    return m_result;
}

QVariant JsToCppProxy::getCursor()
{
    return m_cursor;
}

void JsToCppProxy::setResult(QVariant data)
{
    m_result = data;
    emit replyReady();
}

void JsToCppProxy::setCursor(QVariant cursorPos)
{
    m_cursor = cursorPos;
    emit cursorActivity();
}

void JsToCppProxy::receiveMessage(QString msg, QVariant data)
{
    //TODO: Remove this signal and slot and rely on a more central approach
    emit messageReceived(msg, data);
}

}


