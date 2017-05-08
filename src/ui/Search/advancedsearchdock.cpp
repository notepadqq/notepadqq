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
#include <QFileDialog>

// For the clipboard
#include <QApplication>
#include <QClipboard>

// For the delegate
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QDir>

#include "include/nqqsettings.h"
#include "include/iconprovider.h"
#include "include/Search/searchstring.h"
#include "include/Search/replaceworker.h"

/**
 * @brief addUniqueToList Adds the given item to the given list. Also removes duplicates from list.
 *        This is just a helper used in the updateXHistory functions.
 * @return The updated list, with item added and duplicates removed.
 */
QStringList addUniqueToList(QStringList list, const QString& item) {
    list.prepend(item);
    list.removeDuplicates();
    list = list.mid(0, 10);
    return list;
}

/**
 * @brief getFormattedLocationText Creates a html-formatted string to use as the text of a toplevel QTreeWidget item.
 * @param docResult The DocResult to grab the information from
 * @param searchLocation The base directory that was searched
 * @return The html-formatted string
 */
QString getFormattedLocationText(const DocResult& docResult, const QString& searchLocation) {
    const int commonPathLength = searchLocation.length();
    const QString relativePath = docResult.fileName.mid(commonPathLength);

    return QString("<span style='white-space:pre-wrap;'><b>%1</b> Results for:   '<b>%2</b>'</span>")
            .arg(docResult.results.size(), 4) // Pad the number so all rows line up nicely.
            .arg(relativePath.toHtmlEscaped());
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
 * @brief askConfirmationForReplace Shows a blocking QMessageBox asking whether a user wants to go through with a
 *                                  replacement action.
 * @param replaceText The text that will be used to replace all match instances.
 * @param numReplacements The number of instances that will be replaced.
 * @return True if the user wants to continue. False otherwise.
 */
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

/**
 * @brief makeDivider Creates a QFrame that acts as a visual dividing line
 * @param shape Either QFrame::HLine or QFrame::VLine to determine the line's orientation
 * @param length Sets the maximum length of the divider.
 */
QFrame* makeDivider(QFrame::Shape shape, int length=0) {
    QFrame* line = new QFrame();
    line->setFrameShape(shape);
    line->setFrameShadow(QFrame::Sunken);
    if(length > 0) {
        if(shape == QFrame::VLine)
            line->setMaximumHeight(length);
        else
            line->setMaximumWidth(length);
    }
    return line;
}

QLayout* AdvancedSearchDock::buildLeftTitlebar() {

    QLabel* label = new QLabel("Advanced Search");
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
    layout->addWidget(makeDivider(QFrame::VLine));
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
    NqqSettings& settings = NqqSettings::getInstance();

    m_replaceOptionsLayout = new QVBoxLayout;

    // Add the bar with replace options.
    QHBoxLayout* replaceOptions = new QHBoxLayout;

    m_cmbReplaceText = new QComboBox;
    m_cmbReplaceText->setEditable(true);
    m_cmbReplaceText->lineEdit()->setPlaceholderText("Replace Text");
    m_cmbReplaceText->setMaximumWidth(300);
    m_cmbReplaceText->setMinimumWidth(300);
    m_cmbReplaceText->lineEdit()->setClearButtonEnabled(true);
    m_cmbReplaceText->addItems(settings.Search.getReplaceHistory());
    m_cmbReplaceText->setCurrentText("");

    m_btnReplaceSelected = new QToolButton;
    m_btnReplaceSelected->setText("Replace Selected");
    m_btnReplaceSelected->setToolTip("Replaces all selected search results.");

    m_chkReplaceWithSpecialChars = new QCheckBox("Use Special Characters ('\\n', '\\t', ...)");
    m_chkReplaceWithSpecialChars->setToolTip("Replaces strings like '\\n' with their corresponding special characters.");

    replaceOptions->addWidget(m_cmbReplaceText);
    replaceOptions->addWidget(m_btnReplaceSelected);
    replaceOptions->addWidget(m_chkReplaceWithSpecialChars);
    replaceOptions->setSizeConstraint(QHBoxLayout::SetNoConstraint);

    m_replaceOptionsLayout->addWidget(makeDivider(QFrame::HLine));
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
    NqqSettings& settings = NqqSettings::getInstance();

    QGridLayout* gl = new QGridLayout;

    m_cmbSearchTerm = new QComboBox;
    m_cmbSearchTerm->setEditable(true);
    m_cmbSearchTerm->lineEdit()->setPlaceholderText("Search String");
    m_cmbSearchTerm->setMaximumWidth(300);
    m_cmbSearchTerm->lineEdit()->setClearButtonEnabled(true);
    m_cmbSearchTerm->addItems(settings.Search.getSearchHistory());
    m_cmbSearchTerm->setCurrentText("");

    m_cmbSearchScope = new QComboBox;
    m_cmbSearchScope->addItem("Search in Current Document");
    m_cmbSearchScope->addItem("Search in All Open Documents");
    m_cmbSearchScope->addItem("Search in Files on File System");
    m_cmbSearchScope->setMaximumWidth(260);

    m_cmbSearchPattern = new QComboBox;
    m_cmbSearchPattern->setEditable(true);
    m_cmbSearchPattern->lineEdit()->setPlaceholderText("*ext1, *ext2");
    m_cmbSearchPattern->setMaximumWidth(300);
    m_cmbSearchPattern->lineEdit()->setClearButtonEnabled(true);
    m_cmbSearchPattern->addItems(settings.Search.getFilterHistory());
    m_cmbSearchPattern->setCurrentText("");

    m_cmbSearchDirectory = new QComboBox;
    m_cmbSearchDirectory->setEditable(true);
    m_cmbSearchDirectory->lineEdit()->setPlaceholderText("Directory");
    m_cmbSearchDirectory->setMaximumWidth(260);
    m_cmbSearchDirectory->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_cmbSearchDirectory->lineEdit()->setClearButtonEnabled(true);
    m_cmbSearchDirectory->addItems(settings.Search.getFileHistory());
    m_cmbSearchDirectory->setCurrentText("");

    m_btnSelectSearchDirectory = new QToolButton;
    m_btnSelectSearchDirectory->setIcon(IconProvider::fromTheme("edit-find"));
    m_btnSelectSearchDirectory->setToolTip("Select Search Directory");

    QHBoxLayout* m2 = new QHBoxLayout;
    m2->addWidget(m_cmbSearchDirectory);
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

    m_chkMatchCase = new QCheckBox("Match Case");
    m_chkMatchWords = new QCheckBox("Match Whole Words Only");
    m_chkUseRegex = new QCheckBox("Use Regular Expressions");
    m_chkUseSpecialChars = new QCheckBox("Use Special Characters ('\\t', '\\n', ...)");
    m_chkUseSpecialChars->setToolTip("If set, character sequences like '\\t' will be replaced by their corresponding special characters.");
    m_chkIncludeSubdirs = new QCheckBox("Include Subdirectories");

    QVBoxLayout* mini = new QVBoxLayout;
    mini->addWidget(m_chkMatchCase);
    mini->addWidget(m_chkMatchWords);
    mini->addWidget(m_chkUseRegex);
    mini->addWidget(m_chkUseSpecialChars);
    mini->addWidget(makeDivider(QFrame::HLine, 180));
    mini->addWidget(m_chkIncludeSubdirs);

    gl->addWidget(srl, 0,0);
    gl->addWidget(scl, 1,0);
    gl->addWidget(srd, 2,0);
    gl->addWidget(srp, 3,0);

    gl->addWidget(m_cmbSearchTerm, 0,1);
    gl->addWidget(m_cmbSearchScope, 1,1);
    gl->addLayout(m2, 2,1);
    gl->addWidget(m_cmbSearchPattern, 3,1);


    gl->addLayout(mini, 0, 3, 4, 1);
    gl->addWidget(m_btnSearch, 3,2);

    gl->setSizeConstraint(QGridLayout::SetNoConstraint);

    // Put the gridview into a vbox with a strecher so the gridview doesn't strech when the panelwidget is resized.
    QVBoxLayout* top = new QVBoxLayout();
    top->addWidget( makeDivider(QFrame::HLine) );
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
        return inst->isSearchInProgress();
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

    // TODO: Clean this up, move stuff into a connect/disconnect function, something like that.
    if(m_currentSearchInstance && m_currentSearchInstance->isSearchInProgress()) {
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
        m_cmbSearchTerm->setFocus();
    } else {
        m_currentSearchInstance = m_searchInstances[index-1].get();

        if(m_currentSearchInstance->isSearchInProgress()) {
            connect(m_currentSearchInstance, &SearchInstance::searchCompleted,
                       this, &AdvancedSearchDock::onCurrentSearchInstanceCompleted);
        }

        connect(m_currentSearchInstance, &SearchInstance::resultItemClicked,
                   this, &AdvancedSearchDock::resultItemClicked);

        m_dockWidget->setWidget( m_currentSearchInstance->getResultTreeWidget() );
        m_btnToggleReplaceOptions->setEnabled(true);
        m_btnMoreOptions->setEnabled(true);
        m_btnPrevResult->setEnabled(true);
        m_btnNextResult->setEnabled(true);
        m_actExpandAll->setChecked( m_currentSearchInstance->areResultsExpanded() );
        m_actShowFullLines->setChecked( m_currentSearchInstance->getShowFullLines() );

        updateSearchInProgressUi();
    }
}

void AdvancedSearchDock::onChangeSearchScope(int index)
{
    switch(index) {
    case 0: // Search current document
    case 1: // Search all open documents
        m_cmbSearchPattern->setEnabled(false);
        m_cmbSearchDirectory->setEnabled(false);
        m_btnSelectSearchDirectory->setEnabled(false);
        m_chkIncludeSubdirs->setEnabled(false);
        break;
    case 2: // Search in file system
        m_cmbSearchPattern->setEnabled(true);
        m_cmbSearchDirectory->setEnabled(true);
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
    // Dis- or enable the "Search" button depending on whether the necessary fields are filled out or not.
    switch(m_cmbSearchScope->currentIndex()) {
    case SearchConfig::ScopeCurrentDocument:
    case SearchConfig::ScopeAllOpenDocuments:
        m_btnSearch->setEnabled(!m_cmbSearchTerm->currentText().isEmpty());
        break;
    case SearchConfig::ScopeFileSystem:
        m_btnSearch->setEnabled(!m_cmbSearchTerm->currentText().isEmpty() &&
                                !m_cmbSearchDirectory->currentText().isEmpty());
        break;
    }
}

void AdvancedSearchDock::updateSearchInProgressUi()
{
    const bool progress = m_currentSearchInstance->isSearchInProgress();

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
    config.directory = m_cmbSearchDirectory->currentText();
    config.filePattern = m_cmbSearchPattern->currentText();
    config.searchString = m_cmbSearchTerm->currentText();
    config.setScopeFromInt(m_cmbSearchScope->currentIndex());

    config.matchCase = m_chkMatchCase->isChecked();
    config.matchWord = m_chkMatchWords->isChecked();
    config.searchMode = SearchConfig::ModePlainText;
    if(m_chkUseSpecialChars->isChecked()) config.searchMode = SearchConfig::ModePlainTextSpecialChars;
    else if(m_chkUseRegex->isChecked()) config.searchMode = SearchConfig::ModeRegex;
    config.includeSubdirs = m_chkIncludeSubdirs->isChecked();


    // TODO: Dummy config
/*    config.searchString = "Test\\tMe";
    config.filePattern = "";
    config.directory = "/home/s3rius/dev/nqqtest";
    config.matchWord = false;
    config.matchCase = false;
    config.includeSubdirs = true;
    config.useRegex = true;
    config.useSpecialChars = false;*/

    return config;
}

void AdvancedSearchDock::setInputsFromConfig(const SearchConfig& config)
{
    m_cmbSearchDirectory->setCurrentText( config.directory );
    m_cmbSearchPattern->setCurrentText( config.filePattern );
    m_cmbSearchTerm->setCurrentText( config.searchString );
    m_cmbSearchScope->setCurrentIndex( config.searchScope );

    m_chkMatchCase->setChecked( config.matchCase );
    m_chkMatchWords->setChecked( config.matchWord );
    m_chkUseRegex->setChecked( config.searchMode == SearchConfig::ModeRegex );
    m_chkUseSpecialChars->setChecked( config.searchMode == SearchConfig::ModePlainTextSpecialChars );
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
    if(m_currentSearchInstance) m_currentSearchInstance->selectPreviousResult();
}

void AdvancedSearchDock::selectNextResult()
{
    if(m_currentSearchInstance) m_currentSearchInstance->selectNextResult();
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
            m_cmbReplaceText->setCurrentText("");
        } else
            m_titlebarLayout->removeItem(m_replaceOptionsLayout);
    });

    // Connect right-hand side buttons
    connect(m_btnClose, &QAbstractButton::clicked, dockWidget, &QDockWidget::close);
    connect(m_btnDockUndock, SIGNAL(clicked()), dockWidget, SLOT(_q_toggleTopLevel()));

    // Search panel connections
    connect(m_cmbSearchTerm->lineEdit(), &QLineEdit::textChanged, this, &AdvancedSearchDock::onUserInput);
    connect(m_cmbSearchTerm->lineEdit(), &QLineEdit::returnPressed, [this](){
        runSearch(getConfigFromInputs());
    });
    connect(m_cmbSearchScope, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvancedSearchDock::onChangeSearchScope);
    connect(m_btnSearch, &QToolButton::clicked, [this](){
        runSearch(getConfigFromInputs());
    });
    connect(m_cmbSearchDirectory->lineEdit(), &QLineEdit::textChanged, this, &AdvancedSearchDock::onUserInput);
    connect(m_cmbSearchDirectory->lineEdit(), &QLineEdit::returnPressed, [this](){
        runSearch(getConfigFromInputs());
    });
    connect(m_btnSelectSearchDirectory, &QToolButton::clicked, [this](){
        QString defaultDir = m_cmbSearchDirectory->currentText();
        if (defaultDir.isEmpty()) {
            defaultDir = NqqSettings::getInstance().General.getLastSelectedDir();
        }

        QString dir = QFileDialog::getExistingDirectory(QApplication::activeWindow(),
                                                        QObject::tr("Search in..."), defaultDir,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly |
                                                        QFileDialog::DontResolveSymlinks);

        if (!dir.isEmpty()) {
            m_cmbSearchDirectory->setCurrentText(dir);
        }
    });

    connect(m_chkUseRegex, &QCheckBox::toggled, [this](bool checked){
        m_chkUseSpecialChars->setEnabled(!checked);
        if(checked) m_chkUseSpecialChars->setChecked(false);
    });
    connect(m_chkUseSpecialChars, &QCheckBox::toggled, [this](bool checked){
        m_chkUseRegex->setEnabled(!checked);
        if(checked) m_chkUseRegex->setChecked(false);
    });

    // "More Options" menu connections
    connect(m_actExpandAll, &QAction::toggled, [this](bool checked){
        if(checked) m_currentSearchInstance->expandAllResults();
        else m_currentSearchInstance->collapseAllResults();
    });
    connect(m_actShowFullLines, &QAction::toggled, [this](bool checked){
        m_currentSearchInstance->showFullLines(checked);
    });
    connect(m_actRedoSearch, &QAction::triggered, [this](){
        setInputsFromConfig( m_currentSearchInstance->getSearchConfig() );
        m_cmbSearchHistory->setCurrentIndex(0);
    });
    connect(m_actCopyContents, &QAction::triggered, [this](){
        m_currentSearchInstance->copySelectedLinesToClipboard();
    });
    connect(m_actRemoveSearch, &QAction::triggered, [this](){
        if(m_currentSearchInstance->isSearchInProgress()) {
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

    connect(m_btnReplaceSelected, &QToolButton::clicked, [this](){
        QString replaceText = m_cmbReplaceText->currentText();

        if( !askConfirmationForReplace(replaceText, m_currentSearchInstance->getSearchResult().countResults()) )
            return;

        updateReplaceHistory(replaceText);

        if (m_chkReplaceWithSpecialChars->isChecked())
            replaceText = SearchString::unescape(replaceText);

        ReplaceWorker* w = new ReplaceWorker(m_currentSearchInstance->getSearchResult(), replaceText);

        QMessageBox* msgBox = new QMessageBox(QApplication::activeWindow());

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

        const bool success = !w->hasErrors();

        if (!success) {
            const QVector<QString>& errors = w->getErrors();
            const int numErrors = errors.size();
            const int maxCount = std::min(numErrors, 8);
            QString errorString = QString("Replacing was unsuccessful for %1 file(s):\n").arg(numErrors);

            for(int i=0; i<maxCount; i++) {
                errorString += "\"" + errors[i] + "\"\n";
            }
            if(numErrors > 8)
                errorString += QString("And %1 more.").arg(numErrors-8);

            QMessageBox::warning(QApplication::activeWindow(),
                                 "Replace Results", errorString, QMessageBox::Ok);
        } else {
            QMessageBox::information(QApplication::activeWindow(),
                                     "Replace Results", "All selected matches successfully replaced.", QMessageBox::Ok);
        }

        delete w;
        delete msgBox;
    });

    m_cmbSearchHistory->setCurrentIndex(0);
    onChangeSearchScope(0); // Initializes the status of the search panel
    selectSearchFromHistory(0); // Initializes the status of the search history combo box
    onSearchHistorySizeChange(); // Initializes the status of the remaining title bar (Prev/Next buttons etc)

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

void AdvancedSearchDock::updateSearchHistory(const QString& item) {
    if(item.isEmpty()) return;

    NqqSettings& settings = NqqSettings::getInstance();
    auto history = addUniqueToList(settings.Search.getSearchHistory(), item);
    settings.Search.setSearchHistory(history);
    m_cmbSearchTerm->clear();
    m_cmbSearchTerm->addItems(history);
}

void AdvancedSearchDock::updateReplaceHistory(const QString& item) {
    if(item.isEmpty()) return;

    NqqSettings& settings = NqqSettings::getInstance();
    auto history = addUniqueToList(settings.Search.getReplaceHistory(), item);
    settings.Search.setReplaceHistory(history);
    m_cmbReplaceText->clear();
    m_cmbReplaceText->addItems(history);
}

void AdvancedSearchDock::updateDirectoryhHistory(const QString& item) {
    if(item.isEmpty()) return;

    NqqSettings& settings = NqqSettings::getInstance();
    auto history = addUniqueToList(settings.Search.getFileHistory(), item);
    settings.Search.setFileHistory(history);
    m_cmbSearchDirectory->clear();
    m_cmbSearchDirectory->addItems(history);
}

void AdvancedSearchDock::updateFilterHistory(const QString& item) {
    if(item.isEmpty()) return;

    NqqSettings& settings = NqqSettings::getInstance();
    auto history = addUniqueToList(settings.Search.getFilterHistory(), item);
    settings.Search.setFilterHistory(history);
    m_cmbSearchPattern->clear();
    m_cmbSearchPattern->addItems(history);
}

void AdvancedSearchDock::runSearch(SearchConfig cfg)
{
    if (cfg.searchString.isEmpty())
        return;

    const SearchConfig::SearchScope scope = cfg.searchScope;

    if (scope == SearchConfig::ScopeFileSystem) {
        // If we're searching the file system, check that the search dir is not empty and actually exists
        if (cfg.directory.isEmpty()) return;

        QDir dir(cfg.directory);

        if (!dir.exists()) {
            QMessageBox::warning(QApplication::activeWindow(), "Error",
                                 "Specified directory does not exist.", QMessageBox::Ok);
            return;
        }

        cfg.directory = dir.absolutePath(); // Also cleans path from multiple separators or "..", ".", etc.
    }

    // TODO: Implement other search types
    if(scope != SearchConfig::ScopeFileSystem) {
        QMessageBox::information(nullptr, "NYI", "Only searches in file system scope currently implemented.");
        return;
    }

    // Update history
    updateSearchHistory(cfg.searchString);
    if (scope == SearchConfig::ScopeFileSystem) {
        updateDirectoryhHistory(cfg.directory);
        updateFilterHistory(cfg.filePattern);
    }


    m_searchInstances.push_back( std::unique_ptr<SearchInstance>(new SearchInstance(cfg)) );

    m_cmbSearchHistory->addItem( cfg.getScopeAsString() + ": \"" + cfg.searchString + "\"" );
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
        auto it = m_resultMap.find(item);
        if(it != m_resultMap.end())
            emit resultItemClicked( *m_docMap.at(item->parent()), *(it->second) );
    });

    connect(m_fileSearcher, &FileSearcher::resultProgress, this, &SearchInstance::onSearchProgress);
    connect(m_fileSearcher, &FileSearcher::resultReady, this, &SearchInstance::onSearchCompleted);
    connect(m_fileSearcher, &FileSearcher::finished, m_fileSearcher, [this](){
        m_fileSearcher->deleteLater();
        m_fileSearcher = nullptr;
    });

    m_fileSearcher->start();
}

SearchInstance::~SearchInstance()
{
    // After canceling, m_fileSearcher is deleted through a signal connected in SearchInstance's constructor
    if(m_fileSearcher) m_fileSearcher->cancel();
}

void SearchInstance::showFullLines(bool showFullLines)
{
    if(m_showFullLines == showFullLines)
        return;

    m_showFullLines = showFullLines;

    if(m_isSearchInProgress)
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
    m_resultsAreExpanded = true;
}

void SearchInstance::selectNextResult()
{
    QTreeWidget* treeWidget = getResultTreeWidget();

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

void SearchInstance::selectPreviousResult()
{
    QTreeWidget* treeWidget = getResultTreeWidget();

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

    if(!cp.isEmpty()) // Remove the final '\n' from the text
        cp.remove(cp.length()-1,1);

    QApplication::clipboard()->setText(cp);
}

void SearchInstance::onSearchProgress(int processed, int total)
{
    m_treeWidget->topLevelItem(0)->setText(0,QString("Search in progress [%1/%2 finished]").arg(processed).arg(total));
}

void SearchInstance::onSearchCompleted()
{
    m_isSearchInProgress = false;

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
