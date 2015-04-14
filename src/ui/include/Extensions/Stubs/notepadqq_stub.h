#ifndef EXTENSIONS_STUBS_NOTEPADQQ_H
#define EXTENSIONS_STUBS_NOTEPADQQ_H

#include "include/Extensions/Stubs/stub.h"

namespace Extensions {
    namespace Stubs {

        class Notepadqq : public Stub
        {
            Q_OBJECT
        public:
            Notepadqq();
            ~Notepadqq();

        private:
            NQQ_DECLARE_EXTENSION_METHOD(commandLineArguments)
            NQQ_DECLARE_EXTENSION_METHOD(version)
        };

    }
}

#endif // EXTENSIONS_STUBS_NOTEPADQQ_H
