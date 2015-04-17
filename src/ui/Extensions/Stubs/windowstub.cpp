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
            MainWindow *window = static_cast<MainWindow*>(objectUnmanagedPtr());
            QSharedPointer<Stub> stub = QSharedPointer<Stub>(
                        new EditorStub(window->currentEditorSharedPtr(), runtimeSupport()));
            qint32 stubId = runtimeSupport()->presentObject(stub);

            StubReturnValue ret;
            ret.result = QJsonValue(stubId);
            ret.resultStubType = EditorStub::stubName();
            return ret;
        }

    }
}
