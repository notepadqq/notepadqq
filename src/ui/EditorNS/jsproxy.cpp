#include "include/EditorNS/jsproxy.h"
#include <QDebug>

namespace EditorNS {

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

QVariant JsToCppProxy::getScrollPosition()
{
    return m_scrollPosition;
}

QVariant JsToCppProxy::getLanguage()
{
    return m_language;
}

bool JsToCppProxy::getClean()
{
    return m_clean;
}

QVariant JsToCppProxy::getDetectedIndent()
{
    return m_detectedIndent;
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

void JsToCppProxy::setScrollPosition(QVariant scrollPosition)
{
    m_scrollPosition = scrollPosition;
}

void JsToCppProxy::setLanguage(QVariant language)
{
    m_language = language;
}

void JsToCppProxy::setClean(bool state)
{
    m_clean = state;
}

void JsToCppProxy::setDetectedIndent(QVariant detectedIndent)
{
    m_detectedIndent = detectedIndent;
}

void JsToCppProxy::sendEditorEvent(QString msg, QVariant data)
{
    emit editorEvent(msg, data);
}

}


