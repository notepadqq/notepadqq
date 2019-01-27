#include "include/Search/advancedsearchdock.h"

#include "include/EditorNS/editor.h"
#include "include/Search/filereplacer.h"
#include "include/Search/searchstring.h"
#include "include/iconprovider.h"
#include "include/mainwindow.h"
#include "include/nqqsettings.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

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
 * @brief askConfirmationForReplace Shows a blocking QMessageBox asking whether a user wants to go through with a
 *                                  replacement action.
 * @param replaceText The text that will be used to replace all match instances.
 * @param numReplacements The number of instances that will be replaced.
 * @return True if the user wants to continue. False otherwise.
 */
bool askConfirmationForReplace(QString replaceText, int numReplacements) {
    return QMessageBox::information(QApplication::activeWindow(),
                                    QObject::tr("Confirm Replacement"),
                                    QObject::tr("This will replace %1 selected matches with \"%2\". Continue?")
                                    .arg(numReplacements)
                                    .arg(replaceText),
                                    QMessageBox::Ok | QMessageBox::Cancel,
                                    QMessageBox::Cancel) == QMessageBox::Ok;
}


QSearchDockTitleButton::QSearchDockTitleButton(QDockWidget *dockWidget)
    : QAbstractButton(dockWidget)
{
    setFocusPolicy(Qt::NoFocus);
}

QSize QSearchDockTitleButton::sizeHint() const
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

void QSearchDockTitleButton::enterEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::enterEvent(event);
}

void QSearchDockTitleButton::leaveEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::leaveEvent(event);
}

void QSearchDockTitleButton::paintEvent(QPaintEvent* /*evt*/)
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

/**
 * @brief makeDivider Returns a QFrame that acts as a visual dividing line
 * @param shape Either QFrame::HLine or QFrame::VLine to determine the line's orientation
 * @param length Sets the maximum length of the divider.
 */
QFrame* makeDivider(QFrame::Shape shape, int length=0) {
    QFrame* line = new QFrame();
    line->setFrameShape(shape);
    line->setFrameShadow(QFrame::Sunken);
    if (length > 0) {
        if (shape == QFrame::VLine)
            line->setMaximumHeight(length);
        else
            line->setMaximumWidth(length);
    }
    return line;
}

void showRegexInfo() {
    QString str;

    str =
    QObject::tr("Notepadqq supports most of the <a href='http://perldoc.perl.org/perlre.html'>Perl Regular Expression</a> syntax when 'Use Regular Expressions' is checked.") + "<br>" +
    QObject::tr("Here is a quick reference:") + "<br>"
    "<table>"
    "<tr><td width=20%><b>\\w</b></td><td>" + QObject::tr("Matches a word character") + "</td></tr>"
    "<tr><td><b>\\d</b></td><td>" + QObject::tr("Matches a 0-9 digit") + "</td></tr>"
    "<tr><td><b>[abc]</b></td><td>" + QObject::tr("Matches any of a, b, or c") + "</td></tr>"
    "<tr><td><b>[^abc]</b></td><td>" + QObject::tr("Matches anything but a, b, or c") + "</td></tr>"
    "<tr><td><b>^</b></td><td>" + QObject::tr("Matches the beginning of a line") + "</td></tr>"
    "<tr><td><b>$</b></td><td>" + QObject::tr("Matches the end of a line") + "</td></tr>"
    "<tr><td><b>(abc)</b></td><td>" + QObject::tr("Matches 'abc' and captures it as a group") + "</td></tr>"
    "<tr><td><b>\\n</b></td><td>" + QObject::tr("Use in a replace operation to refer to the n'th capture group") + "</td></tr>"
    "</table>";

    QMessageBox::information(nullptr, "", str);
}

