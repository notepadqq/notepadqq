#ifndef WINDOWSTUB_H
#define WINDOWSTUB_H

#include <QObject>
#include "include/Extensions/Stubs/stub.h"
#include "include/mainwindow.h"

namespace Extensions {
    namespace Stubs {

        class WindowStub : public Stub
        {
            Q_OBJECT
        public:
            WindowStub(MainWindow *object, RuntimeSupport *rts);
            ~WindowStub();

            NQQ_STUB_NAME("Window")
        private:
            NQQ_DECLARE_EXTENSION_METHOD(currentEditor)
            NQQ_DECLARE_EXTENSION_METHOD(addExtensionMenuItem)
        };

    }
}

#endif // WINDOWSTUB_H
