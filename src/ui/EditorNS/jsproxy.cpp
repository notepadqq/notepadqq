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

void JsToCppProxy::setResult(QVariant data)
{
    m_result = data;
    emit replyReady();
}

void JsToCppProxy::setCursor(QVariant cursorData)
{
    m_cursor = cursorData.value<CursorPosition>();
}

void JsToCppProxy::receiveMessage(QString msg, QVariant data)
{
    //TODO: Remove this signal and slot and rely on a more central approach
    emit messageReceived(msg, data);
}

}


