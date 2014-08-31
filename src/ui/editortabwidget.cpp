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

    QString style = QString("QTabBar::tab{min-width:100px; height:24px;}");
    setStyleSheet(style);
}

int EditorTabWidget::addEditorTab(bool setFocus, QString title)
{
    return this->rawAddEditorTab(setFocus, title, 0, 0);
}

void EditorTabWidget::connectEditorSignals(Editor *editor)
{
    connect(editor, &Editor::cleanChanged,
            this, &EditorTabWidget::on_cleanChanged);

    connect(editor, &Editor::gotFocus, this, &EditorTabWidget::gotFocus);
}

void EditorTabWidget::disconnectEditorSignals(Editor *editor)
{
    disconnect(editor, &Editor::cleanChanged,
               this, &EditorTabWidget::on_cleanChanged);

    disconnect(editor, &Editor::gotFocus, this, &EditorTabWidget::gotFocus);
}

int EditorTabWidget::transferEditorTab(bool setFocus, EditorTabWidget *source, int tabIndex)
{
    return this->rawAddEditorTab(setFocus, QString(), source, tabIndex);
}

int EditorTabWidget::rawAddEditorTab(bool setFocus, QString title, EditorTabWidget *source, int sourceTabIndex)
{
#ifdef QT_DEBUG
    QElapsedTimer __aet_timer;
    __aet_timer.start();
#endif

    bool create = (source == 0);

    this->setUpdatesEnabled(false);

    Editor *editor;

    QString oldText;
    QIcon oldIcon;
    QString oldTooltip;

    if (create) {
        editor = new Editor(this);
    } else {
        editor = (Editor *)source->widget(sourceTabIndex);

        oldText = source->tabText(sourceTabIndex);
        oldIcon = source->tabIcon(sourceTabIndex);
        oldTooltip = source->tabToolTip(sourceTabIndex);
    }

    int index = this->addTab(editor, create ? title : oldText);
    if (!create) {
        source->disconnectEditorSignals(editor);
    }
    this->connectEditorSignals(editor);

    if(setFocus) {
        this->setCurrentIndex(index);
        editor->setFocus();
    }

    if (create) {
        this->setSavedIcon(index, true);
    } else {
        this->setTabIcon(index, oldIcon);
        this->setTabToolTip(index, oldTooltip);
    }

    this->setUpdatesEnabled(true);

    emit editorAdded(index);

#ifdef QT_DEBUG
    qint64 __aet_elapsed = __aet_timer.nsecsElapsed();
    qDebug() << QString("Tab opened in " + QString::number(__aet_elapsed / 1000 / 1000) + "msec").toStdString().c_str();
#endif

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

void EditorTabWidget::on_cleanChanged(bool isClean)
{
    int index = this->indexOf((QWidget *)sender());
    if(index >= 0)
        this->setSavedIcon(index, isClean);
}
