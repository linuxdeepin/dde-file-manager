#include "desktopfileinfo.h"

#include "app/define.h"

#include "shutil/properties.h"
#include "shutil/iconprovider.h"

#include "widgets/singleton.h"

#include <QSettings>

DesktopFileInfo::DesktopFileInfo() :
    FileInfo()
{

}

DesktopFileInfo::DesktopFileInfo(const QString &fileUrl)
    : FileInfo(fileUrl)
{
    init(DUrl(fileUrl));
}

DesktopFileInfo::DesktopFileInfo(const DUrl &fileUrl)
    : FileInfo(fileUrl)
{
    init(fileUrl);
}

DesktopFileInfo::DesktopFileInfo(const QFileInfo &fileInfo) :
    FileInfo(fileInfo)
{
    init(DUrl::fromLocalFile(fileInfo.absoluteFilePath()));
}

DesktopFileInfo::~DesktopFileInfo()
{

}

void DesktopFileInfo::setUrl(const DUrl &fileUrl)
{
    DAbstractFileInfo::setUrl(fileUrl);

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
    return fileIconProvider->getDesktopIcon(getIconName(), 256);
}

QString DesktopFileInfo::displayName() const
{
    return name;
}

QMap<QString, QVariant> DesktopFileInfo::getDesktopFileInfo(const DUrl &fileUrl)
{
    QMap<QString, QVariant> map;
    QSettings settings(fileUrl.path(), QSettings::IniFormat);

    settings.beginGroup("Desktop Entry");
    // Loads .desktop file (read from 'Desktop Entry' group)
    Properties desktop(fileUrl.path(), "Desktop Entry");

    const QString &name = desktop.value("Name[" + QLocale::system().name() + "]", settings.value("Name[" + QLocale::system().name() + "]")).toString();

    if (name.isEmpty())
        map["Name"] = desktop.value("Name", settings.value("Name"));
    else
        map["Name"] = name;

    map["Exec"] = desktop.value("Exec", settings.value("Exec"));
    map["Icon"] = desktop.value("Icon", settings.value("Icon"));
    map["Type"] = desktop.value("Type", settings.value("Type", "Application"));
    map["Categories"] = desktop.value("Categories", settings.value("Categories")).toString().remove(" ").split(";");
    map["MimeType"] = desktop.value("MimeType", settings.value("MimeType")).toString().remove(" ").split(";");

    return map;
}

void DesktopFileInfo::init(const DUrl &fileUrl)
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

