#ifndef EDITORSTUB_H
#define EDITORSTUB_H

#include <QObject>
#include "include/Extensions/Stubs/stub.h"

namespace Extensions {
    namespace Stubs {

        class EditorStub : public Stub
        {
        public:
            EditorStub(QSharedPointer<QObject> object, RuntimeSupport *rts);
            ~EditorStub();

            NQQ_STUB_NAME("Editor")
        };

    }
}

#endif // EDITORSTUB_H
