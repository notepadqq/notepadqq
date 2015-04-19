#include "include/Extensions/Stubs/editorstub.h"

namespace Extensions {
    namespace Stubs {

        EditorStub::EditorStub(const QWeakPointer<QObject> &object, RuntimeSupport *rts) : Stub(object, rts)
        {

        }

        EditorStub::~EditorStub()
        {

        }

        NQQ_DEFINE_EXTENSION_METHOD(EditorStub, setValue, args)
        {
            if (!(args.count() >= 1))
                return stubReturnError(ErrorCode::INVALID_ARGUMENT_NUMBER);

            QWeakPointer<EditorNS::Editor> editor = objectWeakPtr().toStrongRef().staticCast<EditorNS::Editor>();
            if (editor.isNull())
                return stubReturnError(ErrorCode::OBJECT_DEALLOCATED);

            Q_ASSERT(args.count() >= 1);

            editor.data()->setValue(convertToString(args.at(0)));
            return StubReturnValue();
        }

    }
}
