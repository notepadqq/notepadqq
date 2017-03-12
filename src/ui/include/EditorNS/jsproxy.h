#ifndef _JSPROXY_H_
#define _JSPROXY_H_

#include <QObject>
#include <QVariant>
#include <QPair>
#include <QQueue>

namespace EditorNS {
    /**
     * @brief QWebChannel control class.
     *        This class is the direct interface for QWebChannel/Javascript
     *        communication.  In order to use this class, you should use:
     *        - sendMsg("C_CMD_SOME_FUNC", qvariant data); to send messages to 
     *          Javascript
     *        - getValue("value_name", local variable); to retrieve common data
     *          types.
     *        - getRawValue("value_name"); to retrieve data types that need
     *          special handling.
    */
    
    class JsToCppProxy : public QObject
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
        bool hasKey(const QString&);
        QHash<QString, QVariant> m_values;
        QQueue<QPair<QString, QVariant>>* m_tempQueue;
        QVariant m_result;
        bool m_ready = false;
    public:
        JsToCppProxy(QObject *parent);

        /**
         * @brief Get the last result returned from javascript.
         * @return QVariant result.
         */
        QVariant getResult();
        /**
         * @brief Get a synced value from javascript.  The variable in
         *        [out]& is set to the data if it is available, otherwise
         *        getValue returns false and does nothing.
         * @param [in]const QString& The key to lookup.
         * @param [out]& A reference to a local variable.
         * @return true on success, false otherwise.
         */
        bool getValue(const QString&, int&);
        bool getValue(const QString&, QPair<int, int>&);
        bool getValue(const QString&, bool&);
        bool getValue(const QString&, QString&);
        bool getValue(const QString&, QStringList&);
        /**
         * @brief Get a synced value from javascript with no error handling.
         *        This method should be used when you need to perform any
         *        custom handling to the data prior to using it.  Or if one
         *        of the getValue() methods doesn't support the data type.
         * @param const QString& The key to lookup.
         * @return QVariant containing the unmodified data of m_values[key].
         */
        QVariant getRawValue(const QString&);
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
        /**
         * @brief Slot which allows Javascript to set m_values[key] data.
         *        Usage example: proxy.setValue("my_key", [10, 2]);
         *        The data variable is automatically converted to a QVariant
         *        when it arrives to CPP.
         * @param QString The key to set in m_values.
         * @param QVariant The data to set m_values[key] to.
         */
        Q_INVOKABLE void setValue(QString name, QVariant data);

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
