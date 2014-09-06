#ifndef EDITOR_H
#define EDITOR_H

#include "include/customqwebview.h"
#include <QObject>
#include <QVariant>
#include <QQueue>
#include <QWheelEvent>

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
    QVariant m_msgData;

public:
    /**
     * @brief Set C++-to-JS message data. This method should
     *        be called from the C++ part
     * @param data
     */
    void setMsgData(QVariant data) { m_msgData = data; }

    /**
     * @brief Get the message data set by setMsgData(). This
     *        method should be called from the JavaScript part.
     */
    Q_INVOKABLE QVariant getMsgData() { return m_msgData; }

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
     * @brief Efficiently returns a new Editor object from an internal buffer.
     * @return
     */
    static Editor *getNewEditor();

    struct LanguageGreater {
        inline bool operator()(const QMap<QString, QString> &v1, const QMap<QString, QString> &v2) const {
            return v1.value("name").toLower() < v2.value("name").toLower();
        }
    };

    /**
     * @brief Adds a new Editor to the internal buffer used by getNewEditor().
     *        You might want to call this method e.g. as soon as the application
     *        starts (so that an Editor is ready as soon as it gets required),
     *        or when the application is idle.
     * @param howMany specifies how many Editors to add
     * @return
     */
    static void addEditorToBuffer(const int howMany = 1);

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
    void setFileName(const QString &filename);

    /**
     * @brief Get the file name associated with this editor
     * @return
     */
    QString fileName() const;

    bool fileOnDiskChanged() const;
    void setFileOnDiskChanged(bool fileOnDiskChanged);

    // Lower-level message wrappers:
    bool isClean();
    QList<QMap<QString, QString> > languages();
    void setLanguage(const QString &language);
    QString setLanguageFromFileName();
    QString value();
    void setIndentationMode(bool useTabs, int size);
    void setIndentationMode(QString language);
    qreal zoomFactor() const;
    void setZoomFactor(const qreal &factor);
private:
    CustomQWebView *m_webView;
    JsToCppProxy *m_jsToCppProxy;
    QString m_fileName = "";
    bool m_fileOnDiskChanged = false;
    bool m_loaded = false;
    static QQueue<Editor*> m_editorBuffer;

    inline void waitAsyncLoad();
    QString jsStringEscape(QString str) const;

private slots:
    void on_javaScriptWindowObjectCleared();
    void on_proxyMessageReceived(QString msg, QVariant data);

signals:
    void messageReceived(QString msg, QVariant data);
    void gotFocus();
    void mouseWheel(QWheelEvent *ev);

    // Pre-interpreted messages:
    void contentChanged();
    void cursorActivity();
    void cleanChanged(bool isClean);

    /**
     * @brief The editor finished loading. There should be
     *        no need to use this signal outside this class.
     */
    void editorReady();

    void currentLanguageChanged(QString id, QString name);

public slots:
    void sendMessage(const QString &msg, const QVariant &data);
    void sendMessage(const QString &msg);
    QVariant sendMessageWithResult(const QString &msg, const QVariant &data);
    QVariant sendMessageWithResult(const QString &msg);
};

#endif // EDITOR_H
