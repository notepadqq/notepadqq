#include "include/Extensions/Stubs/stub.h"
namespace Extensions {
    namespace Stubs {

        Stub::Stub(QWeakPointer<QObject> object) : QObject(0)
        {
            m_weakPointer = object;
            m_weak = true;
        }

        Stub::Stub(QSharedPointer<QObject> object) : QObject(0)
        {
            m_sharedPointer = object;
            m_weak = false;
        }

        Stub::~Stub()
        {

        }

        QWeakPointer<QObject> Stub::object()
        {
            if (m_weak)
                return m_weakPointer;
            else
                return m_sharedPointer.toWeakRef();
        }

        QSharedPointer<QObject> Stub::objectSharedPtr()
        {
            return m_sharedPointer;
        }

        bool Stub::isWeak()
        {
            return m_weak;
        }

    }
}
