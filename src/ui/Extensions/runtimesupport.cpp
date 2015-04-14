#include "include/Extensions/runtimesupport.h"
#include "include/Extensions/Stubs/notepadqq_stub.h"

namespace Extensions {

    RuntimeSupport::RuntimeSupport(QObject *parent) : QObject(parent)
    {
        m_pointers.insert(1, QSharedPointer<Stubs::Stub>(new Stubs::Notepadqq()));
    }

    RuntimeSupport::~RuntimeSupport()
    {

    }

    QJsonObject RuntimeSupport::handleRequest(const QJsonObject &request)
    {
        unsigned long objectId = request.value("objectId").toDouble();
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

    unsigned long RuntimeSupport::presentObject(QSharedPointer<Stubs::Stub> stub)
    {
        static unsigned long counter = 0;

        unsigned long id = ++counter;
        m_pointers.insert(id, stub);
        return id;
    }

}
