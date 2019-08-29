#include "include/Search/searchinstance.h"

#include "include/EditorNS/editor.h"
#include "include/mainwindow.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>

/**
 * @brief getFormattedLocationText Creates a html-formatted string to use as the text of a toplevel QTreeWidget item.
 * @param docResult The DocResult to grab the information from
 * @param searchLocation The base directory that was searched
 * @return The html-formatted string
 */
QString getFormattedLocationText(const DocResult& docResult, const QString& searchLocation) {
    const int commonPathLength = searchLocation.length();
    const QString relativePath = docResult.fileName.startsWith(searchLocation) ?
                docResult.fileName.mid(commonPathLength) : docResult.fileName;

    return QString("<span style='white-space:pre-wrap;'>" +
                   QObject::tr("<b>%1</b> Results for:   '<b>%2</b>'")
                   .arg(docResult.results.size(), 4) // Pad the number so all rows line up nicely.
                   .arg(relativePath.toHtmlEscaped())
                   + "</span>");
}

/**
 * @brief getFormattedLocationText Creates a html-formatted string to use as the text of a sublevel QTreeWidget item.
 * @param result The MatchResult to grab the information from
 * @param showFullText True if the match's full text line should be down. When false, long lines will be shortened.
 * @return The html-formatted string
 */
QString getFormattedResultText(const MatchResult& result, bool showFullText=false) {
    // If, at some point, we want to use different color schemes for the text highlighting, these are ways to grab
    // Colors from specific palettes from Qt.
    //const static QString highlightColor = QApplication::palette().alternateBase().color().name(); /*#ffef0b*/
    //const static QString highlightTextColor = QApplication::palette().highlightedText().color().name(); /*black;*/

    return QString(
                "<span style='white-space:pre-wrap;'>%1:\t"
                "%2"
                "<span style='background-color: #ffef0b; color: black;'>%3</span>"
                "%4</span>")
            .arg(result.lineNumber)
            // Natural tabs are way too large; just replace them.
            .arg(result.getPreMatchString(showFullText).replace('\t', "    ").toHtmlEscaped(),
                result.getMatchString().replace('\t', "    ").toHtmlEscaped(),
                result.getPostMatchString(showFullText).replace('\t', "    ").toHtmlEscaped());
}

/**
 * @brief SearchTreeDelegate Helper class for SearchInstance's tree widget. It allows the use of
 *                           HTML-formatted text in the tree widget items.
 */
class SearchTreeDelegate : public QStyledItemDelegate
{
public:
    SearchTreeDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyleOptionViewItem optionItem = option;
        initStyleOption(&optionItem, index);

        QStyle *style = optionItem.widget? optionItem.widget->style() : QApplication::style();

        QTextDocument doc;
        doc.setHtml(optionItem.text);

        // Painting item without text
        optionItem.text.clear();
        style->drawControl(QStyle::CE_ItemViewItem, &optionItem, painter);

        QAbstractTextDocumentLayout::PaintContext ctx;

        // Highlighting text if item is selected
        if (optionItem.state & QStyle::State_Selected)
            ctx.palette.setColor(QPalette::Text, optionItem.palette.color(QPalette::Active, QPalette::HighlightedText));

        QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionItem);
        painter->save();
        painter->translate(textRect.topLeft());
        painter->setClipRect(textRect.translated(-textRect.topLeft()));
        doc.documentLayout()->draw(painter, ctx);
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem optionItem = option;
        initStyleOption(&optionItem, index);

        QTextDocument doc;
        doc.setHtml(optionItem.text);
        doc.setTextWidth(optionItem.rect.width());
        return QSize(doc.idealWidth(), doc.size().height());
    }
};