QLayout* AdvancedSearchDock::buildLeftTitlebar() {

    QLabel* label = new QLabel(tr("Advanced Search"));
    label->setMaximumWidth(label->fontMetrics().width(label->text()));

    m_btnClearHistory = new QToolButton;
    m_btnClearHistory->setIcon(IconProvider::fromTheme("edit-clear"));
    m_btnClearHistory->setToolTip(tr("Clear Search History"));

    m_cmbSearchHistory = new QComboBox;
    m_cmbSearchHistory->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_cmbSearchHistory->setMinimumWidth(120);
    m_cmbSearchHistory->setMaximumWidth(300);

    m_btnPrevResult = new QToolButton;
    m_btnPrevResult->setIcon(IconProvider::fromTheme("go-previous"));
    m_btnPrevResult->setToolTip(tr("Go To Previous Result (Shift+F4)"));
    m_btnPrevResult->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F4));

    m_btnNextResult = new QToolButton;
    m_btnNextResult->setIcon(IconProvider::fromTheme("go-next"));
    m_btnNextResult->setToolTip(tr("Go To Next Result (F4)"));
    m_btnNextResult->setShortcut(QKeySequence(Qt::Key_F4));

    QMenu* menu = new QMenu();
    m_actExpandAll = menu->addAction(tr("Expand/Collapse All"));
    m_actExpandAll->setCheckable(true);
    m_actRedoSearch = menu->addAction(tr("Redo Search"));
    m_actCopyContents = menu->addAction(tr("Copy Selected Contents To Clipboard"));
    m_actShowFullLines = menu->addAction(tr("Show Full Lines"));
    m_actShowFullLines->setCheckable(true);
    m_actRemoveSearch= menu->addAction(tr("Remove This Search"));

    m_btnMoreOptions = new QToolButton;
    m_btnMoreOptions->setIcon(IconProvider::fromTheme("preferences-other"));
    m_btnMoreOptions->setVisible(false);
    m_btnMoreOptions->setToolTip(tr("More Options"));
    m_btnMoreOptions->setPopupMode(QToolButton::InstantPopup);
    m_btnMoreOptions->setMenu(menu);

    m_btnToggleReplaceOptions = new QToolButton;
    m_btnToggleReplaceOptions->setText(tr("Replace Options"));
    m_btnToggleReplaceOptions->setCheckable(true);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(label);
    layout->addWidget(m_btnClearHistory);
    layout->addWidget(m_cmbSearchHistory);
    layout->addSpacerItem(new QSpacerItem(40, 1, QSizePolicy::Fixed, QSizePolicy::Minimum));
    layout->addWidget(m_btnPrevResult);
    layout->addWidget(m_btnNextResult);
    layout->addWidget(m_btnMoreOptions);
    layout->addWidget(m_btnToggleReplaceOptions);
    layout->setSizeConstraint(QHBoxLayout::SetNoConstraint);

    return layout;
}

