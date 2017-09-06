#include "include/nqqtab.h"

#include <QApplication>

#include "include/docengine.h"
#include "include/iconprovider.h"


NqqTab::NqqTab(Editor* editor)
{
    m_editor = editor;
    setTabTitle(m_editor->fileName().fileName());

    connect(m_editor, &Editor::cleanChanged, [this](bool isClean){
        if(!m_parentTabWidget) return;
        m_parentTabWidget->setTabSavedIcon(this, isClean);
    });

    connect(editor, &Editor::gotFocus, this, &NqqTab::gotFocus);
    connect(editor, &Editor::urlsDropped, this, &NqqTab::urlsDropped);
    connect(editor, &Editor::cursorActivity, this, &NqqTab::cursorActivity);
    connect(editor, &Editor::mouseWheel, this, &NqqTab::mouseWheel);
    connect(editor, &Editor::currentLanguageChanged, this, &NqqTab::languageChanged);
}

NqqTab::~NqqTab()
{
    //TODO: DocEngine::unmonitor(m_editor); <- or somewhere else? Maybe Editor itself should have this as a destructor?
    // Editor should probably be a Document type anyways, with Editor-type stuff like scroll position being saved in NqqTab.
}

QString NqqTab::getTabTitle() const
{
    return m_tabTitle;
}

void NqqTab::setTabTitle(const QString& title)
{
    if(m_parentTabWidget) {
        int index = m_parentTabWidget->getWidget()->indexOf(m_editor);
        m_parentTabWidget->getWidget()->setTabText(index, title);
    }

    m_tabTitle = title;
}

bool NqqTab::getClean() const
{
    return m_editor->isClean();
}

void NqqTab::setClean(bool isClean)
{
    if(isClean) m_editor->markClean();
    else m_editor->markDirty();
}

void NqqTab::closeTab()
{
    int index = m_parentTabWidget->getWidget()->indexOf(m_editor);
    m_parentTabWidget->getWidget()->tabCloseRequested(index);
}

void NqqTab::forceCloseTab()
{
    m_parentTabWidget->detachTab(this);
    delete this;
}

void NqqTab::makeCurrent()
{
    if(m_parentTabWidget)
        m_parentTabWidget->makeCurrent(this);
}

NqqTabWidget::NqqTabWidget(NqqSplitPane* parent)
    : m_parent(parent),
      m_tabWidget(new CustomTabWidget(nullptr))
{
    m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    QString style = QString("QTabBar::tab{min-width:100px; height:24px;}");
    m_tabWidget->setStyleSheet(style);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &NqqTabWidget::onTabCloseRequested);

    connect(m_tabWidget, &QTabWidget::tabBarDoubleClicked, [this](int index){
        qDebug() << "Double clicked" << index;

        if(index == -1)
            createEmptyTab(true);
    });

    connect(m_tabWidget, &QTabWidget::tabBarClicked, this, [this](){
        getCurrentTab()->m_editor->setFocus(); //TODO: don't want to do this
        //emit currentTabChanged(getCurrentTab());
    });

    connect((m_tabWidget->tabBar()), &QTabBar::tabMoved, [this](int to, int from){ //to and from switched. But in Qt
        qDebug() << "tab moved from " << from << "to" << to;

        auto fromIt = m_tabs.begin() + from;
        auto toIt = m_tabs.begin() + to;

        std::swap(*fromIt, *toIt);

        for(NqqTab* tab : m_tabs) qDebug() << tab->getTabTitle();
    });

    connect(m_tabWidget, &QTabWidget::customContextMenuRequested, [this](const QPoint& point) {
        emit customContextMenuRequested(m_tabWidget->mapToGlobal(point));
    });
}

NqqTabWidget::~NqqTabWidget()
{
    //disconnect(m_tabWidget);

    for(auto it=m_tabs.rbegin(); it!=m_tabs.rend(); ++it) {
        onTabCloseRequested(std::distance(m_tabs.rbegin(),it));
    }

    // delete m_tabWidget; ?
}

