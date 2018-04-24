#include "include/EditorNS/editor.h"
#include "include/notepadqq.h"
#include "include/nqqsettings.h"
#include <QWebFrame>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QEventLoop>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QRegExp>

namespace EditorNS
{

    QQueue<Editor*> Editor::m_editorBuffer = QQueue<Editor*>();

    Editor::Editor(QWidget *parent) :
        QWidget(parent)
    {

        QString themeName = NqqSettings::getInstance().Appearance.getColorScheme();
        if (themeName == "")
            themeName = "default";

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

        m_webView->setUrl(url);

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

        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->addWidget(m_webView, 1);
        setLayout(m_layout);

        connect(m_webView->page()->mainFrame(),
                &QWebFrame::javaScriptWindowObjectCleared,
                this,
                &Editor::on_javaScriptWindowObjectCleared);

        connect(m_webView, &CustomQWebView::mouseWheel, this, &Editor::mouseWheel);
        connect(m_webView, &CustomQWebView::urlsDropped, this, &Editor::urlsDropped);
        connect(m_webView, &CustomQWebView::gotFocus, this, &Editor::gotFocus);

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

    void Editor::on_javaScriptWindowObjectCleared()
    {
        m_webView->page()->mainFrame()->
                addToJavaScriptWindowObject("cpp_ui_driver", m_jsToCppProxy);
    }

    void Editor::on_proxyMessageReceived(QString msg, QVariant data)
    {
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
                    auto cb = it->callback;
                    it->value->set_value(data);
                    this->asyncReplies.erase(it);

                    if (cb != 0) {
                        cb(data);
                    }
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
        else if(msg == "J_EVT_CURSOR_ACTIVITY")
            emit cursorActivity();
    }

    void Editor::setFocus()
    {
        m_webView->setFocus();
        sendMessage("C_CMD_SET_FOCUS");
    }

    void Editor::clearFocus()
    {
        m_webView->clearFocus();
        sendMessage("C_CMD_BLUR");
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

    bool Editor::isClean()
    {
        return asyncSendMessageWithResult("C_FUN_IS_CLEAN", QVariant(0)).get().toBool();
    }

    void Editor::markClean()
    {
        sendMessage("C_CMD_MARK_CLEAN");
    }

    void Editor::markDirty()
    {
        sendMessage("C_CMD_MARK_DIRTY");
    }

    int EditorNS::Editor::getHistoryGeneration()
    {
        return asyncSendMessageWithResult("C_FUN_GET_HISTORY_GENERATION", QVariant(0))
                .get().toInt();
    }

	void Editor::setLanguage(const Language& language)
	{
        if (!m_customIndentationMode) {
            setIndentationMode(language.id);
		}
		m_language = language;
		sendMessage("C_CMD_SET_LANGUAGE", m_language.mime.isEmpty() ? m_language.mode : m_language.mime);
		emit currentLanguageChanged(m_language.id, m_language.name);
	}

    void Editor::setLanguage(const QString& language)
    {
		auto& cache = LanguageCache::getInstance();
		auto index = cache.lookupById(language);
		if (index == -1)
			return;
		setLanguage(cache[index]);
		emit currentLanguageChanged(m_language.id, m_language.name);
    }

    QString Editor::setLanguageFromFileName(QString fileName)
    {
		auto& cache = LanguageCache::getInstance();
		auto test = cache.lookupByFileName(fileName);
		auto index = (test != -1) ? test : cache.lookupByExtension(fileName);
		if (index != -1) {
			setLanguage(cache[index]);
			return cache[index].id;
		}
        return "plaintext";
    }

    void Editor::detectLanguageFromContent(QString rawTxt)
    {
        auto& cache = LanguageCache::getInstance();
        QTextStream stream(&rawTxt);
        stream.skipWhiteSpace();
        QString test = stream.readLine();
        for (auto& l : cache.languages()) {
            if (!l.firstNonBlankLine.isEmpty()) {
                for (auto& t : l.firstNonBlankLine) {
                    if (test.contains(QRegularExpression(t))) {
                        setLanguage(l.id);
                        return;
                    }
                }
            }
        }
    }

    QString Editor::setLanguageFromFileName()
    {
        return setLanguageFromFileName(filePath().toString());
    }

    void Editor::setIndentationMode(QString language)
    {
        NqqSettings& s = NqqSettings::getInstance();

        if (s.Languages.getUseDefaultSettings(language))
            language = "default";

        setIndentationMode(!s.Languages.getIndentWithSpaces(language),
                            s.Languages.getTabSize(language));
    }

    void Editor::setIndentationMode(const bool useTabs, const int size)
    {
        QMap<QString, QVariant> data;
        data.insert("useTabs", useTabs);
        data.insert("size", size);
        sendMessage("C_CMD_SET_INDENTATION_MODE", data);
    }

    Editor::IndentationMode Editor::indentationMode()
    {
        QVariantMap indent = asyncSendMessageWithResult("C_FUN_GET_INDENTATION_MODE").get().toMap();
        IndentationMode out;
        out.useTabs = indent.value("useTabs", true).toBool();
        out.size = indent.value("size", 4).toInt();
        return out;
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
        setIndentationMode(getLanguageId());
    }

    bool Editor::isUsingCustomIndentationMode() const
    {
        return m_customIndentationMode;
    }

    void Editor::setSmartIndent(bool enabled)
    {
        sendMessage("C_CMD_SET_SMART_INDENT", enabled);
    }

    void Editor::setValue(const QString &value)
    {
		detectLanguageFromContent(value);
        sendMessage("C_CMD_SET_VALUE", value);
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
        waitAsyncLoad();

        QString funCall = "UiDriver.messageReceived('" +
                jsStringEscape(msg) + "');";

        m_jsToCppProxy->setMsgData(data);

        m_webView->page()->mainFrame()->evaluateJavaScript(funCall);
    }

    void Editor::sendMessage(const QString &msg)
    {
        sendMessage(msg, 0);
    }

    std::shared_future<QVariant> Editor::asyncSendMessageWithResult(const QString &msg, const QVariant &data, std::function<void(QVariant)> callback)
    {
        static unsigned int msgid = 0;
        msgid++;

        std::shared_ptr<std::promise<QVariant>> resultPromise = std::make_shared<std::promise<QVariant>>();

        AsyncReply asyncmsg;
        asyncmsg.id = msgid;
        asyncmsg.value = resultPromise;
        asyncmsg.callback = callback;
        this->asyncReplies.push_back((asyncmsg));

        QString message_id = "[ASYNC_REQUEST]" + msg + "[ID=" + QString::number(msgid) + "]";

        this->sendMessage(message_id, data);

        return resultPromise->get_future();
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

    void Editor::setSelectionsText(const QStringList &texts, selectMode mode)
    {
        QString modeStr = "";
        if (mode == selectMode_cursorAfter)
            modeStr = "after";
        else if (mode == selectMode_cursorBefore)
            modeStr = "before";
        else
            modeStr = "selected";

        QVariantMap data;
        data.insert("text", texts);
        data.insert("select", modeStr);

        sendMessage("C_CMD_SET_SELECTIONS_TEXT", data);
    }

    void Editor::setSelectionsText(const QStringList &texts)
    {
        setSelectionsText(texts, selectMode_cursorAfter);
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
        QList<QWidget *> list = findChildren<QWidget *>(objectName);
        for (int i = 0; i < list.length(); i++) {
            removeBanner(list[i]);
        }
    }

    void Editor::setLineWrap(const bool wrap)
    {
        sendMessage("C_CMD_SET_LINE_WRAP", wrap);
    }

    void Editor::setEOLVisible(const bool showeol)
    {
        sendMessage("C_CMD_SHOW_END_OF_LINE",showeol);
    }

    void Editor::setWhitespaceVisible(const bool showspace)
    {
        sendMessage("C_CMD_SHOW_WHITESPACE",showspace);
    }

    void Editor::setMathEnabled(const bool enabled)
    {
        sendMessage("C_CMD_ENABLE_MATH", enabled);
    }

    QPair<int, int> Editor::cursorPosition()
    {
        QList<QVariant> cursor = asyncSendMessageWithResult("C_FUN_GET_CURSOR").get().toList();
        return QPair<int, int>(cursor[0].toInt(), cursor[1].toInt());
    }

    void Editor::setCursorPosition(const int line, const int column)
    {
        QList<QVariant> arg = QList<QVariant>({line, column});
        sendMessage("C_CMD_SET_CURSOR", QVariant(arg));
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
        QList<QVariant> arg = QList<QVariant>({fromLine, fromCol, toLine, toCol});
        sendMessage("C_CMD_SET_SELECTION", QVariant(arg));
    }

    QPair<int, int> Editor::scrollPosition()
    {
        QList<QVariant> scroll = asyncSendMessageWithResult("C_FUN_GET_SCROLL_POS").get().toList();
        return QPair<int, int>(scroll[0].toInt(), scroll[1].toInt());
    }

    void Editor::setScrollPosition(const int left, const int top)
    {
        QList<QVariant> arg = QList<QVariant>({left, top});
        sendMessage("C_CMD_SET_SCROLL_POS", QVariant(arg));
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
        sendMessage("C_CMD_SET_FONT", tmap);
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
        Theme defaultTheme;
        defaultTheme.name = "default";
        defaultTheme.path = "";

        if (name == "default" || name == "")
            return defaultTheme;

        QFileInfo editorPath = QFileInfo(Notepadqq::editorPath());
        QDir bundledThemesDir = QDir(editorPath.absolutePath() + "/libs/codemirror/theme/");

        Theme t;
        QString themeFile = bundledThemesDir.filePath(name + ".css");
        if (QFile(themeFile).exists()) {
            t.name = name;
            t.path = themeFile;
        } else {
            t = defaultTheme;
        }

        return t;
    }

    QList<Editor::Theme> Editor::themes()
    {
        QFileInfo editorPath = QFileInfo(Notepadqq::editorPath());
        QDir bundledThemesDir = QDir(editorPath.absolutePath() + "/libs/codemirror/theme/");

        QStringList filters;
        filters << "*.css";
        bundledThemesDir.setNameFilters(filters);

        QStringList themeFiles = bundledThemesDir.entryList();

        QList<Theme> out;
        for (QString themeStr : themeFiles) {
            QFileInfo theme = QFileInfo(themeStr);
            QString nameWithoutExt = theme.fileName()
                    .replace(QRegularExpression("\\.css$"), "");

            Theme t;
            t.name = nameWithoutExt;
            t.path = bundledThemesDir.filePath(themeStr);
            out.append(t);
        }

        return out;
    }

    void Editor::setTheme(Theme theme)
    {
        QMap<QString, QVariant> tmap;
        tmap.insert("name", theme.name == "" ? "default" : theme.name);
        tmap.insert("path", theme.path);
        sendMessage("C_CMD_SET_THEME", tmap);
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

    QStringList Editor::selectedTexts()
    {
        QVariant text = asyncSendMessageWithResult("C_FUN_GET_SELECTIONS_TEXT").get();
        return text.toStringList();
    }

    void Editor::setOverwrite(bool overwrite)
    {
        sendMessage("C_CMD_SET_OVERWRITE", overwrite);
    }

    void Editor::forceRender(QSize size)
    {
        QWebPage *page = m_webView->page();

        page->setViewportSize(size);

        QImage image(size.width(), size.height(), QImage::Format_Mono);
        QPainter painter(&image);

        page->mainFrame()->render(&painter);
    }

    void Editor::setTabsVisible(bool visible)
    {
        sendMessage("C_CMD_SET_TABS_VISIBLE", visible);
    }

    Editor::IndentationMode Editor::detectDocumentIndentation(bool *found)
    {
        QVariantMap indent =
                asyncSendMessageWithResult("C_FUN_DETECT_INDENTATION_MODE").get().toMap();

        IndentationMode out;

        bool _found = indent.value("found", false).toBool();
        if (found != nullptr) {
            *found = _found;
        }

        if (_found) {
            out.useTabs = indent.value("useTabs", true).toBool();
            out.size = indent.value("size", 4).toInt();
        }

        return out;
    }

    void Editor::print(QPrinter *printer)
    {
        // 1. Set theme to default because dark themes would force the printer to color the entire
        //    document in the background color. Default theme has white background.
        // 2. Set WebView's bg-color to white to prevent visual artifacts when printing less than one page.
        // 3. Set C_CMD_DISPLAY_PRINT_STYLE to hide UI elements like the gutter.

        setTheme(themeFromName("Default"));
        m_webView->setStyleSheet("background-color: white");
        sendMessage("C_CMD_DISPLAY_PRINT_STYLE");
        m_webView->print(printer);
        sendMessage("C_CMD_DISPLAY_NORMAL_STYLE");
        m_webView->setStyleSheet("");
        setTheme(themeFromName(NqqSettings::getInstance().Appearance.getColorScheme()));
    }

    QString Editor::getCurrentWord()
    {
        return asyncSendMessageWithResult("C_FUN_GET_CURRENT_WORD").get().toString();
    }

    int Editor::lineCount()
    {
        return asyncSendMessageWithResult("C_FUN_GET_LINE_COUNT").get().toInt();
    }
}
