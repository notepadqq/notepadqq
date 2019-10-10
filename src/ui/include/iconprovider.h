#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QIcon>

class IconProvider
{
public:
    IconProvider();
    static QIcon getFallbackIcon(const QString &name);
    static QIcon fromTheme(const QString &iconName, const QString &fallbackName);
    static QIcon fromTheme(const QString &iconName, const bool &fallbackBuiltin = true);
    static QIcon fromTheme(const QStringList &iconNames, const QString &fallbackName);
    static QIcon fromTheme(const QStringList &iconNames, const bool &fallbackBuiltin = true);
    static QIcon fromTheme(const QString &iconName, const QStringList &iconNames, const QString &fallbackName);
    static QIcon fromTheme(const QString &iconName, const QStringList &iconNames, const bool &fallbackBuiltin = true);
};

#endif // ICONPROVIDER_H
