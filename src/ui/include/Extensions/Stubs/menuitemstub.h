#ifndef MENUITEMSTUB_H
#define MENUITEMSTUB_H

#include <QObject>
#include "include/Extensions/Stubs/stub.h"
#include <QAction>

namespace Extensions {
    namespace Stubs {

        class MenuItemStub  : public Stub
        {
            Q_OBJECT
            void on_triggered(bool checked = false);

        public:
            MenuItemStub(QAction *object, RuntimeSupport *rts);
            ~MenuItemStub();

            NQQ_STUB_NAME("MenuItem")

        private:
            NQQ_DECLARE_EXTENSION_METHOD(setShortcut)

        };

    }
}

#endif // MENUITEMSTUB_H
