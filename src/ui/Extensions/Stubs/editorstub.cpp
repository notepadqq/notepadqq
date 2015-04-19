#include "include/Extensions/Stubs/editorstub.h"

namespace Extensions {
    namespace Stubs {

        EditorStub::EditorStub(const QWeakPointer<QObject> &object, RuntimeSupport *rts) : Stub(object, rts)
        {

        }

        EditorStub::~EditorStub()
        {

        }

        EditorNS::Editor *EditorStub::editor()
        {
            QWeakPointer<EditorNS::Editor> editor = objectWeakPtr().toStrongRef().staticCast<EditorNS::Editor>();
            return editor.data();
        }

        NQQ_DEFINE_EXTENSION_METHOD(EditorStub, setValue, args)
        {
            if (!(args.count() >= 1))
                return StubReturnValue(ErrorCode::INVALID_ARGUMENT_NUMBER);

            Q_ASSERT(args.count() >= 1);

            editor()->setValue(convertToString(args.at(0)));
            return StubReturnValue();
        }

        NQQ_DEFINE_EXTENSION_METHOD(EditorStub, value, )
        {
            return StubReturnValue(editor()->value());
        }

    }
}
