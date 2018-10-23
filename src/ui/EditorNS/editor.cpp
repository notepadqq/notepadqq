#include "include/EditorNS/editor.h"

#include "include/notepadqq.h"
#include "include/nqqsettings.h"

#include <QDir>
#include <QEventLoop>
#include <QMessageBox>
#include <QRegExp>
#include <QRegularExpression>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEngineSettings>

namespace EditorNS
{

    QQueue<Editor*> Editor::m_editorBuffer = QQueue<Editor*>();

    Editor::Editor(QWidget *parent) :
        QWidget(parent)
    {

        QString themeName = NqqSettings::getInstance().Appearance.getColorScheme();

        fullConstructor(themeFromName(themeName));
    }

    Editor::Editor(const Theme &theme, QWidget *parent) :
        QWidget(parent)
    {
        fullConstructor(theme);
    }

    void Editor::fullConstructor(const Theme &theme)
    {
        m_jsToCppProxy = new JsToCppProxy(this);
        connect(m_jsToCppProxy,
                &JsToCppProxy::messageReceived,
                this,
                &Editor::on_proxyMessageReceived);

        m_webView = new CustomQWebView(this);

        QUrlQuery query;
        query.addQueryItem("themePath", theme.path);
        query.addQueryItem("themeName", theme.name);

        QUrl url = QUrl("file://" + Notepadqq::editorPath());
        url.setQuery(query);

        QWebChannel * channel = new QWebChannel(this);
        m_webView->page()->setWebChannel(channel);
        channel->registerObject(QStringLiteral("cpp_ui_driver"), m_jsToCppProxy);

        m_webView->page()->setBackgroundColor(qApp->palette().color(QPalette::Background));
        m_webView->setUrl(url);

        // To load the page in the background (http://stackoverflow.com/a/10520029):
        // (however, no noticeable improvement here on an i5, september 2014)
        //QString content = QString("<html><body onload='setTimeout(function() { window.location=\"%1\"; }, 1);'>Loading...</body></html>").arg("file://" + Notepadqq::editorPath());
        //m_webView->setContent(content.toUtf8());

        m_webView->pageAction(QWebEnginePage::InspectElement)->setVisible(false);

        //m_webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

        QWebEngineSettings *pageSettings = m_webView->page()->settings();
        #ifdef QT_DEBUG
        //pageSettings->setAttribute(QWebEngineSettings::DeveloperExtrasEnabled, true);
        #endif
        pageSettings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);

        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->addWidget(m_webView, 1);
        setLayout(m_layout);