NqqTab*NqqTabWidget::createEmptyTab(bool makeCurrent)
{
    qDebug() << "Nqq: createEmptyTab() called.";

    Editor* ed = new Editor();
    ed->setLanguage("plaintext");

    NqqTab* t = new NqqTab(ed);
    t->setTabTitle(DocEngine::getNewDocumentName());

    if (attachTab(t)) {

        if(makeCurrent) {
            m_tabWidget->setCurrentIndex( getIndexOfTab(t) );
            t->m_editor->setFocus();
        }

        emit newTabAdded(t);
        return t;
    }

    // TODO: What happens to the Editor if 't' is deleted?
    delete t;
    return nullptr;
}

NqqTab* NqqTabWidget::createTab(Editor* editor, bool makeCurrent) {
    qDebug() << "Nqq: addEditor called.";

    if(!editor) return nullptr;

    NqqTab* t = new NqqTab(editor);

    if (attachTab(t)) {

        if(makeCurrent) {
            m_tabWidget->setCurrentIndex( getIndexOfTab(t) );
            t->m_editor->setFocus();
        }

        emit newTabAdded(t);
        return t;
    }

    delete t;
    return nullptr;
}

bool NqqTabWidget::detachTab(NqqTab* tab)
{
    const int index = getIndexOfTab(tab);

    if(index < 0) return false;

    disconnectTab(tab);
    tab->m_parentTabWidget = nullptr;

    m_tabWidget->removeTab(index);
    m_tabs.erase(m_tabs.begin()+index);

    if(m_tabs.empty()) {
        qDebug() << "forceCloseTab: TabWidget empty, adding empty tab.";

        // If false, this tab will be deleted
        if(!m_parent->processEmptyTabWidget(this))
            return true;

        createEmptyTab(true);
    }

    makeCurrent( m_tabWidget->currentIndex() );

    return true;
}

bool NqqTabWidget::attachTab(NqqTab* tab)
{
    if(!tab || tab->m_parentTabWidget) return false;

    tab->m_parentTabWidget = this;

    m_tabs.push_back(tab);
    connectTab(tab);
    m_tabWidget->addTab(tab->m_editor, tab->getFileUrl().fileName());

    tab->setTabTitle( tab->getTabTitle() ); // TODO: We don't want to manually update all tab data
    tab->setClean( tab->getClean() );

    return true;
}

void NqqTabWidget::onTabCloseRequested(int index)
{
    qDebug() << "NqqTabWidget::onTabCloseRequested( index =" << index <<")";

    emit tabCloseRequested(m_tabs[index]);
}

void NqqTabWidget::onTabGotFocus()
{
    NqqTab* tab = reinterpret_cast<NqqTab*>(sender());
    emit currentTabChanged(tab);
}

void NqqTabWidget::onTabMouseWheelUsed(QWheelEvent* evt)
{
    NqqTab* tab = reinterpret_cast<NqqTab*>(sender());
    qDebug() << "onTabMouseWheelUsed";
    emit currentTabMouseWheel(tab, evt);
}

void NqqTabWidget::onTabCursorActivity()
{
    qDebug() << "onTabCursorActivity";
    NqqTab* tab = reinterpret_cast<NqqTab*>(sender());
    emit currentTabCursorActivity(tab);
}

void NqqTabWidget::onTabLanguageChanged()
{
    NqqTab* tab = reinterpret_cast<NqqTab*>(sender());
    emit currentTabLanguageChanged(tab);
}

