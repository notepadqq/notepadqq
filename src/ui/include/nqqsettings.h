#ifndef NQQSETTINGS_H
#define NQQSETTINGS_H

#include <QSettings>
#include <QString>
#include <QList>
#include <QAction>
#include <QDebug>

//Macros to make defining settings easier.
#define NQQ_SETTING(Name, Type, Default) \
    Type get##Name() const { return m_settings.value(m_category+#Name,Default).value<Type>(); } \
    void set##Name(const Type& newValue) { m_settings.setValue(m_category+#Name, newValue); } \
    Type reset##Name() { m_settings.setValue(m_category+#Name,Default); return Default; } \
    bool has##Name() const { return m_settings.contains(m_category+#Name); }

#define NQQ_SETTING_WITH_KEY(Name, Type, Default) \
    Type get##Name(const QString& key) const { return m_settings.value(m_category+key+"/"#Name,Default).value<Type>(); } \
    void set##Name(const QString& key, const Type& newValue) { m_settings.setValue(m_category+key+"/"#Name, newValue); } \
    Type reset##Name(const QString& key) { m_settings.setValue(m_category+key+"/"#Name,Default); return Default; } \
    bool has##Name(const QString& key) const { return m_settings.contains(m_category+key+"/"#Name); }

#define BEGIN_CATEGORY(Name) \
    class _Category##Name { \
    private: \
    QSettings& m_settings; \
    const QString m_category = #Name"/"; \
    public: \
    _Category##Name(QSettings& settings) : m_settings(settings) {} \

#define BEGIN_GENERAL_CATEGORY(Name) \
    class _Category##Name { \
    private: \
    QSettings& m_settings; \
    const QString m_category; \
    public: \
    _Category##Name(QSettings& settings) : m_settings(settings) {} \

#define END_CATEGORY(Name) \
    }; _Category##Name Name = _Category##Name(m_settings);

class NqqSettings {

public:

    BEGIN_GENERAL_CATEGORY(General)
        NQQ_SETTING(Localization,                   QString,    "en")
        NQQ_SETTING(CheckVersionAtStartup,          bool,       true)
        NQQ_SETTING(WarnForDifferentIndentation,    bool,       true)

        NQQ_SETTING(WordWrap,                       bool,       false)
        NQQ_SETTING(Zoom,                           qreal,      1.0)

        NQQ_SETTING(ShowAllSymbols,                 bool,       false)
        NQQ_SETTING(TabsVisible,                    bool,       false)
        NQQ_SETTING(SpacesVisisble,                 bool,       false)
        NQQ_SETTING(ShowEOL,                        bool,       false)

        NQQ_SETTING(LastSelectedDir,                QString,    ".")
        NQQ_SETTING(RecentDocuments,                QList<QVariant>, QList<QVariant>())
    END_CATEGORY(General)

    BEGIN_CATEGORY(Appearance)
        NQQ_SETTING(ColorScheme,        QString,    "")
        NQQ_SETTING(OverrideFontFamily, QString,    "")
        NQQ_SETTING(OverrideFontSize,   int,        0)
    END_CATEGORY(Appearance)

    BEGIN_CATEGORY(Search)
        NQQ_SETTING(SearchAsIType, bool, true)
        NQQ_SETTING(SearchHistory, QStringList, QStringList())
        NQQ_SETTING(ReplaceHistory, QStringList, QStringList())
        NQQ_SETTING(FileHistory, QStringList, QStringList())
        NQQ_SETTING(FilterHistory, QStringList, QStringList())

    END_CATEGORY(Search)

    BEGIN_CATEGORY(Extensions)
        NQQ_SETTING(RuntimeNodeJS, QString, QString())
        NQQ_SETTING(RuntimeNpm, QString, QString())
    END_CATEGORY(Extensions)

    BEGIN_CATEGORY(Languages)
        NQQ_SETTING_WITH_KEY(IndentWithSpaces, bool, false)
        NQQ_SETTING_WITH_KEY(TabSize, int, 4)
        NQQ_SETTING_WITH_KEY(UseDefaultSettings, bool, true)
    END_CATEGORY(Languages)

    BEGIN_CATEGORY(MainWindow)
        NQQ_SETTING(Geometry, QByteArray, QByteArray())
        NQQ_SETTING(WindowState, QByteArray, QByteArray())
    END_CATEGORY(MainWindow)



    //A few of the more involved settings can't be handled like above.
    BEGIN_CATEGORY(Shortcuts)

    struct ActionItem {
        QAction* action = nullptr;
        QKeySequence defaultSequence;

        ActionItem(QAction* a, QKeySequence ks)
            : action(a), defaultSequence(ks) {}

        ActionItem(){}
    };

    //Shortcuts:
    QMap<QString, ActionItem> m_shortcuts;
    void initShortcuts(const QList<QAction*>& actions){
        for(QAction* a : actions){
            if(!a->objectName().isEmpty()){
                QString shortcut = m_settings.value("Shortcuts/" + a->objectName()).toString();

                if(shortcut != "") a->setShortcut( shortcut );

                m_shortcuts.insert( a->objectName(), ActionItem{a, a->shortcut()} );
            }
        }

        qDebug() << "shortcuts init'd: " << m_shortcuts.size();
    }

    QKeySequence getShortcut(const QString& actionName) const {
        auto it = m_shortcuts.find(actionName);

        if(it != m_shortcuts.end())
            return it.value().action->shortcut();
        else
            return QKeySequence();
    }

    QKeySequence getShortcut(const QAction* action) const {
        return getShortcut(action->objectName());
    }

    void setShortcut(const QString& actionName, const QKeySequence& sequence){
        auto it = m_shortcuts.find(actionName);

        if(it == m_shortcuts.end())
            return;

        it.value().action->setShortcut(sequence);
        m_settings.setValue("Shortcuts/" + actionName, sequence.toString());
    }

    void setShortcut(const QAction* action, const QKeySequence& sequence){
        setShortcut(action->objectName(), sequence);
    }

    const ActionItem& getShortcutInfo(const QString& actionName){
        return m_shortcuts[actionName];
    }

    const QMap<QString, ActionItem>& getAllShortcuts() const {
        return m_shortcuts;
    }

    /*void resetShortcut(){
        auto it = m_shortcuts.find(actionName);

        if(it != m_shortcuts.end())
            it.value().action->setShortcut(it.value().defaultSequence);
    }*/

    void resetAllShortcuts(){
        for(auto&& it : m_shortcuts)
            it.action->setShortcut( it.defaultSequence );
    }

    END_CATEGORY(Shortcuts)




    static NqqSettings& getInstance(){
        static NqqSettings settings;
        return settings;
    }

private:
    QSettings m_settings;
    //QString m_category;

private:
    NqqSettings(){}
    NqqSettings& operator=(NqqSettings&) = delete;
};

#endif // NQQSETTINGS_H
