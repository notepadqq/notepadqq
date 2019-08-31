#include "include/editortabwidget.h"

#include "include/iconprovider.h"

#include <QApplication>
#include <QFileInfo>
#include <QTabBar>

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

    connect(this, &EditorTabWidget::currentChanged, this, &EditorTabWidget::on_currentTabChanged);

#ifdef Q_OS_MACX
    this->tabBar()->setExpanding(true);
    this->setUsesScrollButtons(true);
#else
    QString style = QString("QTabBar::tab{min-width:100px; height:24px;}");
    setStyleSheet(style);
#endif

}

EditorTabWidget::~EditorTabWidget()
{
    // Manually remove each tab to keep m_editorPointers consistent
    for (int i = this->count() - 1; i >= 0; i--) {
        QSharedPointer<Editor> edt = editor(i);
        m_editorPointers.remove(edt.data());
        // Remove the parent so that QObject cannot destroy the
        // object (QSharedPointer will take care of it).
        edt->setParent(nullptr);
    }
}

int EditorTabWidget::addEditorTab(bool setFocus, const QString &title)
{
    return this->rawAddEditorTab(setFocus, title, 0, 0);
}

void EditorTabWidget::connectEditorSignals(Editor *editor)
{
    connect(editor, &Editor::cleanChanged,
            this, &EditorTabWidget::on_cleanChanged);

    connect(editor, &Editor::gotFocus, this, &EditorTabWidget::gotFocus);

    connect(editor, &Editor::mouseWheel,
            this, &EditorTabWidget::on_editorMouseWheel);

    connect(editor, &Editor::fileNameChanged,
            this, &EditorTabWidget::on_fileNameChanged);
}

void EditorTabWidget::disconnectEditorSignals(Editor *editor)
{
    disconnect(editor, &Editor::cleanChanged,
               this, &EditorTabWidget::on_cleanChanged);

    disconnect(editor, &Editor::gotFocus, this, &EditorTabWidget::gotFocus);

    disconnect(editor, &Editor::mouseWheel,
               this, &EditorTabWidget::on_editorMouseWheel);

    disconnect(editor, &Editor::fileNameChanged,
               this, &EditorTabWidget::on_fileNameChanged);
}

int EditorTabWidget::indexOf(QSharedPointer<Editor> editor) const
{
    return indexOf(editor.data());
}

int EditorTabWidget::indexOf(QWidget *widget) const
{
    return QTabWidget::indexOf(widget);
}

QString EditorTabWidget::tabText(Editor* editor) const
{
    return editor->tabName();
}

QString EditorTabWidget::tabText(int index) const
{
    return editor(index)->tabName();
}

void EditorTabWidget::setTabText(int index, const QString& text)
{
    QTabWidget::setTabText(index, text);
    editor(index)->setTabName(text);
}


void EditorTabWidget::setTabText(Editor* editor, const QString& text)
{
    int idx = indexOf(editor);

    if(idx >= 0) {
        QTabWidget::setTabText(idx, text);
        editor->setTabName(text);
    }
}

int EditorTabWidget::transferEditorTab(bool setFocus, EditorTabWidget *source, int tabIndex)
{
    return this->rawAddEditorTab(setFocus, QString(), source, tabIndex);
}

/**
 * @brief Do NOT directly connect to Editor signals within this method,
 *        or they'll remain attached to this EditorTabWidget whenever the
 *        tab gets moved to another container. Use connectEditorSignals()
 *        and disconnectEditorSignals() methods instead.
 */
int EditorTabWidget::rawAddEditorTab(const bool setFocus, const QString &title, EditorTabWidget *source, const int sourceTabIndex)
{
#ifdef QT_DEBUG
    QElapsedTimer __aet_timer;
    __aet_timer.start();
#endif

    bool create = (source == 0);

    this->setUpdatesEnabled(false);

    QSharedPointer<Editor> editor;

    QString oldText;
    QIcon oldIcon;
    QString oldTooltip;

    if (create) {
        editor = Editor::getNewEditor(this);
    } else {
        editor = source->editor(sourceTabIndex);

        oldText = source->tabText(sourceTabIndex);
        oldIcon = source->tabIcon(sourceTabIndex);
        oldTooltip = source->tabToolTip(sourceTabIndex);
    }

    m_editorPointers.insert(editor.data(), editor);

    // Calling adTab() triggers MainWindow::refreshEditorUiInfo. We want to set the tab title
    // before that happens so it can be displayed properly.
    const QString& tabTitle = create ? title : oldText;
    editor->setTabName(tabTitle);
    int index = addTab(editor.data(), tabTitle);

    if (!create) {
        source->disconnectEditorSignals(editor.data());
    }
    this->connectEditorSignals(editor.data());

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

    // Common setup
    editor->setZoomFactor(m_zoomFactor);

    this->setUpdatesEnabled(true);

    emit editorAdded(index);

#ifdef QT_DEBUG
    qint64 __aet_elapsed = __aet_timer.nsecsElapsed();
    qDebug() << QString("Tab opened in " + QString::number(__aet_elapsed / 1000 / 1000) + "msec").toStdString().c_str();
#endif

    return index;
}

