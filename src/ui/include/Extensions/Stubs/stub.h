/** Declare a method that can be called by the extensions. Add this macro
 *  to the private section of a stub.
 *  For example: NQQ_DECLARE_EXTENSION_METHOD(setValue)
 */
#define NQQ_DECLARE_EXTENSION_METHOD(method_name) \
    StubReturnValue method_name(const QJsonArray &); \
    bool __nqqmeta__##method_name = registerMethod(QString(#method_name), \
              [&](const QJsonArray &args) -> Stub::StubReturnValue { \
        return method_name(args); \
    });

#define NQQ_DEFINE_EXTENSION_METHOD(class_name, method_name, args_name) \
    Stub::StubReturnValue class_name::method_name(const QJsonArray &args_name)

/** Set the current stub name. Add this macro to the public section of a stub.
 *  For example: NQQ_STUB_NAME("Editor")
 */
#define NQQ_STUB_NAME(stub_name) \
    inline static QString stubName() { return stub_name; } \
    inline QString stubName_() const { return stubName(); }

#ifndef EXTENSIONS_STUBS_STUB_H
#define EXTENSIONS_STUBS_STUB_H

#include <QObject>
#include <QWeakPointer>
#include <QJsonValue>
#include <functional>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>
#include <QMetaMethod>

namespace Extensions {

    class RuntimeSupport;

    namespace Stubs {

        class Stub : public QObject
        {
            Q_OBJECT
        public:
            explicit Stub(RuntimeSupport *rts);
            explicit Stub(const QWeakPointer<QObject> &object, RuntimeSupport *rts);
            explicit Stub(const QSharedPointer<QObject> &object, RuntimeSupport *rts);
            explicit Stub(QObject *object, RuntimeSupport *rts);
            virtual ~Stub() = 0;

            enum class ErrorCode {
                NONE = 0,
                INVALID_REQUEST = 1,
                INVALID_ARGUMENT_NUMBER = 2,
                INVALID_ARGUMENT_TYPE = 3,
                OBJECT_DEALLOCATED = 4,
                OBJECT_NOT_FOUND = 5,
                METHOD_NOT_FOUND = 6,
            };

            struct StubReturnValue {
                QJsonValue result;
                ErrorCode error = ErrorCode::NONE;
                QString errorString;

                StubReturnValue() {}
                StubReturnValue(const QJsonValue &_result) :
                    result(_result) {}
                StubReturnValue(const ErrorCode &_error, const QString &_errorString = QString()) :
                    error(_error), errorString(_errorString) {}
                StubReturnValue(const QJsonValue &_result, const ErrorCode &_error, const QString &_errorString = QString()) :
                    result(_result), error(_error), errorString(_errorString) {}

                QJsonObject toJsonObject() {
                    QJsonObject ret;
                    ret.insert("result", result.isUndefined() ? QJsonValue() : result);
                    ret.insert("err", static_cast<int>(error));
                    if (error != ErrorCode::NONE) {
                        ret.insert("errStr", errorString);
                    }
                    return ret;
                }
            };

            enum class PointerType {
                DETACHED,
                WEAK_POINTER,
                SHARED_POINTER,
                UNMANAGED_POINTER
            };

            /**
             * @brief Call this method to know if the object referenced by this stub is still valid.
             * @return
             */
            virtual bool isAlive();

            /**
             * @brief Invoke a registered method. This method must have been registered with
             *        registerMethod(), or using the NQQ_DECLARE_EXTENSION_METHOD macro.
             * @param method method name
             * @param ret return value of the method
             * @param args arguments for the method
             * @return true if the method has been invoked, false otherwise.
             */
            bool invoke(const QString &method, StubReturnValue &ret, const QJsonArray &args);
            virtual QString stubName_() const = 0;

            QString convertToString(const QJsonValue &value);

            bool operator ==(const Stub &other) const;
            bool operator !=(const Stub &other) const;
        signals:

        public slots:

        protected:
            QWeakPointer<QObject> objectWeakPtr();
            QSharedPointer<QObject> objectSharedPtr();
            QObject *objectUnmanagedPtr();
            PointerType pointerType();

            bool registerMethod(const QString &methodName, std::function<StubReturnValue (const QJsonArray &)> method);
            RuntimeSupport *runtimeSupport();

        private:
            RuntimeSupport *m_rts;
            PointerType m_pointerType;
            QWeakPointer<QObject> m_weakPointer;
            QSharedPointer<QObject> m_sharedPointer;
            QObject *m_unmanagedPointer = nullptr;
            QHash<QString, std::function<StubReturnValue (const QJsonArray &)>> m_methods;
            QVariant genericCall(QObject *object, QMetaMethod metaMethod, QVariantList args, ErrorCode &error);
            bool invokeOnRealObject(const QString &method, Stub::StubReturnValue &ret, const QJsonArray &args);
        };

    }
}

#endif // EXTENSIONS_STUBS_STUB_H
