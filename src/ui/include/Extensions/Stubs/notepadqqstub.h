#ifndef EXTENSIONS_STUBS_NOTEPADQQ_H
#define EXTENSIONS_STUBS_NOTEPADQQ_H

#include "include/Extensions/Stubs/stub.h"
#include "include/mainwindow.h"

namespace Extensions {
    namespace Stubs {

        class NotepadqqStub : public Stub
        {
            Q_OBJECT
        public:
            NotepadqqStub(RuntimeSupport *rts);
            ~NotepadqqStub();

            NQQ_STUB_NAME("Notepadqq")

        private:
            NQQ_DECLARE_EXTENSION_METHOD(commandLineArguments)
            NQQ_DECLARE_EXTENSION_METHOD(version)
            NQQ_DECLARE_EXTENSION_METHOD(print)
            NQQ_DECLARE_EXTENSION_METHOD(windows)

            void on_newWindow(MainWindow *window);
        };

    }
}

#endif // EXTENSIONS_STUBS_NOTEPADQQ_H
