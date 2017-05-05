#include "include/Search/advancedsearchdock.h"

#include <QWidget>
#include <QLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QMenu>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QMessageBox>

// For the clipboard
#include <QApplication>
#include <QClipboard>

// For the delegate
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#include <array>

#include "include/iconprovider.h"

#include "include/Search/replaceworker.h"

QString getFormattedLocationText(const DocResult& docResult, const QString& searchLocation) {
    const int commonPathLength = searchLocation.length();
    const QString relativePath = docResult.fileName.mid(commonPathLength);

    return QString("<span style='white-space:pre-wrap;'><b>%1</b> Results for:   '<b>%2</b>'</span>")
            .arg(docResult.results.size(), 4) // Pad the number so all rows line up nicely.
            .arg(relativePath.toHtmlEscaped());
}

QString getFormattedResultText(const MatchResult& result, bool showFullText=false) {
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

bool askConfirmationForReplace(QString replaceText, int numReplacements) {
    return QMessageBox::information(QApplication::activeWindow(),
                                    "Confirm Replacement",
                                    QString("This will replace %1 selected matches with \"%2\"."
                                            " This action cannot be undone. Continue?")
                                    .arg(numReplacements)
                                    .arg(replaceText),
                                    QMessageBox::Ok | QMessageBox::Cancel,
                                    QMessageBox::Cancel) == QMessageBox::Ok;
}


QDockWidgetTitleButton::QDockWidgetTitleButton(QDockWidget *dockWidget)
    : QAbstractButton(dockWidget)
{
    setFocusPolicy(Qt::NoFocus);
}

QSize QDockWidgetTitleButton::sizeHint() const
{
    ensurePolished();

    int size = 2*style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, 0, this);
    if (!icon().isNull()) {
        int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
        QSize sz = icon().actualSize(QSize(iconSize, iconSize));
        size += qMax(sz.width(), sz.height());
    }

    return QSize(size, size);
}

void QDockWidgetTitleButton::enterEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::enterEvent(event);
}

void QDockWidgetTitleButton::leaveEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::leaveEvent(event);
}

void QDockWidgetTitleButton::paintEvent(QPaintEvent* /*evt*/)
{
    QPainter p(this);

    QStyleOptionToolButton opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;

    if (style()->styleHint(QStyle::SH_DockWidget_ButtonsHaveFrame, 0, this))
    {
        if (isEnabled() && underMouse() && !isChecked() && !isDown())
            opt.state |= QStyle::State_Raised;
        if (isChecked())
            opt.state |= QStyle::State_On;
        if (isDown())
            opt.state |= QStyle::State_Sunken;
        style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);
    }

    opt.icon = icon();
    opt.subControls = 0;
    opt.activeSubControls = 0;
    opt.features = QStyleOptionToolButton::None;
    opt.arrowType = Qt::NoArrow;
    int size = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
    opt.iconSize = QSize(size, size);
    style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
}

class SearchTreeDelegate : public QStyledItemDelegate
{
public:
    SearchTreeDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

QSize SearchTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionItem = option;
    initStyleOption(&optionItem, index);