        connect(m_webView, &CustomQWebView::mouseWheel, this, &Editor::mouseWheel);
        connect(m_webView, &CustomQWebView::urlsDropped, this, &Editor::urlsDropped);
        connect(m_webView, &CustomQWebView::gotFocus, this, &Editor::gotFocus);
        setLanguage(nullptr);
        // TODO Display a message if a javascript error gets triggered.
        // Right now, if there's an error in the javascript code, we
        // get stuck waiting a J_EVT_READY that will never come.
    }

    QSharedPointer<Editor> Editor::getNewEditor(QWidget *parent)
    {
        return QSharedPointer<Editor>(getNewEditorUnmanagedPtr(parent), &Editor::deleteLater);
    }

    Editor *Editor::getNewEditorUnmanagedPtr(QWidget *parent)
    {
        Editor *out;

        if (m_editorBuffer.length() == 0) {
            m_editorBuffer.enqueue(new Editor());
            out = new Editor();
        } else if (m_editorBuffer.length() == 1) {
            m_editorBuffer.enqueue(new Editor());
            out = m_editorBuffer.dequeue();
        } else {
            out = m_editorBuffer.dequeue();
        }

        out->setParent(parent);
        return out;
    }

    void Editor::addEditorToBuffer(const int howMany)
    {
        for (int i = 0; i < howMany; i++)
            m_editorBuffer.enqueue(new Editor());
    }

    void Editor::invalidateEditorBuffer()
    {
        m_editorBuffer.clear();
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

    void Editor::on_proxyMessageReceived(QString msg, QVariant data)
    {
        QTimer::singleShot(0, [msg,data,this]{

            emit messageReceived(msg, data);

            if (msg.startsWith("[ASYNC_REPLY]")) {
                QRegExp rgx("\\[ID=(\\d+)\\]$");

                if(rgx.indexIn(msg) == -1)
                    return;

                if (rgx.captureCount() != 1)
                    return;

                unsigned int id = rgx.capturedTexts()[1].toInt();

                // Look into the list of callbacks
                for (auto it = this->asyncReplies.begin(); it != this->asyncReplies.end(); ++it) {
                    if (it->id == id) {
                        AsyncReply r = *it;
                        if (r.value) {
                            r.value->set_value(data);
                        }
                        this->asyncReplies.erase(it);

                        if (r.callback != 0) {
                            QTimer::singleShot(0, [r,data]{ r.callback(data); });
                        }

                        emit asyncReplyReceived(r.id, r.message, data);

                        break;
                    }
                }


            } else if(msg == "J_EVT_READY") {
                m_loaded = true;
                emit editorReady();
            } else if(msg == "J_EVT_CONTENT_CHANGED")
                emit contentChanged();
            else if(msg == "J_EVT_CLEAN_CHANGED")
                emit cleanChanged(data.toBool());
            else if (msg == "J_EVT_CURSOR_ACTIVITY") {
                emit cursorActivity(data.toMap());
            } else if (msg == "J_EVT_DOCUMENT_INFO") {
                emit documentInfoRequested(data.toMap());
            }
        });
    }

    void Editor::setFocus()
    {
        m_webView->setFocus();
    }

    void Editor::clearFocus()
    {
        m_webView->clearFocus();
    }

    /**
     * Automatically converts local relative file names to absolute ones.
     */
    void Editor::setFilePath(const QUrl &filename)
    {
        QUrl old = m_filePath;
        QUrl newUrl = filename;

        if (newUrl.isLocalFile())
            newUrl = QUrl::fromLocalFile(QFileInfo(filename.toLocalFile()).absoluteFilePath());

        m_filePath = newUrl;
        emit fileNameChanged(old, newUrl);
    }

    /**
     * Always returns an absolute url.
     */
    QUrl Editor::filePath() const
    {
        return m_filePath;
    }

    QString Editor::tabName() const
    {
        return m_tabName;
    }

    void Editor::setTabName(const QString& name)
    {
        m_tabName = name;
    }

    QPromise<bool> Editor::isCleanP()
    {
        return asyncSendMessageWithResultP("C_FUN_IS_CLEAN", QVariant(0))
                .then([](QVariant v){ return v.toBool(); });
    }

    bool Editor::isClean()
    {
        QVariant data(0); // avoid crash on Mac OS X, see issue #702
        return asyncSendMessageWithResult("C_FUN_IS_CLEAN", data).get().toBool();
    }

    QPromise<void> Editor::markClean()
    {
        return asyncSendMessageWithResultP("C_CMD_MARK_CLEAN").then([](){})
                .wait(); // FIXME Remove
    }

    QPromise<void> Editor::markDirty()
    {
        return asyncSendMessageWithResultP("C_CMD_MARK_DIRTY").then([](){})
                .wait(); // FIXME Remove
    }

    QPromise<int> Editor::getHistoryGeneration()
    {
        return asyncSendMessageWithResultP("C_FUN_GET_HISTORY_GENERATION")
                .then([](QVariant v){return v.toInt();});
    }

    void Editor::setLanguage(const Language* lang)
    {
        if (lang == nullptr) {
            lang = LanguageService::getInstance().lookupById("plaintext");
        }
        if (m_currentLanguage == lang) {
            return;
        }
        if (!m_customIndentationMode) {
            setIndentationMode(lang);
        }
        m_currentLanguage = lang;
        asyncSendMessageWithResultP("C_CMD_SET_LANGUAGE", lang->mime.isEmpty() ? lang->mode : lang->mime).then([=](){
            emit currentLanguageChanged(m_currentLanguage->id, m_currentLanguage->name);
        });
    }

    void Editor::setLanguage(const QString& language)
    {
        auto& cache = LanguageService::getInstance();
        auto lang = cache.lookupById(language);
        if (lang != nullptr) {
            setLanguage(lang);
        }
    }

    void Editor::setLanguageFromFilePath(const QString& filePath)
    {
        auto name = QFileInfo(filePath).fileName();

        auto& cache = LanguageService::getInstance();
        auto lang = cache.lookupByFileName(name);
        if (lang != nullptr) {
            setLanguage(lang);
            return;
        }
        lang = cache.lookupByExtension(name);
        if (lang != nullptr) {
            setLanguage(lang);
        }
    }

    void Editor::setLanguageFromFilePath()
    {
        setLanguageFromFilePath(filePath().toString());
    }

    QPromise<void> Editor::setIndentationMode(const Language* lang)
    {
        const auto& s = NqqSettings::getInstance().Languages;
        const bool useDefaults = s.getUseDefaultSettings(lang->id);
        const auto& langId = useDefaults ? "default" : lang->id;

        return setIndentationMode(!s.getIndentWithSpaces(langId), s.getTabSize(langId));
    }

    QPromise<void> Editor::setIndentationMode(const bool useTabs, const int size)
    {
        return asyncSendMessageWithResultP("C_CMD_SET_INDENTATION_MODE",
                                           QVariantMap{{"useTabs", useTabs}, {"size", size}}).then([](){});
    }

    Editor::IndentationMode Editor::indentationMode()
    {
        QVariantMap indent = asyncSendMessageWithResult("C_FUN_GET_INDENTATION_MODE").get().toMap();
        IndentationMode out;
        out.useTabs = indent.value("useTabs", true).toBool();
        out.size = indent.value("size", 4).toInt();
        return out;
    }

    QPromise<Editor::IndentationMode> Editor::indentationModeP()
    {
        return asyncSendMessageWithResultP("C_FUN_GET_INDENTATION_MODE").then([](QVariant result){
            QVariantMap indent = result.toMap();
            IndentationMode out;
            out.useTabs = indent.value("useTabs", true).toBool();
            out.size = indent.value("size", 4).toInt();
            return out;
        });
    }

    void Editor::setCustomIndentationMode(const bool useTabs, const int size)
    {
        m_customIndentationMode = true;
        setIndentationMode(useTabs, size);
    }

    void Editor::setCustomIndentationMode(const bool useTabs)
    {
        m_customIndentationMode = true;
        setIndentationMode(useTabs, 0);
    }

    void Editor::clearCustomIndentationMode()
    {
        m_customIndentationMode = false;
        setIndentationMode(getLanguage());
    }

    bool Editor::isUsingCustomIndentationMode() const
    {
        return m_customIndentationMode;
    }

    void Editor::setSmartIndent(bool enabled)
    {
        asyncSendMessageWithResultP("C_CMD_SET_SMART_INDENT", enabled);
    }

    QPromise<void> Editor::setValue(const QString &value)
    {
        auto lang = LanguageService::getInstance().lookupByContent(value);
        if (lang != nullptr) {
            setLanguage(lang);
        }
        return asyncSendMessageWithResultP("C_CMD_SET_VALUE", value).then([](){})
                .wait(); // FIXME Remove
    }

    QString Editor::value()
    {
        return asyncSendMessageWithResult("C_FUN_GET_VALUE").get().toString();
    }

    bool Editor::fileOnDiskChanged() const
    {
        return m_fileOnDiskChanged;
    }

    void Editor::setFileOnDiskChanged(bool fileOnDiskChanged)
    {
        m_fileOnDiskChanged = fileOnDiskChanged;
    }

    QString Editor::jsStringEscape(QString str) const {
        return str.replace("\\", "\\\\")
                .replace("'", "\\'")
                .replace("\"", "\\\"")
                .replace("\n", "\\n")
                .replace("\r", "\\r")
                .replace("\t", "\\t")
                .replace("\b", "\\b");
    }

    void Editor::sendMessage(const QString &msg, const QVariant &data)
    {
#ifdef QT_DEBUG
        qDebug() << "Legacy message " << msg << " sent.";
#endif
        waitAsyncLoad();

        emit m_jsToCppProxy->messageReceivedByJs(msg, data);
    }

    void Editor::sendMessage(const QString &msg)
    {
        sendMessage(msg, 0);
    }

    unsigned int messageIdentifier = 0;

    QPromise<QVariant> Editor::asyncSendMessageWithResultP(const QString &msg, const QVariant &data)
    {
        unsigned int currentMsgIdentifier = ++messageIdentifier;

        QPromise<QVariant> resultPromise = QPromise<QVariant>([&](
                                                              const QPromiseResolve<QVariant>& resolve,
                                                              const QPromiseReject<QVariant>& /* reject */) {

            auto conn = std::make_shared<QMetaObject::Connection>();
            *conn = QObject::connect(this, &Editor::asyncReplyReceived, this, [=](unsigned int id, QString, QVariant data){
                if (id == currentMsgIdentifier) {
                    QObject::disconnect(*conn);
                    resolve(data);
                }
            });

        });

        // FIXME We can probably remove this->asyncReplies after we've converted everything
        AsyncReply asyncmsg;
        asyncmsg.id = currentMsgIdentifier;
        asyncmsg.message = msg;
        asyncmsg.value = nullptr;
        asyncmsg.callback = nullptr;
        this->asyncReplies.push_back((asyncmsg));

        QString message_id = "[ASYNC_REQUEST]" + msg + "[ID=" + QString::number(currentMsgIdentifier) + "]";

        if (m_loaded) {
            // Send it right now
            emit m_jsToCppProxy->messageReceivedByJs(message_id, data);
        } else {
            // Send it as soon as the editor becomes ready
            auto conn = std::make_shared<QMetaObject::Connection>();
            *conn = QObject::connect(this, &Editor::editorReady, this, [=](){
                QObject::disconnect(*conn);
                m_loaded = true;
                emit m_jsToCppProxy->messageReceivedByJs(message_id, data);
            });
        }

        return resultPromise;
    }

    QPromise<QVariant> Editor::asyncSendMessageWithResultP(const QString &msg)
    {
        return this->asyncSendMessageWithResultP(msg, 0);
    }

    std::shared_future<QVariant> Editor::asyncSendMessageWithResult(const QString &msg, const QVariant &data, std::function<void(QVariant)> callback)
    {
        unsigned int currentMsgIdentifier = ++messageIdentifier;

        std::shared_ptr<std::promise<QVariant>> resultPromise = std::make_shared<std::promise<QVariant>>();

        AsyncReply asyncmsg;
        asyncmsg.id = currentMsgIdentifier;
        asyncmsg.message = msg;
        asyncmsg.value = resultPromise;
        asyncmsg.callback = callback;
        this->asyncReplies.push_back((asyncmsg));

        QString message_id = "[ASYNC_REQUEST]" + msg + "[ID=" + QString::number(currentMsgIdentifier) + "]";

        this->sendMessage(message_id, data);

        std::shared_future<QVariant> fut = resultPromise->get_future().share();

        while (fut.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        }

        return fut;
    }

    std::shared_future<QVariant> Editor::asyncSendMessageWithResult(const QString &msg, std::function<void(QVariant)> callback)
    {
        return this->asyncSendMessageWithResult(msg, 0, callback);
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

    void Editor::setSelectionsText(const QStringList &texts, SelectMode mode)
    {
        QVariantMap data {{"text", texts}};
        switch (mode) {
            case SelectMode::After:
                data.insert("select", "after"); break;
            case SelectMode::Before:
                data.insert("select", "before"); break;
            default:
                data.insert("select", "selected"); break;
        }
        sendMessage("C_CMD_SET_SELECTIONS_TEXT", data);
    }

    void Editor::setSelectionsText(const QStringList &texts)
    {
        setSelectionsText(texts, SelectMode::After);
    }

    void Editor::insertBanner(QWidget *banner)
    {
        m_layout->insertWidget(0, banner);
    }

    void Editor::removeBanner(QWidget *banner)
    {
        if (banner != m_webView && m_layout->indexOf(banner) >= 0) {
            m_layout->removeWidget(banner);
            emit bannerRemoved(banner);
        }
    }

    void Editor::removeBanner(QString objectName)
    {
        for (auto&& banner : findChildren<QWidget *>(objectName)) {
            removeBanner(banner);
        }
    }

    void Editor::setLineWrap(const bool wrap)
    {
        asyncSendMessageWithResultP("C_CMD_SET_LINE_WRAP", wrap);
    }

    void Editor::setEOLVisible(const bool showeol)
    {
        asyncSendMessageWithResultP("C_CMD_SHOW_END_OF_LINE", showeol);
    }

    void Editor::setWhitespaceVisible(const bool showspace)
    {
        asyncSendMessageWithResultP("C_CMD_SHOW_WHITESPACE", showspace);
    }

    void Editor::setMathEnabled(const bool enabled)
    {
        asyncSendMessageWithResultP("C_CMD_ENABLE_MATH", enabled);
    }

    QPromise<QPair<int, int>> Editor::cursorPositionP()
    {
        return asyncSendMessageWithResultP("C_FUN_GET_CURSOR")
               .then([](QVariant v){
             QList<QVariant> cursor = v.toList();
             return QPair<int, int>(cursor[0].toInt(), cursor[1].toInt());
        });

    }

    void Editor::requestDocumentInfo()
    {
        asyncSendMessageWithResultP("C_CMD_GET_DOCUMENT_INFO");
    }

    QPair<int, int> Editor::cursorPosition()
    {
        QList<QVariant> cursor = asyncSendMessageWithResult("C_FUN_GET_CURSOR").get().toList();
        return {cursor[0].toInt(), cursor[1].toInt()};
    }

    void Editor::setCursorPosition(const int line, const int column)
    {
        asyncSendMessageWithResultP("C_CMD_SET_CURSOR", QList<QVariant>{line, column});
    }

    void Editor::setCursorPosition(const QPair<int, int> &position)
    {
        setCursorPosition(position.first, position.second);
    }

    void Editor::setCursorPosition(const Cursor &cursor)
    {
        setCursorPosition(cursor.line, cursor.column);
    }

    void Editor::setSelection(int fromLine, int fromCol, int toLine, int toCol)
    {
        QVariantList arg{fromLine, fromCol, toLine, toCol};
        asyncSendMessageWithResultP("C_CMD_SET_SELECTION", QVariant(arg));
    }

    QPair<int, int> Editor::scrollPosition()
    {
        QVariantList scroll = asyncSendMessageWithResult("C_FUN_GET_SCROLL_POS").get().toList();
        return {scroll[0].toInt(), scroll[1].toInt()};
    }

    void Editor::setScrollPosition(const int left, const int top)
    {
        asyncSendMessageWithResultP("C_CMD_SET_SCROLL_POS", QVariantList{left, top});
    }

    void Editor::setScrollPosition(const QPair<int, int> &position)
    {
        setScrollPosition(position.first, position.second);
    }

    QString Editor::endOfLineSequence() const
    {
        return m_endOfLineSequence;
    }

    void Editor::setEndOfLineSequence(const QString &newLineSequence)
    {
        m_endOfLineSequence = newLineSequence;
    }

    void Editor::setFont(QString fontFamily, int fontSize, double lineHeight)
    {
        QMap<QString, QVariant> tmap;
        tmap.insert("family", fontFamily == nullptr ? "" : fontFamily);
        tmap.insert("size", QString::number(fontSize));
        tmap.insert("lineHeight", QString::number(lineHeight,'f',2));
        asyncSendMessageWithResultP("C_CMD_SET_FONT", tmap);
    }

    QTextCodec *Editor::codec() const
    {
        return m_codec;
    }

    void Editor::setCodec(QTextCodec *codec)
    {
        m_codec = codec;
    }

    bool Editor::bom() const
    {
        return m_bom;
    }

    void Editor::setBom(bool bom)
    {
        m_bom = bom;
    }

    Editor::Theme Editor::themeFromName(QString name)
    {
        if (name == "default" || name.isEmpty())
            return Theme();

        QFileInfo editorPath(Notepadqq::editorPath());
        QDir bundledThemesDir(editorPath.absolutePath() + "/libs/codemirror/theme/");

        if (bundledThemesDir.exists(name + ".css"))
            return Theme(name, bundledThemesDir.filePath(name + ".css"));

        return Theme();
    }

    QList<Editor::Theme> Editor::themes()
    {
        auto editorPath = QFileInfo(Notepadqq::editorPath());
        QDir bundledThemesDir(editorPath.absolutePath() + "/libs/codemirror/theme/", "*.css");

        QList<Theme> out;
        for (auto&& theme : bundledThemesDir.entryInfoList()) {
            out.append(Theme(theme.completeBaseName(), theme.filePath()));
        }
        return out;
    }

    void Editor::setTheme(Theme theme)
    {
        sendMessage("C_CMD_SET_THEME", QVariantMap{{"name",theme.name},{"path",theme.path}});
    }

    QList<Editor::Selection> Editor::selections()
    {
        QList<Selection> out;

        QList<QVariant> sels = asyncSendMessageWithResult("C_FUN_GET_SELECTIONS").get().toList();
        for (int i = 0; i < sels.length(); i++) {
            QVariantMap selMap = sels[i].toMap();
            QVariantMap from = selMap.value("anchor").toMap();
            QVariantMap to = selMap.value("head").toMap();

            Selection sel;
            sel.from.line = from.value("line").toInt();
            sel.from.column = from.value("ch").toInt();
            sel.to.line = to.value("line").toInt();
            sel.to.column = to.value("ch").toInt();

            out.append(sel);
        }

        return out;
    }

    QPromise<QStringList> Editor::selectedTexts()
    {
        return asyncSendMessageWithResultP("C_FUN_GET_SELECTIONS_TEXT")
                .then([](QVariant text){ return text.toStringList(); });
    }

    void Editor::setOverwrite(bool overwrite)
    {
        asyncSendMessageWithResultP("C_CMD_SET_OVERWRITE", overwrite);
    }

    void Editor::setTabsVisible(bool visible)
    {
        asyncSendMessageWithResultP("C_CMD_SET_TABS_VISIBLE", visible);
    }

    QPromise<std::pair<Editor::IndentationMode, bool>> Editor::detectDocumentIndentation()
    {
        return asyncSendMessageWithResultP("C_FUN_DETECT_INDENTATION_MODE").then([](QVariant result){
            QVariantMap indent = result.toMap();
            IndentationMode out;

            bool found = indent.value("found", false).toBool();

            if (found) {
                out.useTabs = indent.value("useTabs", true).toBool();
                out.size = indent.value("size", 4).toInt();
            }

            return std::make_pair(out, found);
        });
    }

    void Editor::print(std::shared_ptr<QPrinter> printer)
    {
        // 1. Set theme to default because dark themes would force the printer to color the entire
        //    document in the background color. Default theme has white background.
        // 2. Set WebView's bg-color to white to prevent visual artifacts when printing less than one page.
        // 3. Set C_CMD_DISPLAY_PRINT_STYLE to hide UI elements like the gutter.

#if QT_VERSION >= QT_VERSION_CHECK(5,8,0)
        QColor prevBackgroundColor = m_webView->page()->backgroundColor();
        QString prevStylesheet = m_webView->styleSheet();

        this->setLineWrap(true);
        setTheme(themeFromName("default"));
        m_webView->page()->setBackgroundColor(Qt::transparent);
        m_webView->setStyleSheet("background-color: white");
        sendMessage("C_CMD_DISPLAY_PRINT_STYLE");
        m_webView->page()->print(printer.get(), [=](bool /*success*/) {
            // Note: it is important to capture "printer" in order to keep the shared_ptr alive.
            sendMessage("C_CMD_DISPLAY_NORMAL_STYLE");
            m_webView->setStyleSheet(prevStylesheet);
            m_webView->page()->setBackgroundColor(prevBackgroundColor);
            setTheme(themeFromName(NqqSettings::getInstance().Appearance.getColorScheme()));
            this->setLineWrap(NqqSettings::getInstance().General.getWordWrap());
        });
#endif
    }

    QPromise<QByteArray> Editor::printToPdf(const QPageLayout& pageLayout)
    {
        // 1. Set theme to default because dark themes would force the printer to color the entire
        //    document in the background color. Default theme has white background.
        // 2. Set WebView's bg-color to white to prevent visual artifacts when printing less than one page.
        // 3. Set C_CMD_DISPLAY_PRINT_STYLE to hide UI elements like the gutter.

        return QPromise<QByteArray>(
            [&](const QPromiseResolve<QByteArray>& resolve, const QPromiseReject<QByteArray>& reject) {

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
                QColor prevBackgroundColor = m_webView->page()->backgroundColor();
                QString prevStylesheet = m_webView->styleSheet();

                this->setLineWrap(true);
                setTheme(themeFromName("default"));
                m_webView->page()->setBackgroundColor(Qt::transparent);
                m_webView->setStyleSheet("background-color: white");
                asyncSendMessageWithResultP("C_CMD_DISPLAY_PRINT_STYLE").wait();

                m_webView->page()->printToPdf(
                    [=](const QByteArray& data) {
                        QTimer::singleShot(0, [=]() {
                            asyncSendMessageWithResultP("C_CMD_DISPLAY_NORMAL_STYLE").wait();
                            m_webView->setStyleSheet(prevStylesheet);
                            m_webView->page()->setBackgroundColor(prevBackgroundColor);
                            setTheme(themeFromName(NqqSettings::getInstance().Appearance.getColorScheme()));
                            this->setLineWrap(NqqSettings::getInstance().General.getWordWrap());
                        });

                        if (data.isEmpty() || data.isNull()) {
                            reject(QByteArray());
                        } else {
                            resolve(data);
                        }
                    },
                    pageLayout);

#else
                reject(QByteArray());
#endif
            });
    }

    QPromise<QString> Editor::getCurrentWord()
    {
        return asyncSendMessageWithResultP("C_FUN_GET_CURRENT_WORD")
                .then([](QVariant v){ return v.toString(); });
    }

    QPromise<int> Editor::lineCount()
    {
        return asyncSendMessageWithResultP("C_FUN_GET_LINE_COUNT")
                .then([](QVariant v){ return v.toInt(); });
    }
}
