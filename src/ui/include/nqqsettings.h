#ifndef NQQSETTINGS_H
#define NQQSETTINGS_H

#include <QAction>
#include <QList>
#include <QSettings>
#include <QString>

/*
 * The use of NQQ_SETTING creates several functions:
 * getXXX(), setXXX(value), resetXXX(), hasXXX()
 *
 * hasXXX() returns whether the setting exists or not.
 *
 * NQQ_SETTING_WITH_KEY creates the same functions, but inserts an additional key param.
 * This is useful when there are a variable number of settings.
 *
 * BEGIN_CATEGORY and END_CATEGORY create a nested class that is used to group settings
 * together. Because of a quick of QSettings, BEGIN_GENERAL_CATEGORY has to be used if the
 * keys are supposed to be written to the [General] section of the .ini file.
 *
 * The BEGIN_CATEGORY macro automagically generates two member variables:
 * [const QString _m_category] is the name of the category given to the macro.
 * [QSettings _m_settings] is the QSettings objects from NqqSettings.
 * It is a good idea to prefix all private members with _ when creating custom categories.
 * This way they will be moved to the very bottom of the auto-complete list.
 *
 * */

#define NQQ_SETTING(Name, Type, Default) \
    Type get##Name() const { return _m_settings.value(_m_category+#Name,Default).value<Type>(); } \
    void set##Name(const Type& newValue) { _m_settings.setValue(_m_category+#Name, newValue); } \
    Type reset##Name() { _m_settings.setValue(_m_category+#Name,Default); return Default; } \
    bool has##Name() const { return _m_settings.contains(_m_category+#Name); }

#define NQQ_SETTING_WITH_KEY(Name, Type, Default) \
    Type get##Name(const QString& key) const { return _m_settings.value(_m_category+key+"/"#Name,Default).value<Type>(); } \
    void set##Name(const QString& key, const Type& newValue) { _m_settings.setValue(_m_category+key+"/"#Name, newValue); } \
    Type reset##Name(const QString& key) { _m_settings.setValue(_m_category+key+"/"#Name,Default); return Default; } \
    bool has##Name(const QString& key) const { return _m_settings.contains(_m_category+key+"/"#Name); }

#define BEGIN_CATEGORY(Name) \
    class _Category##Name { \
    private: \
    QSettings& _m_settings; \
    const QString _m_category = #Name"/"; \
    public: \
    _Category##Name(QSettings& settings) : _m_settings(settings) {} \

#define BEGIN_GENERAL_CATEGORY(Name) \
    class _Category##Name { \
    private: \
    QSettings& _m_settings; \
    const QString _m_category; \
    public: \
    _Category##Name(QSettings& settings) : _m_settings(settings) {} \

#define END_CATEGORY(Name) \
    }; _Category##Name Name = _Category##Name(_m_settings);

class NqqSettings {

public:

    BEGIN_GENERAL_CATEGORY(General)
        NQQ_SETTING(Localization,                   QString,    "")
        NQQ_SETTING(WarnForDifferentIndentation,    bool,       true)
        NQQ_SETTING(ExitOnLastTabClose,             bool,       false)

        NQQ_SETTING(CollectStatistics,              bool,       false)
        NQQ_SETTING(LastStatisticTransmissionTime,  qint64,     0)
        NQQ_SETTING(StatisticsDialogShown,          int,        0)

        NQQ_SETTING(WordWrap,                       bool,       false)
        NQQ_SETTING(Zoom,                           qreal,      1.0)

        NQQ_SETTING(ShowAllSymbols,                 bool,       false)
        NQQ_SETTING(TabsVisible,                    bool,       false)
        NQQ_SETTING(SpacesVisisble,                 bool,       false)
        NQQ_SETTING(ShowEOL,                        bool,       false)

        NQQ_SETTING(RememberTabsOnExit,             bool,       true)
        NQQ_SETTING(AutosaveInterval,               int,        15)      // In seconds
        NQQ_SETTING(LastSelectedDir,                QString,    ".")
        NQQ_SETTING(LastSelectedSessionDir,         QString,    QString())
        NQQ_SETTING(RecentDocuments,                QList<QVariant>, QList<QVariant>())
        NQQ_SETTING(WarnIfFileLargerThan,           int,        1)

        NQQ_SETTING(NotepadqqVersion,               QString,    QString())
        NQQ_SETTING(SmartIndentation,               bool,       true)
        NQQ_SETTING(MathRendering,                  bool,       false)
        NQQ_SETTING(UseNativeFilePicker,            bool,       true)
    END_CATEGORY(General)

    BEGIN_CATEGORY(Appearance)
        NQQ_SETTING(ColorScheme,        QString,    "")
        NQQ_SETTING(OverrideFontFamily, QString,    "")
        NQQ_SETTING(OverrideFontSize,   int,        0)
        NQQ_SETTING(OverrideLineHeight, double,     0)
        NQQ_SETTING(ShowLineNumbers, bool,       true)
    END_CATEGORY(Appearance)

