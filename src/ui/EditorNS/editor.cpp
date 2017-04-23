#include <QDir>
#include <QMessageBox>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWebEngineSettings>

#include "include/EditorNS/editor.h"
#include "include/notepadqq.h"
#include "include/nqqsettings.h"

namespace EditorNS
{
    QQueue<Editor*> Editor::m_editorBuffer = QQueue<Editor*>();

    Editor::Editor(QWidget *parent) :
        QWidget(parent)
    {
        auto& s = NqqSettings::getInstance();
        /* Initialize some values here so we don't have issues*/
        auto& indents = m_info.content.indentMode;
        indents.useTabs = !s.Languages.getIndentWithSpaces("default");
        indents.size = s.Languages.getTabSize("default");
        indents.custom = false;

        initJsProxy();
        initWebView();
        initContextMenu();

        auto vbox = new QVBoxLayout(this);
        vbox->setContentsMargins(0, 0, 0, 0);
        vbox->setSpacing(0);
        vbox->addWidget(m_webView, 1);
        setLayout(vbox);

    }

    void Editor::initWebView()
    {
        // Get the currently active color scheme/theme
        auto themeName = NqqSettings::getInstance().Appearance.getColorScheme();
        if (themeName.isEmpty()) {
            themeName = "default";
        }
        const auto theme = themeFromName(themeName);

        m_webView = new CustomQWebView(this);
        //m_webView->page()->setBackgroundColor(Qt::transparent);
        connect(m_webView->page(),
                &QWebEnginePage::loadStarted,
                this,
                &Editor::on_javaScriptWindowObjectCleared);
        connect(m_webView, &CustomQWebView::mouseWheel, this, &Editor::mouseWheel);
        connect(m_webView, &CustomQWebView::urlsDropped, this, &Editor::urlsDropped);

        QUrlQuery query;
        query.addQueryItem("themePath", theme.path);
        query.addQueryItem("themeName", theme.name);

        QUrl url = QUrl("file://" + Notepadqq::editorPath());
        url.setQuery(query);
        m_webView->setEnabled(false);
        m_webView->page()->load(url);
        m_webView->setEnabled(true);
        QWebEngineSettings *pageSettings = m_webView->page()->settings();
        pageSettings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    }


    void Editor::initJsProxy()
    {
        m_jsProxy = new JsProxy(this);
        connect(m_jsProxy,
                &JsProxy::replyReady,
                &m_processLoop, 
                &QEventLoop::quit);
        connect(m_jsProxy,
                &JsProxy::editorEvent,
                this, 
                &Editor::on_proxyMessageReceived
                );
    }

