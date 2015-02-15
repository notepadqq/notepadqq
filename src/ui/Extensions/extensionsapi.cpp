#include "include/Extensions/extensionsapi.h"
#include <QMutex>

ExtensionsApi* ExtensionsApi::m_instance;

ExtensionsApi::ExtensionsApi(QObject *parent) : QObject(parent)
{

}

ExtensionsApi *ExtensionsApi::instance()
{
    static QMutex mutex;
    if (!m_instance)
    {
        mutex.lock();

        if (!m_instance)
            m_instance = new ExtensionsApi();

        mutex.unlock();
    }

    return m_instance;
}