int EditorTabWidget::findOpenEditorByUrl(const QUrl &filename)
{
    QUrl absFileName = filename;
    if (absFileName.isLocalFile())
        absFileName = QUrl::fromLocalFile(QFileInfo(filename.toLocalFile()).absoluteFilePath());

    for (int i = 0; i < count(); i++) {
        auto editor = this->editor(i);
        if (editor->filePath() == filename)
            return i;
    }

    return -1;
}

QSharedPointer<Editor> EditorTabWidget::editor(int index) const
{
    Editor *ed = dynamic_cast<Editor *>(this->widget(index));
    return m_editorPointers.value(ed);
}

QSharedPointer<Editor> EditorTabWidget::editor(Editor *editor) const
{
    return m_editorPointers.value(editor);
}

void EditorTabWidget::tabRemoved(int)
{
    // FIXME Find a more efficient way to get the deleted editor

    QList<QWidget*> tabs;
    for (int i = 0; i < this->count(); i++) {
        tabs.append(widget(i));
    }

    for (QSharedPointer<Editor> editor : m_editorPointers) {
        if (!tabs.contains(editor.data())) {
            // Editor is the one that has been removed!
            if (editor.data() != nullptr) {
                // Set no parent, so that QObject won't delete
                // the editor: that's what QSharedPointer should do.
                editor->setParent(nullptr);
                disconnectEditorSignals(editor.data());
            }

            m_editorPointers.remove(editor.data());
            break;
        }
    }
}

QSharedPointer<Editor> EditorTabWidget::currentEditor()
{
    return editor(currentIndex());
}

QString EditorTabWidget::tabTextFromEditor(QSharedPointer<EditorNS::Editor> ed)
{
    for(int i=0; i<count(); ++i)
        if (editor(i) == ed) return tabText(i);

    return QString();
}

qreal EditorTabWidget::zoomFactor() const
{
    return m_zoomFactor;
}

void EditorTabWidget::setZoomFactor(const qreal &zoomFactor)
{
    m_zoomFactor = zoomFactor;

    for (int i = 0; i < count(); i++) {
        editor(i)->setZoomFactor(zoomFactor);
    }
}

void EditorTabWidget::deleteIfEmpty()
{
    EditorTabWidget::deleteIfEmpty(this);
}

void EditorTabWidget::deleteIfEmpty(EditorTabWidget *tabWidget) {
    if(tabWidget->count() == 0) {
        delete tabWidget;
    }
}

void EditorTabWidget::setSavedIcon(int index, bool saved)
{
    if (saved)
        this->setTabIcon(index, IconProvider::fromTheme("document-saved"));
    else
        this->setTabIcon(index, IconProvider::fromTheme("document-unsaved"));
}

void EditorTabWidget::setTabBarHidden(bool yes)
{
    tabBar()->setHidden(yes);
}

void EditorTabWidget::setTabBarHighlight(bool yes)
{
    // Get colors from palette so it doesn't look fugly.
    QPalette palette = tabBar()->palette();
    palette.setColor(QPalette::Highlight, yes ? QApplication::palette().highlight().color() : QApplication::palette().light().color());
    tabBar()->setPalette(palette);
}

void EditorTabWidget::on_cleanChanged(bool isClean)
{
    Editor *editor = dynamic_cast<Editor *>(sender());
    if (!editor)
        return;

    int index = indexOf(editor);
    if(index >= 0)
        setSavedIcon(index, isClean);
}

void EditorTabWidget::on_editorMouseWheel(QWheelEvent *ev)
{
    Editor *editor = dynamic_cast<Editor *>(sender());
    if (!editor)
        return;

    emit editorMouseWheel(indexOf(editor), ev);
}

void EditorTabWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::MiddleButton) {
        int index = tabBar()->tabAt(ev->pos());

        if (index != -1) {
            emit tabCloseRequested(index);
            ev->accept();
            return;
        }
    }

    QTabWidget::mouseReleaseEvent(ev);
}

QString EditorTabWidget::generateTabTitleForUrl(const QUrl &filename) const
{
    QString fileName = QFileInfo(filename.toDisplayString(QUrl::RemoveScheme |
                                                   QUrl::RemovePassword |
                                                   QUrl::RemoveUserInfo |
                                                   QUrl::RemovePort |
                                                   QUrl::RemoveAuthority |
                                                   QUrl::RemoveQuery |
                                                   QUrl::RemoveFragment |
                                                   QUrl::PreferLocalFile
                                                   )).fileName();
    return fileName;
}

void EditorTabWidget::on_fileNameChanged(const QUrl & /*oldFileName*/, const QUrl &newFileName)
{
    Editor *editor = dynamic_cast<Editor *>(sender());
    if (!editor)
        return;

    int index = indexOf(editor);

    QString fullFileName = newFileName.toDisplayString(QUrl::PreferLocalFile |
                                                       QUrl::RemovePassword);

    setTabText(index, generateTabTitleForUrl(newFileName));
    setTabToolTip(index, fullFileName);
}

int EditorTabWidget::formerTabIndex()
{
    return m_formerTabIndex;
}

void EditorTabWidget::on_currentTabChanged(int index)
{
    // Store current index to become former index on next tab change.
    if (m_mostRecentTabIndex != index) {
        m_formerTabIndex = m_mostRecentTabIndex;
        m_mostRecentTabIndex = index;
    }
}
