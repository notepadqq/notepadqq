#ifndef EDITORSTUB_H
#define EDITORSTUB_H

#include "include/EditorNS/editor.h"
#include "include/Extensions/Stubs/stub.h"

#include <QObject>

namespace Extensions {
    namespace Stubs {

        class EditorStub : public Stub
        {
        public:
            EditorStub(const QWeakPointer<QObject> &object, RuntimeSupport *rts);
            ~EditorStub();

            NQQ_STUB_NAME("Editor")

        private:
            NQQ_DECLARE_EXTENSION_METHOD(setValue)
            NQQ_DECLARE_EXTENSION_METHOD(value)
            NQQ_DECLARE_EXTENSION_METHOD(setSelectionsText)

            EditorNS::Editor *editor();
        };

    }
}

#endif // EDITORSTUB_H