QLayout* AdvancedSearchDock::buildUpperTitlebarLayout() {
    QHBoxLayout* top = new QHBoxLayout;

    // Construct left side. Buttons etc.
    QLayout* leftSide = buildLeftTitlebar();

    // Replacing a dock's titlebar means we have to add the top-right button manually
    m_btnDockUndock = new QSearchDockTitleButton(m_dockWidget.data());
    m_btnDockUndock->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarNormalButton));
    m_btnDockUndock->setToolTip(tr("Dock/Undock this panel"));

    m_btnClose = new QSearchDockTitleButton(m_dockWidget.data());
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

    // Create the bar with replace options.
    QHBoxLayout* replaceOptions = new QHBoxLayout;

    m_cmbReplaceText = new QComboBox;
    m_cmbReplaceText->setEditable(true);
    m_cmbReplaceText->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_cmbReplaceText->completer()->setCaseSensitivity(Qt::CaseSensitive);
    m_cmbReplaceText->lineEdit()->setPlaceholderText(tr("Replace Text"));
    m_cmbReplaceText->setMaximumWidth(300);
    m_cmbReplaceText->setMinimumWidth(300);
    m_cmbReplaceText->lineEdit()->setClearButtonEnabled(true);
    m_cmbReplaceText->addItems(settings.Search.getReplaceHistory());
    m_cmbReplaceText->setCurrentText("");

    m_btnReplaceSelected = new QToolButton;
    m_btnReplaceSelected->setText(tr("Replace Selected"));
    m_btnReplaceSelected->setToolTip(tr("Replace all selected search results."));

    m_chkReplaceWithSpecialChars = new QCheckBox(tr("Use Special Characters ('\\n', '\\t', ...)"));
    m_chkReplaceWithSpecialChars->setToolTip(tr("Replace strings like '\\n' with their respective special characters."));

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
    /*QLayout* lowerBar =*/ buildReplaceOptionsLayout(); // <- Hidden by default, aka not added yet

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
    m_cmbSearchTerm->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_cmbSearchTerm->completer()->setCaseSensitivity(Qt::CaseSensitive);
    m_cmbSearchTerm->lineEdit()->setPlaceholderText(tr("Search String"));
    m_cmbSearchTerm->setMaximumWidth(300);
    m_cmbSearchTerm->lineEdit()->setClearButtonEnabled(true);
    m_cmbSearchTerm->addItems(settings.Search.getSearchHistory());
    m_cmbSearchTerm->setCurrentText("");

    m_cmbSearchScope = new QComboBox;
    m_cmbSearchScope->addItem(tr("Search in Current Document"));
    m_cmbSearchScope->addItem(tr("Search in All Open Documents"));
    m_cmbSearchScope->addItem(tr("Search in Files on File System"));
    m_cmbSearchScope->setMaximumWidth(260);

    m_cmbSearchPattern = new QComboBox;
    m_cmbSearchPattern->setEditable(true);
    m_cmbSearchPattern->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_cmbSearchPattern->completer()->setCaseSensitivity(Qt::CaseSensitive);
    m_cmbSearchPattern->lineEdit()->setPlaceholderText("*ext1, *ext2");
    m_cmbSearchPattern->setMaximumWidth(300);
    m_cmbSearchPattern->lineEdit()->setClearButtonEnabled(true);
    m_cmbSearchPattern->addItems(settings.Search.getFilterHistory());
    m_cmbSearchPattern->setCurrentText("");

    m_cmbSearchDirectory = new QComboBox;
    m_cmbSearchDirectory->setEditable(true);
    m_cmbSearchDirectory->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_cmbSearchDirectory->completer()->setCaseSensitivity(Qt::CaseSensitive);
    m_cmbSearchDirectory->lineEdit()->setPlaceholderText(tr("Directory"));
    m_cmbSearchDirectory->setMaximumWidth(260);
    m_cmbSearchDirectory->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_cmbSearchDirectory->lineEdit()->setClearButtonEnabled(true);
    m_cmbSearchDirectory->addItems(settings.Search.getFileHistory());
    m_cmbSearchDirectory->setCurrentText("");

    m_btnSelectCurrentDirectory = new QToolButton;
    m_btnSelectCurrentDirectory->setIcon(IconProvider::fromTheme("go-bottom"));
    m_btnSelectCurrentDirectory->setToolTip(tr("Select the directory of the active document"));

    m_btnSelectSearchDirectory = new QToolButton;
    m_btnSelectSearchDirectory->setIcon(IconProvider::fromTheme("edit-find"));
    m_btnSelectSearchDirectory->setToolTip(tr("Select search directory"));

    QHBoxLayout* m2 = new QHBoxLayout;
    m2->addWidget(m_cmbSearchDirectory);
    m2->addWidget(m_btnSelectSearchDirectory);

    m_btnSearch = new QToolButton;
    m_btnSearch->setText(tr("Search"));
    m_btnSearch->setEnabled(false);

    QLabel* scl = new QLabel(tr("Scope:"));
    scl->setMaximumWidth(80);
    QLabel* srl = new QLabel(tr("Search:"));
    srl->setMaximumWidth(80);
    QLabel* srp = new QLabel(tr("Pattern:"));
    srp->setMaximumWidth(80);
    QLabel* srd = new QLabel(tr("Location:"));
    srd->setMaximumWidth(80);

    m_chkMatchCase = new QCheckBox(tr("Match Case"));
    m_chkMatchWords = new QCheckBox(tr("Match Whole Words Only"));
    m_chkUseRegex = new QCheckBox(tr("Use Regular Expressions"));
    m_chkUseSpecialChars = new QCheckBox(tr("Use Special Characters ('\\t', '\\n', ...)"));
    m_chkUseSpecialChars->setToolTip(tr("If set, character sequences like '\\t' will be replaced by their respective special characters."));
    m_chkIncludeSubdirs = new QCheckBox(tr("Include Subdirectories"));
    m_chkIncludeSubdirs->setChecked(true);

    m_chkMatchCase->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    m_chkMatchWords->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    m_chkUseRegex->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    m_chkUseSpecialChars->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    m_chkIncludeSubdirs->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    QGridLayout* mini = new QGridLayout;
    mini->addWidget(m_chkMatchCase, 0, 0);
    mini->addWidget(m_chkMatchWords, 1, 0);
    mini->addWidget(m_chkUseRegex, 2, 0);
    mini->addWidget(m_chkUseSpecialChars, 3, 0);
    mini->addWidget(makeDivider(QFrame::HLine, 180), 4, 0);
    mini->addWidget(m_chkIncludeSubdirs, 5, 0);
    mini->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding), 6, 0);

    QLabel* regexInfo = new QLabel("(<a href='info'>?</a>)");
    QObject::connect(regexInfo, &QLabel::linkActivated, &showRegexInfo);
    mini->addWidget(regexInfo, 2, 1);

    gl->addWidget(srl, 0,0);
    gl->addWidget(scl, 1,0);
    gl->addWidget(srd, 2,0);
    gl->addWidget(srp, 3,0);

    gl->addWidget(m_cmbSearchTerm, 0,1);
    gl->addWidget(m_btnSearch, 0,2);
    gl->addWidget(m_cmbSearchScope, 1,1);
    gl->addLayout(m2, 2,1);
    gl->addWidget(m_cmbSearchPattern, 3,1);

    gl->addLayout(mini, 0, 3, 4, 1);
    gl->addWidget(m_btnSelectCurrentDirectory, 2, 2);

    gl->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding), 4, 0);


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

    if (anySearchInProgess) {
        QMessageBox msgBox(nullptr);
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("<h3>One or more searches are still in progress</h3>"));
        msgBox.setInformativeText(tr("All searches will be canceled and their results discarded if you continue."));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setEscapeButton(QMessageBox::Cancel);
        msgBox.exec();

        if (msgBox.standardButton(msgBox.clickedButton()) == QMessageBox::Cancel)
            return;
    }

    // First clear the history, then the SearchInstance array. Necessary because changing history calls
    // selectSearchFromHistory() which wants to disconnect connections from the currently active SearchInstance.
    // Clearing the SearchInstance array first invalidates the pointers.
    m_cmbSearchHistory->clear();
    m_cmbSearchHistory->addItem(tr("New Search"));
    m_searchInstances.clear();
    onSearchHistorySizeChange();
}