void NqqTabWidget::connectTab(NqqTab* tab) {
    /* Connections should only be made from signal to a slot. Using a lambda as a slot means the connection might
     * not get properly cleaned up. Connections need to be closed through disconnectTab() which calls QObject::disconnect.
     * However, lambdas cannot be disconnected this way due to a limitation of Qt. So just stick to slots. */
    connect(tab, &NqqTab::gotFocus, this, &NqqTabWidget::onTabGotFocus);
    connect(tab, &NqqTab::mouseWheel, this, &NqqTabWidget::onTabMouseWheelUsed);
    connect(tab, &NqqTab::urlsDropped, this, &NqqTabWidget::urlsDropped);
    connect(tab, &NqqTab::cursorActivity, this, &NqqTabWidget::onTabCursorActivity);
    connect(tab, &NqqTab::languageChanged, this, &NqqTabWidget::onTabLanguageChanged);
}

void NqqTabWidget::disconnectTab(NqqTab* tab) {
    /* Using a blanket disconnect like disconnect(tab) doesn't seem to properly disconnect all connections.
     * This is either a bug or my ignorance about Qt. Either way, specifically disconnecting all connections
     * works. So anything connected in connectTab() should be disconnected here the same way. */
    disconnect(tab, &NqqTab::gotFocus, this, &NqqTabWidget::onTabGotFocus);
    disconnect(tab, &NqqTab::mouseWheel, this, &NqqTabWidget::onTabMouseWheelUsed);
    disconnect(tab, &NqqTab::urlsDropped, this, &NqqTabWidget::urlsDropped);
    disconnect(tab, &NqqTab::cursorActivity, this, &NqqTabWidget::onTabCursorActivity);
    disconnect(tab, &NqqTab::languageChanged, this, &NqqTabWidget::onTabLanguageChanged);
}

NqqTab* NqqTabWidget::getCurrentTab() const
{
    int index = m_tabWidget->currentIndex();
    return m_tabs[index];
}

int NqqTabWidget::getCurrentIndex() const
{
    return m_tabWidget->currentIndex();
}
QTabWidget*NqqTabWidget::getWidget() const
{
    return m_tabWidget;
}

NqqTab*NqqTabWidget::findTabByUrl(const QUrl& fileUrl) const
{
    for(NqqTab* tab : m_tabs)
        if(tab->getFileUrl() == fileUrl) return tab;

    return nullptr;
}

int NqqTabWidget::getIndexOfTab(NqqTab* tab) const
{
    const auto it = std::find(m_tabs.begin(), m_tabs.end(), tab);

    if (it==m_tabs.end())
        return -1;
    else
        return std::distance(m_tabs.begin(), it);
}

void NqqTabWidget::setFocus(NqqTab* tab)
{
    auto it = std::find(m_tabs.begin(), m_tabs.end(), tab);
    if(it == m_tabs.end())
        return;

    tab->m_editor->setFocus();
}

void NqqTabWidget::makeCurrent(NqqTab* tab)
{
    if(!tab) return;

    m_tabWidget->setCurrentWidget(tab->m_editor);
    tab->m_editor->setFocus();
}

void NqqTabWidget::makeCurrent(int index)
{
    if(index >= 0 && static_cast<uint>(index) < m_tabs.size())
        makeCurrent(m_tabs[index]);
}

void NqqTabWidget::setTabSavedIcon(NqqTab* tab, bool saved)
{
    const int index = getIndexOfTab(tab);
    if(index < 0) return;

    const QIcon& icon = saved ? IconProvider::fromTheme("document-saved")
                              : IconProvider::fromTheme("document-unsaved");

    m_tabWidget->setTabIcon(index, icon);
}

void NqqSplitPane::connectTabWidget(NqqTabWidget* tabWidget)
{
    connect(tabWidget, &NqqTabWidget::currentTabChanged, this, &NqqSplitPane::currentTabChanged);
    connect(tabWidget, &NqqTabWidget::tabCloseRequested, this, &NqqSplitPane::tabCloseRequested);
    connect(tabWidget, &NqqTabWidget::newTabAdded, this, &NqqSplitPane::newTabAdded);
    connect(tabWidget, &NqqTabWidget::customContextMenuRequested, this, &NqqSplitPane::customContextMenuRequested);
    connect(tabWidget, &NqqTabWidget::urlsDropped, this, &NqqSplitPane::urlsDropped);

    connect(tabWidget, &NqqTabWidget::currentTabChanged, [tabWidget, this](){
        if(m_activeTabWidget==tabWidget) return;

        setActiveTabWidget(tabWidget);
        qDebug() << "Active TabWidget changed.";
    });

}