SearchInstance::SearchInstance(const SearchConfig& config)
    : QObject(nullptr),
      m_searchConfig(config),
      m_treeWidget(new QTreeWidget())
{
    QTreeWidget* treeWidget = getResultTreeWidget();

    QString searchLocation;

    switch(config.searchScope) {
    case SearchConfig::ScopeCurrentDocument:
        searchLocation = tr("current document"); break;
    case SearchConfig::ScopeAllOpenDocuments:
        searchLocation = tr("open documents"); break;
    case SearchConfig::ScopeFileSystem:
        searchLocation = '"' + config.directory + '"'; break;
    }

    m_contextMenu = new QMenu(treeWidget);

    // Create actions for the custom context menu
    m_actionCopyLine = new QAction(tr("Copy Line to Clipboard"), m_contextMenu);
    connect(m_actionCopyLine, &QAction::triggered, this, [this, treeWidget](){
        auto* item = treeWidget->currentItem();
        auto it = m_resultMap.find(item);
        if (it != m_resultMap.end())
            QApplication::clipboard()->setText( m_resultMap.at(it->first)->matchLineString );
    });

    m_actionOpenDocument = new QAction(tr("Open Document"), m_contextMenu);
    connect(m_actionOpenDocument, &QAction::triggered, this, [this, treeWidget](){
        auto* item = treeWidget->currentItem();
        auto it = m_resultMap.find(item);
        auto* resultItem = it != m_resultMap.end() ? it->second : nullptr;
        auto* docItem = m_docMap.at(resultItem ? item->parent() : item);
        emit itemInteracted( *docItem, resultItem, SearchUserInteraction::OpenDocument );
    });

    m_actionOpenFolder = new QAction(tr("Open Folder in File Browser"), m_contextMenu);
    connect(m_actionOpenFolder, &QAction::triggered, this, [this, treeWidget](){
        auto* item = treeWidget->currentItem();
        auto it = m_resultMap.find(item);
        auto* resultItem = it != m_resultMap.end() ? it->second : nullptr;
        auto* docItem = m_docMap.at(resultItem ? item->parent() : item);
        emit itemInteracted( *docItem, resultItem, SearchUserInteraction::OpenContainingFolder );
    });

    m_contextMenu->addAction(m_actionCopyLine);
    m_contextMenu->addAction(m_actionOpenDocument);
    m_contextMenu->addAction(m_actionOpenFolder);

    treeWidget->setHeaderLabel(tr("Search Results in: %1").arg(searchLocation));
    treeWidget->setItemDelegate(new SearchTreeDelegate(treeWidget));
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(treeWidget, &QTreeWidget::itemChanged, [](QTreeWidgetItem *item, int column){
        // When checking/unchecking a toplevel item we want to propagate it to all children
        if (column!=0 || item->parent()) return;
        const Qt::CheckState checkState = item->checkState(0);

        for (int i=0; i<item->childCount(); i++) {
            item->child(i)->setCheckState(0, checkState);
        }
    });

    connect(treeWidget, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *item) {
        auto it = m_resultMap.find(item);
        if (it != m_resultMap.end()) // Don't emit the interaction if no ResultItem was clicked
            emit itemInteracted( *m_docMap.at(item->parent()), (it->second), SearchUserInteraction::OpenDocument );
    });

    connect(treeWidget, &QTreeWidget::customContextMenuRequested, [this, treeWidget](const QPoint &pos){
        if (!treeWidget->currentItem())
            return;

        // Disable to CopyLines action if a DocResult was clicked. Doesn't make sense since no single line
        // was selected in this case.
        bool isTopLevelItem = treeWidget->currentItem()->childCount() > 0;
        m_actionCopyLine->setEnabled(!isTopLevelItem);

        auto localPos = treeWidget->mapToGlobal(pos);
        localPos.setY(localPos.y() + treeWidget->header()->height());
        m_contextMenu->exec( localPos );
    });

    // If we're searching through documents we'll just do the search right now. File System searches are
    // delegated to a FileSearcher instance because they can take a while to finish.
    if (config.searchScope == SearchConfig::ScopeCurrentDocument ||
            config.searchScope == SearchConfig::ScopeAllOpenDocuments) {

        // This is a mess because Nqq's Editor management is a mess.
        // We'll grab all Editors that want to be searched, then search them one-by-one and add the results
        // to our SearchResult instance.
        std::vector<QSharedPointer<Editor>> editorsToSearch;

        MainWindow* mw = config.targetWindow;
        TopEditorContainer* tec = mw->topEditorContainer();

        if (config.searchScope == SearchConfig::ScopeCurrentDocument)
            editorsToSearch.push_back( mw->currentEditor() );
        else
            editorsToSearch = tec->getOpenEditors();

        if (config.searchMode == SearchConfig::ModePlainText ||
            config.searchMode == SearchConfig::ModePlainTextSpecialChars) {
            for (auto ed : editorsToSearch) {
                DocResult dr = FileSearcher::searchPlainText(config, ed->value());
                dr.docType = DocResult::TypeDocument;
                dr.fileName = tec->tabWidgetFromEditor(ed)->tabTextFromEditor(ed);
                dr.editor = ed;
                if (!dr.results.empty())
                    m_searchResult.results.push_back(dr);
            }
        } else if (config.searchMode == SearchConfig::ModeRegex) {
            QRegularExpression regex = FileSearcher::createRegexFromConfig(config);
            for (auto ed : editorsToSearch) {
                DocResult dr = FileSearcher::searchRegExp(regex, ed->value());
                dr.docType = DocResult::TypeDocument;
                dr.fileName = tec->tabWidgetFromEditor(ed)->tabTextFromEditor(ed);
                dr.editor = ed;
                if (!dr.results.empty())
                    m_searchResult.results.push_back(dr);
            }
        }
        onSearchCompleted();
    } else if (config.searchScope == SearchConfig::ScopeFileSystem) {
        QTreeWidgetItem* toplevelitem = new QTreeWidgetItem(treeWidget);
        toplevelitem->setText(0, tr("Calculating..."));

        m_fileSearcher = FileSearcher::prepareAsyncSearch(config);
        connect(m_fileSearcher, &FileSearcher::resultProgress, this, &SearchInstance::onSearchProgress);
        connect(m_fileSearcher, &FileSearcher::resultReady, this, &SearchInstance::onSearchCompleted);
        connect(m_fileSearcher, &FileSearcher::finished, m_fileSearcher, &FileSearcher::deleteLater);
        connect(m_fileSearcher, &FileSearcher::finished, this, [this]() {
            // 'this' may be deleted during fileSearcher's lifetime, so this needs its own signal connection.
            m_fileSearcher = nullptr;
        });

        m_fileSearcher->start();
    }
}

