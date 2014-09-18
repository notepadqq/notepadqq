#include "include/iconprovider.h"

IconProvider::IconProvider()
{
}

QIcon IconProvider::fromTheme(const QString &name)
{
    // FIXME Cache the icons
    QIcon icon = QIcon::fromTheme(name);
    if (icon.isNull()) {
        QString basePath = ":/icons/default/%1/%2.%3";
        QList<QPair<int, QString>> sizes {
                    QPair<int, QString>(16, "16x16"),
                    QPair<int, QString>(64, "64x64"),
                    QPair<int, QString>(128, "128x128"),
        };

        for (const QPair<int, QString> & size : sizes) {
            icon.addFile(basePath.arg(size.second).arg(name).arg("png"),
                         QSize(size.first, size.first));

        }

        icon.addFile(basePath.arg("scalable").arg(name).arg("svg"),
                     QSize(512, 512));
    }

    return icon;
}
