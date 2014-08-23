#include "include/editortabwidget.h"
#include <QTabBar>
#include <QApplication>

#ifdef QT_DEBUG
#include <QElapsedTimer>
#endif

EditorTabWidget::EditorTabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setDocumentMode(true);
    this->setTabsClosable(true);
    this->setMovable(true);

    this->setTabBarHidden(false);
    this->setTabBarHighlight(false);
    this->setTabBarVertical(false);
}

int EditorTabWidget::addEditorTab(bool setFocus, QString title)
{
#ifdef QT_DEBUG
    QElapsedTimer __aet_timer;
    __aet_timer.start();
#endif

    this->setUpdatesEnabled(false);

    Editor *editor = new Editor(this);

    int index = this->addTab(editor, title);
    if(setFocus) {
        this->setCurrentIndex(index);
        editor->setFocus();
    }

    this->setSavedIcon(index, true);

    connect(editor, &Editor::cleanChanged,
            this, &EditorTabWidget::on_cleanChanged);

    this->setUpdatesEnabled(true);

    emit editorAdded(index);

#ifdef QT_DEBUG
    qint64 __aet_elapsed = __aet_timer.nsecsElapsed();
    qDebug() << "Tab opened in " + QString::number(__aet_elapsed / 1000 / 1000) + "msec";
#endif

    return index;
}

int EditorTabWidget::transferEditorTab(bool setFocus, EditorTabWidget *source, int tabIndex)
{
    this->setUpdatesEnabled(false);

    Editor *editor = (Editor *)source->widget(tabIndex);
    QString text = source->tabText(tabIndex);
    QIcon icon = source->tabIcon(tabIndex);
    QString tooltip = source->tabToolTip(tabIndex);

    int index = this->addTab(editor, text);
    if(setFocus) {
        this->setCurrentIndex(index);
        editor->setFocus();
    }

    this->setTabIcon(index, icon);
    this->setTabToolTip(index, tooltip);

    this->setUpdatesEnabled(true);

    emit editorAdded(index);

    return index;
}

int EditorTabWidget::findOpenEditorByFileName(QString filename)
{
    for (int i = 0; i < this->count(); i++) {
        Editor *editor = (Editor *)this->widget(i);
        if (editor->fileName() == filename)
            return i;
    }

    return -1;
}

Editor *EditorTabWidget::editor(int index)
{
    return (Editor *)this->widget(index);
}

Editor *EditorTabWidget::currentEditor()
{
    return (Editor *)this->currentWidget();
}

void EditorTabWidget::setSavedIcon(int index, bool saved)
{
    if(saved)
        this->setTabIcon(index, QIcon(":/icons/icons/saved.png"));
    else
        this->setTabIcon(index, QIcon(":/icons/icons/unsaved.png"));
}

void EditorTabWidget::setTabBarHidden(bool yes)
{
    tabBar()->setHidden(yes);
}

void EditorTabWidget::setTabBarHighlight(bool yes)
{
    //Get colors from palette so it doesn't look fugly.
    QPalette palette = tabBar()->palette();
    palette.setColor(QPalette::Highlight,yes ? QApplication::palette().highlight().color() : QApplication::palette().light().color());
    tabBar()->setPalette(palette);
}

void EditorTabWidget::setTabBarVertical(bool yes)
{
    QString prestyle = "";
    int reduced = 24;//MainWindow::instance()->getSettings()->value(widesettings::SETTING_TABBAR_REDUCE,true).toBool() ? 24 : 30;
    if (yes) {
        setTabPosition(QTabWidget::West);
        prestyle.append(QString("QTabBar::tab{min-height:100px;width:%1;}").arg(reduced));
    } else {
        setTabPosition(QTabWidget::North);
        prestyle.append(QString("QTabBar::tab{min-width:100px;height:%1;}").arg(reduced));
    }
    setStyleSheet(prestyle);
    //setTabBarHighlight(MainWindow::instance()->getSettings()->value(widesettings::SETTING_TABBAR_HIGHLIGHT,true).toBool());
}

void EditorTabWidget::on_cleanChanged(bool isClean)
{
    int index = this->indexOf((QWidget *)sender());
    if(index >= 0)
        this->setSavedIcon(index, isClean);
}
