#include "include/iconprovider.h"
#include "include/svgiconengine.h"

#include <QDebug>
#include <QFileInfo>

IconProvider::IconProvider()
{
}

QIcon IconProvider::fromTheme(const QString &name)
{
    // FIXME Cache the icons

    QString basePath = ":/icons/notepadqq/%1/%2.%3";

    if (QIcon::hasThemeIcon(name)) {
        return QIcon::fromTheme(name);

    } else if (QFileInfo(basePath.arg("scalable").arg(name).arg("svg")).exists()) {
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
            icon.addFile(basePath.arg(size.second).arg(name).arg("png"),
                         QSize(size.first, size.first));
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
