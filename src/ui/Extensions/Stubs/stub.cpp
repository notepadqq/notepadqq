#include "include/Extensions/Stubs/stub.h"

namespace Extensions {
    namespace Stubs {

        Stub::Stub(RuntimeSupport *rts) :
            QObject(0),
            m_rts(rts),
            m_pointerType(PointerType::DETACHED)
        {

        }

        Stub::Stub(const QWeakPointer<QObject> &object, RuntimeSupport *rts) :
            QObject(0),
            m_rts(rts),
            m_pointerType(PointerType::WEAK_POINTER),
            m_weakPointer(object)
        {

        }

        Stub::Stub(const QSharedPointer<QObject> &object, RuntimeSupport *rts) :
            QObject(0),
            m_rts(rts),
            m_pointerType(PointerType::SHARED_POINTER),
            m_sharedPointer(object)
        {

        }

        Stub::Stub(QObject *object, RuntimeSupport *rts) :
            QObject(0),
            m_rts(rts),
            m_pointerType(PointerType::UNMANAGED_POINTER),
            m_unmanagedPointer(object)
        {
            connect(object, &QObject::destroyed, this, [&] {
                m_unmanagedPointer = nullptr;
            });
        }

        Stub::~Stub()
        {

        }

        QWeakPointer<QObject> Stub::objectWeakPtr()
        {
            if (m_pointerType == PointerType::WEAK_POINTER)
                return m_weakPointer;
            else if (m_pointerType == PointerType::SHARED_POINTER)
                return m_sharedPointer.toWeakRef();
            else
                return QWeakPointer<QObject>();
        }

        QSharedPointer<QObject> Stub::objectSharedPtr()
        {
            return m_sharedPointer;
        }

        Stub::PointerType Stub::pointerType()
        {
            return m_pointerType;
        }

        QObject *Stub::objectUnmanagedPtr()
        {
            return m_unmanagedPointer;
        }

        bool Stub::invoke(const QString &method, Stub::StubReturnValue &ret, const QJsonArray &args)
        {
            auto fun = m_methods.value(method);
            if (fun != nullptr) {
                ret = fun(args);
                return true;
            }
            return false;
        }

        bool Stub::registerMethod(const QString &methodName, std::function<Stub::StubReturnValue (const QJsonArray &)> method)
        {
            m_methods.insert(methodName, method);
            return true;
        }

        RuntimeSupport *Stub::runtimeSupport()
        {
            return m_rts;
        }

        QString Stub::convertToString(const QJsonValue &value)
        {
            if (value.isString()) {
                return value.toString();
            } else if (value.isDouble()) {
                return QString::number(value.toDouble());
            } else {
                return QString();
            }
        }

        bool Stub::operator==(const Stub &other) const
        {
            return m_pointerType == other.m_pointerType
                    && m_sharedPointer == other.m_sharedPointer
                    && m_weakPointer == other.m_weakPointer
                    && m_unmanagedPointer == other.m_unmanagedPointer
                    && m_rts == other.m_rts
                    && stubName_() == other.stubName_();
        }

        bool Stub::operator!=(const Stub &other) const
        {
            return !(*this == other);
        }

    }
}
