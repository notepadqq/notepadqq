#include "include/Extensions/runtimesupport.h"

namespace Extensions {

    RuntimeSupport::RuntimeSupport(QObject *parent) : QObject(parent)
    {

    }

    RuntimeSupport::~RuntimeSupport()
    {

    }

    QJsonObject RuntimeSupport::handleRequest(const QJsonObject &request)
    {
        unsigned long objectId = request.value("objectId").toDouble();
        QString method = request.value("method").toString();

        if (objectId <= 0 || method.isEmpty()) {
            QJsonObject retJson;
            retJson.insert("err", "Invalid request");
            return retJson;
        }

        QJsonValue jsonArgs = request.value("args");

        QSharedPointer<Stubs::Stub> object = m_pointers.value(objectId);

        if (!object.isNull()) {
            Stubs::Stub::StubReturnValue ret;
            bool invoked = QMetaObject::invokeMethod(object.data(),
                                      method.toStdString().c_str(),
                                      Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(Stubs::Stub::StubReturnValue, ret),
                                      Q_ARG(QJsonValue, jsonArgs));

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
