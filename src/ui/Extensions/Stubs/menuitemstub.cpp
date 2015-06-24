#include "include/Extensions/Stubs/menuitemstub.h"
#include "include/Extensions/runtimesupport.h"

namespace Extensions {
    namespace Stubs {

        MenuItemStub::MenuItemStub(QAction *object, RuntimeSupport *rts) : Stub(object, rts)
        {
            connect(object, &QAction::triggered, this, &MenuItemStub::on_triggered);
        }

        MenuItemStub::~MenuItemStub()
        {

        }

        void MenuItemStub::on_triggered(bool checked)
        {
            RuntimeSupport *rts = runtimeSupport();

            QJsonArray args;
            args.append(checked);

            rts->emitEvent(this, "triggered", args);
        }

        NQQ_DEFINE_EXTENSION_METHOD(MenuItemStub, setShortcut, args)
        {
            if (!(args.count() == 1))
                return StubReturnValue(ErrorCode::INVALID_ARGUMENT_NUMBER);

            Q_ASSERT(args.count() == 1);

            QAction *action = static_cast<QAction*>(objectUnmanagedPtr());
            action->setShortcut(QKeySequence::fromString(convertToString(args.at(0))));

            return StubReturnValue();
        }

    }
}
