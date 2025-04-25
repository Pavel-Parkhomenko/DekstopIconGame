#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QRegularExpression>
#include <QTimer>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  setMouseTracking(true);
  this->setFixedSize(1920, 1080);
//  this->setFixedSize(800, 800);
  listDesktopShortcuts();
//  this->showMaximized();
  setWindowFlags(windowFlags() |  Qt::FramelessWindowHint);


  QDomDocument doc = readXmlFromFile("/usr/share/desktop-base/active-theme/wallpaper/gnome-background.xml");
  ///home/user/.local/share/RecentDocuments
  if (!doc.isNull()) {
    QString xmlStr = doc.toString();
    QString path = getWallpaperPathBySize(xmlStr);
    wallPaper = path;
  }

//  readWallPepeeFile();

  //
}

void MainWindow::readWallPepeeFile() {
  QFile file("/home/user/.config/session/dolphin_dolphin_dolphin");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString fileContent = file.readAll();
    parseWallPeperFile(fileContent);
    file.close();
  }
  else {
    qWarning() << "Не удалось открыть файл.";
  }
}

void MainWindow::parseWallPeperFile(QString text) {
  QRegularExpression regex(R"(usr[\w\W]*svg)");
  QRegularExpressionMatch match = regex.match(text);

  if (match.hasMatch()) {
    QStringList parts = match.captured(0).split(":");
    QRegularExpression regex(R"(usr[\w\W]*svg)");

    if(parts.length()) {
      QRegularExpressionMatch match = regex.match(parts[1]);
      qDebug() << match.captured(0);

      if(match.hasMatch()) {
        wallPaper = "/" + match.captured(0);
//        qDebug() << match.captured(0);
      }
    }
  } else {
    qDebug() << "нет таких строк в файле";
  }
}

int overPos = 0;
void MainWindow::createElement(QString path, int pos, int type, QString iconName) {
  Element *el = new Element(path, type, iconName, this);
  if(50 + (pos * 120) + 200 > this->height()) {
    if(overPos == 0) overPos = pos;
    int x = 50 + 64 + 30;
    int y = 50 + ((pos - overPos) * 120);
    el->move(x, y);
    iconsPos.push_back(new QPoint(x, y));
  } else {
    el->move(50, 50 + (pos * 120));
    iconsPos.push_back(new QPoint(50, 50 + (pos * 120)));
  }

  icons.push_back(el);

  connect(el, &Element::moveElementSig,  this, &MainWindow::moveElementSlot);

  QTimer::singleShot(5000, this, [el, this]() { //-------------------------------------------------------
    el->startFallAnimation();
    canDrawRect = true;
    update();
  });
}

void MainWindow::moveElementSlot(int x, int y, int id) {
  if(x < iconsPos[id]->x() + 10 &&  x > iconsPos[id]->x() - 10 && y < iconsPos[id]->y() + 10 && y > iconsPos[id]->y() - 10) {
    icons[id]->fixedPosPlay = true;
    icons[id]->switchDragging();
    icons[id]->move(iconsPos[id]->x(), iconsPos[id]->y());
    this->idNoDrawRect.push_back(id);
    update();
  }

  for(int i = 0; i < iconsPos.count(); i++) {
    if(i == id) continue;

    if(std::find(idNoDrawRect.begin(), idNoDrawRect.end(), i) != idNoDrawRect.end()) {
      if(x < iconsPos[i]->x() + 10 &&  x > iconsPos[i]->x() - 10 && y < iconsPos[i]->y() + 10 && y > iconsPos[i]->y() - 10) {
        icons[i]->incorrectPosition();
        icons[i]->fixedPosPlay = false;

        idNoDrawRect.erase(std::remove(idNoDrawRect.begin(), idNoDrawRect.end(), i), idNoDrawRect.end());
        update();
      }
      continue;
    }

    if(x < iconsPos[i]->x() + 10 &&  x > iconsPos[i]->x() - 10 && y < iconsPos[i]->y() + 10 && y > iconsPos[i]->y() - 10) {
      icons[id]->incorrectPosition();
    }
  }
}

