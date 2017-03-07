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
        QVariant m_cursor;
        QVariant m_textLength;
        QVariant m_lineCount;
        QVariant m_selections;
        QVariant m_selectionsText;
        QVariant m_scrollPosition;

    public:
        JsToCppProxy(QObject *parent) : QObject(parent) { }

        // These are our data retrieval mechanisms.
        QVariant getMsgData();
        QVariant getResult();
        QVariant getCursor();
        QVariant getTextLength();
        QVariant getLineCount();
        QVariant getSelections();
        QVariant getSelectionsText();
        QVariant getScrollPosition();

        // Functions to allow the proxy to set data on the CPP side.
        void setResult(QVariant data);
        void setCursor(QVariant cursorPos);
        void setTextLength(QVariant textLength);
        void setLineCount(QVariant lineCount);
        void setSelections(QVariant selections);
        void setSelectionsText(QVariant selectionsText);
        void setScrollPosition(QVariant scrollPosition);

        // Expose our properties to the JS-side.
        Q_PROPERTY(QVariant result READ getResult WRITE setResult NOTIFY replyReady);
        Q_PROPERTY(QVariant cursor READ getCursor WRITE setCursor NOTIFY cursorActivity);
        Q_PROPERTY(QVariant textLength READ getTextLength WRITE setTextLength NOTIFY cursorActivity);
        Q_PROPERTY(QVariant lineCount READ getLineCount WRITE setLineCount NOTIFY cursorActivity);
        Q_PROPERTY(QVariant selections READ getSelections WRITE setSelections NOTIFY cursorActivity);
        Q_PROPERTY(QVariant selectionsText READ getSelectionsText WRITE setSelectionsText NOTIFY cursorActivity);
        Q_PROPERTY(QVariant scrollPosition READ getScrollPosition WRITE setScrollPosition NOTIFY cursorActivity);
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
        void cursorActivity();
    };

}


Q_DECLARE_METATYPE(EditorNS::CursorPosition);

#endif//_JSPROXY_H_