    QTextDocument doc;
    doc.setHtml(optionItem.text);
    doc.setTextWidth(optionItem.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}

void SearchTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{

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

QLayout* AdvancedSearchDock::buildLeftTitlebar() {

    QLabel* label = new QLabel;
    label->setText("Advanced Search");
    label->setMaximumWidth( label->fontMetrics().width(label->text()) );

    m_btnClearHistory = new QToolButton;
    m_btnClearHistory->setIcon(IconProvider::fromTheme("document-new"));
    m_btnClearHistory->setToolTip("Clear Search History");

    m_cmbSearchHistory = new QComboBox;
    m_cmbSearchHistory->addItem("New Search");
    m_cmbSearchHistory->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_cmbSearchHistory->setMinimumWidth(120);
    m_cmbSearchHistory->setMaximumWidth(300);

    m_btnPrevResult = new QToolButton;
    m_btnPrevResult->setIcon(IconProvider::fromTheme("go-previous"));
    m_btnPrevResult->setToolTip("Go To Previous Result");

    m_btnNextResult = new QToolButton;
    m_btnNextResult->setIcon(IconProvider::fromTheme("go-next"));
    m_btnNextResult->setToolTip("Go To Next Result");

    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);

    QMenu* menu = new QMenu();
    m_actExpandAll = menu->addAction("Expand/Collapse All");
    m_actExpandAll->setCheckable(true);
    m_actRedoSearch = menu->addAction("Redo Search");
    m_actCopyContents = menu->addAction("Copy Selected Contents To Clipboard");
    m_actShowFullLines = menu->addAction("Show Full Lines");
    m_actShowFullLines->setCheckable(true);
    m_actRemoveSearch= menu->addAction("Remove This Search");

    m_btnMoreOptions = new QToolButton;
    m_btnMoreOptions->setIcon(IconProvider::fromTheme("preferences-other"));
    m_btnMoreOptions->setEnabled(false);
    m_btnMoreOptions->setToolTip("More Options");
    m_btnMoreOptions->setPopupMode(QToolButton::InstantPopup);
    m_btnMoreOptions->setMenu(menu);

    m_btnToggleReplaceOptions = new QToolButton;
    m_btnToggleReplaceOptions->setText("Replace Options");
    m_btnToggleReplaceOptions->setCheckable(true);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(label);
    layout->addWidget(m_btnClearHistory);
    layout->addWidget(m_cmbSearchHistory);
    layout->addWidget(m_btnPrevResult);
    layout->addWidget(m_btnNextResult);
    layout->addWidget(separator);
    layout->addWidget(m_btnMoreOptions);
    layout->addWidget(m_btnToggleReplaceOptions);
    layout->setSizeConstraint(QHBoxLayout::SetNoConstraint);

    return layout;
}

QLayout* AdvancedSearchDock::buildUpperTitlebarLayout() {
    QHBoxLayout* top = new QHBoxLayout;

    // Construct left side. Buttons etc.
    QLayout* leftSide = buildLeftTitlebar();

    m_btnDockUndock = new QDockWidgetTitleButton(m_dockWidget.data());
    m_btnDockUndock->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarNormalButton));

    m_btnClose = new QDockWidgetTitleButton(m_dockWidget.data());
    m_btnClose->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton));

    top->addLayout(leftSide);
    top->setAlignment(leftSide, Qt::AlignLeft);
    top->addWidget(m_btnDockUndock);
    top->setAlignment(m_btnDockUndock, Qt::AlignRight);
    top->addWidget(m_btnClose);
    top->setAlignment(m_btnClose, Qt::AlignRight);

    return top;
}

QLayout* AdvancedSearchDock::buildReplaceOptionsLayout() {
    m_replaceOptionsLayout = new QVBoxLayout;

    // Add a separating line.
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    // Add the bar with replace options.
    QHBoxLayout* replaceOptions = new QHBoxLayout;

    m_edtReplaceText = new QLineEdit;
    m_edtReplaceText->setMaximumWidth(300);
    m_edtReplaceText->setPlaceholderText("Replace Text");

    m_btnReplaceOne = new QToolButton;
    m_btnReplaceOne->setText("Replace One");
    m_btnReplaceOne->setToolTip("Replaces the currently selected search result.");

    m_btnReplaceSelected = new QToolButton;
    m_btnReplaceSelected->setText("Replace Selected");
    m_btnReplaceOne->setToolTip("Replaces all selected search results.");

    replaceOptions->addWidget(m_edtReplaceText);
    replaceOptions->addWidget(m_btnReplaceOne);
    replaceOptions->addWidget(m_btnReplaceSelected);
    replaceOptions->setSizeConstraint(QHBoxLayout::SetNoConstraint);

    m_replaceOptionsLayout->addWidget(line);
    m_replaceOptionsLayout->addLayout(replaceOptions);
    m_replaceOptionsLayout->setAlignment(replaceOptions, Qt::AlignLeft);

    return m_replaceOptionsLayout;
}

