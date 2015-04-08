#ifndef EXTENSIONS_STUBS_EDITOR_H
#define EXTENSIONS_STUBS_EDITOR_H

#include <QObject>
#include <QScriptEngine>

namespace Extensions {
    namespace Stubs {

        class Editor : public QObject
        {
            Q_OBJECT
        public:
            explicit Editor(Editor realObject, QObject *parent = 0);
            ~Editor();

            /*QString call(QString name) const {
                m_realObject.
            }*/
        signals:

        public slots:

        private:
            Editor m_realObject;

        };

    }
}

#endif // EXTENSIONS_STUBS_EDITOR_H

