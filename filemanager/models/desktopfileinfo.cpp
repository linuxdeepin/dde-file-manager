#include "desktopfileinfo.h"

#include "../app/global.h"

#include "../shutil/properties.h"
#include "../shutil/iconprovider.h"

#include <QSettings>

DesktopFileInfo::DesktopFileInfo() :
    FileInfo()
{

}

DesktopFileInfo::DesktopFileInfo(const QString &fileUrl) :
    FileInfo(fileUrl)
{
    init(fileUrl);
}

DesktopFileInfo::DesktopFileInfo(const QFileInfo &fileInfo) :
    FileInfo(fileInfo)
{
    init( QString(FILE_SCHEME) + "://" + fileInfo.absoluteFilePath());
}

DesktopFileInfo::~DesktopFileInfo()
{

}

void DesktopFileInfo::setFile(const QString &fileUrl)
{
    FileInfo::setFile(fileUrl);

    init(fileUrl);
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

QMap<QString, QVariant> DesktopFileInfo::getDesktopFileInfo(const QString &fileUrl)
{
    QUrl url(fileUrl);

    QMap<QString, QVariant> map;
    QSettings settings(url.path(), QSettings::IniFormat);

    settings.beginGroup("Desktop Entry");
    // Loads .desktop file (read from 'Desktop Entry' group)
    Properties desktop(url.path(), "Desktop Entry");

    map["Name"] = desktop.value("Name", settings.value("Name"));
    map["Exec"] = desktop.value("Exec", settings.value("Exec"));
    map["Icon"] = desktop.value("Icon", settings.value("Icon"));
    map["Type"] = desktop.value("Type", settings.value("Type", "Application"));
    map["Categories"] = desktop.value("Categories", settings.value("Categories")).toString().remove(" ").split(";");
    map["MimeType"] = desktop.value("MimeType", settings.value("MimeType")).toString().remove(" ").split(";");

    return map;
}

void DesktopFileInfo::init(const QString &fileUrl)
{
    const QMap<QString, QVariant> &map = getDesktopFileInfo(fileUrl);

    name = map.value("Name").toString();
    exec = map.value("Exec").toString();
    iconName = map.value("Icon").toString();
    type = map.value("Type").toString();
    categories = map.value("Categories").toStringList();
    mimeType = map.value("MimeType").toStringList();
    // Fix categories
    if (categories.first().compare("") == 0) {
      categories.removeFirst();
    }
}

