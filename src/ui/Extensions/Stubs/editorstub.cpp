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
            return static_cast<EditorNS::Editor*>(objectUnmanagedPtr());
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

        NQQ_DEFINE_EXTENSION_METHOD(EditorStub, setSelectionsText, args)
        {
            if (!args.at(0).isArray())
                return StubReturnValue(ErrorCode::INVALID_ARGUMENT_TYPE);

            QStringList strList;
            QJsonArray jsonArr = args.at(0).toArray();
            for (int i = 0; i < jsonArr.count(); i++) {
                strList.append(convertToString(jsonArr.at(i)));
            }

            editor()->setSelectionsText(strList);
            return StubReturnValue();
        }

        /*Stub::StubReturnValue EditorStub::markClean(const QJsonArray &args)
        {
            EditorNS::Editor *tmp = static_cast<EditorNS::Editor*>(objectUnmanagedPtr());
            tmp->setValue(args.at(0).toBool());
            bool param0;
            //QJsonValue(a);
            QJsonValue val = args.at(0);
            if (val.isBool()) {
                param0 = val.toBool();
            } else if (val.isString()) {
                param0 = val.toString();
            } else if (val.isObject()) {
                param0 = val.toObject();
            }
            return StubReturnValue();
        }*/
    }
}
