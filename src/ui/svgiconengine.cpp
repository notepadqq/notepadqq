#include "include/svgiconengine.h"
#include <QFile>
#include <QPainter>
#include <QGraphicsColorizeEffect>


SVGIconEngine::SVGIconEngine(const std::string &iconBuffer) {
    //std::string new_str = QString::fromStdString(iconBuffer).replace("fill=\"none\"", "fill=\"#ff5544\"").toStdString();
    data = QByteArray::fromStdString(iconBuffer);
}

SVGIconEngine* SVGIconEngine::fromFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return new SVGIconEngine("<?xml version=\"1.0\" encoding=\"UTF-8\" ?><svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\"></svg>");
    return new SVGIconEngine(file.readAll().toStdString());
}

void SVGIconEngine::paint(QPainter *painter, const QRect &rect,
                          QIcon::Mode mode, QIcon::State) {
  QSvgRenderer renderer(data);
  renderer.render(painter, rect);



  // FIXME Handle different modes and states
}

QIconEngine *SVGIconEngine::clone() const { return new SVGIconEngine(*this); }

QPixmap SVGIconEngine::pixmap(const QSize &size, QIcon::Mode mode,
                              QIcon::State state) {
  // This function is necessary to create an EMPTY pixmap. It's called always
  // before paint()

  QImage img(size, QImage::Format_ARGB32);
  img.fill(qRgba(0, 0, 0, 0));
  QPixmap pix = QPixmap::fromImage(img, Qt::NoFormatConversion);
  {
    QPainter painter(&pix);
    QRect r(QPoint(0.0, 0.0), size);
    this->paint(&painter, r, mode, state);
  }
  return pix;
}
