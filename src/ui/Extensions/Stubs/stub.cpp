#include "include/Extensions/Stubs/stub.h"

namespace Extensions {
    namespace Stubs {

        Stub::Stub() : QObject(0)
        {
            m_hasObject = false;
        }

        Stub::Stub(QWeakPointer<QObject> object) : QObject(0)
        {
            m_weakPointer = object;
            m_weak = true;
            m_hasObject = true;
        }

        Stub::Stub(QSharedPointer<QObject> object) : QObject(0)
        {
            m_sharedPointer = object;
            m_weak = false;
            m_hasObject = true;
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

        bool Stub::hasObject()
        {
            return m_hasObject;
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

    }
}
