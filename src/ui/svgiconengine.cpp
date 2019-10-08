#include "include/svgiconengine.h"
#include <QFile>
#include <QPainter>
#include <QGraphicsColorizeEffect>
#include <QApplication>
#include <QPalette>
#include <QRegularExpression>

SVGIconEngine::SVGIconEngine(const std::string &iconBuffer) {
    auto data = QByteArray::fromStdString(iconBuffer);

    m_darkIcon = replaceSvgMainFillColor(QString(data), QColor(65,65,65)).toUtf8();
    m_lightIcon = replaceSvgMainFillColor(QString(data), QColor(205,205,205)).toUtf8();
}

SVGIconEngine* SVGIconEngine::fromFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return new SVGIconEngine("<?xml version=\"1.0\" encoding=\"UTF-8\" ?><svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\"></svg>");
    }
    return new SVGIconEngine(file.readAll().toStdString());
}

void SVGIconEngine::paint(QPainter *painter, const QRect &rect,
                          QIcon::Mode mode, QIcon::State) {

    auto bgColor = QApplication::palette().color(QPalette::Window);
    bool darkUI = bgColor.lightnessF() < 0.5;

    QSvgRenderer renderer(darkUI ? m_lightIcon : m_darkIcon);
    renderer.render(painter, rect);
}

QIconEngine *SVGIconEngine::clone() const {
    return new SVGIconEngine(*this);
}

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

QString SVGIconEngine::replaceSvgMainFillColor(const QString& svg, const QColor& color) const
{
    static const QRegularExpression reFillTag(R"PRE(<svg(.|\n|\r)*?fill="([^"]*?)">)PRE");
    static const QRegularExpression reNoFillTag(R"PRE(<svg(.|\n|\r)*?>)PRE");

    // Try replacing the color within the already existing "fill" attribute, if it exists
    auto match = reFillTag.match(svg);
    if (match.hasMatch()) {
        auto a = match.capturedStart(1); // start of the attribute value
        auto b = match.capturedEnd(1); // end of the attribute value
        QString new_svg = svg.mid(0, a) + color.name(QColor::HexRgb) + svg.mid(b);
        return new_svg;
    }

    match = reNoFillTag.match(svg);
    if (match.hasMatch()) {
        auto b = match.capturedEnd(0) - 1; // end of the opening <svg> tag
        QString new_svg = svg.mid(0, b) + " fill=\"" + color.name(QColor::HexRgb) + "\"" + svg.mid(b);
        return new_svg;
    }

    return svg;
}
