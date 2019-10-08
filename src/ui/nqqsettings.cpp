#include "include/nqqsettings.h"

#include "include/notepadqq.h"

#ifdef QT_DEBUG
#include <QDebug>
#endif

void NqqSettings::ensureBackwardsCompatibility()
{
    QSettings s;

    // Check the Nqq version, if it's below 0.53.0 we're using the old settings.
    const QString nqqVersion = s.value("NotepadqqVersion").toString();
    const QStringList versionList = nqqVersion.split(".");

    // Only proceed with checking version if the key seems valid
    if (versionList.size() < 3)
        return;

    const int major = versionList[0].toInt();
    const int minor = versionList[1].toInt();
    const int revision = versionList[2].toInt();

    auto replace = [&](const QString& newKey, const QString& oldKey) {
        s.setValue(newKey, s.value(oldKey));
        s.remove(oldKey);
    };

    // Just clear all settings if we're dealing with stone-age 0.53.0
    if (major == 0 && (minor <= 53 || (minor == 53 && revision > 0))) {
        s.clear();
        return;
    }

    // For versions of 1.2.0 and below we need to adjust action names
    if(major == 1 && minor < 3) {

        struct Pair {
            const char* oldName;
            const char* newName;
        };

        Pair arr[] =
        {
            {"actionC_lose_All","actionClose_All"},
            {"actionCu_t","actionCut"},
            {"actionE_xit","actionExit"},
            {"actionGo_to_line","actionGo_to_Line"},
            {"actionReload_file_interpreted_as","actionReload_File_Interpreted_As"},
            {"action_Copy","actionCopy"},
            {"action_Delete","actionDelete"},
            {"action_New","actionNew"},
            {"action_Open","actionOpen"},
            {"action_Paste","actionPaste"},
            {"action_Redo","actionRedo"},
            {"action_Run","actionRun"},
            {"action_Undo","actionUndo"},
            {"actionCurrent_Full_File_path_to_Clipboard","actionCurrent_Full_File_Path_to_Clipboard"},
            {"actionIndentation_Default_settings","actionIndentation_Default_Settings"},
            {"actionInterpret_as","actionInterpret_As"},

        };

        for (const QString& key : s.allKeys()){
            if (key.isEmpty() || !key.startsWith("Shortcuts/"))
                continue;

            QString actionName = key.mid( strlen("Shortcuts/"));

            auto it = std::find_if(std::begin(arr), std::end(arr), [&actionName](const Pair& pair){
                return pair.oldName == actionName;
            });

            if (it == std::end(arr))
                continue;

            replace(QString("Shortcuts/") + it->newName, key);
        }

    }

}

NqqSettings&NqqSettings::getInstance(){
    static NqqSettings settings;
    return settings;
}
