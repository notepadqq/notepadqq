#include "include/Search/searchinstance.h"

#include <QApplication>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QClipboard>

#include "include/mainwindow.h"
#include "include/EditorNS/editor.h"

/**
 * @brief getFormattedLocationText Creates a html-formatted string to use as the text of a toplevel QTreeWidget item.
 * @param docResult The DocResult to grab the information from
 * @param searchLocation The base directory that was searched
 * @return The html-formatted string
 */
QString getFormattedLocationText(const DocResult& docResult, const QString& searchLocation) {
    const int commonPathLength = searchLocation.length();
    const QString relativePath = docResult.fileName.mid(commonPathLength);

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
            .arg(result.m_lineNumber)
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
    QTreeWidget* treeWidget = m_treeWidget.get();

    treeWidget->setHeaderLabel(tr("Search Results in") + " \"" + config.directory + "\"");
    treeWidget->setItemDelegate(new SearchTreeDelegate(treeWidget));

    connect(treeWidget, &QTreeWidget::itemChanged, [](QTreeWidgetItem *item, int column){
        // When checking/unchecking a toplevel item we want to propagate it to all children
        if(column!=0 || item->parent()) return;
        const Qt::CheckState checkState = item->checkState(0);

        for (int i=0; i<item->childCount(); i++) {
            item->child(i)->setCheckState(0, checkState);
        }
    });

    connect(treeWidget, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *item) {
        auto it = m_resultMap.find(item);
        if (it != m_resultMap.end())
            emit resultItemClicked( *m_docMap.at(item->parent()), *(it->second) );
    });

    // TODO: Can only do plain text search at the moment. Also, refactor
    if (config.searchScope == SearchConfig::ScopeCurrentDocument) {
        MainWindow* mw = config.targetWindow;
        TopEditorContainer* tec = mw->topEditorContainer();
        EditorTabWidget* tw = tec->currentTabWidget();
        Editor* ed = mw->currentEditor();
        QString title = tw->tabText(tw->indexOf(ed));

        DocResult dr = FileSearcher::searchPlainText(config, ed->value());
        dr.docType = DocResult::TypeDocument;
        dr.fileName = title;
        dr.editor = ed;

        if (!dr.results.empty())
            m_searchResult.results.push_back(dr);

        onSearchCompleted();
    } else if (config.searchScope == SearchConfig::ScopeAllOpenDocuments) {
        MainWindow* mw = config.targetWindow;
        TopEditorContainer* tec = mw->topEditorContainer();
        EditorTabWidget* tw = tec->currentTabWidget();

        const int numEditors = tw->count();

        for (int i=0; i<numEditors; ++i) {
            Editor* ed = tw->editor(i);
            QString title = tw->tabText(i);

            DocResult dr = FileSearcher::searchPlainText(config, ed->value());
            dr.docType = DocResult::TypeDocument;
            dr.fileName = title;
            dr.editor = ed;
            if (!dr.results.empty())
                m_searchResult.results.push_back(dr);
        }

        onSearchCompleted();
    } else if (config.searchScope == SearchConfig::ScopeFileSystem) {
        QTreeWidgetItem* toplevelitem = new QTreeWidgetItem(treeWidget);
        toplevelitem->setText(0, tr("Calculating..."));

        m_fileSearcher = FileSearcher::prepareAsyncSearch(config);
        connect(m_fileSearcher, &FileSearcher::resultProgress, this, &SearchInstance::onSearchProgress);
        connect(m_fileSearcher, &FileSearcher::resultReady, this, &SearchInstance::onSearchCompleted);
        connect(m_fileSearcher, &FileSearcher::finished, m_fileSearcher, [this](){
            m_fileSearcher->deleteLater();
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

    const QTreeWidget* tree = m_treeWidget.get();
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
        if(!r.results.empty()) result.results.push_back(r);
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
            if(nextTop >= treeWidget->topLevelItemCount())
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
            if(prevTop < 0)
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
    const QTreeWidget* tree = m_treeWidget.get();
    for (int i=0; i<tree->topLevelItemCount(); i++) {
        for (int c=0; c<tree->topLevelItem(i)->childCount(); c++) {
            QTreeWidgetItem* it = tree->topLevelItem(i)->child(c);

            if (it->checkState(0) == Qt::Checked)
                cp += m_resultMap.at(it)->m_matchLineString + '\n';
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

    // If m_fileSearcher is set we get the results from it. Otherwise we assume m_searchResult already has all
    // results in it.
    if (m_fileSearcher) {
        m_searchResult = std::move(m_fileSearcher->getResult());
        delete m_fileSearcher;
        m_fileSearcher = nullptr;
    }

    m_treeWidget->setHeaderLabel(tr("Search Results in")
                                 + " \""
                                 + m_searchConfig.directory
                                 + "\" ("
                                 + tr("completed in ")
                                 + QString::number(m_searchResult.m_timeToComplete)
                                 + "ms)");

    m_treeWidget->clear();
    for (const auto& doc : m_searchResult.results) {
        QTreeWidgetItem* toplevelitem = new QTreeWidgetItem(m_treeWidget.get());
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

    emit searchCompleted();
}