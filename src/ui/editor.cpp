#include "include/editor.h"
#include "include/constants.h"
#include <QWebFrame>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>

Editor::Editor(QWidget *parent) :
    QWidget(parent), m_fileName("")
{
    this->jsToCppProxy = new JsToCppProxy();
    connect(this->jsToCppProxy,
            SIGNAL(messageReceived(QString,QVariant)),
            this,
            SLOT(on_proxyMessageReceived(QString,QVariant)));

    QEventLoop loop;
    connect(this, SIGNAL(editorReady()), &loop, SLOT(quit()));

    QString editorPath = ApplicationEditorPath();

    this->webView = new QWebView();
    this->webView->setUrl(QUrl("file://" + editorPath));
    this->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    QWebSettings *pageSettings = this->webView->page()->settings();
    #ifdef QT_DEBUG
    pageSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    #endif
    pageSettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(this->webView);
    this->setLayout(layout);

    connect(this->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(on_javaScriptWindowObjectCleared()));


    // Block until a J_EVT_READY message is received
    loop.exec();
}

Editor::~Editor()
{
    delete webView;
    delete jsToCppProxy;
}

void Editor::on_javaScriptWindowObjectCleared()
{
    this->webView->page()->mainFrame()->addToJavaScriptWindowObject("cpp_ui_driver", this->jsToCppProxy);
}

void Editor::on_proxyMessageReceived(QString msg, QVariant data)
{
    emit messageReceived(msg, data);

    if(msg == "J_EVT_READY") {
        emit editorReady();
    } else if(msg == "J_EVT_CONTENT_CHANGED")
        emit contentChanged();
    else if(msg == "J_EVT_CLEAN_CHANGED")
        emit cleanChanged(data.toBool());
    else if(msg == "J_EVT_CURSOR_ACTIVITY")
        emit cursorActivity();
}

void Editor::setFocus()
{
    this->webView->setFocus();
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
    return this->sendMessageWithResult("C_FUN_IS_CLEAN", 0).toBool();
}

QMap<QString, QList<QString> > Editor::languages()
{
    QMap<QString, QVariant> modes = this->
            sendMessageWithResult("C_FUN_GET_LANGUAGES").toMap();

    QMap<QString, QList<QString> > out = QMap<QString, QList<QString> >();
    foreach (QString key, modes.keys()) {
        QList<QVariant> raw_mimes = modes.value(key).toList();

        QList<QString> mimes = QList<QString>();
        for (int i = 0; i < raw_mimes.length(); i++)
            mimes.append(raw_mimes.at(i).toString());

        out.insert(key, mimes);
    }

    return out;
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
    this->sendMessageWithResult(msg, data);
}

void Editor::sendMessage(QString msg)
{
    this->sendMessage(msg, 0);
}

QVariant Editor::sendMessageWithResult(QString msg, QVariant data)
{
    QString funCall = "UiDriver.messageReceived('" +
            jsStringEscape(msg) + "');";

    this->jsToCppProxy->setMsgData(data);

    return this->webView->page()->mainFrame()->evaluateJavaScript(funCall);
}

QVariant Editor::sendMessageWithResult(QString msg)
{
    return this->sendMessageWithResult(msg, 0);
}
