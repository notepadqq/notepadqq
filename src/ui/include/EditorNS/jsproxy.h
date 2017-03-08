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
        QVariant m_result;

        // Some editor data we push on the fly
        QVariant m_textLength;
        QVariant m_lineCount;
        QVariant m_selections;
        QVariant m_selectionsText;
        QVariant m_scrollPosition;
        QVariant m_language;
        bool m_clean;
        QVariant m_detectedIndent;
        QHash<QString, QVariant> m_values;

    public:
        JsToCppProxy(QObject *parent) : QObject(parent) { }

        // These are our data retrieval mechanisms.
        QVariant getResult();
        QVariant getTextLength();
        QVariant getLineCount();
        QVariant getSelections();
        QVariant getSelectionsText();
        QVariant getScrollPosition();
        QVariant getLanguage();
        bool getClean();
        QVariant getDetectedIndent();
        template <class T> bool getValue(T& local, const QString& dataName);
        bool getValue(QPair<int, int>&, const QString&);

        // Functions to allow the proxy to set data on the CPP side.
        void setResult(QVariant data);
        void setTextLength(QVariant textLength);
        void setLineCount(QVariant lineCount);
        void setSelections(QVariant selections);
        void setSelectionsText(QVariant selectionsText);
        void setScrollPosition(QVariant scrollPosition);
        void setLanguage(QVariant language);
        void setClean(bool clean);
        void setDetectedIndent(QVariant detectedIndent);

        // Expose our properties to the JS-side.
        // TODO: voidActivity signal for stuff that don't matter
        Q_PROPERTY(QVariant result READ getResult WRITE setResult NOTIFY replyReady);
        Q_PROPERTY(QVariant textLength READ getTextLength WRITE setTextLength NOTIFY cursorActivity);
        Q_PROPERTY(QVariant lineCount READ getLineCount WRITE setLineCount NOTIFY cursorActivity);
        Q_PROPERTY(QVariant selections READ getSelections WRITE setSelections NOTIFY cursorActivity);
        Q_PROPERTY(QVariant selectionsText READ getSelectionsText WRITE setSelectionsText NOTIFY cursorActivity);
        Q_PROPERTY(QVariant scrollPosition READ getScrollPosition WRITE setScrollPosition NOTIFY cursorActivity);
        Q_PROPERTY(QVariant language READ getLanguage WRITE setLanguage NOTIFY languageChange);
        Q_PROPERTY(bool clean READ getClean WRITE setClean NOTIFY cursorActivity);
        Q_PROPERTY(QVariant detectedIndent READ getDetectedIndent WRITE setDetectedIndent NOTIFY cursorActivity);
    public slots:
        Q_INVOKABLE void sendEditorEvent(QString msg, QVariant data);
        Q_INVOKABLE void setValue(QString name, QVariant data);

    signals:
        /**
             * @brief A JavaScript message has been received.
             * @param msg Message type
             * @param data Message data
             */
        void sendMsg(QString msg, QVariant data);
        void replyReady();
        void languageChange();
        void editorEvent(QString msg, QVariant data);
        void cursorActivity();
    };

}


Q_DECLARE_METATYPE(EditorNS::CursorPosition);

#endif//_JSPROXY_H_
