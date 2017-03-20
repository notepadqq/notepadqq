#include "include/EditorNS/editor.h"
#include "include/notepadqq.h"
#include "include/nqqsettings.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QEventLoop>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QWebEngineSettings>
#include <QtWebChannel/QWebChannel>
#include <QJsonDocument>

namespace EditorNS
{

    QQueue<Editor*> Editor::m_editorBuffer = QQueue<Editor*>();

    Editor::Editor(QWidget *parent) :
        QWidget(parent)
    {

        QString themeName = NqqSettings::getInstance().Appearance.getColorScheme();
        if (themeName == "") {
            themeName = "default";
        }

        fullConstructor(themeFromName(themeName));
    }

    Editor::Editor(const Theme &theme, QWidget *parent) :
        QWidget(parent)
    {
        fullConstructor(theme);
    }

    void Editor::fullConstructor(const Theme &theme)
    {
        initJsProxy();
        initWebView(theme);
        initContextMenu();
        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->addWidget(m_webView, 1);
        setLayout(m_layout);
        
        connect(m_webView->page(),
                &QWebEnginePage::loadFinished,
                this,
                &Editor::on_javaScriptWindowObjectCleared);
        connect(m_webView, &CustomQWebView::mouseWheel, this, &Editor::mouseWheel);
        connect(m_webView, &CustomQWebView::urlsDropped, this, &Editor::urlsDropped);
    }

    
    void Editor::initWebView(const Theme &theme)
    {
        m_webView = new CustomQWebView(this);
        QUrlQuery query;
        query.addQueryItem("themePath", theme.path);
        query.addQueryItem("themeName", theme.name);
        QUrl url = QUrl("file://" + Notepadqq::editorPath());
        url.setQuery(query);

        m_webView->connectJavaScriptObject("cpp_ui_driver", m_jsProxy);
        m_webView->page()->load(url);
        m_webView->page()->setBackgroundColor(Qt::transparent);
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
        if(msg == "J_EVT_READY") {
            m_loaded = true;
            emit editorReady();
        } else if(msg == "J_EVT_CONTENT_CHANGED") {
            emit contentChanged(buildContentChangedEventData(data));
        } else if(msg == "J_EVT_CLEAN_CHANGED") {
            m_contentInfo.clean = data.toBool();
            emit cleanChanged(m_contentInfo.clean);
        } else if(msg == "J_EVT_CURSOR_ACTIVITY") {
            emit cursorActivity(buildCursorEventData(data));
        } else if(msg == "J_EVT_GOT_FOCUS") {
            emit gotFocus();
        } else if(msg == "J_EVT_CURRENT_LANGUAGE_CHANGED") {
            emit languageChanged(buildLanguageChangedEventData(data));
        } else if(msg == "J_EVT_DOCUMENT_LOADED") {
            emit documentLoaded(m_alreadyLoaded);
            m_alreadyLoaded = true;
        }
    }

    Editor::ContentInfo Editor::buildContentChangedEventData(
            const QVariant& data, 
            bool cache)
    {
        QVariantMap v = data.toMap();
        ContentInfo temp;
        temp.charCount = v["charCount"].toInt();
        temp.lineCount = v["lineCount"].toInt();
        if (cache) {
            m_contentInfo = temp;
        }
        return temp;
    }


    Editor::CursorInfo Editor::buildCursorEventData(const QVariant& data, 
            bool cache)
    {
        QVariantMap v = data.toMap();
        CursorInfo  temp;
        temp.line = v["cursorLine"].toInt();
        temp.column = v["cursorColumn"].toInt();
        temp.selectionCharCount = v["selectionCharCount"].toInt();
        temp.selectionLineCount = v["selectionLineCount"].toInt();
        if (cache) {
            m_cursorInfo = temp;
        }
        return temp;
    }

