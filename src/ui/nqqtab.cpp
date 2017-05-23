#include "include/nqqtab.h"

#include <QApplication>

#include "include/docengine.h"
#include "include/iconprovider.h"


NqqTab::NqqTab(Editor* editor)
{
    m_editor = editor;

    connect(m_editor, &Editor::cleanChanged, [this](bool isClean){
        if(!m_parentTabWidget) return;
        m_parentTabWidget->setTabSavedIcon(this, isClean);
    });
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

void NqqTab::closeTab()
{
    int index = m_parentTabWidget->getWidget()->indexOf(m_editor);
    m_parentTabWidget->getWidget()->tabCloseRequested(index);
}

void NqqTab::forceCloseTab()
{
    // TODO: This function should probably call delete this. Check if parent even exists.
    // Also, rename parent->forceCloseTab to something else? Just detach/remove tab instead? For close needed?
    m_parentTabWidget->forceCloseTab(this);
}

NqqTabWidget::NqqTabWidget()
    : m_tabWidget(new CustomTabWidget(nullptr))
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
    for(auto it=m_tabs.rbegin(); it!=m_tabs.rend(); ++it) {
        onTabCloseRequested(std::distance(m_tabs.rbegin(),it));
    }

    // delete m_tabWidget; ?
}

NqqTab*NqqTabWidget::createEmptyTab(bool makeCurrent)
{
    Editor* ed = new Editor();
    ed->setLanguage("plaintext");
    NqqTab* t = createTab(ed, makeCurrent);
    t->setTabTitle(DocEngine::getNewDocumentName()); //Issue: this is called after 'emit newTabAdded'
    return t;
}

NqqTab* NqqTabWidget::createTab(Editor* editor, bool makeCurrent) {
    qDebug() << "Nqq: addEditor called.";

    if(!editor) return nullptr;

    NqqTab* t = new NqqTab(editor);
    t->m_parentTabWidget = this;

    m_tabs.push_back(t);

    connectTab(t);

    int index = m_tabWidget->addTab(editor, editor->fileName().fileName());
    t->setTabTitle(m_tabWidget->tabText(index));

    const QIcon& ic = editor->fileOnDiskChanged() ?
                IconProvider::fromTheme("document-unsaved") : IconProvider::fromTheme("document-saved");

    m_tabWidget->setTabIcon(index, ic);
    if(makeCurrent) {
        m_tabWidget->setCurrentIndex(index);
        t->m_editor->setFocus();
    }

    emit newTabAdded(t);

    return t;
}

bool NqqTabWidget::detachTab(NqqTab* tab)
{
    // TODO: can we use this to implement forceCloseTab?
    const int index = getIndexOfTab(tab);

    if(index < 0) return false;

    disconnectTab(tab);
    tab->m_parentTabWidget = nullptr;

    m_tabWidget->removeTab(index);
    m_tabs.erase(m_tabs.begin()+index);

    if(m_tabs.empty()) {
        qDebug() << "forceCloseTab: TabWidget empty, adding empty tab.";
        createEmptyTab(true);
    }

    // m_tabs mustn't be empty at this point
    makeCurrent( m_tabWidget->currentIndex() );

    return true;
}

bool NqqTabWidget::attachTab(NqqTab* tab)
{
    // TODO: Can we use this to implement createTab?

    if(tab->m_parentTabWidget != nullptr) return false;

    NqqTab* t = createTab(tab->m_editor);
    t->setTabTitle(tab->getTabTitle());
    delete tab;

    return true;
}

void NqqTabWidget::onTabCloseRequested(int index)
{
    qDebug() << "NqqTabWidget::onTabCloseRequested( index =" << index <<")";

    emit tabCloseRequested(m_tabs[index]);
}

void NqqTabWidget::onTabMouseWheelUsed(NqqTab* tab, QWheelEvent* evt)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        const qreal curZoom = tab->getZoomFactor();
        qreal diff = evt->delta() / 120. / 10.; // Increment/Decrement by 0.1 each step

        const qreal newZoom = curZoom + diff;

        // TODO: If we want all tabs to change zoom we should send a signal to MainWindow
        /*for(NqqTab* t : getAllTabs())
            t->setZoomFactor(newZoom);*/
        getCurrentTab()->setZoomFactor(newZoom);

        evt->accept(); // By accepting the event, it won't be handled by the WebView anymore
    }
    //m_settings.General.setZoom(newZoom); //TODO: We don't save the zoom factor after changing it at the moment
    //We used to do that in MainWindow through a signal
}

void NqqTabWidget::connectTab(NqqTab* tab) {
    connect(tab, &NqqTab::gotFocus, tab, [tab, this](){
        qDebug() << "Is this event being called right now?"; //TODO want to connect to editor directly or to NqqTab?
        emit currentTabChanged(tab);
    });
    connect(tab->m_editor, &Editor::mouseWheel, tab, [tab, this](QWheelEvent* evt) {
        onTabMouseWheelUsed(tab, evt);
    });
    connect(tab->m_editor, &Editor::urlsDropped, this, &NqqTabWidget::urlsDropped);
    connect(tab->m_editor, &Editor::cursorActivity, tab, [tab, this](){
        emit currentTabCursorActivity(tab);
    });
    connect(tab->m_editor, &Editor::currentLanguageChanged, [tab, this](){
        emit currentTabLanguageChanged(tab);
    });
    connect(tab->m_editor, &Editor::gotFocus, tab, [tab, this](){
        emit currentTabChanged(tab);
    });
}

void NqqTabWidget::disconnectTab(NqqTab* tab) {
    disconnect(tab->m_editor);
    disconnect(tab);
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

void NqqTabWidget::forceCloseTab(NqqTab* tab)
{
    auto it = std::find(m_tabs.begin(), m_tabs.end(), tab);
    if(it == m_tabs.end())
        return;

    qDebug() << "NqqTabWidget::forceCloseTab()";

    m_tabWidget->removeTab(std::distance(m_tabs.begin(), it));

    delete tab;
    m_tabs.erase(it);

    if(m_tabs.empty()) {
        qDebug() << "forceCloseTab: TabWidget empty, adding empty tab.";
        createEmptyTab(true);
    }

    // m_tabs mustn't be empty at this point
    makeCurrent( m_tabWidget->currentIndex() );
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
    }

    m_activeTabWidget = tabWidget;

    connect(tabWidget, &NqqTabWidget::currentTabCursorActivity, this, &NqqSplitPane::currentTabCursorActivity);
    connect(tabWidget, &NqqTabWidget::currentTabLanguageChanged, this, &NqqSplitPane::currentTabLanguageChanged);
    connect(tabWidget, &NqqTabWidget::currentTabCleanStatusChanged, this, &NqqSplitPane::currentTabCleanStatusChanged);
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

NqqTabWidget* NqqSplitPane::createNewTabWidget(NqqTab* newTab) {
    NqqTabWidget* w = new NqqTabWidget();

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

    if(!m_activeTabWidget) m_activeTabWidget = w;

    return w;
}
