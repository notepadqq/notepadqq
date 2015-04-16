#include "include/Extensions/runtimesupport.h"
#include "include/Extensions/Stubs/notepadqqstub.h"

namespace Extensions {

    RuntimeSupport::RuntimeSupport(QObject *parent) : QObject(parent)
    {
        m_pointers.insert(1, QSharedPointer<Stubs::Stub>(new Stubs::NotepadqqStub(this)));
    }

    RuntimeSupport::~RuntimeSupport()
    {

    }

    QJsonObject RuntimeSupport::handleRequest(const QJsonObject &request)
    {
        quint32 objectId = request.value("objectId").toDouble();
        QString method = request.value("method").toString();

        // Fail if some fields are missing
        if (objectId <= 0 || method.isEmpty()) {
            QJsonObject retJson;
            retJson.insert("err", "Invalid request");
            return retJson;
        }

        QSharedPointer<Stubs::Stub> object = m_pointers.value(objectId);

        if (!object.isNull()) {
            QJsonValue jsonArgs = request.value("args");

            Stubs::Stub::StubReturnValue ret;
            bool invoked = object->invoke(method, ret, jsonArgs);

            if (invoked) {
                QJsonObject retJson;
                retJson.insert("return", ret.result);
                retJson.insert("err", ret.error);
                if (!ret.resultStubName.isNull())
                    retJson.insert("resultStubType", ret.resultStubName);
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

    quint32 RuntimeSupport::presentObject(QSharedPointer<Stubs::Stub> stub)
    {
        static quint32 counter = 100;
        // FIXME Do not reallocate same object

        quint32 id = ++counter;
        m_pointers.insert(id, stub);
        return id;
    }

}
