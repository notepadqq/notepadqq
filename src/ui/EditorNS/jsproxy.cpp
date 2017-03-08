#include "include/EditorNS/jsproxy.h"
#include <QDebug>

namespace EditorNS {

bool JsToCppProxy::hasKey(const QString& key)
{
    if(!m_values.contains(key) || m_values[key].isNull()) {
        return false;
    }
    return true;
}

void JsToCppProxy::sendEditorEvent(QString msg, QVariant data)
{
    emit editorEvent(msg, data);
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


bool JsToCppProxy::getValue(const QString& key, int& r)
{
    if (!hasKey(key))
        return false;
    r = m_values.value(key).toInt();
    return true;
}

bool JsToCppProxy::getValue(const QString& key, bool& r)
{
    if (!hasKey(key))
        return false;
    r = m_values.value(key).toBool();
    return true;
}

bool JsToCppProxy::getValue(const QString& key, QPair<int, int>& r)
{
    if (!hasKey(key))
        return false;
    QVariantList data = m_values.value(key).toList();
    r = qMakePair(data[0].toInt(), data[1].toInt());
    return true;
}

bool JsToCppProxy::getValue(const QString& key, QString& r)
{
    if (!hasKey(key))
        return false;
    r = m_values.value(key).toString();
    return true;
}

bool JsToCppProxy::getValue(const QString& key, QStringList& r)
{
    if (!hasKey(key))
        return false;
    r = m_values.value(key).toStringList();
    return true;
}

QVariant JsToCppProxy::getRawValue(const QString& key)
{
    return m_values.value(key);
}

void JsToCppProxy::setValue(QString name, QVariant data)
{
    m_values[name] = data;
}


}

