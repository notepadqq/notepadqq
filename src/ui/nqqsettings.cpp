#include "include/nqqsettings.h"

void NqqSettings::ensureBackwardsCompatibility()
{
    QSettings s;

    const QStringList keys = s.allKeys();

    //If this string is in the settings we either have the new ini settings
    if(keys.contains("BackwardsCompatible"))
            return;

    qDebug() << "Old settings detected. Replacing keys.";

    auto replace = [&](const QString& newKey, const QString& oldKey) {
        s.setValue(newKey, s.value(oldKey));
        s.remove(oldKey);
    };


    for(const QString key : keys){
        if(key.isEmpty()) return;

        QString newKey = key;

        if(!key.contains('/')) {
            //Key is from the [General] section. All keys here just need their first letter fixed.
            newKey[0] = newKey[0].toUpper();
        }

        else if(key == "Extensions/Runtime_Nodejs")
            newKey = "Extensions/RuntimeNodeJS";
        else if(key == "Extensions/Runtime_Npm")
            newKey = "Extensions/RuntimeNpm";
        else if(key.startsWith("MainWindow/") || key.startsWith(("Search/"))) {
            //Only capitalize first letter after the '/'
            const int c = key.indexOf('/');
            newKey[c+1] = newKey[c+1].toUpper();
        }else if(key.startsWith("Languages/")) {
            //Capitalize first letter after the second '/'
            int c = key.indexOf('/',10);

            //If there is no second '/', we found the settings for the 'default' language.
            //They'll be migrated to Languages/default.
            if(c==-1) {
                newKey.replace("Languages/", "Languages/default/");
                c = newKey.indexOf('/',10);
            }

            newKey[c+1] = newKey[c+1].toUpper();
        }else
            continue;

        replace(newKey, key);
    }

    s.setValue("BackwardsCompatible", true);
}

NqqSettings&NqqSettings::getInstance(){
    static NqqSettings settings;
    return settings;
}
