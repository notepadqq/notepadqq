#define NQQ_DECLARE_EXTENSION_METHOD(method_name) \
    StubReturnValue method_name(const QJsonValue &); \
    bool __nqqmeta__##method_name = registerMethod(QString(#method_name), \
              [&](const QJsonValue &args) -> Stub::StubReturnValue { \
        return method_name(args); \
    });

#define NQQ_DEFINE_EXTENSION_METHOD(class_name, method_name, args_name) \
    Stub::StubReturnValue class_name::method_name(const QJsonValue &args_name)

#ifndef EXTENSIONS_STUBS_STUB_H
#define EXTENSIONS_STUBS_STUB_H

#include <QObject>
#include <QWeakPointer>
#include <QJsonValue>
#include <functional>
#include <QHash>

namespace Extensions {
    namespace Stubs {

        class Stub : public QObject
        {
            Q_OBJECT
        public:
            explicit Stub();
            explicit Stub(QWeakPointer<QObject> object);
            explicit Stub(QSharedPointer<QObject> object);
            ~Stub();

            struct StubReturnValue {
                QJsonValue result;
                QJsonValue error;
            };

            bool invoke(const QString &method, StubReturnValue &ret, const QJsonValue &args);

        signals:

        public slots:

        protected:
            QWeakPointer<QObject> object();
            QSharedPointer<QObject> objectSharedPtr();
            bool isWeak();
            bool hasObject();

            bool registerMethod(const QString &methodName, std::function<StubReturnValue (const QJsonValue &)> method);

        private:
            QWeakPointer<QObject> m_weakPointer;
            QSharedPointer<QObject> m_sharedPointer;
            bool m_weak;
            bool m_hasObject;
            QHash<QString, std::function<StubReturnValue (const QJsonValue &)>> m_methods;
        };

    }
}

#endif // EXTENSIONS_STUBS_STUB_H
