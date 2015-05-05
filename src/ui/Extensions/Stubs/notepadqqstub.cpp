#include "include/globals.h"
#include "include/Extensions/Stubs/notepadqqstub.h"
#include "include/Extensions/Stubs/windowstub.h"
#include "include/notepadqq.h"
#include "include/Extensions/runtimesupport.h"
#include <QApplication>

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
            return StubReturnValue(arr);
        }

        NQQ_DEFINE_EXTENSION_METHOD(NotepadqqStub, version, )
        {
            return StubReturnValue(QJsonValue(::Notepadqq::version));
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

        NQQ_DEFINE_EXTENSION_METHOD(NotepadqqStub, windows, )
        {
            QList<MainWindow *> windows = MainWindow::instances();
            RuntimeSupport *rts = runtimeSupport();

            QJsonArray jsonWindows;

            for (int i = 0; i < windows.length(); i++) {
                QSharedPointer<Extensions::Stubs::WindowStub> windowStub =
                        QSharedPointer<Extensions::Stubs::WindowStub>(
                            new Extensions::Stubs::WindowStub(windows[i], rts));

                QJsonObject stub = rts->getJSONStub(rts->presentObject(windowStub), windowStub->stubName());
                jsonWindows.append(stub);
            }

            return StubReturnValue(jsonWindows);
        }

        // TODO Implement messagebox
    }
}