void AdvancedSearchDock::selectSearchFromHistory(int index)
{
    if (index==-1)
        return;

    // These signals were connected farther down this function in a previous invokation.
    if (m_currentSearchInstance && m_currentSearchInstance->isSearchInProgress()) {
        disconnect(m_currentSearchInstance, &SearchInstance::searchCompleted,
                   this, &AdvancedSearchDock::onCurrentSearchInstanceCompleted);
    }

    if (m_currentSearchInstance) {
        disconnect(m_currentSearchInstance, &SearchInstance::itemInteracted,
                   this, &AdvancedSearchDock::itemInteracted);
    }

    if (index==0) {
        m_currentSearchInstance = nullptr;

        m_dockWidget->setWidget(m_searchPanelWidget);
        m_btnToggleReplaceOptions->setChecked(false);
        m_btnToggleReplaceOptions->setVisible(false);
        m_btnMoreOptions->setVisible(false);
        m_btnPrevResult->setVisible(false);
        m_btnNextResult->setVisible(false);
        m_cmbSearchTerm->setFocus();
    } else {
        m_currentSearchInstance = m_searchInstances[index-1].get();

        if (m_currentSearchInstance->isSearchInProgress()) {
            connect(m_currentSearchInstance, &SearchInstance::searchCompleted,
                       this, &AdvancedSearchDock::onCurrentSearchInstanceCompleted);
        }

        connect(m_currentSearchInstance, &SearchInstance::itemInteracted,
                   this, &AdvancedSearchDock::itemInteracted);

        m_dockWidget->setWidget( m_currentSearchInstance->getResultTreeWidget() );
        m_btnToggleReplaceOptions->setVisible(true);
        m_btnMoreOptions->setVisible(true);
        m_btnPrevResult->setVisible(true);
        m_btnNextResult->setVisible(true);
        m_actExpandAll->setChecked( m_currentSearchInstance->areResultsExpanded() );
        m_actShowFullLines->setChecked( m_currentSearchInstance->getShowFullLines() );

        updateSearchInProgressUi();
    }
}

