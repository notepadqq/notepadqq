#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QIcon>

class IconProvider
{
public:
    IconProvider();
    static QIcon fromTheme(const QString &name);
};

#endif // ICONPROVIDER_H
