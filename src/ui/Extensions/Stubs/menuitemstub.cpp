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

    }
}