void MainWindow::drawPlayRect(QPainter &painter) {
  for (int i = 0; i < iconsPos.size(); i += 1) {

    if(std::find(idNoDrawRect.begin(), idNoDrawRect.end(), i) != idNoDrawRect.end()) {
      continue;
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::yellow);
    QPointF topLeft = *iconsPos[i];
    QPointF bottomRight = *iconsPos[i] + QPointF(68, 75);
    QRectF rect(topLeft, bottomRight);
    painter.drawRect(rect);
  }
}

void MainWindow::getUrlToIcon(QString nameIcon, int pos) {
  QString desktopFilePath = "/usr/share/applications/" + nameIcon;

  QSettings desktopFile(desktopFilePath, QSettings::IniFormat);
  desktopFile.beginGroup("Desktop Entry");
  QString iconPath = desktopFile.value("Icon").toString();
  QString iconName = desktopFile.value("Name").toString();
  desktopFile.endGroup();

  if(iconName.isEmpty()) iconName = iconPath;

  QString iconPathFull = "/usr/share/icons/hicolor/64x64/apps/" + iconPath + ".png";

  if (QFileInfo::exists(iconPathFull)) {
    return createElement(iconPathFull, pos, 0, iconName);
  }

  iconPathFull = "/usr/share/icons/breeze/apps/48/" + iconPath + ".svg";

  if(QFileInfo::exists(iconPathFull)) {
    return createElement(iconPathFull, pos, 1, iconName);
  }

  iconPathFull = "/usr/share/pixmaps/" + iconPath + ".svg";

  if(QFileInfo::exists(iconPathFull)) {
    return createElement(iconPathFull, pos, 1, iconName);
  }

}

void MainWindow::listDesktopShortcuts() {
  QString desktopPath = QDir::homePath() + "/Рабочий стол"; // Путь к рабочему столу

  QDir desktopDir(desktopPath);
  QStringList filters;
  filters << "*.desktop";

  int pos = 0;
  QFileInfoList shortcutFiles = desktopDir.entryInfoList(filters, QDir::Files);
  for (const QFileInfo &fileInfo : shortcutFiles) {
    getUrlToIcon(fileInfo.fileName(), pos++);
  }
}

QDomDocument MainWindow::readXmlFromFile(const QString &filePath) {
  QDomDocument xmlDoc;
  QFile file(filePath);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Не удалось открыть файл:" << filePath;
    return xmlDoc;
  }

  QString errorMsg;
  int errorLine = 0, errorColumn = 0;

  if (!xmlDoc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
    qWarning() << "Ошибка парсинга XML:" << errorMsg
               << "на строке" << errorLine << ", столбце" << errorColumn;

    return QDomDocument();
  }

  return xmlDoc;
}

QString MainWindow::getWallpaperPathBySize(const QString &xmlData, int width , int height) {
  QDomDocument doc;
  QString errorMsg;
  int errorLine, errorColumn;

  if (!doc.setContent(xmlData, &errorMsg, &errorLine, &errorColumn)) {
    qWarning() << "Ошибка парсинга XML:" << errorMsg
                  << "в строке" << errorLine << ", столбец" << errorColumn;
    return QString();
  }

  QDomNodeList sizeNodes = doc.elementsByTagName("size");
  for (int i = 0; i < sizeNodes.count(); ++i) {
    QDomElement sizeElem = sizeNodes.at(i).toElement();
    if (sizeElem.isNull()) continue;

    // Проверяем атрибуты width и height
    int currentWidth = sizeElem.attribute("width").toInt();
    int currentHeight = sizeElem.attribute("height").toInt();

    if (currentWidth == width && currentHeight == height) {
      return sizeElem.text().trimmed();
    }
  }

  qWarning() << "Размер" << width << "x" << height << "не найден в XML.";
  return QString();
}

MainWindow::~MainWindow()
{
  for(auto *i : qAsConst(icons)) {
    if(i) {
      delete i;
      i = nullptr;
    }
  }
  for(auto *ip : qAsConst(iconsPos)) {
    if(ip) {
      delete ip;
      ip = nullptr;
    }
  }
  delete ui;
}

