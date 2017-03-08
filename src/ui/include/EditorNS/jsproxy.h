#ifndef _JSPROXY_H_
#define _JSPROXY_H_

#include <QObject>
#include <QVariant>
#include <QPair>
#include <QQueue>

namespace EditorNS {
    /**
     * @brief An Object injectable into the javascript page, that allows
     *        the javascript code to send messages to an Editor object.
     *        It also allows the js instance to retrieve message data 
     *        information.
    */
    
    class JsToCppProxy : public QObject
    {
        Q_OBJECT

    private:
        void pushQueuedMessages();
        bool hasKey(const QString&);

        QHash<QString, QVariant> m_values;
        QQueue<QPair<QString, QVariant>> m_queuedMessages;
        QVariant m_result;
        bool m_ready = false;
    public:
        JsToCppProxy(QObject *parent) : QObject(parent) { }

        // These are our data retrieval mechanisms.
        QVariant getResult();
        bool getValue(const QString&, int&);
        bool getValue(const QString&, QPair<int, int>&);
        bool getValue(const QString&, bool&);
        bool getValue(const QString&, QString&);
        bool getValue(const QString&, QStringList&);
        QVariant getRawValue(const QString&);
        // Functions to allow the proxy to set data on the CPP side.
        void setResult(QVariant data);
        void sendMsg(QString msg, QVariant data);
        Q_PROPERTY(QVariant result READ getResult WRITE setResult NOTIFY replyReady);
    public slots:
        Q_INVOKABLE void sendEditorEvent(QString msg, QVariant data);
        Q_INVOKABLE void setValue(QString name, QVariant data);

    signals:
        void sendMsgInternal(QString msg, QVariant data);
        void replyReady();
        void editorEvent(QString msg, QVariant data);
    };

}

#endif//_JSPROXY_H_
