#include "include/Extensions/Stubs/windowstub.h"
#include "include/Extensions/runtimesupport.h"
#include "include/Extensions/Stubs/editorstub.h"

namespace Extensions {
    namespace Stubs {

        WindowStub::WindowStub(MainWindow *object, RuntimeSupport *rts) : Stub(object, rts)
        {

        }

        WindowStub::~WindowStub()
        {

        }

        NQQ_DEFINE_EXTENSION_METHOD(WindowStub, currentEditor, )
        {
            RuntimeSupport *rts = runtimeSupport();
            MainWindow *window = static_cast<MainWindow*>(objectUnmanagedPtr());
            QSharedPointer<Stub> stub = QSharedPointer<Stub>(
                        new EditorStub(window->currentEditorSharedPtr().toWeakRef(), rts));
            qint32 stubId = rts->presentObject(stub);

            StubReturnValue ret;
            ret.result = rts->getJSONStub(stubId, stub->stubName_());
            return ret;
        }

    }
}
