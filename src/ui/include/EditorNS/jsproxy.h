#ifndef _JSPROXY_H_
#define _JSPROXY_H_

#include <QObject>
#include <QVariant>
#include <QPair>

namespace EditorNS {
    /**
     * @brief An Object injectable into the javascript page, that allows
     *        the javascript code to send messages to an Editor object.
     *        It also allows the js instance to retrieve message data 
     *        information.
    */
    
    typedef QPair<int, int> CursorPosition;

    class JsToCppProxy : public QObject
    {
        Q_OBJECT

    private:
        QVariant m_msgData;
        QVariant m_result;

        // Some editor data we push on the fly
        CursorPosition m_cursor;
    public:
        JsToCppProxy(QObject *parent) : QObject(parent) { }

        // These are our data retrieval mechanisms.
        QVariant getMsgData();
        QVariant getResult();

        // Functions to allow the proxy to set data on the CPP side.
        void setResult(QVariant data);
        void setCursor(QVariant cursorData);

        // Expose our properties to the JS-side.
        Q_PROPERTY(QVariant m_result READ getResult WRITE setResult NOTIFY replyReady);
        Q_PROPERTY(QVariant m_cursor WRITE setCursor NOTIFY cursorActivity);
    public slots:
        Q_INVOKABLE void receiveMessage(QString msg, QVariant data);

    signals:
        /**
             * @brief A JavaScript message has been received.
             * @param msg Message type
             * @param data Message data
             */
        void messageReceived(QString msg, QVariant data);
        void sendMsg(QString msg, QVariant data);
        void replyReady();
    };

}


Q_DECLARE_METATYPE(EditorNS::CursorPosition);

#endif//_JSPROXY_H_
