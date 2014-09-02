#include "include/editor.h"
#include "include/notepadqq.h"
#include <QWebFrame>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>

Editor::Editor(QWidget *parent) :
    QWidget(parent)
{
    m_jsToCppProxy = new JsToCppProxy();
    connect(m_jsToCppProxy,
            &JsToCppProxy::messageReceived,
            this,
            &Editor::on_proxyMessageReceived);

    QEventLoop loop;
    connect(this, &Editor::editorReady, &loop, &QEventLoop::quit);

    m_webView = new QWebView();
    m_webView->setUrl(QUrl("file://" + Notepadqq::editorPath()));
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

    // TODO Display a message if a javascript error gets triggered.
    // Right now, if there's an error in the javascript code, we
    // get stuck waiting a J_EVT_READY that will never come.

    // Block until a J_EVT_READY message is received
    loop.exec();
}

Editor::~Editor()
{
    delete m_webView;
    delete m_jsToCppProxy;
}

void Editor::on_javaScriptWindowObjectCleared()
{
    m_webView->page()->mainFrame()->
            addToJavaScriptWindowObject("cpp_ui_driver", m_jsToCppProxy);
}

void Editor::on_proxyMessageReceived(QString msg, QVariant data)
{
    emit messageReceived(msg, data);

    if(msg == "J_EVT_READY")
        emit editorReady();
    else if(msg == "J_EVT_CONTENT_CHANGED")
        emit contentChanged();
    else if(msg == "J_EVT_CLEAN_CHANGED")
        emit cleanChanged(data.toBool());
    else if(msg == "J_EVT_CURSOR_ACTIVITY")
        emit cursorActivity();
    else if(msg == "J_EVT_GOT_FOCUS")
        emit gotFocus();
}

void Editor::setFocus()
{
    m_webView->setFocus();
}

void Editor::setFileName(QString filename)
{
    m_fileName = filename;
}

QString Editor::fileName()
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
}

void Editor::setLanguageFromFileName()
{
    sendMessage("C_FUN_SET_LANGUAGE_FROM_FILENAME", fileName());
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
    QString funCall = "UiDriver.messageReceived('" +
            jsStringEscape(msg) + "');";

    m_jsToCppProxy->setMsgData(data);

    return m_webView->page()->mainFrame()->evaluateJavaScript(funCall);
}

QVariant Editor::sendMessageWithResult(QString msg)
{
    return sendMessageWithResult(msg, 0);
}