SearchInstance::~SearchInstance()
{
    // After canceling, m_fileSearcher is deleted through a signal connected in SearchInstance's constructor
    if (m_fileSearcher) m_fileSearcher->cancel();
}

SearchResult SearchInstance::getFilteredSearchResult() const
{
    SearchResult result;

    const QTreeWidget* tree = getResultTreeWidget();
    for (int i=0; i<tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* docWidget = tree->topLevelItem(i);
        const DocResult* fullResult = m_docMap.at(docWidget);
        DocResult r = *fullResult;
        r.results.clear();

        for (int c=0; c<docWidget->childCount(); c++) {
            QTreeWidgetItem* it = tree->topLevelItem(i)->child(c);
            if (it->checkState(0) == Qt::Checked)
                r.results.push_back( *m_resultMap.at(it) );
        }
        if (!r.results.empty()) result.results.push_back(r);
    }

    return result;
}

void SearchInstance::showFullLines(bool showFullLines)
{
    if (m_showFullLines == showFullLines)
        return;

    m_showFullLines = showFullLines;

    if (m_isSearchInProgress)
        return;

    for (auto& item : m_resultMap) {
        QTreeWidgetItem* treeItem = item.first;
        const MatchResult& res = *item.second;
        treeItem->setText(0, getFormattedResultText(res, showFullLines));
    }
    // TODO: This doesn't actually resize the widget view area.
    //m_treeWidget->resizeColumnToContents(0);
}

void SearchInstance::expandAllResults()
{
    m_treeWidget->expandAll();
    m_resultsAreExpanded = true;
}

void SearchInstance::collapseAllResults()
{
    m_treeWidget->collapseAll();
    m_resultsAreExpanded = false;
}

