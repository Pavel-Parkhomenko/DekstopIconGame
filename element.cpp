#include "element.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QSvgRenderer>
#include <QLabel>

int id = 0;

Element::Element(const QString &imagePath, int type, QString iconName, QWidget *parent)
    : QWidget(parent)
{
  if(type == 0) pixmap.load(imagePath);
  else if(type == 1) {
    pixmap = renderSvgToPixmap(imagePath, QSize(64, 64));
  }

  this->iconName = iconName;

  setFixedSize(pixmap.size() + QSize(0, 15));
  setMouseTracking(true);

  connect(this, &Element::posYChanged, this, &Element::onPosYChanged);
  connect(this, &Element::posXChanged, this, &Element::onPosXChanged);

  this->idElement = id;
  id++;
}

void Element::onPosYChanged() {}
void Element::onPosXChanged() {}

int Element::getRand(int min, int max) {
  return QRandomGenerator::global()->bounded(min, max + 1);
}

QPixmap Element::renderSvgToPixmap(const QString &filePath, const QSize &size) {
  QSvgRenderer svgRenderer(filePath);
  QPixmap pixmap(size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  svgRenderer.render(&painter);
  return pixmap;
}

void Element::startFallAnimation() {
  if(this->wasStart) return;
  this->wasStart = true;
  QPropertyAnimation *fallAnimation = new QPropertyAnimation(this, "posY");
  fallAnimation->setDuration(getRand(1000, 2000));
  fallAnimation->setStartValue(this->y());
  fallAnimation->setEndValue(parentWidget()->height() - height() - 48);
//  animation->setEasingCurve(QEasingCurve::OutBounce);

  connect(fallAnimation, &QPropertyAnimation::finished, this, &Element::startCombinedAnimation);

  connect(fallAnimation, &QPropertyAnimation::finished, this, [fallAnimation]() {
    fallAnimation->deleteLater();
  });

  fallAnimation->start();
}

bool elseDirection = false;
int prevDirectionX = 0;
void Element::startCombinedAnimation() {

  if (repeatCount <= 0) return;
  int directionX = QRandomGenerator::global()->bounded(2) == 0 ? -1 : 1;

  if(elseDirection) directionX = -1 * prevDirectionX;
  prevDirectionX = directionX;

  if(dragging) {
    directionX = mouseMoveDirX;
  }

  const int startX = this->x();
  const int startY = this->y();
  const int endX = startX + directionX * getRand(150, 500);
  const int endY = parentWidget()->height() - height() - 48;
  const int duration = 1000;

  combinedAnimation = new QPropertyAnimation(this, "pos");
  combinedAnimation->setDuration(duration);

  for (int t = 0; t <= duration; t += 100) {
    double progress = static_cast<double>(t) / duration;

    // Линейное движение по X
    double x = startX + progress * (endX - startX);

    double arcEffect = sin(progress * M_PI) * 200; // Высота дуги
    double y = startY + progress * (endY - startY) - arcEffect;

    // Проверка выхода за границы окна
    if (x < 0 || x > parentWidget()->width() - width()) {
      directionX *= -1; // Изменяем направление
      x = qBound(0, static_cast<int>(x), parentWidget()->width() - width()); // Ограничиваем X
      elseDirection = true;
    } else elseDirection = false;

    combinedAnimation->setKeyValueAt(progress, QPoint(static_cast<int>(x), static_cast<int>(y)));
  }

  combinedAnimation->setEasingCurve(QEasingCurve::Linear); // Плавное движение

  connect(combinedAnimation, &QPropertyAnimation::finished, this, [this]() {
    combinedAnimation->deleteLater();
    combinedAnimation = nullptr;
    startCombinedAnimation();
    repeatCount--;
  });

  combinedAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Element::stopFallAnimation() {
  if(combinedAnimation) {
    combinedAnimation->stop();
    combinedAnimation->deleteLater();
    combinedAnimation = nullptr;
  }
}
