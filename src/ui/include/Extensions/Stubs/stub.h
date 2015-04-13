#ifndef STUB_H
#define STUB_H

#include <QObject>
#include <QWeakPointer>
#include <QJsonValue>

namespace Extensions {
    namespace Stubs {

        class Stub : public QObject
        {
            Q_OBJECT
        public:
            explicit Stub(QWeakPointer<QObject> object);
            explicit Stub(QSharedPointer<QObject> object);
            ~Stub();

            struct StubReturnValue {
                QJsonValue result;
                QJsonValue error;
            };

        signals:

        public slots:

        protected:
            QWeakPointer<QObject> object();
            QSharedPointer<QObject> objectSharedPtr();
            bool isWeak();

        private:
            QWeakPointer<QObject> m_weakPointer;
            QSharedPointer<QObject> m_sharedPointer;
            bool m_weak;
        };

    }
}

#endif // STUB_H
