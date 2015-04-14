#include "include/Extensions/Stubs/notepadqq_stub.h"
#include "include/notepadqq.h"
#include <QApplication>
#include <QJsonArray>

namespace Extensions {
    namespace Stubs {

        Notepadqq::Notepadqq() : Stub()
        {

        }

        Notepadqq::~Notepadqq()
        {

        }

        NQQ_DEFINE_EXTENSION_METHOD(Notepadqq, commandLineArguments, )
        {
            QJsonArray arr = QJsonArray::fromStringList(QApplication::arguments());
            StubReturnValue ret;
            ret.result = arr;
            return ret;
        }

        NQQ_DEFINE_EXTENSION_METHOD(Notepadqq, version, )
        {
            StubReturnValue ret;
            ret.result = QJsonValue(::Notepadqq::version);
            return ret;
        }

    }
}
