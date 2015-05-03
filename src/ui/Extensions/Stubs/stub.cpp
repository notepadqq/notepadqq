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

        bool Stub::isAlive()
        {
            if (m_pointerType == PointerType::DETACHED) {
                return true;
            } else {
                return m_unmanagedPointer != nullptr || !m_weakPointer.isNull() || !m_sharedPointer.isNull();
            }
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
            if (m_pointerType == PointerType::WEAK_POINTER)
                return m_weakPointer.data();
            else if (m_pointerType == PointerType::SHARED_POINTER)
                return m_sharedPointer.data();
            else if (m_pointerType == PointerType::UNMANAGED_POINTER)
                return m_unmanagedPointer;
            else
                return nullptr;
        }

        bool Stub::invoke(const QString &method, Stub::StubReturnValue &ret, const QJsonArray &args)
        {
            auto fun = m_methods.value(method);
            if (fun == nullptr)
            {
                // No explicit stub method found: try to invoke it on the real object
                if (m_pointerType == PointerType::DETACHED) {
                    return false;
                } else {

                    // FIXME See this: https://gist.github.com/andref/2838534

                    if (args.count() > 10) {
                        return false;
                    }

                    bool ok = true;
                    QGenericArgument arg0 = jsonValueToArgument(args, 0, ok);
                    QGenericArgument arg1 = jsonValueToArgument(args, 1, ok);
                    QGenericArgument arg2 = jsonValueToArgument(args, 2, ok);
                    QGenericArgument arg3 = jsonValueToArgument(args, 3, ok);
                    QGenericArgument arg4 = jsonValueToArgument(args, 4, ok);
                    QGenericArgument arg5 = jsonValueToArgument(args, 5, ok);
                    QGenericArgument arg6 = jsonValueToArgument(args, 6, ok);
                    QGenericArgument arg7 = jsonValueToArgument(args, 7, ok);
                    QGenericArgument arg8 = jsonValueToArgument(args, 8, ok);
                    QGenericArgument arg9 = jsonValueToArgument(args, 9, ok);

                    if (!ok) {
                        return false;
                    }

                    QJsonValue retval;
                    bool invoked = QMetaObject::invokeMethod(
                                objectUnmanagedPtr(),
                                method.toStdString().c_str(),
                                Qt::DirectConnection,
                                Q_RETURN_ARG(QJsonValue, retval),
                                arg0, arg1, arg2, arg3, arg4,
                                arg5, arg6, arg7, arg8, arg9);

                    if (invoked) {
                        ret.error = ErrorCode::NONE;
                        ret.result = retval;
                    } else {
                        return false;
                    }
                }

            } else {
                // Explicit stub method found: call it
                ret = fun(args);
                return true;
            }
            return false;
        }

        QGenericArgument Stub::jsonValueToArgument(const QJsonArray &args, int i, bool &ok)
        {
            if (i >= args.size()) {
                // It's ok! We need an empty QGenericArgument
                return QGenericArgument();
            }

            if (args.at(i).isBool()) {
                return Q_ARG(bool, args.at(i).toBool());
            } else if (args.at(i).isDouble()) {
                return Q_ARG(bool, args.at(i).toDouble());
            } else if (args.at(i).isNull()) {
                return Q_ARG(void *, nullptr);
            } else if (args.at(i).isString()) {
                return Q_ARG(QString, args.at(i).toString());
            } else {
                ok = false;
                return QGenericArgument();
            }
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
