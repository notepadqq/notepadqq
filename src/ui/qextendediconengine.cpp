#include "include/qextendediconengine.h"

#include "include/iconprovider.h"

#include <QPainter>

#include <iterator>

QT_BEGIN_NAMESPACE

QExtendedIconEngine::QExtendedIconEngine()
    : QIconEngine()
{
}

QExtendedIconEngine::QExtendedIconEngine(const QExtendedIconEngine& extendedIconEngine)
    : QIconEngine(extendedIconEngine)
{
    this->icon = QIcon(extendedIconEngine.getIconDefault());
    if (extendedIconEngine.hasFallback()) {
        this->fallback =
            std::make_shared<QExtendedIconEngine>(QExtendedIconEngine(*(extendedIconEngine.getFallback())));
    }
}

QExtendedIconEngine::QExtendedIconEngine(const QIcon& icon, const QString& fallbackName)
    : QIconEngine()
{
    this->icon = QIcon(icon);
    if (!fallbackName.isEmpty()) {
        QIcon iconFallback = IconProvider::getFallbackIcon(fallbackName);
        if (!iconFallback.isNull()) {
            this->fallback = std::make_shared<QExtendedIconEngine>(QExtendedIconEngine(iconFallback, false));
        }
    }
}

QExtendedIconEngine::QExtendedIconEngine(const QIcon& icon, const QExtendedIconEngine& fallback)
    : QIconEngine()
{
    this->icon = QIcon(icon);
    this->fallback = std::make_shared<QExtendedIconEngine>(QExtendedIconEngine(fallback));
}

QExtendedIconEngine::QExtendedIconEngine(const QIcon& icon, std::shared_ptr<QExtendedIconEngine> fallback)
    : QIconEngine()
{
    this->icon = QIcon(icon);
    this->fallback = fallback;
}

QExtendedIconEngine::QExtendedIconEngine(const QStringList& iconNames, const QString& fallbackName)
    : QIconEngine()
{
    QIcon iconFallback = QIcon();
    if (!fallbackName.isEmpty()) {
        iconFallback = IconProvider::getFallbackIcon(fallbackName);
    }

    if (iconNames.isEmpty()) {
        if (!iconFallback.isNull()) {
            this->icon = iconFallback;
        }
    } else {
        this->icon = QIcon::fromTheme(iconNames.constFirst());
        std::shared_ptr<QExtendedIconEngine> extendedIconEngine = nullptr;
        if (!iconFallback.isNull()) {
            extendedIconEngine = std::make_shared<QExtendedIconEngine>(QExtendedIconEngine(iconFallback, false));
        }

        if (iconNames.count() >= 2) {
            for (QStringList::const_reverse_iterator it = iconNames.crbegin(), end = std::prev(iconNames.crend());
                 it != end;
                 ++it) {
                extendedIconEngine =
                    std::make_shared<QExtendedIconEngine>(QExtendedIconEngine(*it, extendedIconEngine));
            }
        }

        this->fallback = extendedIconEngine;
    }
}

void QExtendedIconEngine::paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
{
    QSize pixmapSize = rect.size();
#if defined(Q_WS_MAC)
    pixmapSize *= qt_mac_get_scalefactor();
#endif
    painter->drawPixmap(rect, pixmap(pixmapSize, mode, state));
}

QSize QExtendedIconEngine::actualSize(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
    return getIcon().actualSize(size, mode, state);
}

QPixmap QExtendedIconEngine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
    return getIcon().pixmap(size, mode, state);
}

QString QExtendedIconEngine::key() const
{
    return QString("QExtendedIconEngine");
}

QIconEngine* QExtendedIconEngine::clone() const
{
    return new QExtendedIconEngine(*this);
}

QList<QSize> QExtendedIconEngine::availableSizes(QIcon::Mode mode, QIcon::State state) const
{
    return getIcon().availableSizes(mode, state);
}

QString QExtendedIconEngine::iconName() const
{
    return getIcon().name();
}

/*
bool QExtendedIconEngine::isNull() const;
{
    return getIcon().isNull();
}
*/

void QExtendedIconEngine::virtual_hook(int id, void* data)
{
    switch (id) {
    case QIconEngine::IsNullHook: {
        *reinterpret_cast<bool*>(data) = getIcon().isNull();
        break;
    }
    default: {
        QIconEngine::virtual_hook(id, data);
        break;
    }
    }
}

void QExtendedIconEngine::setIcon(const QIcon& icon)
{
    this->icon = QIcon(icon);
}

void QExtendedIconEngine::setFallback(const QExtendedIconEngine& fallback)
{
    this->fallback = std::make_shared<QExtendedIconEngine>(QExtendedIconEngine(fallback));
}

void QExtendedIconEngine::setFallback(std::shared_ptr<QExtendedIconEngine> fallback)
{
    this->fallback = fallback;
}

void QExtendedIconEngine::set(const QIcon& icon, const QExtendedIconEngine& fallback)
{
    setIcon(icon);
    setFallback(fallback);
}

void QExtendedIconEngine::set(const QIcon& icon, std::shared_ptr<QExtendedIconEngine> fallback)
{
    setIcon(icon);
    setFallback(fallback);
}

void QExtendedIconEngine::unsetIcon()
{
    this->icon = QIcon();
}

void QExtendedIconEngine::unsetFallback()
{
    this->fallback = nullptr;
}

void QExtendedIconEngine::unset()
{
    unsetIcon();
    unsetFallback();
}

bool QExtendedIconEngine::hasFallback() const
{
    return (this->fallback != nullptr);
}

QIcon QExtendedIconEngine::getIcon() const
{
    if (this->icon.isNull()) {
        if (this->fallback != nullptr) {
            return this->fallback.get()->getIcon();
        } else {
            return QIcon();
        }
    } else {
        return this->icon;
    }
}

QIcon QExtendedIconEngine::getIconDefault() const
{
    return this->icon;
}

std::shared_ptr<QExtendedIconEngine> QExtendedIconEngine::getFallback() const
{
    return this->fallback;
}

QT_END_NAMESPACE