void SearchInstance::selectNextResult()
{
    QTreeWidget* treeWidget = getResultTreeWidget();

    QTreeWidgetItem* curr = treeWidget->currentItem();
    QTreeWidgetItem* next = nullptr;

    if (!curr)
        next = treeWidget->topLevelItem(0)->child(0);
    else if (!curr->parent()) {
        next = curr->child(0);
    } else {
        QTreeWidgetItem* top = curr->parent();
        int nextIndex = top->indexOfChild(curr) + 1;

        if (nextIndex < top->childCount())
            next = top->child(nextIndex);
        else {
            int nextTop = treeWidget->indexOfTopLevelItem(top) + 1;
            if (nextTop >= treeWidget->topLevelItemCount())
                nextTop = 0;
            next = treeWidget->topLevelItem(nextTop)->child(0);
        }
    }

    treeWidget->setCurrentItem(next);
    emit treeWidget->doubleClicked(treeWidget->currentIndex());
}

void SearchInstance::selectPreviousResult()
{
    QTreeWidget* treeWidget = getResultTreeWidget();

    QTreeWidgetItem* curr = treeWidget->currentItem();
    QTreeWidgetItem* prev = nullptr;

    if (!curr) {
        QTreeWidgetItem* lastTop = treeWidget->topLevelItem(treeWidget->topLevelItemCount()-1);
        prev = lastTop->child(lastTop->childCount()-1);
    } else if (!curr->parent()) {
        prev = curr->child(curr->childCount()-1);
    } else {
        QTreeWidgetItem* top = curr->parent();
        int prevIndex = top->indexOfChild(curr) - 1;

        if (prevIndex >= 0)
            prev = top->child(prevIndex);
        else {
            int prevTop = treeWidget->indexOfTopLevelItem(top) - 1;
            if (prevTop < 0)
                prevTop = treeWidget->topLevelItemCount() - 1;
            prev = treeWidget->topLevelItem(prevTop)->child(treeWidget->topLevelItem(prevTop)->childCount()-1);
        }
    }

    treeWidget->setCurrentItem(prev);
    emit treeWidget->doubleClicked(treeWidget->currentIndex());
}

void SearchInstance::copySelectedLinesToClipboard() const
{
    QString cp;

    //This loops through all QTreeWidgetItems and copies their
    //contents if they are checked. This way proper item order is preserved.
    const QTreeWidget* tree = getResultTreeWidget();
    for (int i=0; i<tree->topLevelItemCount(); i++) {
        for (int c=0; c<tree->topLevelItem(i)->childCount(); c++) {
            QTreeWidgetItem* it = tree->topLevelItem(i)->child(c);

            if (it->checkState(0) == Qt::Checked)
                cp += m_resultMap.at(it)->matchLineString + '\n';
        }
    }

    if (!cp.isEmpty()) // Remove the final '\n' from the text
        cp.remove(cp.length()-1,1);

    QApplication::clipboard()->setText(cp);
}

void SearchInstance::onSearchProgress(int processed, int total)
{
    m_treeWidget->topLevelItem(0)->setText(0,QString(tr("Search in progress [%1/%2 finished]")).arg(processed).arg(total));
}

void SearchInstance::onSearchCompleted()
{
    m_isSearchInProgress = false;

    // m_fileSearcher is only instantiated when we've done a filesystem search. If so, get the results
    // from there. Otherwise all search results were already added to m_searchResult
    if (m_fileSearcher) {
        m_searchResult = std::move(m_fileSearcher->getResult());
    }

    m_treeWidget->clear();
    for (const auto& doc : m_searchResult.results) {
        QTreeWidgetItem* toplevelitem = new QTreeWidgetItem(getResultTreeWidget());
        toplevelitem->setText(0, getFormattedLocationText(doc, m_searchConfig.directory));
        toplevelitem->setCheckState(0, Qt::Checked);
        m_docMap[toplevelitem] = &doc;

        for (const auto& res : doc.results) {
            QTreeWidgetItem* it = new QTreeWidgetItem(toplevelitem);
            it->setText(0, getFormattedResultText(res, m_showFullLines));
            it->setCheckState(0, Qt::Checked);
            m_resultMap[it] = &res;
        }
    }

    if (m_searchResult.results.size() == 1)
        m_treeWidget->expandAll();

    emit searchCompleted();
}
