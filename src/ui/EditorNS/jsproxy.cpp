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

template<class T>
bool JsToCppProxy::getValue(T& local, const QString &dataName)
{
    // If for some reason the editor data isn't initialized, this gives us
    // a way to handle it cleanly.
    if (!m_values.contains(dataName) || m_values[dataName].isNull()) {
        qDebug() << "JsProxy Failure: \n" << "dataName: " << dataName
            << "\nNo value detected for the provided dataName, please report this at: \n" <<
            "https://github.com/notepadqq/notepadqq/issues";
        return false;
    }
    // FIXME: Maybe use enums?
    if (dataName == "cursor" || dataName == "detectedIndent" || dataName == "indentMode") {
        QVariantList data = m_values.value(dataName).toList();
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


