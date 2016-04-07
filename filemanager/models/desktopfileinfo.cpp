#include "desktopfileinfo.h"

#include "../app/global.h"

#include "../shutil/properties.h"
#include "../shutil/iconprovider.h"

#include <QSettings>

DesktopFileInfo::DesktopFileInfo() :
    FileInfo()
{

}

DesktopFileInfo::DesktopFileInfo(const QString &file) :
    FileInfo(file)
{
    init(file);
}

DesktopFileInfo::DesktopFileInfo(const QFileInfo &fileInfo) :
    FileInfo(fileInfo)
{
    init(fileInfo.absoluteFilePath());
}

DesktopFileInfo::~DesktopFileInfo()
{

}

void DesktopFileInfo::setFile(const QString &file)
{
    FileInfo::setFile(file);

    init(file);
}

QString DesktopFileInfo::getName() const
{
    return name;
}

QString DesktopFileInfo::getExec() const
{
    return exec;
}

QString DesktopFileInfo::getIconName() const
{
    return iconName;
}

QString DesktopFileInfo::getType() const
{
    return type;
}

QStringList DesktopFileInfo::getCategories() const
{
    return categories;
}

QIcon DesktopFileInfo::fileIcon() const
{
    return iconProvider->getDesktopIcon(getIconName(), 256);
}

void DesktopFileInfo::init(const QString &fileName)
{
    QSettings settings(fileName, QSettings::IniFormat);

    settings.beginGroup("Desktop Entry");
    // Loads .desktop file (read from 'Desktop Entry' group)
    Properties desktop(fileName, "Desktop Entry");

    name = desktop.value("Name", settings.value("Name")).toString();
    exec = desktop.value("Exec", settings.value("Exec")).toString();
    iconName = desktop.value("Icon", settings.value("Icon")).toString();
    type = desktop.value("Type", settings.value("Type", "Application")).toString();
    categories = desktop.value("Categories", settings.value("Categories").toString()).toString().remove(" ").split(";");
    mimeType = desktop.value("MimeType", settings.value("MimeType").toString()).toString().remove(" ").split(";");
    // Fix categories
    if (categories.first().compare("") == 0) {
      categories.removeFirst();
    }
}

