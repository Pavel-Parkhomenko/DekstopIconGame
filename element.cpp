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

  setFixedSize(pixmap.size() + QSize(0, 11));
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
  fallAnimation = new QPropertyAnimation(this, "posY");
  fallAnimation->setDuration(getRand(1000, 2000));
  fallAnimation->setStartValue(this->y());
  fallAnimation->setEndValue(parentWidget()->height() - height() - 46);
//  animation->setEasingCurve(QEasingCurve::OutBounce);

  connect(fallAnimation, &QPropertyAnimation::finished, this, &Element::startCombinedAnimation);

  connect(fallAnimation, &QPropertyAnimation::finished, this, [this]() {
    fallAnimation->deleteLater();
    fallAnimation = nullptr;
  });

  fallAnimation->start();
}


void Element::startCombinedAnimation() {

  if (repeatCount <= 0) return;
  int directionX = QRandomGenerator::global()->bounded(2) == 0 ? -1 : 1;

  if(dragging) {
    directionX = mouseMoveDirX;
  }

  const int startX = this->x();
  const int startY = this->y();
  const int endX = startX + directionX * getRand(150, 300); // Отлет в сторону на 150 пикселей
  const int endY = parentWidget()->height() - height() - 46;
  const int duration = 1000;

  combinedAnimation = new QPropertyAnimation(this, "pos");
  combinedAnimation->setDuration(duration);

  for (int t = 0; t <= duration; t += 100) {
    double progress = static_cast<double>(t) / duration;

    // Линейное движение по X
    double x = startX + progress * (endX - startX);

    // Падение с использованием эффекта дуги (например, синус)
    double arcEffect = sin(progress * M_PI) * 200; // Высота дуги
    double y = startY + progress * (endY - startY) - arcEffect;

    // Проверка выхода за границы окна
    if (x < 0 || x > parentWidget()->width() - width()) {
      directionX *= -1; // Изменяем направление
      x = qBound(0, static_cast<int>(x), parentWidget()->width() - width()); // Ограничиваем X
    }
    // Устанавливаем ключевые точки
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

void Element::startArcAnimation() {
  const int centerX = parentWidget()->width() / 2;  // Центр дуги по X
  const int centerY = parentWidget()->height() / 2; // Центр дуги по Y
  const int radius = 100;  // Радиус дуги
  const int duration = 3000; // Длительность анимации

  QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
  animation->setDuration(duration);

  for (int t = 0; t <= duration; t += 100) {
    double angle = (360.0 * t) / duration; // Угол по времени
    double radians = qDegreesToRadians(angle); // Перевод градусов в радианы
    int x = centerX + radius * cos(radians); // Координата X
    int y = centerY - radius * sin(radians); // Координата Y (минус для направления вверх)

    animation->setKeyValueAt(static_cast<double>(t) / duration, QPoint(x, y));
  }

  animation->setEasingCurve(QEasingCurve::Linear);
  animation->start(QAbstractAnimation::DeleteWhenStopped);
}


void Element::startBounceAnimation() {
  qDebug() << __FUNCTION__;
  QPropertyAnimation *bounceAnimation = new QPropertyAnimation(this, "posX");
  bounceAnimation->setDuration(3000);
  bounceAnimation->setStartValue(this->x());
  bounceAnimation->setEndValue(this->x() + 500);
  bounceAnimation->setEasingCurve(QEasingCurve::OutQuad);
  bounceAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Element::stopFallAnimation() {
  if (fallAnimation) {
    fallAnimation->stop();
    fallAnimation->deleteLater();
    fallAnimation = nullptr;
  }
  if(combinedAnimation) {
    combinedAnimation->stop();
    combinedAnimation->deleteLater();
    combinedAnimation = nullptr;
  }
}