    void Editor::initContextMenu()
    {
        m_webView->setContextMenuPolicy(
                Qt::ContextMenuPolicy::ActionsContextMenu);
        QWebEnginePage* page = m_webView->page();
        m_webView->addAction(page->action(QWebEnginePage::Cut));
        m_webView->addAction(page->action(QWebEnginePage::Copy));
        m_webView->addAction(page->action(QWebEnginePage::Paste));
        m_webView->addAction(page->action(QWebEnginePage::SelectAll));
        page->action(QWebEnginePage::Undo)->setEnabled(false);
        page->action(QWebEnginePage::Redo)->setEnabled(false);
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

    void Editor::updateBackground(const QString& colour)
    {
        QColor newBG = QColor(colour);
        QPalette pal = m_webView->palette();
        m_webView->page()->setBackgroundColor(newBG);
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
        m_webView->connectJavaScriptObject("cpp_ui_driver", m_jsProxy);
    }

    void Editor::on_proxyMessageReceived(QString msg, QVariant data)
    {
        if (msg == QLatin1String("J_EVT_READY")) {
            m_loaded = true;
            emit editorReady();
        } else if (msg == QLatin1String("J_EVT_CONTENT_CHANGED")) {
            emit contentChanged(buildContentChangedEventData(data));
        } else if (msg == QLatin1String("J_EVT_CLEAN_CHANGED")) {
            m_info.content.clean = data.toBool();
            emit cleanChanged(m_info.content.clean);
        } else if (msg == QLatin1String("J_EVT_CURSOR_ACTIVITY")) {
            emit cursorActivity(buildCursorEventData(data));
        } else if (msg == QLatin1String("J_EVT_GOT_FOCUS")) {
            emit gotFocus();
        } else if (msg == QLatin1String("J_EVT_DOCUMENT_LOADED")) {
            emit documentLoaded(m_alreadyLoaded, buildDocumentLoadedEventData(data));
            m_alreadyLoaded = true;
        } else if (msg == QLatin1String("J_EVT_SCROLL_CHANGED")) {
            QPair<int, int> scrollPos;
            scrollPos.first = data.toList().at(0).toInt();
            scrollPos.second = data.toList().at(1).toInt();
        } else if (msg == QLatin1String("J_EVT_OPTION_CHANGED")) {
            // This may need its own function later
            auto v = data.toMap();
            auto key = v.value("key").toString();
            if (key == QLatin1String("language")) {
                auto& cache = LanguageCache::getInstance();
                emit languageChanged(cache[cache.lookupById(v.value("value").toString())]);
            }else if (key == QLatin1String("theme")) {
                updateBackground(v.value("value").toString());
            }
        }
    }

    Editor::IndentationMode Editor::buildDocumentLoadedEventData(
            const QVariant& data,
            bool /*cache*/)
    {
        if (!data.canConvert<QVariantList>()) {
            return m_info.content.indentMode;
        }
        auto v = data.toList();
        IndentationMode indentMode;
        indentMode.useTabs = v.at(0).toBool();
        indentMode.size = v.at(1).toInt();
        return indentMode;
    }

    Editor::ContentInfo Editor::buildContentChangedEventData(
            const QVariant& data, 
            bool cache)
    {
        auto v = data.toMap();
        ContentInfo temp;
        temp.charCount = v["charCount"].toInt();
        temp.lineCount = v["lineCount"].toInt();
        if (cache) {
            m_info.content.charCount = temp.charCount;
            m_info.content.lineCount = temp.lineCount;
        }
        return temp;
    }

    Editor::CursorInfo Editor::buildCursorEventData(const QVariant& data, 
            bool cache)
    {
        auto v = data.toMap();
        CursorInfo temp = {
            .line = v["cursorLine"].toInt(),
            .column = v["cursorLine"].toInt(),
            .selectionCharCount = v["selectionCharCount"].toInt(),
            .selectionLineCount = v["selectionLineCount"].toInt(),
            .selections = QList<Selection>()
        };
        for (auto selection : v["selections"].toList()) {
            QVariantMap anchor = selection.toMap().value("anchor").toMap();
            QVariantMap head = selection.toMap().value("head").toMap();
            Cursor from = { 
                anchor["line"].toInt(), 
                anchor["ch"].toInt() 
            };
            Cursor to = { 
                head["line"].toInt(), 
                head["ch"].toInt() 
            };
            temp.selections.push_back({ from, to });
        }
        
        if (cache) {
            m_info.cursor = temp;
        }
        return temp;
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
    void Editor::setFileName(const QUrl &filename)
    {
        QUrl old = m_fileName;
        QUrl newUrl = filename;

        if (newUrl.isLocalFile())
            newUrl = QUrl::fromLocalFile(QFileInfo(filename.toLocalFile()).absoluteFilePath());

        m_fileName = newUrl;
        emit fileNameChanged(old, newUrl);
    }

    /**
     * Always returns an absolute url.
     */
    QUrl Editor::fileName() const
    {
        return m_fileName;
    }

    bool Editor::isClean()
    {
        return m_info.content.clean;
    }

    void Editor::markClean()
    {
        sendMessage("C_CMD_MARK_CLEAN");
    }

    void Editor::markDirty()
    {
        sendMessage("C_CMD_MARK_DIRTY");
    }

    const Language& Editor::getLanguage()
    {
        auto& cache = LanguageCache::getInstance();
        return cache[m_info.language];
    }

    void Editor::setLanguage(const Language& language)
    {
        auto& cache = LanguageCache::getInstance();
        if (!m_info.content.indentMode.custom) {
            setIndentationMode(language.id);
        }
        auto& cLang = cache[m_info.language];
        QVariantMap data {
            {"id", cLang.id},
            {"mode", cLang.mime.isEmpty() ? cLang.mode : cLang.mime}
        };
        sendMessage("C_CMD_SET_LANGUAGE", data);

    }

    void Editor::setLanguage(const QString &language)
    {
        auto& cache = LanguageCache::getInstance();
        m_info.language = cache.lookupById(language);
        setLanguage(cache[m_info.language]);
    }


    void Editor::setLanguageFromFileName(const QString& fileName)
    {
        auto& cache = LanguageCache::getInstance();
        auto success = cache.lookupByFileName(fileName);
        if (success != -1) {
            m_info.language = success;
            setLanguage(cache[m_info.language]);
            return;
        }
        success = cache.lookupByExtension(fileName);
        if (success != -1) {
            m_info.language = success;
            setLanguage(cache[m_info.language]);
        }
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

    void Editor::setLanguageFromFileName()
    {
        setLanguageFromFileName(fileName().toString());
    }

    void Editor::setIndentationMode(QString language)
    {
        auto& ls = NqqSettings::getInstance().Languages;

        if (ls.getUseDefaultSettings(language))
            language = "default";

        setIndentationMode(!ls.getIndentWithSpaces(language),
                            ls.getTabSize(language),
                            false);
    }

    void Editor::setIndentationMode(const bool useTabs, const int size, 
            const bool custom)
    {
        m_info.content.indentMode.useTabs = useTabs;
        m_info.content.indentMode.size = size;
        m_info.content.indentMode.custom = custom;
        QVariantMap data {
            {"useTabs", useTabs}, 
            {"size", size}
        };
        sendMessage("C_CMD_SET_INDENTATION_MODE", data);
    }

    Editor::IndentationMode Editor::indentationMode()
    {
        return m_info.content.indentMode;
    }

    void Editor::setCustomIndentationMode(const bool useTabs, const int size)
    {
        setIndentationMode(useTabs, size, true);
    }

    void Editor::setCustomIndentationMode(const bool useTabs)
    {
        setIndentationMode(useTabs, 0, true);
    }

    void Editor::clearCustomIndentationMode()
    {
        m_info.content.indentMode.custom = false;
        setIndentationMode(getLanguage().id);
    }

    bool Editor::isUsingCustomIndentationMode() const
    {
        return m_info.content.indentMode.custom;
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
        m_jsProxy->sendMsg(jsStringEscape(msg), data);
    }

    QVariant Editor::sendMessageWithResult(const QString &msg, const QVariant &data)
    {
        if (m_processLoop.isRunning())
            throw std::runtime_error("m_processLoop must never be running at this point. Did this function get called from another thread?");

        emit m_jsProxy->sendMsg(jsStringEscape(msg), data);
        m_processLoop.exec();
        return m_jsProxy->getResult();
    }

    void Editor::setZoomFactor(const qreal &factor)
    {
        auto normFact = factor;
        if (normFact > 14) normFact = 14;
        else if (normFact < 0.10) normFact = 0.10;

        m_webView->setZoomFactor(normFact);
    }

    qreal Editor::zoomFactor() const
    {
        return m_webView->zoomFactor();
    }

    int Editor::getLineCount()
    {
        return m_info.content.lineCount;
    }

    int Editor::getCharCount()
    {
        return m_info.content.charCount;
    }

    void Editor::setSelectionsText(const QStringList &texts, SelectMode mode)
    {
        QVariantMap data {{"text", texts}};
        switch (mode) {
            case SelectMode::CursorAfter:
                data.insert("select", "after");
                break;
            case SelectMode::CursorBefore:
                data.insert("select", "before");
                break;
            default:
                data.insert("select", "selected");
                break;
        }
        sendMessage("C_CMD_SET_SELECTIONS_TEXT", data);
    }

    void Editor::setSelectionsText(const QStringList &texts)
    {
        setSelectionsText(texts, SelectMode::CursorAfter);
    }

    void Editor::insertBanner(QWidget *banner)
    {
        static_cast<QVBoxLayout*>(layout())->insertWidget(0, banner);
    }

    void Editor::removeBanner(QWidget *banner)
    {
        if (banner != m_webView && layout()->indexOf(banner) >= 0) {
            layout()->removeWidget(banner);
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

    void Editor::requestCursorInfo()
    {
        sendMessage("C_CMD_REQUEST_CURSOR_INFO");
    }

    void Editor::requestContentInfo()
    {
        sendMessage("C_CMD_REQUEST_DOCUMENT_INFO");
    }

    QPair<int, int> Editor::getCursorPosition()
    {
        return qMakePair(m_info.cursor.line, m_info.cursor.column);
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

    QPair<int, int> Editor::getScrollPosition()
    {
        return m_info.content.scrollPosition;
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

    const QString& Editor::endOfLineSequence() const
    {
        return m_info.content.newLine;
    }

    void Editor::setEndOfLineSequence(const QString &newLineSequence)
    {
        m_info.content.newLine = newLineSequence;
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
        Theme theme = { "default", "" };
        if (name == "default" || name.isEmpty())
            return theme;

        auto editorPath = QFileInfo(Notepadqq::editorPath());
        auto bundledThemesDir = QDir(editorPath.absolutePath() + "/libs/codemirror/theme/");
        auto themeFile = bundledThemesDir.filePath(name + ".css");
        if (QFile(themeFile).exists()) {
            theme.name = name;
            theme.path = themeFile;
        }
        return theme;
    }

    QList<Editor::Theme> Editor::themes()
    {
        auto editorPath = QFileInfo(Notepadqq::editorPath());
        auto bundledThemesDir = 
            QDir(editorPath.absolutePath() + "/libs/codemirror/theme/");
        auto filters = QStringList("*.css");
        bundledThemesDir.setNameFilters(filters);

        auto themeFiles = bundledThemesDir.entryList();

        QList<Theme> out;
        for (QString themeStr : themeFiles) {
            const auto theme = QFileInfo(themeStr);
            const auto nameWithoutExt = theme.fileName()
                    .replace(QRegularExpression("\\.css$"), "");

            out.append({ nameWithoutExt, bundledThemesDir.filePath(themeStr) });
        }

        return out;
    }

    void Editor::setTheme(Theme theme)
    {
        QVariantMap tmap;
        tmap.insert("name", theme.name == "" ? "default" : theme.name);
        tmap.insert("path", theme.path);
        sendMessage("C_CMD_SET_THEME", tmap);
    }

    void Editor::setOverwrite(bool overwrite)
    {
        sendMessage("C_CMD_SET_OVERWRITE", overwrite);
    }

    void Editor::setTabsVisible(bool visible)
    {
        sendMessage("C_CMD_SET_TABS_VISIBLE", visible);
    }

    void Editor::print(QPrinter *printer)
    {
        sendMessage("C_CMD_DISPLAY_PRINT_STYLE");
        //m_webView->print(printer); // FIXME
        sendMessage("C_CMD_DISPLAY_NORMAL_STYLE");
    }

    const QList<Editor::Selection>& Editor::getSelections() const
    {
        return m_info.cursor.selections;
    }
    
    void Editor::getSelectedTexts(std::function<void(const QStringList&)> callback)
    {
        sendMessageWithCallback("C_FUN_GET_SELECTIONS_TEXT",
        [callback](const QVariant& v) mutable {
            callback(v.toStringList());
        });
    }

    void Editor::getValue(std::function<void(const QString&)> callback) {
        sendMessageWithCallback("C_FUN_GET_VALUE",
        [callback](const QVariant &v) {
            QString langId = v.toString();
            callback(langId);
        });
    }

    void Editor::getCurrentWordOrSelections(std::function<void(const QStringList&)> callback) {
        sendMessageWithCallback("C_FUN_GET_SELECTIONS_TEXT",
        [&, callback](const QVariant& v) mutable {
            if(v.isNull()) {
                sendMessageWithCallback("C_FUN_GET_CURRENT_WORD",
                [callback](const QVariant& v) mutable {
                    callback(v.toStringList());
                });
            }else {
                callback(v.toStringList());
            }
        });
    }
}