QWidget* AdvancedSearchDock::buildTitlebarWidget() {
    QLayout* upperBar = buildUpperTitlebarLayout();
    /*QLayout* lowerBar =*/ buildReplaceOptionsLayout(); // <- Hidden by default, aka not added

    m_titlebarLayout = new QVBoxLayout;
    m_titlebarLayout->addLayout(upperBar);

    // Create the actual titlebar widget
    QWidget* titlebar = new QWidget();
    titlebar->setLayout(m_titlebarLayout);

    return titlebar;
}

QWidget* AdvancedSearchDock::buildSearchPanelWidget() {
    QGridLayout* gl = new QGridLayout;

    m_edtSearchTerm = new QLineEdit;
    m_edtSearchTerm->setPlaceholderText("Search String");
    m_edtSearchTerm->setMaximumWidth(300);
    m_edtSearchTerm->setClearButtonEnabled(true);

    m_cmbSearchScope = new QComboBox;
    m_cmbSearchScope->addItem("Search in Current Document");
    m_cmbSearchScope->addItem("Search in All Open Documents");
    m_cmbSearchScope->addItem("Search in Files in File System");
    m_cmbSearchScope->setMaximumWidth(260);

    m_edtSearchPattern = new QLineEdit;
    m_edtSearchPattern->setPlaceholderText("*ext1, *ext2");
    m_edtSearchPattern->setMaximumWidth(300);
    m_edtSearchPattern->setClearButtonEnabled(true);

    m_edtSearchDirectory = new QLineEdit;
    m_edtSearchDirectory->setPlaceholderText("Directory");
    m_edtSearchDirectory->setMaximumWidth(260);
    m_edtSearchDirectory->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_edtSearchDirectory->setClearButtonEnabled(true);

    m_btnSelectSearchDirectory = new QToolButton;
    m_btnSelectSearchDirectory->setIcon(IconProvider::fromTheme("edit-find"));
    m_btnSelectSearchDirectory->setToolTip("Select Search Directory");

    QHBoxLayout* m2 = new QHBoxLayout;
    m2->addWidget(m_edtSearchDirectory);
    m2->addWidget(m_btnSelectSearchDirectory);


    m_btnSearch = new QToolButton;
    m_btnSearch->setText("Search");
    m_btnSearch->setEnabled(false);

    QLabel* scl = new QLabel("Scope:");
    scl->setMaximumWidth(80);
    QLabel* srl = new QLabel("Search:");
    srl->setMaximumWidth(80);
    QLabel* srp = new QLabel("Pattern:");
    srp->setMaximumWidth(80);
    QLabel* srd = new QLabel("Location:");
    srd->setMaximumWidth(80);

    m_chkMatchCase = new QCheckBox;
    m_chkMatchCase->setText("Match Case");

    m_chkMatchWords = new QCheckBox;
    m_chkMatchWords->setText("Match Whole Word Only");

    m_chkUseRegex = new QCheckBox;
    m_chkUseRegex->setText("Use Regular Expressions");

    m_chkIncludeSubdirs = new QCheckBox;
    m_chkIncludeSubdirs->setText("Include Subdirectories");

    QVBoxLayout* mini = new QVBoxLayout;
    mini->addWidget(m_chkMatchCase);
    mini->addWidget(m_chkMatchWords);
    mini->addWidget(m_chkUseRegex);

    gl->addWidget(srl, 0,0);
    gl->addWidget(scl, 1,0);
    gl->addWidget(srd, 2,0);
    gl->addWidget(srp, 3,0);

    gl->addWidget(m_edtSearchTerm, 0,1);
    gl->addWidget(m_cmbSearchScope, 1,1);
    gl->addLayout(m2, 2,1);
    gl->addWidget(m_edtSearchPattern, 3,1);
    gl->addWidget(m_chkIncludeSubdirs, 2, 3);


    gl->addLayout(mini, 0, 3, 2, 1);
    gl->addWidget(m_btnSearch, 3,2);

    gl->setSizeConstraint(QGridLayout::SetNoConstraint);

    // Put the gridview into a vbox with a strecher so the gridview doesn't strech when the panelwidget is resized.
    QVBoxLayout* top = new QVBoxLayout();

    // Add a separating line.
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    top->addWidget(line);
    top->addLayout(gl);
    top->addStretch();

    m_searchPanelWidget = new QWidget();
    m_searchPanelWidget->setLayout(top);
    return m_searchPanelWidget;
}

