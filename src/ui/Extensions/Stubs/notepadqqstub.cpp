#include "include/Extensions/Stubs/notepadqqstub.h"
#include "include/notepadqq.h"
#include <QApplication>
#include <QJsonArray>

namespace Extensions {
    namespace Stubs {

        NotepadqqStub::NotepadqqStub(RuntimeSupport *rts) : Stub(rts)
        {

        }

        NotepadqqStub::~NotepadqqStub()
        {

        }

        NQQ_DEFINE_EXTENSION_METHOD(NotepadqqStub, commandLineArguments, )
        {
            QJsonArray arr = QJsonArray::fromStringList(QApplication::arguments());
            StubReturnValue ret;
            ret.result = arr;
            return ret;
        }

        NQQ_DEFINE_EXTENSION_METHOD(NotepadqqStub, version, )
        {
            StubReturnValue ret;
            ret.result = QJsonValue(::Notepadqq::version);
            return ret;
        }

        NQQ_DEFINE_EXTENSION_METHOD(NotepadqqStub, print, args)
        {
            qDebug() << args.toArray().at(0).toString().toStdString().c_str();
            return StubReturnValue();
        }

    }
}
