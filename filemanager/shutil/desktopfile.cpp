#include "desktopfile.h"
#include "properties.h"
#include <QFile>
#include <QSettings>
#include <QDebug>

/**
 * @brief Loads desktop file
 * @param fileName
 */
DesktopFile::DesktopFile(const QString &fileName) {

  // Store file name
  this->fileName = fileName;

  // File validity
  if (!QFile::exists(fileName)) {
    return;
  }

  QSettings settings(fileName, QSettings::IniFormat);
  settings.beginGroup("Desktop Entry");
  // Loads .desktop file (read from 'Desktop Entry' group)
  Properties desktop(fileName, "Desktop Entry");
  name = desktop.value("Name", settings.value("Name")).toString();
  exec = desktop.value("Exec", settings.value("Exec")).toString();
  icon = desktop.value("Icon", settings.value("Icon")).toString();
  type = desktop.value("Type", settings.value("Type", "Application")).toString();
  categories = desktop.value("Categories", settings.value("Categories").toString()).toString().remove(" ").split(";");
  mimeType = desktop.value("MimeType", settings.value("MimeType").toString()).toString().remove(" ").split(";");
  // Fix categories
  if (categories.first().compare("") == 0) {
    categories.removeFirst();
  }
}
//---------------------------------------------------------------------------

QString DesktopFile::getFileName() const {
  return fileName;
}
//---------------------------------------------------------------------------

QString DesktopFile::getPureFileName() const {
  return fileName.split("/").last().remove(".desktop");
}
//---------------------------------------------------------------------------

QString DesktopFile::getName() const {
  return name;
}
//---------------------------------------------------------------------------

QString DesktopFile::getExec() const {
  return exec;
}
//---------------------------------------------------------------------------

QString DesktopFile::getIcon() const {
  return icon;
}
//---------------------------------------------------------------------------

QString DesktopFile::getType() const {
  return type;
}
//---------------------------------------------------------------------------

QStringList DesktopFile::getCategories() const {
  return categories;
}
//---------------------------------------------------------------------------

QStringList DesktopFile::getMimeType() const {
  return mimeType;
}
//---------------------------------------------------------------------------
