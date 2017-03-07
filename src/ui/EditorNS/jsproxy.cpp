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

QVariant JsToCppProxy::getTextLength()
{
    return m_textLength;
}

QVariant JsToCppProxy::getLineCount()
{
    return m_lineCount;
}

QVariant JsToCppProxy::getSelections()
{
    return m_selections;
}

QVariant JsToCppProxy::getSelectionsText()
{
    return m_selectionsText;
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

void JsToCppProxy::setTextLength(QVariant textLength)
{
    m_textLength = textLength;
}

void JsToCppProxy::setLineCount(QVariant lineCount)
{
    m_lineCount = lineCount;
}

void JsToCppProxy::setSelections(QVariant selections)
{
    m_selections = selections;
}

void JsToCppProxy::setSelectionsText(QVariant selectionsText)
{
    m_selectionsText = selectionsText;
}

void JsToCppProxy::receiveMessage(QString msg, QVariant data)
{
    //TODO: Remove this signal and slot and rely on a more central approach
    emit messageReceived(msg, data);
}

}


