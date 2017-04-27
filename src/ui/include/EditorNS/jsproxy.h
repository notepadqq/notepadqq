#ifndef _JSPROXY_H_
#define _JSPROXY_H_

#include <QObject>
#include <QPair>
#include <QQueue>
#include <QVariant>

namespace EditorNS {
    /**
     * @brief QWebChannel control class.
     *        This class is the direct interface for QWebChannel/Javascript
     *        communication.  In order to use this class, you should use:
     *        - sendMsg("C_CMD_SOME_FUNC", qvariant data); to send messages to 
     *          Javascript
    */
    
    class JsProxy : public QObject
    {
        Q_OBJECT

    private:
        /**
         * @brief Internal JsProxy function which pushes messages in
         *        m_tempQueue to javascript, once the proxy is available.
         *        The m_tempQueue object is deleted once this function is
         *        called to conserve memory.
         */
        void pushQueuedMessages();
        /**
         * @brief Internal JsProxy function which performs a few small tests
         *        to ensure the validity and existence of a key within m_values.
         * @param const QString& The key to check.
         * @return true if the key exists and has a value, false otherwise.
         */
        QQueue<QPair<QString, QVariant>>* m_tempQueue;
        QVariant m_result;
        bool m_ready = false;
    public:
        JsProxy(QObject *parent);

        /**
         * @brief Get the last result returned from javascript.
         * @return QVariant result.
         */
        QVariant getResult();
        /**
         * @brief Sets the m_result variable in JsProxy.
         *        This function should not be used outside of the class, as it
         *        is meant for interfacing with Javascript.
         * @param QVariant The data to set m_result to.
         */
        void setResult(QVariant data);
        /**
         * @brief Send a message to javascript.  This allows CPP to interface
         *        with javascript by sending messages.  If the proxy isn't 
         *        available, messages will be queued in m_tempQueue and pushed
         *        in bulk once the proxy is accessible.
         * @param const QString& The message to send, please refer to app.js
         *        for a list of available messages.
         * @param const QVariant& The QVariant data to be sent to Javascript.
         */
        void sendMsg(const QString& msg, const QVariant& data);
        Q_PROPERTY(QVariant result READ getResult WRITE setResult NOTIFY replyReady);
    public slots:
        /**
         * @brief Slot which emits the editorEvent signal.
         * @param QString The message Javascript sent.
         * @param QVariant The data Javascript sent, if any.
         */
        Q_INVOKABLE void sendEditorEvent(QString msg, QVariant data);
    signals:
        /**
         * @brief Internal message handler for JsProxy.  This is called when
         *        sendMsg is invoked, assuming the proxy is ready.
         * @param const QString& The message to send.
         * @param const QVariant& The QVariant data to be sent to Javascript.
         */
        void sendMsgInternal(const QString& msg, const QVariant& data);
        /**
         * @brief Signal for when m_result has changed.
         */
        void replyReady();
        /**
         * @brief Passthrough which forwards signals to our Editor class.
         * @param QString The message Javascript sent.
         * @param QVariant The data Javascript sent, if any.
         */
        void editorEvent(QString msg, QVariant data);
    };

}

#endif//_JSPROXY_H_
