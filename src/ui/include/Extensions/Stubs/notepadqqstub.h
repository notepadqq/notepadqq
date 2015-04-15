#ifndef EXTENSIONS_STUBS_NOTEPADQQ_H
#define EXTENSIONS_STUBS_NOTEPADQQ_H

#include "include/Extensions/Stubs/stub.h"

namespace Extensions {
    namespace Stubs {

        class NotepadqqStub : public Stub
        {
            Q_OBJECT
        public:
            NotepadqqStub(RuntimeSupport *rts);
            ~NotepadqqStub();

        private:
            NQQ_DECLARE_EXTENSION_METHOD(commandLineArguments)
            NQQ_DECLARE_EXTENSION_METHOD(version)
            NQQ_DECLARE_EXTENSION_METHOD(print)
        };

    }
}

#endif // EXTENSIONS_STUBS_NOTEPADQQ_H