void AdvancedSearchDock::clearHistory()
{
    const bool anySearchInProgess = std::any_of(m_searchInstances.begin(),
                                                m_searchInstances.end(),
                                                [](const std::unique_ptr<SearchInstance>& inst) {
        return inst->searchInProgress;
    });

    if(anySearchInProgess) {
        QMessageBox msgBox(nullptr);
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("<h3>One or more searches are still in progress</h3>");
        msgBox.setInformativeText("All searches will be canceled and their results discarded if you continue.");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setEscapeButton(QMessageBox::Cancel);
        msgBox.exec();

        if (msgBox.standardButton(msgBox.clickedButton()) == QMessageBox::Cancel)
            return;
    }

    m_searchInstances.clear();
    m_cmbSearchHistory->clear();
    m_cmbSearchHistory->addItem("New Search"); //TODO: bad idea, move to reset() function or something
    onSearchHistorySizeChange();
}

void AdvancedSearchDock::selectSearchFromHistory(int index)
{
    if(index==-1)
        return;

    qDebug() << "Index is " << index << "and size is " << m_cmbSearchHistory->count();

    // TODO: Clean this up, move stuff into a connect/disconnect function, something like that.
    if(m_currentSearchInstance && m_currentSearchInstance->searchInProgress) {
        disconnect(m_currentSearchInstance, &SearchInstance::searchCompleted,
                   this, &AdvancedSearchDock::onCurrentSearchInstanceCompleted);
    }

    if (m_currentSearchInstance) {
        disconnect(m_currentSearchInstance, &SearchInstance::resultItemClicked,
                   this, &AdvancedSearchDock::resultItemClicked);
    }

    if (index==0) {
        m_currentSearchInstance = nullptr;

        m_dockWidget->setWidget(m_searchPanelWidget);
        m_btnToggleReplaceOptions->setChecked(false);
        m_btnToggleReplaceOptions->setEnabled(false);
        m_btnMoreOptions->setEnabled(false);
        m_btnPrevResult->setEnabled(false);
        m_btnNextResult->setEnabled(false);
        m_edtSearchTerm->setFocus();
    } else {
        m_currentSearchInstance = m_searchInstances[index-1].get();

        if(m_currentSearchInstance->searchInProgress) {
            connect(m_currentSearchInstance, &SearchInstance::searchCompleted,
                       this, &AdvancedSearchDock::onCurrentSearchInstanceCompleted);
        }

        connect(m_currentSearchInstance, &SearchInstance::resultItemClicked,
                   this, &AdvancedSearchDock::resultItemClicked);

        QTreeWidget* treeWidget = m_currentSearchInstance->m_treeWidget.get();

        m_dockWidget->setWidget(treeWidget);
        m_btnToggleReplaceOptions->setEnabled(true);
        m_btnMoreOptions->setEnabled(true);
        m_btnPrevResult->setEnabled(true);
        m_btnNextResult->setEnabled(true);

        m_actExpandAll->setChecked( m_currentSearchInstance->isMaximized );
        m_actShowFullLines->setChecked( m_currentSearchInstance->getShowFullLines() );

        updateSearchInProgressUi();
    }
}

void AdvancedSearchDock::onChangeSearchScope(int index)
{
    switch(index) {
    case 0: // Search current document
    case 1: // Search all open documents
        m_edtSearchPattern->setEnabled(false);
        m_edtSearchDirectory->setEnabled(false);
        m_btnSelectSearchDirectory->setEnabled(false);
        m_chkIncludeSubdirs->setEnabled(false);
        break;
    case 2: // Search in file system
        m_edtSearchPattern->setEnabled(true);
        m_edtSearchDirectory->setEnabled(true);
        m_btnSelectSearchDirectory->setEnabled(true);
        m_chkIncludeSubdirs->setEnabled(true);
        break;
    }
    onUserInput();
}

