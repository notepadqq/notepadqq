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
        };

    }
}

#endif // EDITORSTUB_H
