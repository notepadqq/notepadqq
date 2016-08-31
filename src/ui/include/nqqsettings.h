#ifndef NQQSETTINGS_H
#define NQQSETTINGS_H

#include <QSettings>
#include <QString>
#include <QList>
#include <QAction>
#include <QDebug>

//Macros to make defining settings easier.
#define NQQ_SETTING(Name, Type, Default) \
    Type get##Name() const { return m_settings.value(#Name,Default).value<Type>(); } \
    void set##Name(const Type& newValue) { m_settings.setValue(#Name, newValue); } \
    Type reset##Name() { m_settings.setValue(#Name,Default); return Default; }

#define NQQ_SETTING_C(Category, Name, Type, Default) \
    Type get##Name() const { return m_settings.value(#Category"/"#Name,Default).value<Type>(); } \
    void set##Name(const Type& newValue) { m_settings.setValue(#Category"/"#Name, newValue); } \
    Type reset##Name() { m_settings.setValue(#Category"/"#Name,Default); return Default; }


class NqqSettings{

public:
    //General settings here. Each entry follows the same pattern:
    //          Name                            Type        Default Value
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


    //If the items are supposed to appear in a group, this macro can be used instead.
    //Same deal, just with an additional parameter denoting the group it belongs to.
    NQQ_SETTING_C("Appearance", OverrideFontFamily, QString,    "")
    NQQ_SETTING_C("Appearance", OverrideFontSize,   int,        0)



    /*NqqSettings(QObject* parent = nullptr)
        : m_settings(parent)
    {}*/

    //A few of the more involved settings can't be handled like above.

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

        qDebug() << "Set Shortcut start";

        if(it == m_shortcuts.end())
            return;

        it.value().action->setShortcut(sequence);
        m_settings.setValue("Shortcuts/" + actionName, sequence.toString());

        qDebug() << "Set Shortcut finish";
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


    static NqqSettings& getInstance(){
        static NqqSettings settings;
        return settings;
    }

    //Not private for now since not NqqSettings can't handle everything yet.
    QSettings m_settings;

private:
    NqqSettings(){}
};

#endif // NQQSETTINGS_H