    BEGIN_CATEGORY(Search)
        NQQ_SETTING(SearchAsIType,  bool,           true)
        NQQ_SETTING(SaveHistory,    bool,           true)
        NQQ_SETTING(SearchHistory,  QStringList,    QStringList())
        NQQ_SETTING(ReplaceHistory, QStringList,    QStringList())
        NQQ_SETTING(FileHistory,    QStringList,    QStringList())
        NQQ_SETTING(FilterHistory,  QStringList,    QStringList())
    END_CATEGORY(Search)

    BEGIN_CATEGORY(Extensions)
        NQQ_SETTING(RuntimeNodeJS,  QString, QString())
        NQQ_SETTING(RuntimeNpm,     QString, QString())
    END_CATEGORY(Extensions)

    BEGIN_CATEGORY(Languages)
        NQQ_SETTING_WITH_KEY(IndentWithSpaces,      bool,   false)
        NQQ_SETTING_WITH_KEY(TabSize,               int,    4)
        NQQ_SETTING_WITH_KEY(UseDefaultSettings,    bool,   true)
    END_CATEGORY(Languages)

    BEGIN_CATEGORY(MainWindow)
        NQQ_SETTING(Geometry,       QByteArray, QByteArray())
        NQQ_SETTING(WindowState,    QByteArray, QByteArray())
        NQQ_SETTING(MenuBarVisible, bool,       true)
        NQQ_SETTING(ToolBarItems,   QString,    QString())
    END_CATEGORY(MainWindow)


    //A few of the more involved settings can't be handled like above.
    BEGIN_CATEGORY(Shortcuts)

        /**
         * @brief Since default shortcuts aren't stored in the settings, we have to set them manually.
         * @param actions List of actions that have or could have shortcuts.
         */
        void initShortcuts(const QList<QAction*>& actions){
            for (QAction* a : actions){
                if (a->objectName().isEmpty())
                    continue;

                const QString key = "Shortcuts/" + a->objectName();

                //Only update the current shortcut if it's actually set in the settings.
                QKeySequence shortcut = _m_settings.contains(key) ?
                                            _m_settings.value(key).toString() : a->shortcut();

                _m_shortcuts.insert( a->objectName(), _ActionItem{a->shortcut(), shortcut} );
            }
        }

        QKeySequence getDefaultShortcut(const QString& actionName) const {
            auto it = _m_shortcuts.find(actionName);

            if (it == _m_shortcuts.end())
                return QKeySequence();

            return it.value().defaultSequence;
        }

        QKeySequence getShortcut(const QString& actionName) const {
            auto it = _m_shortcuts.find(actionName);

            if (it == _m_shortcuts.end())
                return QKeySequence();

            return it.value().sequence;
        }

        void setShortcut(const QString& actionName, const QKeySequence& sequence){
            auto it = _m_shortcuts.find(actionName);

            if (it == _m_shortcuts.end())
                return;

            it.value().sequence = sequence;
            _m_settings.setValue("Shortcuts/" + actionName, sequence.toString());
        }

    private:
        struct _ActionItem {
            QKeySequence defaultSequence;
            QKeySequence sequence;
        };
        QMap<QString, _ActionItem> _m_shortcuts;

    END_CATEGORY(Shortcuts)
    BEGIN_CATEGORY(Run)
        void resetCommands()
        {
            _m_settings.beginGroup("Run");
            _m_settings.remove("");
            _m_settings.endGroup();
        }

        QMap <QString, QString> getCommands()
        {
            QMap <QString, QString> ret;
            _m_settings.beginGroup("Run");
            QStringList groups = _m_settings.childGroups();
            for (int i = 0; i < groups.size(); ++i) {
                _m_settings.beginGroup(groups.at(i));
                const QString& name = _m_settings.value("name","").toString();
                const QString& cmd = _m_settings.value("command","").toString();
                ret.insert(name, cmd);
                _m_settings.endGroup();
            }
            _m_settings.endGroup();
            return ret;
        }
        
        void setCommand(const QString &cmdName, const QString &cmdRun)
        {
            _m_settings.beginGroup("Run");
            const int pos = _m_settings.childGroups().size()+1;
            QString group = "c" + QString::number(pos);
            _m_settings.setValue(group + "/name", cmdName);
            _m_settings.setValue(group + "/command", cmdRun);
            _m_settings.endGroup();
        }

    END_CATEGORY(Run)
    /**
     * @brief Some keys have changed since v0.53. To maintain compatibility, this function
     *        parses through the QSettings file and fixes these entries.
     *        Should be called once at program initiation, before getInstance() is called.
     */
    static void ensureBackwardsCompatibility();

    static NqqSettings& getInstance();

private:
    QSettings _m_settings;

    NqqSettings(){}
    NqqSettings& operator=(NqqSettings&) = delete;
};

#endif // NQQSETTINGS_H
