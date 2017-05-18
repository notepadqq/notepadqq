#include "include/nqqtab.h"

#include <QApplication>

#include "include/docengine.h"
#include "include/iconprovider.h"


NqqTab::~NqqTab()
{
    //TODO: DocEngine::unmonitor(m_editor); <- or somewhere else? Maybe Editor itself should have this as a destructor?
    // Editor should probably be a Document type anyways, with Editor-type stuff like scroll position being saved in NqqTab.
}

QString NqqTab::getTabTitle() const
{
    int index = m_parentTabWidget->getWidget()->indexOf(m_editor);
    return m_parentTabWidget->getWidget()->tabText(index);
}

void NqqTab::setTabTitle(const QString& title)
{
    int index = m_parentTabWidget->getWidget()->indexOf(m_editor);
    m_parentTabWidget->getWidget()->setTabText(index, title);
}

void NqqTab::closeTab()
{
    int index = m_parentTabWidget->getWidget()->indexOf(m_editor);
    m_parentTabWidget->getWidget()->tabCloseRequested(index);
}

void NqqTab::forceCloseTab()
{
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

    connect((m_tabWidget->tabBar()), &QTabBar::tabMoved, [this](int to, int from){ //to and from switched. But in Qt
        qDebug() << "tab moved from " << from << "to" << to;

        auto fromIt = m_tabs.begin() + from;
        auto toIt = m_tabs.begin() + to;

        std::swap(*fromIt, *toIt);

        for(NqqTab* tab : m_tabs) qDebug() << tab->getTabTitle();
    });

    connect(m_tabWidget, &QTabWidget::currentChanged, [this](int index) {
        if(index >= 0)
            emit currentTabChanged(m_tabs[index]);
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

    NqqTab* t = new NqqTab();
    t->m_editor = editor;
    t->m_parentTabWidget = this;

    m_tabs.push_back(t);

    connectTab(t);

    int index = m_tabWidget->addTab(editor, editor->fileName().fileName());

    const QIcon& ic = editor->fileOnDiskChanged() ?
                IconProvider::fromTheme("document-unsaved") : IconProvider::fromTheme("document-saved");

    m_tabWidget->setTabIcon(index, ic);
    if(makeCurrent)
        m_tabWidget->setCurrentIndex(index);

    emit newTabAdded(t);

    return t;
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
        qreal diff = evt->delta() / 120 / 10; // Increment/Decrement by 0.1 each step

        const qreal newZoom = curZoom + diff;

        for(NqqTab* t : getAllTabs())
            t->setZoomFactor(newZoom);
    }
}

void NqqTabWidget::connectTab(NqqTab* tab) {
    connect(tab, &NqqTab::gotFocus, tab, [this](){
        emit currentTabChanged(reinterpret_cast<NqqTab*>(sender()));
    });
    connect(tab->m_editor, &Editor::mouseWheel, this, [tab, this](QWheelEvent* evt) {
        onTabMouseWheelUsed(tab, evt);
    });
    connect(tab->m_editor, &Editor::urlsDropped, this, &NqqTabWidget::urlsDropped);
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
