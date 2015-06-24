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
            if (fun == nullptr) {
                // No explicit stub method found: try to invoke it on the real object
                return invokeOnRealObject(method, ret, args);
            } else {
                // Explicit stub method found: call it
                ret = fun(args);
                return true;
            }
        }

        bool Stub::invokeOnRealObject(const QString &method, Stub::StubReturnValue &ret, const QJsonArray &args)
        {
            if (m_pointerType == PointerType::DETACHED) {
                ret = Stub::StubReturnValue(ErrorCode::METHOD_NOT_FOUND,
                                            QString("Method not found for %1: %2").arg(stubName_(), method));
                return false;
            } else {

                ErrorCode err = ErrorCode::NONE;

                QObject *obj = objectUnmanagedPtr();

                // Find a method with the same name of the one we're looking for
                int methodCount = obj->metaObject()->methodCount();
                for (int i = 0; i < methodCount; i++) {
                    QMetaMethod metaMethod = obj->metaObject()->method(i);

                    if (metaMethod.access() != QMetaMethod::Public)
                        continue;

                    if (metaMethod.name() != method)
                        continue;

                    QVariant retval = genericCall(obj,
                                      metaMethod,
                                      args.toVariantList(),
                                      err);

                    if (err == ErrorCode::NONE) {
                        // Ok!!
                        ret = Stub::StubReturnValue(QJsonValue::fromVariant(retval));
                        return true;
                    } else {
                        // Keep searching: maybe it's another overload
                        continue;
                    }
                }

                ret = Stub::StubReturnValue(ErrorCode::METHOD_NOT_FOUND,
                                            QString("Method not found for %1: %2 with %3 args")
                                            .arg(stubName_(), method).arg(args.count()));
                return false;
            }
        }

        // https://gist.github.com/andref/2838534
        QVariant Stub::genericCall(QObject* object, QMetaMethod metaMethod, QVariantList args, ErrorCode &error)
        {
            // Convert the arguments

            QVariantList converted;

            // We need enough arguments to perform the conversion.

            QList<QByteArray> methodTypes = metaMethod.parameterTypes();
            if (methodTypes.size() < args.size()) {
                //qWarning() << "Insufficient arguments to call" << metaMethod.signature();
                error = ErrorCode::INVALID_ARGUMENT_NUMBER;
                return QVariant();
            }

            for (int i = 0; i < methodTypes.size(); i++) {
                const QVariant& arg = args.at(i);

                QByteArray methodTypeName = methodTypes.at(i);
                //QByteArray argTypeName = arg.typeName();

                QVariant::Type methodType = QVariant::nameToType(methodTypeName);
                //QVariant::Type argType = arg.type();

                QVariant copy = QVariant(arg);

                // If the types are not the same, attempt a conversion. If it
                // fails, we cannot proceed.

                if (copy.type() != methodType) {
                    if (copy.canConvert(methodType)) {
                        if (!copy.convert(methodType)) {
                            /*qWarning() << "Cannot convert" << argTypeName
                                       << "to" << methodTypeName;*/
                            error = ErrorCode::INVALID_ARGUMENT_TYPE;
                            return QVariant();
                        }
                    }
                }

                converted << copy;
            }

            QList<QGenericArgument> arguments;

            for (int i = 0; i < converted.size(); i++) {

                // Notice that we have to take a reference to the argument, else
                // we'd be pointing to a copy that will be destroyed when this
                // loop exits.

                QVariant& argument = converted[i];

                // A const_cast is needed because calling data() would detach
                // the QVariant.

                QGenericArgument genericArgument(
                    QMetaType::typeName(argument.userType()),
                    const_cast<void*>(argument.constData())
                );

                arguments << genericArgument;
            }

            QVariant returnValue;
            if (QString(metaMethod.typeName()) != "void") {
                returnValue = QVariant(QMetaType::type(metaMethod.typeName()),
                    static_cast<void*>(NULL));
            }

            QGenericReturnArgument returnArgument(
                metaMethod.typeName(),
                const_cast<void*>(returnValue.constData())
            );

            // Perform the call

            // FIXME Crash when invoking method with complex return type (e.g. QList<Selection>)
            bool ok = metaMethod.invoke(
                object,
                Qt::DirectConnection,
                returnArgument,
                arguments.value(0),
                arguments.value(1),
                arguments.value(2),
                arguments.value(3),
                arguments.value(4),
                arguments.value(5),
                arguments.value(6),
                arguments.value(7),
                arguments.value(8),
                arguments.value(9)
            );

            if (!ok) {
                //qWarning() << "Calling" << metaMethod.signature() << "failed.";
                error = ErrorCode::METHOD_NOT_FOUND;
                return QVariant();
            } else {
                return returnValue;
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
