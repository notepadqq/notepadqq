#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include <QIconEngine>
#include <QSvgRenderer>

class SVGIconEngine : public QIconEngine {

  QByteArray m_lightIcon;
  QByteArray m_darkIcon;

public:
  explicit SVGIconEngine(const std::string &iconBuffer);
  void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode,
             QIcon::State state) override;
  QIconEngine *clone() const override;
  QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                 QIcon::State state) override;

  static SVGIconEngine *fromFile(const QString &fileName);
signals:

public slots:

private:
  /**
   * @brief Tries to set a custom default fill color for the provided SVG.
   *        The color is applied to all those paths and shapes without a
   *        specified color, so the icon file must be compatible.
   * @param svg
   * @param color
   * @return
   */
  QString replaceSvgMainFillColor(const QString& svg, const QColor& color) const;

};

#endif // SVGICONENGINE_H
