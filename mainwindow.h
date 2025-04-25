#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QFile>
#include <QDomDocument>
#include "element.h"
#include "qpainter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void listDesktopShortcuts();
  void getUrlToIcon(QString nameIcon, int pos);
  void createElement(QString path, int pos, int type, QString iconName);
  QDomDocument readXmlFromFile(const QString &filePath);
  QString getWallpaperPathBySize(const QString &xmlData, int width = 1920, int height = 1080);
  void readWallPepeeFile();
  void parseWallPeperFile(QString text);

  void drawPlayRect(QPainter &painter);

protected:
  void paintEvent(QPaintEvent *event) override {
    QMainWindow::paintEvent(event);
    QPixmap backgroundImage(wallPaper);

    QPainter painter(this);

    painter.drawPixmap(rect(), backgroundImage);

    if(canDrawRect)
      drawPlayRect(painter);
  }

  void keyPressEvent(QKeyEvent *event) override {
    if (event->key() == Qt::Key_Space) {
      idNoDrawRect.clear();
      for(auto icon: qAsConst(icons)) {
        icon->startFallAnimation();
        icon->fixedPosPlay = false;
      }
      canDrawRect = true;
      update();
    }

    QWidget::keyPressEvent(event);
  }

  void moveEvent(QMoveEvent *event) override {
  }

  void showEvent(QShowEvent *event) override {
  }

private slots:
  void moveElementSlot(int x, int y, int id);

private:
  Ui::MainWindow *ui;
  QVector<Element*> icons;
  QVector<QPoint*> iconsPos;
  QString wallPaper;
  QWidget *widget;
  QPointF lockedPosition;

  bool canDrawRect = false;
  QVector<int> idNoDrawRect;
};
#endif // MAINWINDOW_H
