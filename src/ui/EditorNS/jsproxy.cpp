#include "include/EditorNS/jsproxy.h"
#include <QDebug>

namespace EditorNS {

QVariant JsToCppProxy::getResult()
{
    return m_result;
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

template<class T>
bool JsToCppProxy::getValue(T& local, const QString &dataName)
{
    // If for some reason the editor data isn't initialized, this gives us
    // a way to handle it cleanly
    if (!m_values.contains(dataName)) {
        return false;
    }
    // FIXME: Maybe use enums?
    if(dataName == "cursor") {
        QVariantList data = m_values.value("cursor").toList();
        local = qMakePair(data[0].toInt(), data[1].toInt());
    }

    return true;
}

bool JsToCppProxy::getValue(QPair<int, int>& local, const QString& dataName)
{
    return getValue<>(local, dataName);
}

void JsToCppProxy::setValue(QString name, QVariant data)
{
    m_values[name] = data;
}

void JsToCppProxy::sendEditorEvent(QString msg, QVariant data)
{
    emit editorEvent(msg, data);
}

}


