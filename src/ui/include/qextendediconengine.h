#ifndef QEXTENDEDICONENGINE_H
#define QEXTENDEDICONENGINE_H

#include <QIcon>
#include <QIconEngine>

#include <memory>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QExtendedIconEngine : public QIconEngine {
public:
    QExtendedIconEngine();
    QExtendedIconEngine(const QExtendedIconEngine& extendedIconEngine);
    QExtendedIconEngine(const QIcon& icon, const QString& fallbackName);
    QExtendedIconEngine(const QIcon& icon, const bool& fallbackBuiltin = true)
        : QExtendedIconEngine(icon, fallbackBuiltin ? icon.name() : QString())
    {
    }
    QExtendedIconEngine(const QString& iconName, const bool& fallbackBuiltin = true)
        : QExtendedIconEngine(QIcon::fromTheme(iconName), fallbackBuiltin)
    {
    }
    QExtendedIconEngine(const QString& iconName, const QString& fallbackName)
        : QExtendedIconEngine(QIcon::fromTheme(iconName), fallbackName)
    {
    }
    QExtendedIconEngine(const QIcon& icon, const QExtendedIconEngine& fallback);
    QExtendedIconEngine(const QString& iconName, const QExtendedIconEngine& fallback)
        : QExtendedIconEngine(QIcon::fromTheme(iconName), fallback)
    {
    }
    QExtendedIconEngine(const QIcon& icon, std::shared_ptr<QExtendedIconEngine> fallback);
    QExtendedIconEngine(const QString& iconName, std::shared_ptr<QExtendedIconEngine> fallback)
        : QExtendedIconEngine(QIcon::fromTheme(iconName), fallback)
    {
    }
    QExtendedIconEngine(const QStringList& iconNames, const QString& fallbackName);
    QExtendedIconEngine(const QStringList& iconNames, const bool& fallbackBuiltin = true)
        : QExtendedIconEngine(
              iconNames, fallbackBuiltin ? (!iconNames.isEmpty() ? iconNames.constFirst() : QString()) : QString())
    {
    }
    QExtendedIconEngine(const QString& iconName, const QStringList& iconNames, const QString& fallbackName)
        : QExtendedIconEngine(QStringList(iconName) + iconNames, fallbackName)
    {
    }
    QExtendedIconEngine(const QString& iconName, const QStringList& iconNames, const bool& fallbackBuiltin = true)
        : QExtendedIconEngine(iconName, iconNames, fallbackBuiltin ? iconName : QString())
    {
    }
    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;
    QSize actualSize(const QSize& size, QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;

    QString key() const override;
    QIconEngine* clone() const override;

    QList<QSize> availableSizes(QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off) const override;

    QString iconName() const override;
    // bool isNull() const override; // ### Qt6 make virtual

    void virtual_hook(int id, void* data) override;

    void setIcon(const QIcon& icon);
    void setFallback(const QExtendedIconEngine& fallback);
    void setFallback(std::shared_ptr<QExtendedIconEngine> fallback);
    void set(const QIcon& icon, const QExtendedIconEngine& fallback);
    void set(const QIcon& icon, std::shared_ptr<QExtendedIconEngine> fallback);
    void unsetIcon();
    void unsetFallback();
    void unset();
    bool hasFallback() const;

    QIcon getIcon() const;
    QIcon getIconDefault() const;
    std::shared_ptr<QExtendedIconEngine> getFallback() const;

private:
    QIcon icon = QIcon();
    std::shared_ptr<QExtendedIconEngine> fallback = nullptr;
};

QT_END_NAMESPACE

#endif // QEXTENDEDICONENGINE_H