void AdvancedSearchDock::onChangeSearchScope(int index)
{
    switch (index) {
    case 0: // Search current document
    case 1: // Search all open documents
        m_cmbSearchPattern->setEnabled(false);
        m_cmbSearchDirectory->setEnabled(false);
        m_btnSelectSearchDirectory->setEnabled(false);
        m_btnSelectCurrentDirectory->setEnabled(false);
        m_chkIncludeSubdirs->setVisible(false);
        break;
    case 2: // Search in file system
        m_cmbSearchPattern->setEnabled(true);
        m_cmbSearchDirectory->setEnabled(true);
        m_btnSelectSearchDirectory->setEnabled(true);
        m_btnSelectCurrentDirectory->setEnabled(true);
        m_chkIncludeSubdirs->setVisible(true);
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
    switch (m_cmbSearchScope->currentIndex()) {
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
    // Some actions are only available when the search is finished
    const bool progress = m_currentSearchInstance->isSearchInProgress();

    m_actExpandAll->setEnabled(!progress);
    m_actCopyContents->setEnabled(!progress);
    m_actShowFullLines->setEnabled(!progress);
    m_btnToggleReplaceOptions->setVisible(!progress);

    m_btnPrevResult->setVisible(!progress);
    m_btnNextResult->setVisible(!progress);
}

void AdvancedSearchDock::startReplace()
{
    if (!m_currentSearchInstance)
        return;

    QString replaceText = m_cmbReplaceText->currentText();
    const SearchResult filteredResults = m_currentSearchInstance->getFilteredSearchResult();

    if (!askConfirmationForReplace(replaceText, filteredResults.countResults()))
        return;

    updateReplaceHistory(replaceText);

    if (m_chkReplaceWithSpecialChars->isChecked())
        replaceText = SearchString::unescape(replaceText);

    const SearchConfig& config = m_currentSearchInstance->getSearchConfig();
    const SearchConfig::SearchScope scope = config.searchScope;

    if (scope == SearchConfig::ScopeCurrentDocument || scope == SearchConfig::ScopeAllOpenDocuments) {
        // Since doc management is a mess we've got to go through all DocResults manually here.
        TopEditorContainer* tec = config.targetWindow->topEditorContainer();

        for (const DocResult& res : filteredResults.results) {
            Editor* ed = res.editor;

            // The editor might not be open anymore. Try to find it first
            if(!tec->tabWidgetFromEditor(ed)) continue;

            QString content = ed->value();
            FileReplacer::replaceAll(res, content, replaceText);
            ed->setValue(content);
        }
        return;
    } else if (scope == SearchConfig::ScopeFileSystem) {
        showReplaceDialog(filteredResults, replaceText);
    }
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
    if (m_chkUseSpecialChars->isChecked())
        config.searchMode = SearchConfig::ModePlainTextSpecialChars;
    else if (m_chkUseRegex->isChecked())
        config.searchMode = SearchConfig::ModeRegex;
    config.includeSubdirs = m_chkIncludeSubdirs->isChecked();
    config.targetWindow = m_mainWindow;

    return config;
}

void AdvancedSearchDock::setInputsFromConfig(const SearchConfig& config)
{
    m_cmbSearchDirectory->setCurrentText(config.directory);
    m_cmbSearchPattern->setCurrentText(config.filePattern);
    m_cmbSearchTerm->setCurrentText(config.searchString);
    m_cmbSearchScope->setCurrentIndex(config.searchScope);

    m_chkMatchCase->setChecked(config.matchCase);
    m_chkMatchWords->setChecked(config.matchWord);
    m_chkUseRegex->setChecked(config.searchMode == SearchConfig::ModeRegex);
    m_chkUseSpecialChars->setChecked(config.searchMode == SearchConfig::ModePlainTextSpecialChars);
    m_chkIncludeSubdirs->setChecked(config.includeSubdirs);
}

void AdvancedSearchDock::onSearchHistorySizeChange()
{
    const bool historyNotEmpty = m_cmbSearchHistory->count() > 1; // First item is "New Search"

    m_btnClearHistory->setEnabled(historyNotEmpty);
    m_cmbSearchHistory->setEnabled(historyNotEmpty);
}

void AdvancedSearchDock::selectPrevResult()
{
    if (m_currentSearchInstance) m_currentSearchInstance->selectPreviousResult();
}

void AdvancedSearchDock::selectNextResult()
{
    if (m_currentSearchInstance) m_currentSearchInstance->selectNextResult();
}

bool AdvancedSearchDock::isVisible() const
{
    return getDockWidget()->isVisible();
}

void AdvancedSearchDock::show(bool show, bool setFocus)
{
    getDockWidget()->setVisible(show);
    if (show && setFocus)
        m_cmbSearchTerm->setFocus();
}

AdvancedSearchDock::AdvancedSearchDock(MainWindow* mainWindow)
    : QObject(mainWindow),
      m_mainWindow(mainWindow),
      m_dockWidget(new QDockWidget())
{
    QDockWidget* dockWidget = m_dockWidget.data();
    dockWidget->setWindowTitle(tr("Advanced Search"));
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
        startSearch(getConfigFromInputs());
    });
    connect(m_cmbSearchScope, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvancedSearchDock::onChangeSearchScope);
    connect(m_btnSearch, &QToolButton::clicked, [this](){
        startSearch(getConfigFromInputs());
    });
    connect(m_cmbSearchDirectory->lineEdit(), &QLineEdit::textChanged, this, &AdvancedSearchDock::onUserInput);
    connect(m_cmbSearchDirectory->lineEdit(), &QLineEdit::returnPressed, [this](){
        startSearch(getConfigFromInputs());
    });
    connect(m_btnSelectCurrentDirectory, &QToolButton::clicked, [this, mainWindow](){
        auto* tabW = mainWindow->topEditorContainer()->currentTabWidget();
        auto* editor = tabW->currentEditor();

        QString dir;
        if (editor && !editor->filePath().isEmpty()) {
            dir = QFileInfo(editor->filePath().toLocalFile()).dir().path();
        }
        m_cmbSearchDirectory->setCurrentText(dir);
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
        if (m_currentSearchInstance->isSearchInProgress()) {
            const auto response = QMessageBox::warning(
                        QApplication::activeWindow(),
                        tr("Search in progress"),
                        tr("<h3>This search is still in progress.</h3> " \
                        "The search will be canceled and all results discarded if you continue."),
                        QMessageBox::Ok | QMessageBox::Cancel,
                        QMessageBox::Cancel);

            if (response == QMessageBox::Cancel) return;
        }

        m_searchInstances.erase(m_searchInstances.begin() + m_cmbSearchHistory->currentIndex()-1);
        m_currentSearchInstance = nullptr;
        m_cmbSearchHistory->removeItem(m_cmbSearchHistory->currentIndex());
        onSearchHistorySizeChange();
    });
    connect(m_btnReplaceSelected, &QToolButton::clicked, this, &AdvancedSearchDock::startReplace);

    // m_cmbSearchHistory is completely empty by this point. clearHistory() can be used to initialize it.
    clearHistory();
    onChangeSearchScope(0); // Initializes the status of the search panel
}

