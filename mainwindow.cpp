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
//  this->setFixedSize(800, 600);
  listDesktopShortcuts();
//  this->showMaximized();


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

void MainWindow::createElement(QString path, int pos, int type, QString iconName) {
  Element *el = new Element(path, type, iconName, this);
  el->move(50, 50 + (pos * 120));
  icons.push_back(el);
  iconsPos.push_back(new QPoint(50, 50 + (pos * 120)));

  connect(el, &Element::moveElementSig,  this, &MainWindow::moveElementSlot);

  QTimer::singleShot(5000, this, [el, this]() { //-------------------------------------------------------
    el->startFallAnimation();
    canDrawRect = true;
    update();
  });

//  el->startFallAnimation();
  return;
}

void MainWindow::moveElementSlot(int x, int y, int id) {
  if(x < iconsPos[id]->x() + 10 &&  x > iconsPos[id]->x() - 10 && y < iconsPos[id]->y() + 10 && y > iconsPos[id]->y() - 10) {
    icons[id]->fixedPosPlay = true;
    icons[id]->move(iconsPos[id]->x(), iconsPos[id]->y());
    this->idNoDrawRect.push_back(id);
    update();
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

//  iconPathFull = "/usr/share/icons/hicolor/48x48/apps/" + iconPath + ".png";

//  if(QFileInfo::exists(iconPathFull)) {
//    return createElement(iconPathFull, pos, 0);
//  }

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
//    qDebug() << fileInfo.fileName();
    getUrlToIcon(fileInfo.fileName(), pos++);
  }
}

QDomDocument MainWindow::readXmlFromFile(const QString &filePath) {
  QDomDocument xmlDoc;
  QFile file(filePath);

  // Проверяем, можно ли открыть файл
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Не удалось открыть файл:" << filePath;
    return xmlDoc;  // Возвращаем пустой документ
  }

  // Читаем и парсим XML содержимое
  QString errorMsg;
  int errorLine = 0, errorColumn = 0;

  if (!xmlDoc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
    qWarning() << "Ошибка парсинга XML:" << errorMsg
               << "на строке" << errorLine << ", столбце" << errorColumn;
        return QDomDocument();  // Пустой документ при ошибке
  }

  return xmlDoc;  // Возвращаем загруженный XML документ
}

QString MainWindow::getWallpaperPathBySize(const QString &xmlData, int width , int height) {
  QDomDocument doc;
  QString errorMsg;
  int errorLine, errorColumn;

  // Парсим XML
  if (!doc.setContent(xmlData, &errorMsg, &errorLine, &errorColumn)) {
    qWarning() << "Ошибка парсинга XML:" << errorMsg
                  << "в строке" << errorLine << ", столбец" << errorColumn;
        return QString();
  }

  // Ищем все теги <size>
  QDomNodeList sizeNodes = doc.elementsByTagName("size");
  for (int i = 0; i < sizeNodes.count(); ++i) {
    QDomElement sizeElem = sizeNodes.at(i).toElement();
    if (sizeElem.isNull()) continue;

    // Проверяем атрибуты width и height
    int currentWidth = sizeElem.attribute("width").toInt();
    int currentHeight = sizeElem.attribute("height").toInt();

    if (currentWidth == width && currentHeight == height) {
      return sizeElem.text().trimmed(); // Возвращаем путь к изображению
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