void NqqSplitPane::setActiveTabWidget(NqqTabWidget* tabWidget)
{
    if(m_activeTabWidget){
        disconnect(m_activeTabWidget, &NqqTabWidget::currentTabCursorActivity, this, &NqqSplitPane::currentTabCursorActivity);
        disconnect(m_activeTabWidget, &NqqTabWidget::currentTabLanguageChanged, this, &NqqSplitPane::currentTabLanguageChanged);
        disconnect(m_activeTabWidget, &NqqTabWidget::currentTabCleanStatusChanged, this, &NqqSplitPane::currentTabCleanStatusChanged);
        disconnect(m_activeTabWidget, &NqqTabWidget::currentTabMouseWheel, this, &NqqSplitPane::currentTabMouseWheel);
    }

    m_activeTabWidget = tabWidget;

    connect(tabWidget, &NqqTabWidget::currentTabCursorActivity, this, &NqqSplitPane::currentTabCursorActivity);
    connect(tabWidget, &NqqTabWidget::currentTabLanguageChanged, this, &NqqSplitPane::currentTabLanguageChanged);
    connect(tabWidget, &NqqTabWidget::currentTabCleanStatusChanged, this, &NqqSplitPane::currentTabCleanStatusChanged);
    connect(tabWidget, &NqqTabWidget::currentTabMouseWheel, this, &NqqSplitPane::currentTabMouseWheel);
}

NqqTabWidget*NqqSplitPane::getPrevTabWidget() const
{
    NqqTabWidget* current = getCurrentTabWidget();

    if(!current)
        return m_panels.back();

    auto it = std::find(m_panels.begin(), m_panels.end(), current);

    return it==m_panels.begin() ? m_panels.back() : *(it-1);
}

NqqTabWidget*NqqSplitPane::getNextTabWidget() const
{
    NqqTabWidget* current = getCurrentTabWidget();

    if(!current)
        return m_panels.front();

    auto it = std::find(m_panels.begin(), m_panels.end(), current);

    return it==(m_panels.end()-1) ? m_panels.front() : *(it+1);
}

const std::vector<NqqTab*> NqqSplitPane::getAllTabs() const
{
    std::vector<NqqTab*> allTabs;

    for(const auto& vec : getAllTabWidgets()) {
        const auto& tabs = vec->getAllTabs();
        allTabs.reserve( allTabs.size() + tabs.size() );
        allTabs.insert(allTabs.end(), tabs.begin(), tabs.end());
    }

    return allTabs;
}

bool NqqSplitPane::processEmptyTabWidget(NqqTabWidget* tabW)
{
    if(m_panels.size() == 1)
        return true;    // Want to keep only panel
    else {
        m_panels.erase( std::find(m_panels.begin(), m_panels.end(), tabW) );
        tabW->deleteLater();
        tabW->getWidget()->setParent(nullptr);
        tabW->getWidget()->deleteLater();
        return false;
    }
}

NqqTabWidget* NqqSplitPane::createNewTabWidget(NqqTab* newTab) {
    NqqTabWidget* w = new NqqTabWidget(this);

    connectTabWidget(w);

    m_panels.push_back(w);
    m_splitter->addWidget(w->getWidget());

    // Resize all panels faily
    const int currentViewCount = m_splitter->count();
    const int tabSize = m_splitter->contentsRect().width() / currentViewCount;
    QList<int> sizes;
    for(int i=0; i<currentViewCount; ++i)
        sizes << tabSize;

    m_splitter->setSizes( sizes );

    if(!newTab || !w->attachTab(newTab))
        w->createEmptyTab();

    if(!m_activeTabWidget) setActiveTabWidget(w);

    return w;
}
