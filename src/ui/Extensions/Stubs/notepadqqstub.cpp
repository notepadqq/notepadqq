#include "include/Extensions/Stubs/notepadqqstub.h"
#include "include/Extensions/Stubs/windowstub.h"
#include "include/notepadqq.h"
#include "include/Extensions/runtimesupport.h"
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

        NQQ_DEFINE_EXTENSION_METHOD(NotepadqqStub, testGetWindow, )
        {
            QSharedPointer<Stub> stub = QSharedPointer<Stub>(
                        new WindowStub(MainWindow::instances().first(), runtimeSupport()));
            qint32 stubId = runtimeSupport()->presentObject(stub);

            StubReturnValue ret;
            ret.result = QJsonValue(static_cast<qint64>(stubId));
            ret.resultStubName = WindowStub::stubName();
            return ret;
        }

    }
}
