#include "include/globals.h"
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
            connect(&Notepadqq::getInstance(), &Notepadqq::newWindow, this, &NotepadqqStub::on_newWindow);
        }

        NotepadqqStub::~NotepadqqStub()
        {

        }

        void NotepadqqStub::on_newWindow(MainWindow *window)
        {
            RuntimeSupport *rts = runtimeSupport();
            QSharedPointer<Extensions::Stubs::WindowStub> windowStub =
                    QSharedPointer<Extensions::Stubs::WindowStub>(
                        new Extensions::Stubs::WindowStub(window, rts));

            QJsonArray args;
            args.append(rts->getJSONStub(rts->presentObject(windowStub), windowStub->stubName()));

            rts->emitEvent(this, "newWindow", args);
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
            QString output = "";

            for (int i = 0; i < args.count(); i++) {
                QJsonValue val = args.at(i);

                if (i != 0) {
                    output += " ";
                }

                if (val.isString()) {
                    output += val.toString();
                } else if (val.isDouble()) {
                    output += QString::number(val.toDouble());
                } else if (val.isBool()) {
                    output += val.toBool() ? "true" : "false";
                } else if (val.isNull()) {
                    output += "null";
                } else if (val.isUndefined()) {
                    output += "undefined";
                } else if (val.isArray()) {
                    output += "[Array]";
                } else if (val.isObject()) {
                    output += "[Object]";
                }
            }

            println(output);
            return StubReturnValue();
        }

        NQQ_DEFINE_EXTENSION_METHOD(NotepadqqStub, testGetWindow, )
        {
            QSharedPointer<Stub> stub = QSharedPointer<Stub>(
                        new WindowStub(MainWindow::instances().first(), runtimeSupport()));
            qint32 stubId = runtimeSupport()->presentObject(stub);

            StubReturnValue ret;
            ret.result = runtimeSupport()->getJSONStub(stubId, stub->stubName_());
            return ret;
        }

    }
}
