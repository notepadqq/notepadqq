#ifndef EXTENSIONSRTS_H
#define EXTENSIONSRTS_H

#include <QObject>
#include <QHash>
#include <QSharedPointer>
#include <QJsonObject>
#include "include/Extensions/Stubs/stub.h"

namespace Extensions {

    class ExtensionsServer;

    class RuntimeSupport : public QObject
    {
        Q_OBJECT
    public:
        explicit RuntimeSupport(QObject *parent = 0);
        ~RuntimeSupport();

        QJsonObject handleRequest(const QJsonObject &request);
        qint64 presentObject(QSharedPointer<Stubs::Stub> stub);
        QJsonObject getJSONStub(qint64 objectId, QString stubType);
        void emitEvent(Stubs::Stub *sender, QString event, const QJsonArray &args);

        QJsonObject getCurrentExtensionStartedEvent();
    signals:

    public slots:

    private:
        const qint64 NQQ_STUB_ID = 1;
        QHash<qint64, QSharedPointer<Stubs::Stub>> m_pointers;
        //QHash<QSharedPointer<Stubs::Stub>, qint64> m_pointersRev;
        QSharedPointer<ExtensionsServer> m_extensionServer;
        qint64 findStubId(Stubs::Stub *stub);
    };

}

#endif // EXTENSIONSRTS_H
