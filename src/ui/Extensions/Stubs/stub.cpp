#include "include/Extensions/Stubs/stub.h"

namespace Extensions {
    namespace Stubs {

        Stub::Stub(RuntimeSupport *rts) :
            QObject(0),
            m_rts(rts),
            m_stubType(StubType::DETACHED)
        {

        }

        Stub::Stub(QWeakPointer<QObject> object, RuntimeSupport *rts) :
            QObject(0),
            m_rts(rts),
            m_stubType(StubType::WEAK_POINTER),
            m_weakPointer(object)
        {

        }

        Stub::Stub(QSharedPointer<QObject> object, RuntimeSupport *rts) :
            QObject(0),
            m_rts(rts),
            m_stubType(StubType::SHARED_POINTER),
            m_sharedPointer(object)
        {

        }

        Stub::Stub(QObject *object, RuntimeSupport *rts) :
            QObject(0),
            m_rts(rts),
            m_stubType(StubType::UNMANAGED_POINTER),
            m_unmanagedPointer(object)
        {

        }

        Stub::~Stub()
        {

        }

        QWeakPointer<QObject> Stub::objectWeakPtr()
        {
            if (m_stubType == StubType::WEAK_POINTER)
                return m_weakPointer;
            else if (m_stubType == StubType::SHARED_POINTER)
                return m_sharedPointer.toWeakRef();
            else
                return QWeakPointer<QObject>();
        }

        QSharedPointer<QObject> Stub::objectSharedPtr()
        {
            return m_sharedPointer;
        }

        Stub::StubType Stub::stubType()
        {
            return m_stubType;
        }

        QObject *Stub::objectUnmanagedPtr()
        {
            return m_unmanagedPointer;
        }

        bool Stub::invoke(const QString &method, Stub::StubReturnValue &ret, const QJsonValue &args)
        {
            auto fun = m_methods.value(method);
            if (fun != nullptr) {
                ret = fun(args);
                return true;
            }
            return false;
        }

        bool Stub::registerMethod(const QString &methodName, std::function<Stub::StubReturnValue (const QJsonValue &)> method)
        {
            m_methods.insert(methodName, method);
            return true;
        }

        RuntimeSupport *Stub::runtimeSupport()
        {
            return m_rts;
        }

    }
}