    //NOTE: We don't cache these as of yet.
    Editor::LanguageInfo Editor::buildLanguageChangedEventData(
            const QVariant& data,
            bool /*cache*/)
    {
        QVariantMap v = data.toMap();
        LanguageInfo temp;
        temp.id = v.value("id").toString();
        temp.name = v.value("lang").toMap().value("name").toString();
        if (!m_customIndentationMode) {
            setIndentationMode(temp.id);
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
        return m_contentInfo.clean;
    }

    void Editor::markClean()
    {
        sendMessage("C_CMD_MARK_CLEAN");
    }

    void Editor::markDirty()
    {
        sendMessage("C_CMD_MARK_DIRTY");
    }
 
    QVariant Editor::getLanguageData(const QString& langId)
    {
        QVariant comp = QVariant::fromValue(langId);
        for (auto& l : m_langCache) {
            if (l.id == langId) {
                QVariantMap langData;
                langData.insert("id", l.id);
                langData.insert("mime", l.mime);
                langData.insert("mode", l.mode);
                langData.insert("fileNames", l.fileNames);
                langData.insert("fileExtensions", l.fileExtensions);
                return QVariant::fromValue(langData);
            }
        }
        return QVariant();
    }
   
    void Editor::initLanguageCache()
    {
        QFileInfo fileInfo(Notepadqq::editorPath());
        QString fileName = fileInfo.absolutePath() + "/classes/Languages.json";
        QFile scriptFile(fileName);
        scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QJsonDocument json = QJsonDocument::fromJson(scriptFile.readAll());
        scriptFile.close();
        for (auto it = json.object().constBegin(); it != json.object().constEnd(); ++it) {
            QJsonObject mode = it.value().toObject();
            LanguageData newMode;
            newMode.id = it.key();
            newMode.name = mode.value("name").toString();
            newMode.mime = mode.value("mime").toString();
            newMode.mode = mode.value("mode").toString();
            newMode.fileNames = mode.value("fileNames").toVariant().toStringList();
            newMode.fileExtensions = mode.value("fileExtensions").toVariant().toStringList();
            newMode.firstNonBlankLine = mode.value("firstNonBlankLine").toVariant().toStringList();
            m_langCache.append(newMode);
        }
    }

    QVector<Editor::LanguageData> Editor::languages()
    {
        return m_langCache;
    }

    QString Editor::getLanguage()
    {
        QVariantMap data = m_jsProxy->getRawValue("language").toMap();
        return data.value("id").toString();
    }

    void Editor::setLanguage(const QString &language)
    {
        sendMessage("C_CMD_SET_LANGUAGE", getLanguageData(language));
        if (!m_customIndentationMode)
            setIndentationMode(language);
    }

    void Editor::setLanguageFromFileName(const QString& fileName)
    {
        // Case-sensitive fileName search
        for (auto& l : m_langCache) {
            if (l.fileNames.contains(fileName)) {
                setLanguage(l.id);
                return;
            }
        }

        // Case insensitive file extension search
        int pos = fileName.lastIndexOf('.');
        if (pos != -1) {
            QString ext = fileName.mid(pos+1);
            for (auto& l : m_langCache) {
                if (l.fileExtensions.contains(ext, Qt::CaseInsensitive)) {
                    setLanguage(l.id);
                    return;
                }
            }
        }
        
        // First non-blank line
        getValue([&] (QString rawTxt) {
            QTextStream stream(&rawTxt);
            stream.skipWhiteSpace();
            QString test = stream.readLine();
            for (auto& l : m_langCache) {
                if (!l.firstNonBlankLine.isEmpty()) {
                    for (auto& t : l.firstNonBlankLine) {
                        if (test.contains(QRegularExpression(t))) {
                            setLanguage(l.id);
                            return;
                        }
                    }
                }
            }
        });
    }

    void Editor::setLanguageFromFileName()
    {
        setLanguageFromFileName(fileName().toString());
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
        QPair<int, int> indent;
        m_jsProxy->getValue("indentMode", indent);
        IndentationMode out;
        out.useTabs = indent.first;
        out.size = indent.second;
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
        setIndentationMode(getLanguage());
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
        qreal normFact = factor;
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
        return m_contentInfo.lineCount;
    }

    int Editor::getCharCount()
    {
        return m_contentInfo.charCount;
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
        return qMakePair(m_cursorInfo.line, m_cursorInfo.column);
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
        QPair<int, int> scrollPosition;
        bool valid = m_jsProxy->getValue("scrollPosition", scrollPosition);
        if(!valid) {
            return qMakePair(0, 0);
        }
        return scrollPosition;
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

    void Editor::setOverwrite(bool overwrite)
    {
        sendMessage("C_CMD_SET_OVERWRITE", overwrite);
    }

    void Editor::setTabsVisible(bool visible)
    {
        sendMessage("C_CMD_SET_TABS_VISIBLE", visible);
    }

    Editor::IndentationMode Editor::detectDocumentIndentation(bool *found)
    {
        IndentationMode out;
        QPair<int, int> indent;
        bool _found = m_jsProxy->getValue("detectedIndent", indent);

        if (found != nullptr) {
            *found = _found;
        }

        if (_found) {
            out.useTabs = indent.first;
            out.size = indent.second;
        }

        return out;
    }

    void Editor::print(QPrinter *printer)
    {
        sendMessage("C_CMD_DISPLAY_PRINT_STYLE");
        //m_webView->print(printer); // FIXME
        sendMessage("C_CMD_DISPLAY_NORMAL_STYLE");
    }

    void Editor::getSelections(std::function<void(const QList<Editor::Selection>&)> callback)
    {
        sendMessageWithCallback("C_FUN_GET_SELECTIONS",
        [callback](const QVariant& v) {
            QList<Selection> out;
            QList<QVariant> sels = v.toList();
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
            callback(out);
        });
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

    void Editor::getLanguage(std::function<void(const QString&)> callback) {
        sendMessageWithCallback("C_FUN_GET_CURRENT_LANGUAGE",
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
QVector<Editor::LanguageData> Editor::m_langCache;
}
