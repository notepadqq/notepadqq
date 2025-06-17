#include "include/iconprovider.h"

#include "include/qextendediconengine.h"
#include "include/svgiconengine.h"

#include <QDebug>
#include <QFileInfo>

IconProvider::IconProvider()
{
}

QIcon IconProvider::getFallbackIcon(const QString& name)
{
    // FIXME Cache the icons

    QString basePath = ":/icons/notepadqq/%1/%2.%3";

    if (QFileInfo(basePath.arg("scalable").arg(name).arg("svg")).exists()) {
        return QIcon(SVGIconEngine::fromFile(basePath.arg("scalable").arg(name).arg("svg")));

    } else {
        // QIcon::setThemeName("notepadqq");
        QIcon icon;

        QList<QPair<int, QString>> sizes {
                    QPair<int, QString>(16, "16x16"),
                    QPair<int, QString>(64, "64x64"),
                    QPair<int, QString>(128, "128x128"),
        };

        if (QFileInfo(basePath.arg("scalable").arg(name).arg("svg")).exists()) {
            icon.addFile(basePath.arg("scalable").arg(name).arg("svg"),
                         QSize(512, 512));
        }

        for (const QPair<int, QString> & size : sizes) {
            if (QFileInfo(basePath.arg(size.second).arg(name).arg("png")).exists()) {
                icon.addFile(basePath.arg(size.second).arg(name).arg("png"), QSize(size.first, size.first));
            }
        }

        // Warn about missing icons.
        // This works also as a workaround for this Qt bug in macOS: https://bugreports.qt.io/browse/QTBUG-58344
        if (icon.pixmap(1, 1).isNull()) {
            qDebug() << "Missing icon: " << name;
            return QIcon();
        }

        return icon;
    }
}

QIcon IconProvider::fromTheme(const QString& iconName, const QString& fallbackName)
{
    return QIcon(new QExtendedIconEngine(iconName, fallbackName));
}

QIcon IconProvider::fromTheme(const QString& iconName, const bool& fallbackBuiltin)
{
    return IconProvider::fromTheme(iconName, fallbackBuiltin ? iconName : QString());
}

QIcon IconProvider::fromTheme(const QStringList& iconNames, const QString& fallbackName)
{
    return QIcon(new QExtendedIconEngine(iconNames, fallbackName));
}

QIcon IconProvider::fromTheme(const QStringList& iconNames, const bool& fallbackBuiltin)
{
    return IconProvider::fromTheme(
        iconNames, fallbackBuiltin ? (!iconNames.isEmpty() ? iconNames.constFirst() : QString()) : QString());
}

QIcon IconProvider::fromTheme(const QString& iconName, const QStringList& iconNames, const QString& fallbackName)
{
    return IconProvider::fromTheme(QStringList(iconName) + iconNames, fallbackName);
}

QIcon IconProvider::fromTheme(const QString& iconName, const QStringList& iconNames, const bool& fallbackBuiltin)
{
    return IconProvider::fromTheme(iconName, iconNames, fallbackBuiltin ? iconName : QString());
}
