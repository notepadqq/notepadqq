#include "include/Extensions/runtimesupport.h"
#include "include/Extensions/Stubs/notepadqqstub.h"
#include "include/Extensions/extensionsserver.h"
#include "include/Extensions/extensionsloader.h"
#include <QJsonArray>

namespace Extensions {

    RuntimeSupport::RuntimeSupport(QObject *parent) : QObject(parent)
    {
        QSharedPointer<Stubs::Stub> nqqStub = QSharedPointer<Stubs::Stub>(new Stubs::NotepadqqStub(this));
        m_pointers.insert(1, nqqStub);
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
            QJsonObject retJson;
            retJson.insert("err", "Invalid request");
            return retJson;
        }

        Q_ASSERT(objectId >= 0 && method.length() > 0);

        QSharedPointer<Stubs::Stub> object = m_pointers.value(objectId);

        if (!object.isNull()) {
            // FIXME if not object->isAlive(), then send an error and remove the stub from m_pointers

            QJsonArray jsonArgs = request.value("args").toArray();

            Stubs::Stub::StubReturnValue ret;
            bool invoked = object->invoke(method, ret, jsonArgs);

            if (invoked) {
                QJsonObject retJson;
                retJson.insert("result", ret.result);
                retJson.insert("err", ret.error);
                return retJson;
            } else {
                QJsonObject retJson;
                retJson.insert("err", "Invalid method");
                return retJson;
            }

        } else {
            m_pointers.remove(objectId);

            QJsonObject retJson;
            retJson.insert("err", "Invalid object"); // FIXME Better error codes

            return retJson;
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

}
