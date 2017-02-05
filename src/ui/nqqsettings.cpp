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
    if (versionList.size() == 3){
        const int major = versionList[0].toInt();
        const int minor = versionList[1].toInt();
        const int revision = versionList[2].toInt();

        // return if we're above 0.53.0
        if (major > 0 || minor > 53 || (minor == 53 && revision > 0))
            return;
    }

    const QStringList keys = s.allKeys();

#ifdef QT_DEBUG
    qDebug() << "Old Nqq version detected. Replacing keys.";
#endif

    auto replace = [&](const QString& newKey, const QString& oldKey) {
        s.setValue(newKey, s.value(oldKey));
        s.remove(oldKey);
    };


    for (const QString key : keys){
        if (key.isEmpty())
            return;

        QString newKey = key;

        if (!key.contains('/')) {
            // Key is from the [General] section. All keys here just need their first letter fixed.
            newKey[0] = newKey[0].toUpper();
        } else if (key == "Extensions/Runtime_Nodejs") {
            newKey = "Extensions/RuntimeNodeJS";
        } else if (key == "Extensions/Runtime_Npm") {
            newKey = "Extensions/RuntimeNpm";
        } else if (key.startsWith("MainWindow/") || key.startsWith(("Search/"))) {
            // Only capitalize first letter after the '/'
            const int c = key.indexOf('/');
            newKey[c+1] = newKey[c+1].toUpper();
        } else if (key.startsWith("Languages/")) {
            // Capitalize first letter after the second '/'
            int c = key.indexOf('/', 10);

            // If there is no second '/', we found the settings for the 'default' language.
            // They'll be migrated to Languages/default.
            if (c == -1) {
                newKey.replace("Languages/", "Languages/default/");
                c = newKey.indexOf('/', 10);
            }

            newKey[c+1] = newKey[c+1].toUpper();
        } else {
            continue;
        }

        replace(newKey, key);
    }
}

NqqSettings&NqqSettings::getInstance(){
    static NqqSettings settings;
    return settings;
}
