#include "include/Extensions/Stubs/windowstub.h"

#include "include/Extensions/Stubs/editorstub.h"
#include "include/Extensions/Stubs/menuitemstub.h"
#include "include/Extensions/runtimesupport.h"

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
                        new EditorStub(window->currentEditor().toWeakRef(), rts));
            qint32 stubId = rts->presentObject(stub);

            return StubReturnValue(rts->getJSONStub(stubId, stub->stubName_()));
        }

        NQQ_DEFINE_EXTENSION_METHOD(WindowStub, addExtensionMenuItem, args)
        {
            if (!(args.count() >= 2))
                return StubReturnValue(ErrorCode::INVALID_ARGUMENT_NUMBER);

            RuntimeSupport *rts = runtimeSupport();
            MainWindow *window = static_cast<MainWindow*>(objectUnmanagedPtr());

            Q_ASSERT(args.count() >= 2);
            QAction *menuItem = window->addExtensionMenuItem(args.at(0).toString(), convertToString(args.at(1)));

            QSharedPointer<Stub> stub = QSharedPointer<Stub>(
                        new MenuItemStub(menuItem, rts));
            qint32 stubId = rts->presentObject(stub);

            return StubReturnValue(rts->getJSONStub(stubId, stub->stubName_()));
        }

    }
}
