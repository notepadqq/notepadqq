#ifndef EDITOR_H
#define EDITOR_H

#include <QObject>
#include <QVariant>
#include <QWebView>
#include <QQueue>

/**
 * @brief An Object injectable into the javascript page, that allows
 *        the javascript code to send messages to an Editor object.
 *        It also allows the js instance to retrieve message data information.
 *
 * Note that this class is only needed for the current Editor
 * implementation, that uses QWebView.
 */
class JsToCppProxy : public QObject
{
    Q_OBJECT

private:
    QVariant msgData;

public:
    /**
     * @brief Set C++-to-JS message data. This method should
     *        be called from the C++ part
     * @param data
     */
    void setMsgData(QVariant data) { msgData = data; }

    /**
     * @brief Get the message data set by setMsgData(). This
     *        method should be called from the JavaScript part.
     */
    Q_INVOKABLE QVariant getMsgData() { return msgData; }

signals:
    /**
     * @brief A JavaScript message has been received.
     * @param msg Message type
     * @param data Message data
     */
    void messageReceived(QString msg, QVariant data);
};


/**
 * @brief Provides a JavaScript CodeMirror instance.
 *
 * Communication works by sending messages to the javascript Editor using
 * the sendMessage() method. On the other side, when a javascript event
 * occurs, the messageReceived() signal will be emitted.
 *
 * In addition to messageReceived(), other signals could be emitted at the
 * same time, for example currentLineChanged(). This is simply for
 * convenience, so that the user of this class doesn't need to manually parse
 * the arguments for pre-defined messages.
 *
 */
class Editor : public QWidget
{
    Q_OBJECT
public:
    explicit Editor(QWidget *parent = 0);
    ~Editor();

    /**
     * @brief Give focus to the editor, so that the user can start
     *        typing. Note that calling won't automatically switch to
     *        the tab where the editor is. Use EditorTabWidget::setCurrentIndex()
     *        and TopEditorContainer::setFocus() for that. (actually it's a bug)
     */
    void setFocus();

    /**
     * @brief Set the file name associated with this editor
     * @param filename full path of the file
     */
    void setFileName(QString filename);

    /**
     * @brief Get the file name associated with this editor
     * @return
     */
    QString fileName();

    // Common messages:
    bool isClean();
    QMap<QString, QList<QString> > languages();
    void setLanguage(QString language);
    QString value();
private:
    QWebView *webView;
    JsToCppProxy *jsToCppProxy;
    QString m_fileName;
    QString jsStringEscape(QString str);

private slots:
    void on_javaScriptWindowObjectCleared();
    void on_proxyMessageReceived(QString msg, QVariant data);

signals:
    void messageReceived(QString msg, QVariant data);

    void gotFocus();

    // Pre-interpreted messages:
    void contentChanged();
    void cursorActivity();
    void cleanChanged(bool isClean);

    /**
     * @brief The editor finished loading. There should be
     *        no need to use this signal outside this class.
     */
    void editorReady();

public slots:
    void sendMessage(QString msg, QVariant data);
    void sendMessage(QString msg);
    QVariant sendMessageWithResult(QString msg, QVariant data);
    QVariant sendMessageWithResult(QString msg);
};

#endif // EDITOR_H