void AdvancedSearchDock::onCurrentSearchInstanceCompleted()
{
    disconnect(m_currentSearchInstance, &SearchInstance::searchCompleted,
               this, &AdvancedSearchDock::onCurrentSearchInstanceCompleted);

    updateSearchInProgressUi();
}

void AdvancedSearchDock::onUserInput()
{
    //0 == this doc, 1 == all docs, 2 == file system
    const int scope = m_cmbSearchScope->currentIndex();

    if(scope==0 || scope==1)
        m_btnSearch->setEnabled(!m_edtSearchTerm->text().isEmpty());
    else if(scope==2) {
        m_btnSearch->setEnabled(!m_edtSearchTerm->text().isEmpty() &&
                                !m_edtSearchDirectory->text().isEmpty());
    }
}

void AdvancedSearchDock::updateSearchInProgressUi()
{
    const bool progress = m_currentSearchInstance->searchInProgress;

    m_actExpandAll->setEnabled(!progress);
    m_actCopyContents->setEnabled(!progress);
    m_actShowFullLines->setEnabled(!progress);
    m_btnToggleReplaceOptions->setEnabled(!progress);

    m_btnPrevResult->setEnabled(!progress);
    m_btnNextResult->setEnabled(!progress);
}

SearchConfig AdvancedSearchDock::getConfigFromInputs()
{
    SearchConfig config;
    config.directory = m_edtSearchDirectory->text();
    config.filePattern = m_edtSearchPattern->text();
    config.searchString = m_edtSearchTerm->text();
    config.scope = m_cmbSearchScope->currentIndex();

    config.matchCase = m_chkMatchCase->isChecked();
    config.matchWord = m_chkMatchWords->isChecked();
    config.searchMode = m_chkUseRegex->isChecked() ?
                SearchHelpers::SearchMode::Regex : SearchHelpers::SearchMode::PlainText;
    config.includeSubdirs = m_chkIncludeSubdirs->isChecked();


    // TODO: Dummy config
    config.searchString = "variable variable";
    //config.filePattern = "*.cpp";
    config.directory = "/home/s3rius/dev/nqqtest";
    config.matchWord = false;
    config.matchCase = false;
    config.includeSubdirs = true;
    config.searchMode = SearchHelpers::SearchMode::Regex; //PlainText

    return config;
}

void AdvancedSearchDock::setInputsFromConfig(const SearchConfig& config)
{
    m_edtSearchDirectory->setText( config.directory );
    m_edtSearchPattern->setText( config.filePattern );
    m_edtSearchTerm->setText( config.searchString );
    m_cmbSearchScope->setCurrentIndex( config.scope );

    m_chkMatchCase->setChecked( config.matchCase );
    m_chkMatchWords->setChecked( config.matchWord );
    m_chkUseRegex->setChecked( config.searchMode == SearchHelpers::SearchMode::Regex );
    m_chkIncludeSubdirs->setChecked( config.includeSubdirs );
}

void AdvancedSearchDock::onSearchHistorySizeChange()
{
    const bool historyNotEmpty = m_cmbSearchHistory->count() > 1;

    m_btnClearHistory->setEnabled(historyNotEmpty);
    m_cmbSearchHistory->setEnabled(historyNotEmpty);
}

