#ifndef ELEMENT_H
#define ELEMENT_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>
#include <QFont>

class Element : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(int posY READ posY WRITE setPosY  NOTIFY posYChanged)
  Q_PROPERTY(int posX READ posX WRITE setPosX  NOTIFY posXChanged)

public:
  Element(const QString &imagePath, int type = 0, QString iconName = "", QWidget *parent = nullptr); // type: 0 - png, 1 - svg

  int getRand(int min, int max);
  bool fixedPosPlay = false;
  bool wasStart = false;
  int idElement;

  // это нужно для MainWindow
  void switchDragging() {
    dragging == true ? (dragging = false) : (dragging = true);
  }

  void startFallAnimation();
  void startCombinedAnimation();

  void stopFallAnimation();

  int posY() const {
    return m_posY;
  }

  void setPosY(int yy) {
    m_posY = yy;
    move(this->x(), m_posY);
    emit posYChanged();
  }

  int posX() const {
    return m_posX;
  }

  void setPosX(int xx) {
    m_posX = xx;
    move(this->x(), m_posX); //?? x()
    emit posYChanged();
  }

  void incorrectPosition() {
    incorrectPositionFlag = true;
    startCombinedAnimation();
    dragging = false;
  }

protected:
  void paintEvent(QPaintEvent *) override {
    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap);

    QFont font = painter.font();
    font.setPointSize(10);
    font.setItalic(true);
    painter.setFont(font);
    painter.setPen(Qt::white);

    QRect textRect = painter.fontMetrics().boundingRect(this->iconName);

    if(textRect.width() > this->width()) {
      QStringList lst = this->iconName.split(" ");

      if(lst.size() < 2) {
        int textX = (pixmap.width() - textRect.width()) / 2;
        int textY = pixmap.height() + 10;

        painter.drawText(textX, textY, this->iconName);
        painter.end();
        return;
      }

      QRect lst0 = painter.fontMetrics().boundingRect(lst[0]);
      QRect lst1 = painter.fontMetrics().boundingRect(lst[1]);

      int textX0 = 0;
      int textX1 = 0;
      textX0 = (pixmap.width() - lst0.width()) / 2;
      textX1 = (pixmap.width() - lst1.width()) / 2;

      int textY = pixmap.height() + 15;
      this->setFixedHeight(textY + 24);
      painter.drawText(textX0, textY, lst[0]);
      painter.drawText(textX1, textY + 20, lst[1]);
      painter.end();
    }
    else {
      int textX = (pixmap.width() - textRect.width()) / 2;
      int textY = pixmap.height() + 10;

      painter.drawText(textX, textY, this->iconName);
      painter.end();
    }
  }

  void mousePressEvent(QMouseEvent *event) override {
    if(this->wasStart == false) return;

    incorrectPositionFlag = false;
    if (event->button() == Qt::LeftButton && fixedPosPlay == false) {
      dragging = true;
      lastMousePosition = event->pos();
      stopFallAnimation();
      repeatCount = 3;
    }
  }

  void mouseMoveEvent(QMouseEvent *event) override {
    if (dragging && fixedPosPlay == false) {
      int dx = event->pos().x() - lastMousePosition.x();
      int dy = event->pos().y() - lastMousePosition.y();
      move(x() + dx, y() + dy);

      if(dx > 0) mouseMoveDirX = 1;
      if(dx <= 0) mouseMoveDirX = -1;

      emit moveElementSig(this->x(), this->y(), this->idElement);
    }
  }

  void mouseReleaseEvent(QMouseEvent *event) override {
    if(incorrectPositionFlag) return;
    this->wasStart = true;

    if (event->button() == Qt::LeftButton && fixedPosPlay == false) {
      startCombinedAnimation();
      dragging = false;
    }
  }


signals:
  void posYChanged();
  void posXChanged();

  void moveElementSig(int x, int y, int id);

private slots:
  void onPosYChanged();
  void onPosXChanged();


private:
  QPixmap renderSvgToPixmap(const QString &filePath, const QSize &size);
  bool dragging = false; // Флаг для отслеживания состояния перетаскивания
  QPoint lastMousePosition; // Последняя позиция мыши
  QPixmap pixmap;
  int m_posY;
  int m_posX;
//  QPropertyAnimation *fallAnimation;
  QPropertyAnimation *combinedAnimation;

  int mouseMoveDirX = 0; //1 вправо. -1 влево
  int repeatCount = 5;

  bool incorrectPositionFlag = false;

  QString iconName;

};

#endif // ELEMENT_H
