#ifndef NQQTAB_H
#define NQQTAB_H

#include <QTabWidget>
#include <QWheelEvent>
#include <vector>
#include <QSplitter>

#include <QTabBar>

#include "include/EditorNS/editor.h"

using namespace EditorNS;

class NqqTabWidget;

class NqqTab : public QObject {
    Q_OBJECT

public:

    NqqTab(Editor* editor);
    ~NqqTab();

    QString getTextContents() const { return m_editor->value(); }
    void setTextContents(const QString& text) { m_editor->setValue(text); }

    QString getTabTitle() const;
    void setTabTitle(const QString& title);

    qreal getZoomFactor() const { return m_editor->zoomFactor(); }
    void setZoomFactor(qreal factor) { m_editor->setZoomFactor(factor); }

    QUrl getFileUrl() const { return m_editor->fileName(); }

    void closeTab();
    void forceCloseTab();

    NqqTabWidget* m_parentTabWidget;
    Editor* m_editor;
    QString m_tabTitle;

signals:
    void gotFocus();
};


class CustomTabWidget : public QTabWidget {
  Q_OBJECT

public:

    CustomTabWidget(QWidget* parent) : QTabWidget(parent) {
        //setFocusPolicy(Qt::StrongFocus);
    }

    /*virtual void tabInserted(int index) {
        qDebug() << "tab inserted at index " << index;
    }
    virtual void tabRemoved(int index) {
        qDebug() << "tab removed at index " << index;
    }*/

    virtual void mouseReleaseEvent(QMouseEvent* evt) {
        if(evt->button() != Qt::MiddleButton) {
            QTabWidget::mouseReleaseEvent(evt);
            return;
        }

        int index = tabBar()->tabAt(evt->pos());
        if(index >= 0) {
            emit tabCloseRequested(index);
            evt->accept();
        }
    }

};

class NqqTabWidget : public QObject {
    Q_OBJECT

public:

    NqqTabWidget();
    ~NqqTabWidget();


    NqqTab* createEmptyTab(bool makeCurrent=true);
    NqqTab* createTab(Editor* editor, bool makeCurrent=true);

    bool detachTab(NqqTab* tab);
    bool attachTab(NqqTab* tab);

    const std::vector<NqqTab*> getAllTabs() const { return m_tabs; }
    NqqTab* getCurrentTab() const;
    int getCurrentIndex() const;
    QTabWidget* getWidget() const;

    bool isEmpty() const { return m_tabs.empty(); }

    NqqTab* findTabByUrl(const QUrl& fileUrl) const;
    int getIndexOfTab(NqqTab* tab) const;

    void forceCloseTab(NqqTab* tab);

    void setFocus(NqqTab* tab);

    void makeCurrent(NqqTab* tab);
    void makeCurrent(int index);

    void setTabSavedIcon(NqqTab* tab, bool saved);

signals:
    void currentTabChanged(NqqTab* newFocus);
    void currentTabCursorActivity(NqqTab* tab);
    void currentTabLanguageChanged(NqqTab* tab);
    void currentTabCleanStatusChanged(NqqTab* tab);

    void tabCloseRequested(NqqTab* tab);
    void newTabAdded(NqqTab* tab);
    void customContextMenuRequested(const QPoint& point);
    void urlsDropped(const QList<QUrl>& urls);

    void gotFocus();
    void tabBarClicked();


private slots:
    void onTabCloseRequested(int index);
    void onTabMouseWheelUsed(NqqTab* tab, QWheelEvent* evt);

private:
    void connectTab(NqqTab* tab);
    void disconnectTab(NqqTab* tab);

    std::vector<NqqTab*> m_tabs;
    QTabWidget* m_tabWidget;
};

class NqqSplitPane : public QObject {
    Q_OBJECT


signals:
    void currentTabChanged(NqqTab* newFocus);
    void currentTabCursorActivity(NqqTab* tab);
    void currentTabLanguageChanged(NqqTab* tab);
    void currentTabCleanStatusChanged(NqqTab* tab);

    void tabCloseRequested(NqqTab* tab);
    void newTabAdded(NqqTab* tab);
    void customContextMenuRequested(const QPoint& point);
    void urlsDropped(const QList<QUrl>& urls);

public:

    void connectTabWidget(NqqTabWidget* tabWidget);
    void disconnectTabWidget(NqqTabWidget* tabWidget);
    void setActiveTabWidget(NqqTabWidget* tabWidget);

    NqqTabWidget* getCurrentTabWidget() const { return m_activeTabWidget; }
    NqqTabWidget* getPrevTabWidget() const;
    NqqTabWidget* getNextTabWidget() const;


    NqqTabWidget* createNewTabWidget(NqqTab* newTab=nullptr);

    std::vector<NqqTabWidget*> m_panels;
    NqqTabWidget* m_activeTabWidget = nullptr;

    QSplitter* m_splitter = new QSplitter();
};


#endif // NQQTAB_H