QDockWidget* AdvancedSearchDock::getDockWidget() const
{
    return m_dockWidget.data();
}

// Returns a QStringList of all items in the combo box.
static QStringList getComboBoxContents(const QComboBox* cb) {
    QStringList list;
    const int size = cb->count();
    for (int index = 0; index < size; index++) {
        list << cb->itemText(index);
    }
    return list;
}

void AdvancedSearchDock::updateSearchHistory(const QString& item) {
    if (item.isEmpty()) return;

    NqqSettings& settings = NqqSettings::getInstance();
    const QStringList& currHistory = settings.Search.getSaveHistory() ?
                settings.Search.getSearchHistory() :
                getComboBoxContents(m_cmbSearchTerm) ;

    const QStringList newHistory = addUniqueToList(currHistory, item);
    m_cmbSearchTerm->clear();
    m_cmbSearchTerm->addItems(newHistory);

    if(settings.Search.getSaveHistory())
        settings.Search.setSearchHistory(newHistory);
}

void AdvancedSearchDock::updateReplaceHistory(const QString& item) {
    if (item.isEmpty()) return;

    NqqSettings& settings = NqqSettings::getInstance();
    const QStringList& currHistory = settings.Search.getSaveHistory() ?
                settings.Search.getReplaceHistory() :
                getComboBoxContents(m_cmbReplaceText) ;

    const QStringList newHistory = addUniqueToList(currHistory, item);
    m_cmbReplaceText->clear();
    m_cmbReplaceText->addItems(newHistory);

    if(settings.Search.getSaveHistory())
        settings.Search.setReplaceHistory(newHistory);
}

