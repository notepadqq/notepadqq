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
            StubReturnValue ret;
            //MainWindow *window = objectUnsafePtr();

            //window->currentEditor()
            //QSharedPointer<Stub> obj = QSharedPointer<Stub>(new EditorStub(runtimeSupport()));
            //ret.result = QJsonValue(static_cast<qint64>(runtimeSupport()->presentObject(obj)));
            return ret;
        }

    }
}