void AdvancedSearchDock::selectPrevResult()
{
    QTreeWidget* treeWidget = m_currentSearchInstance->m_treeWidget.get();

    QTreeWidgetItem* curr = treeWidget->currentItem();
    QTreeWidgetItem* prev = nullptr;

    if(!curr) {
        QTreeWidgetItem* lastTop = treeWidget->topLevelItem(treeWidget->topLevelItemCount()-1);
        prev = lastTop->child(lastTop->childCount()-1);
    } else if(!curr->parent()) {
        prev = curr->child(curr->childCount()-1);
    } else {
        QTreeWidgetItem* top = curr->parent();
        int prevIndex = top->indexOfChild(curr) - 1;

        if(prevIndex >= 0)
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

void AdvancedSearchDock::selectNextResult()
{
    QTreeWidget* treeWidget = m_currentSearchInstance->m_treeWidget.get();

    QTreeWidgetItem* curr = treeWidget->currentItem();
    QTreeWidgetItem* next = nullptr;

    if(!curr)
        next = treeWidget->topLevelItem(0)->child(0);
    else if(!curr->parent()) {
        next = curr->child(0);
    } else {
        QTreeWidgetItem* top = curr->parent();
        int nextIndex = top->indexOfChild(curr) + 1;

        if(nextIndex < top->childCount())
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

AdvancedSearchDock::AdvancedSearchDock()
    : QObject(nullptr),
      m_dockWidget( new QDockWidget() )
{
    QDockWidget* dockWidget = m_dockWidget.data();
    dockWidget->setWindowTitle("Advanced Search");
    dockWidget->setObjectName("advancedSearchDockWidget");

    dockWidget->setTitleBarWidget( buildTitlebarWidget() );
    dockWidget->setWidget( buildSearchPanelWidget() );

    // Titlebar connections
    connect(m_btnClearHistory, &QToolButton::clicked, this, &AdvancedSearchDock::clearHistory);
    connect(m_cmbSearchHistory, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvancedSearchDock::selectSearchFromHistory);
    connect(m_btnPrevResult, &QToolButton::clicked, this, &AdvancedSearchDock::selectPrevResult);
    connect(m_btnNextResult, &QToolButton::clicked, this, &AdvancedSearchDock::selectNextResult);
    connect(m_btnToggleReplaceOptions, &QToolButton::toggled, [this](bool checked){
        if (checked) {
            m_titlebarLayout->addLayout(m_replaceOptionsLayout);
            m_edtReplaceText->clear();
        } else
            m_titlebarLayout->removeItem(m_replaceOptionsLayout);
    });

    // Connect right-hand side buttons
    connect(m_btnClose, &QAbstractButton::clicked, dockWidget, &QDockWidget::close);
    connect(m_btnDockUndock, SIGNAL(clicked()), dockWidget, SLOT(_q_toggleTopLevel()));

    // Search panel connections
    connect(m_edtSearchTerm, &QLineEdit::textChanged, this, &AdvancedSearchDock::onUserInput);
    connect(m_edtSearchTerm, &QLineEdit::returnPressed, [this](){
        runSearch(getConfigFromInputs());
    });
    connect(m_cmbSearchScope, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvancedSearchDock::onChangeSearchScope);
    connect(m_btnSearch, &QToolButton::clicked, [this](){
        runSearch(getConfigFromInputs());
    });
    connect(m_edtSearchDirectory, &QLineEdit::textChanged, this, &AdvancedSearchDock::onUserInput);

    // "More Options" menu connections
    connect(m_actExpandAll, &QAction::toggled, [this](bool checked){
        QTreeWidget* treeWidget = m_currentSearchInstance->m_treeWidget.get();

        if(checked) treeWidget->expandAll();
        else treeWidget->collapseAll();

        m_currentSearchInstance->isMaximized = checked;
    });
    connect(m_actShowFullLines, &QAction::toggled, [this](bool checked){
        m_currentSearchInstance->setShowFullLines(checked);
    });
    connect(m_actRedoSearch, &QAction::triggered, [this](){
        setInputsFromConfig( m_currentSearchInstance->m_searchConfig );
        m_cmbSearchHistory->setCurrentIndex(0);
    });
    connect(m_actCopyContents, &QAction::triggered, [this](){
        //const SearchResult& sr = m_currentSearchInstance->searchResult;

        QString cp;

        /*for(const auto& docResult : sr.results)
            for(const auto& matchResult : docResult.results)
              cp += matchResult.m_matchLine + "\n";*/

        //This terrible abomination loops through all QTreeWidgetItems and copies their
        //contents if they are checked. This way proper item order is preserved.
        //TODO: rework
        QTreeWidget* tree = m_currentSearchInstance->m_treeWidget.get();
        for(int i=0; i<tree->topLevelItemCount(); i++) {
            for(int c=0; c<tree->topLevelItem(i)->childCount();c++) {
                QTreeWidgetItem* it = tree->topLevelItem(i)->child(c);

                if(it->checkState(0) == Qt::Checked)
                    cp += m_currentSearchInstance->resultMap[it]->m_matchLineString + '\n';
            }
        }

        if(!cp.isEmpty()) // Remove the final '\n' from the text
            cp.remove(cp.length()-1,1);

        QApplication::clipboard()->setText(cp);
    });
    connect(m_actRemoveSearch, &QAction::triggered, [this](){
        if(m_currentSearchInstance->searchInProgress) {
            const auto response = QMessageBox::warning(
                        QApplication::activeWindow(),
                        "Search in progress",
                        "<h3>This search is still in progress.</h3> " \
                        "The search will be canceled and all results discarded if you continue.",
                        QMessageBox::Ok | QMessageBox::Cancel,
                        QMessageBox::Cancel);

            if (response == QMessageBox::Cancel) return;
        }

        m_searchInstances.erase(m_searchInstances.begin() + m_cmbSearchHistory->currentIndex()-1);
        m_currentSearchInstance = nullptr;
        m_cmbSearchHistory->removeItem(m_cmbSearchHistory->currentIndex());
        onSearchHistorySizeChange();
    });

    connect(m_btnReplaceOne, &QToolButton::clicked, [this](){
        QString replaceText = m_edtReplaceText->text();

        if( !askConfirmationForReplace(replaceText, 2) )
            return;
    });

    connect(m_btnReplaceSelected, &QToolButton::clicked, [this](){
        QString replaceText = m_edtReplaceText->text();

        if( !askConfirmationForReplace(replaceText, m_currentSearchInstance->m_searchResult.countResults()) )
            return;

        ReplaceWorker* w = new ReplaceWorker(m_currentSearchInstance->m_searchResult, replaceText);

        QMessageBox* msgBox = new QMessageBox(nullptr);

        connect(w, &ReplaceWorker::resultReady, msgBox, &QMessageBox::close);
        connect(w, &ReplaceWorker::resultProgress, msgBox, [msgBox](int curr, int total) {
            msgBox->setInformativeText(QString("Matches in %1 of %2 files replaced.").arg(curr).arg(total));
        });
        connect(msgBox, &QMessageBox::buttonClicked, w, &ReplaceWorker::cancel);

        w->start();

        msgBox->setWindowTitle(QCoreApplication::applicationName());
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText("<h3>Replacing selected matches...</h3>");
        msgBox->setInformativeText("Replacing in progress");
        msgBox->setStandardButtons(QMessageBox::Cancel);
        msgBox->exec();

        w->wait();
        delete w;
        delete msgBox;
    });

    m_cmbSearchHistory->setCurrentIndex(0);
    onChangeSearchScope(0);
    onSearchHistorySizeChange();

    // Custom context menu
    /*QMenu* menu = new QMenu();
    menu->addAction("Copy Match");
    menu->addAction("Copy Line");*/

    /*connect(m_resultsTree, &QTreeWidget::customContextMenuRequested, [this, menu](const QPoint& point){
        auto* item = m_resultsTree->itemAt(point);
        if( !item || !item->parent() )
            return;

        menu->exec( m_resultsTree->mapToGlobal(point) );
    });*/
}

QDockWidget* AdvancedSearchDock::getDockWidget() const
{
    return m_dockWidget.data();
}

void AdvancedSearchDock::runSearch(const SearchConfig& cfg)
{
    if(cfg.searchString.isEmpty())
        return;

    if(m_cmbSearchScope->currentIndex() != 2) {
        QMessageBox::information(nullptr, "NYI", "Only searches in file system scope currently implemented.");
        return;
    }

    m_searchInstances.push_back( std::unique_ptr<SearchInstance>(new SearchInstance(cfg)) );

    const int idx = cfg.scope;
    QString scope = idx==0 ? "Current Document: " : idx==1 ? "All Documents: " : "File System: ";

    m_cmbSearchHistory->addItem( scope + "\"" + cfg.searchString + "\"" );
    m_cmbSearchHistory->setCurrentIndex( m_cmbSearchHistory->count()-1 );

    onSearchHistorySizeChange();
}


SearchInstance::SearchInstance(const SearchConfig& config)
    : QObject(nullptr),
      m_searchConfig(config),
      m_treeWidget(new QTreeWidget()),
      m_fileSearcher( new FileSearcher(config) )
{
    QTreeWidget* treeWidget = m_treeWidget.get();

    treeWidget->setHeaderLabel("Search Results in \"" + config.directory + "\"");
    //treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    treeWidget->setItemDelegate(new SearchTreeDelegate(treeWidget));

    QTreeWidgetItem* toplevelitem = new QTreeWidgetItem(treeWidget);
    toplevelitem->setText(0, "Calculating...");

    connect(treeWidget, &QTreeWidget::itemChanged, [](QTreeWidgetItem *item, int column){
        if(column!=0 || item->parent()) return;
        bool checked = item->checkState(0) == Qt::Checked;

        for (int i=0; i<item->childCount(); i++) {
            item->child(i)->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
        }
    });

    connect(treeWidget, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *item) {
        auto it = resultMap.find(item);
        if(it != resultMap.end())
            emit resultItemClicked( *docMap.at(item->parent()), *(it->second) );
    });

    connect(m_fileSearcher, &FileSearcher::resultProgress, this, &SearchInstance::onSearchProgress);
    connect(m_fileSearcher, &FileSearcher::resultReady, this, &SearchInstance::onSearchCompleted);
    connect(m_fileSearcher, &FileSearcher::finished, m_fileSearcher, [this](){
        m_fileSearcher->deleteLater();
        m_fileSearcher = nullptr;
    });

    //TODO only start once searchConfig etc is set
    m_fileSearcher->start();
}

SearchInstance::~SearchInstance()
{
    if(m_fileSearcher) m_fileSearcher->cancel();
}

void SearchInstance::setShowFullLines(bool showFullLines)
{
    m_showFullLines = showFullLines;

    if(searchInProgress)
        return;

    for (auto& item : resultMap) {
        QTreeWidgetItem* treeItem = item.first;
        const MatchResult& res = *item.second;
        treeItem->setText(0, getFormattedResultText(res, m_showFullLines));
    }
    // TODO: This doesn't actually resize the widget view area.
    //m_treeWidget->resizeColumnToContents(0);
}

void SearchInstance::onSearchProgress(int processed, int total)
{
    m_treeWidget->topLevelItem(0)->setText(0,QString("Search in progress [%1/%2 finished]").arg(processed).arg(total));
}

void SearchInstance::onSearchCompleted()
{
    searchInProgress = false;

    // FileSearcher is not needed once we got the result. Thus we can move out of it and delete it asap.
    m_searchResult = std::move(m_fileSearcher->getResult());
    delete m_fileSearcher;
    m_fileSearcher = nullptr;

    m_treeWidget->setHeaderLabel("Search Results in \""
                                 + m_searchConfig.directory
                                 + "\" (completed in "
                                 + QString::number(m_searchResult.m_timeToComplete)
                                 + "ms)");

    m_treeWidget->clear();
    for (const auto& doc : m_searchResult.results) {
        QTreeWidgetItem* toplevelitem = new QTreeWidgetItem(m_treeWidget.get());
        toplevelitem->setText(0, getFormattedLocationText(doc, m_searchConfig.directory));
        toplevelitem->setCheckState(0, Qt::Checked);
        docMap[toplevelitem] = &doc;

        for (const auto& res : doc.results) {
            QTreeWidgetItem* it = new QTreeWidgetItem(toplevelitem);
            it->setText(0, getFormattedResultText(res, m_showFullLines));
            it->setCheckState(0, Qt::Checked);
            resultMap[it] = &res;
        }
    }

    emit searchCompleted();
}