void AdvancedSearchDock::updateDirectoryhHistory(const QString& item) {
    if (item.isEmpty()) return;

    NqqSettings& settings = NqqSettings::getInstance();
    const QStringList& currHistory = settings.Search.getSaveHistory() ?
                settings.Search.getFileHistory() :
                getComboBoxContents(m_cmbSearchDirectory) ;

    const QStringList newHistory = addUniqueToList(currHistory, item);
    m_cmbSearchDirectory->clear();
    m_cmbSearchDirectory->addItems(newHistory);

    if(settings.Search.getSaveHistory())
        settings.Search.setFileHistory(newHistory);
}

void AdvancedSearchDock::updateFilterHistory(const QString& item) {
    NqqSettings& settings = NqqSettings::getInstance();
    const QStringList& currHistory = settings.Search.getSaveHistory() ?
                settings.Search.getFilterHistory() :
                getComboBoxContents(m_cmbSearchPattern) ;

    const QStringList newHistory = addUniqueToList(currHistory, item);
    m_cmbSearchPattern->clear();
    m_cmbSearchPattern->addItems(newHistory);

    if(settings.Search.getSaveHistory())
        settings.Search.setFilterHistory(newHistory);
}

void AdvancedSearchDock::startSearch(SearchConfig cfg)
{
    if (cfg.searchString.isEmpty())
        return;

    const SearchConfig::SearchScope scope = cfg.searchScope;

    if (scope == SearchConfig::ScopeFileSystem) {
        // If we're searching the file system, check that the search dir is not empty and actually exists
        if (cfg.directory.isEmpty()) return;

        QDir dir(cfg.directory);

        if (!dir.exists()) {
            QMessageBox::warning(QApplication::activeWindow(), tr("Error"),
                                 tr("Specified directory does not exist."), QMessageBox::Ok);
            return;
        }

        cfg.directory = dir.absolutePath(); // Also cleans path from multiple separators or "..", ".", etc.
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


void AdvancedSearchDock::showReplaceDialog(const SearchResult& filteredResults, const QString& replaceText) const
{
    FileReplacer* w = new FileReplacer(filteredResults, replaceText);
    QMessageBox* msgBox = new QMessageBox(QApplication::activeWindow());

    connect(w, &FileReplacer::resultReady, msgBox, &QMessageBox::close);
    connect(w, &FileReplacer::resultProgress, msgBox, [msgBox](int curr, int total) {
        msgBox->setInformativeText(tr("Matches in %1 of %2 files replaced.").arg(curr).arg(total));
    });
    connect(msgBox, &QMessageBox::buttonClicked, w, &FileReplacer::cancel);

    w->start();

    msgBox->setWindowTitle(QCoreApplication::applicationName());
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setText(tr("<h3>Replacing selected matches...</h3>"));
    msgBox->setInformativeText(tr("Replacing in progress"));
    msgBox->setStandardButtons(QMessageBox::Cancel);
    msgBox->exec();

    w->wait();

    const bool success = !w->hasErrors();

    if (!success) {
        const QVector<QString>& errors = w->getErrors();
        const int numErrors = errors.size();
        const int maxCount = std::min(numErrors, 8);
        QString errorString = tr("Replacing was unsuccessful for %1 file(s):\n").arg(numErrors);

        for (int i=0; i<maxCount; i++) {
            errorString += "\"" + errors[i] + "\"\n";
        }
        if (numErrors > 8)
            errorString += tr("And %1 more.").arg(numErrors-8);

        QMessageBox::warning(QApplication::activeWindow(),
                             tr("Replacement Results"), errorString, QMessageBox::Ok);
    } else {
        QMessageBox::information(QApplication::activeWindow(),
                                 tr("Replacement Results"),
                                 tr("All selected matches successfully replaced."), QMessageBox::Ok);
    }

    delete w;
    delete msgBox;
}
