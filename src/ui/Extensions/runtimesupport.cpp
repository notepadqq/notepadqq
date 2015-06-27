#include "include/Extensions/runtimesupport.h"
#include "include/Extensions/Stubs/notepadqqstub.h"
#include "include/Extensions/extensionsserver.h"
#include "include/Extensions/extensionsloader.h"
#include <QJsonArray>

namespace Extensions {

    RuntimeSupport::RuntimeSupport(QObject *parent) : QObject(parent)
    {
        QSharedPointer<Stubs::Stub> nqqStub = QSharedPointer<Stubs::Stub>(new Stubs::NotepadqqStub(this));
        m_pointers.insert(NQQ_STUB_ID, nqqStub);
    }

    RuntimeSupport::~RuntimeSupport()
    {

    }

    QJsonObject RuntimeSupport::handleRequest(const QJsonObject &request)
    {
        qint64 objectId = request.value("objectId").toDouble();
        QString method = request.value("method").toString();

        // Fail if some fields are missing
        if (objectId <= 0 || method.isEmpty()) {
            return Stubs::Stub::StubReturnValue(
                        Stubs::Stub::ErrorCode::INVALID_REQUEST,
                        QString("Invalid request (objectId: %1, method: %2)").arg(objectId).arg(method)
                        ).toJsonObject();
        }

        Q_ASSERT(objectId >= 0 && method.length() > 0);

        QSharedPointer<Stubs::Stub> object = m_pointers.value(objectId);

        if (!object.isNull()) {

            if (object->isAlive()) {
                QJsonArray jsonArgs = request.value("args").toArray();

                Stubs::Stub::StubReturnValue ret;
                object->invoke(method, ret, jsonArgs);

                return ret.toJsonObject();

            } else {
                m_pointers.remove(objectId);
                return Stubs::Stub::StubReturnValue(
                            Stubs::Stub::ErrorCode::OBJECT_DEALLOCATED,
                            QString("Object id %1 is deallocated.").arg(objectId)
                            ).toJsonObject();
            }

        } else {
            m_pointers.remove(objectId);
            return Stubs::Stub::StubReturnValue(
                        Stubs::Stub::ErrorCode::OBJECT_NOT_FOUND,
                        QString("Object id %1 doesn't exist.").arg(objectId)
                        ).toJsonObject();
        }
    }

    qint64 RuntimeSupport::presentObject(QSharedPointer<Stubs::Stub> stub)
    {
        static qint64 counter = 100;

        qint64 oldId = findStubId(stub.data());
        if (oldId != -1) {
            return oldId;
        }

        qint64 id = ++counter;
        m_pointers.insert(id, stub);
        Q_ASSERT(findStubId(stub.data()) != -1);
        return id;
    }

    QJsonObject RuntimeSupport::getJSONStub(qint64 objectId, QString stubType)
    {
        QJsonObject obj;
        obj.insert("$__nqq__stub_type", stubType);
        obj.insert("id", objectId);
        return obj;
    }

    qint64 RuntimeSupport::findStubId(Stubs::Stub *stub)
    {
        // FIXME This runs in O(n) time. Consider using a reverse hash too.

        QHashIterator<qint64, QSharedPointer<Stubs::Stub>> i(m_pointers);

        while (i.hasNext()) {
            i.next();
            if (*(i.value().data()) == *stub) {
                return i.key();
            }
        }

        return -1;
    }

    void RuntimeSupport::emitEvent(Stubs::Stub *sender, QString event, const QJsonArray &args)
    {
        qint64 objectId = findStubId(sender);
        if (objectId != -1) {
            QJsonObject retJson;
            retJson.insert("objectId", objectId);
            retJson.insert("event", event);
            retJson.insert("args", args);

            ExtensionsLoader::extensionsServer()->broadcastMessage(retJson);
        }
    }

    QJsonObject RuntimeSupport::getCurrentExtensionStartedEvent()
    {
        QJsonObject retJson;
        retJson.insert("objectId", NQQ_STUB_ID);
        retJson.insert("event", QStringLiteral("currentExtensionStarted"));
        retJson.insert("args", QJsonArray());
        return retJson;
    }

}
