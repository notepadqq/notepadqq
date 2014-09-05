#include "include/editor.h"
#include "include/notepadqq.h"
#include <QWebFrame>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QEventLoop>
#include <QSettings>

QQueue<Editor*> Editor::m_editorBuffer = QQueue<Editor*>();

Editor::Editor(QWidget *parent) :
    QWidget(parent)
{
    m_jsToCppProxy = new JsToCppProxy();
    connect(m_jsToCppProxy,
            &JsToCppProxy::messageReceived,
            this,
            &Editor::on_proxyMessageReceived);

    m_webView = new CustomQWebView();
    m_webView->setUrl(QUrl("file://" + Notepadqq::editorPath()));

    // To load the page in the background (http://stackoverflow.com/a/10520029):
    // (however, no noticeable improvement here on an i5, september 2014)
    //QString content = QString("<html><body onload='setTimeout(function() { window.location=\"%1\"; }, 1);'>Loading...</body></html>").arg("file://" + Notepadqq::editorPath());
    //m_webView->setContent(content.toUtf8());

    m_webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    QWebSettings *pageSettings = m_webView->page()->settings();
    #ifdef QT_DEBUG
    pageSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    #endif
    pageSettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_webView);
    setLayout(layout);

    connect(m_webView->page()->mainFrame(),
            &QWebFrame::javaScriptWindowObjectCleared,
            this,
            &Editor::on_javaScriptWindowObjectCleared);

    connect(m_webView, &CustomQWebView::mouseWheel, this, &Editor::mouseWheel);

    // TODO Display a message if a javascript error gets triggered.
    // Right now, if there's an error in the javascript code, we
    // get stuck waiting a J_EVT_READY that will never come.
}

Editor::~Editor()
{
    delete m_webView;
    delete m_jsToCppProxy;
}

Editor *Editor::getNewEditor()
{
    if (m_editorBuffer.length() == 0) {
        m_editorBuffer.enqueue(new Editor());
        return new Editor();

    } else if (m_editorBuffer.length() == 1) {
        m_editorBuffer.enqueue(new Editor());
        return m_editorBuffer.dequeue();

    } else
        return m_editorBuffer.dequeue();
}

void Editor::addEditorToBuffer(int howMany)
{
    for (int i = 0; i < howMany; i++)
        m_editorBuffer.enqueue(new Editor());
}

void Editor::waitAsyncLoad()
{
    if (!m_loaded) {
        QEventLoop loop;
        connect(this, &Editor::editorReady, &loop, &QEventLoop::quit);
        // Block until a J_EVT_READY message is received
        loop.exec();
    }
}

void Editor::on_javaScriptWindowObjectCleared()
{
    m_webView->page()->mainFrame()->
            addToJavaScriptWindowObject("cpp_ui_driver", m_jsToCppProxy);
}

void Editor::on_proxyMessageReceived(QString msg, QVariant data)
{
    emit messageReceived(msg, data);

    if(msg == "J_EVT_READY") {
        m_loaded = true;
        emit editorReady();
    } else if(msg == "J_EVT_CONTENT_CHANGED")
        emit contentChanged();
    else if(msg == "J_EVT_CLEAN_CHANGED")
        emit cleanChanged(data.toBool());
    else if(msg == "J_EVT_CURSOR_ACTIVITY")
        emit cursorActivity();
    else if(msg == "J_EVT_GOT_FOCUS")
        emit gotFocus();
    else if(msg == "J_EVT_CURRENT_LANGUAGE_CHANGED") {
        QVariantMap map = data.toMap();
        emit currentLanguageChanged(map.value("id").toString(),
                                    map.value("name").toString());
    }
}

void Editor::setFocus()
{
    m_webView->setFocus();
}

void Editor::setFileName(QString filename)
{
    m_fileName = filename;
}

QString Editor::fileName() const
{
    return m_fileName;
}

bool Editor::isClean()
{
    return sendMessageWithResult("C_FUN_IS_CLEAN", 0).toBool();
}

QList<QMap<QString, QString>> Editor::languages()
{
    QMap<QString, QVariant> languages =
            sendMessageWithResult("C_FUN_GET_LANGUAGES").toMap();

    QList<QMap<QString, QString>> out;

    QMap<QString, QVariant>::iterator lang;
    for (lang = languages.begin(); lang != languages.end(); ++lang) {
        QMap<QString, QVariant> mode = lang.value().toMap();

        QMap<QString, QString> newMode;
        newMode.insert("id", lang.key());
        newMode.insert("name", mode.value("name").toString());
        newMode.insert("mime", mode.value("mime").toString());
        newMode.insert("mode", mode.value("mode").toString());

        out.append(newMode);
    }

    return out;
}

void Editor::setLanguage(QString language)
{
    sendMessage("C_CMD_SET_LANGUAGE", language);
    setIndentationMode(language);
}

QString Editor::setLanguageFromFileName()
{
    QString lang = sendMessageWithResult("C_FUN_SET_LANGUAGE_FROM_FILENAME",
                                         fileName()).toString();

    setIndentationMode(lang);

    return lang;
}

void Editor::setIndentationMode(QString language)
{
    QSettings s;
    QString keyPrefix = "Languages/" + language + "/";

    if (s.value(keyPrefix + "useDefaultSettings", true).toBool())
        keyPrefix = "Languages/";

    setIndentationMode(!s.value(keyPrefix + "indentWithSpaces", false).toBool(),
                       s.value(keyPrefix + "tabSize", 4).toInt());
}

void Editor::setIndentationMode(bool useTabs, int size)
{
    QMap<QString, QVariant> data;
    data.insert("useTabs", useTabs);
    data.insert("size", size);
    sendMessage("C_CMD_SET_INDENTATION_MODE", data);
}

QString Editor::value()
{
    return sendMessageWithResult("C_FUN_GET_VALUE").toString();
}

bool Editor::fileOnDiskChanged() const
{
    return m_fileOnDiskChanged;
}

void Editor::setFileOnDiskChanged(bool fileOnDiskChanged)
{
    m_fileOnDiskChanged = fileOnDiskChanged;
}

QString Editor::jsStringEscape(QString str) {
    return str.replace("\\", "\\\\")
            .replace("'", "\\'")
            .replace("\"", "\\\"")
            .replace("\n", "\\n")
            .replace("\r", "\\r")
            .replace("\t", "\\t")
            .replace("\b", "\\b");
}

void Editor::sendMessage(QString msg, QVariant data)
{
    sendMessageWithResult(msg, data);
}

void Editor::sendMessage(QString msg)
{
    sendMessage(msg, 0);
}

QVariant Editor::sendMessageWithResult(QString msg, QVariant data)
{
    waitAsyncLoad();

    QString funCall = "UiDriver.messageReceived('" +
            jsStringEscape(msg) + "');";

    m_jsToCppProxy->setMsgData(data);

    return m_webView->page()->mainFrame()->evaluateJavaScript(funCall);
}

QVariant Editor::sendMessageWithResult(QString msg)
{
    return sendMessageWithResult(msg, 0);
}

void Editor::setZoomFactor(const qreal &factor)
{
    qreal normFact = factor;
    if (normFact > 14) normFact = 14;
    else if (normFact < 0.10) normFact = 0.10;

    m_webView->setZoomFactor(normFact);
}

qreal Editor::zoomFactor() const
{
    return m_webView->zoomFactor();
}
