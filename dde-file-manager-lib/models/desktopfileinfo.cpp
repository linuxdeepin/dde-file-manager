#include "desktopfileinfo.h"
#include "private/dfileinfo_p.h"
#include "app/define.h"
#include "controllers/trashmanager.h"

#include "shutil/properties.h"
#include "interfaces/dfileservices.h"

#include "widgets/singleton.h"

#include <QSettings>
#include <QLocale>

class DesktopFileInfoPrivate : public DFileInfoPrivate
{
public:
    DesktopFileInfoPrivate(const DUrl &url, DesktopFileInfo *qq)
        : DFileInfoPrivate(url, qq)
    {
        updateInfo(url);
    }

    QString name;
    QString exec;
    QString iconName;
    QString type;
    QStringList categories;
    QStringList mimeType;

    void updateInfo(const DUrl &fileUrl);
};

DesktopFileInfo::DesktopFileInfo(const DUrl &fileUrl)
    : DFileInfo(*new DesktopFileInfoPrivate(fileUrl, this))
{

}

DesktopFileInfo::DesktopFileInfo(const QFileInfo &fileInfo)
    : DesktopFileInfo(DUrl::fromLocalFile(fileInfo.absoluteFilePath()))
{
    d_func()->fileInfo = fileInfo;
}

DesktopFileInfo::~DesktopFileInfo()
{

}

QString DesktopFileInfo::getName() const
{
    Q_D(const DesktopFileInfo);

    return d->name;
}

QString DesktopFileInfo::getExec() const
{
    Q_D(const DesktopFileInfo);

    return d->exec;
}

QString DesktopFileInfo::getIconName() const
{
    Q_D(const DesktopFileInfo);

     //special handling for trash desktop file which has tash datas
    if (d->iconName == "user-trash") {
        if (!TrashManager::isEmpty())
            return "user-trash-full";
    }

    return d->iconName;
}

QString DesktopFileInfo::getType() const
{
    Q_D(const DesktopFileInfo);

    return d->type;
}

QStringList DesktopFileInfo::getCategories() const
{
    Q_D(const DesktopFileInfo);

    return d->categories;
}

QIcon DesktopFileInfo::fileIcon() const
{
    Q_D(const DesktopFileInfo);

    if (Q_LIKELY(!d->icon.isNull()))
        return d->icon;

    const QString &iconName = this->iconName();

    if (iconName.startsWith("data:image/")) {
        int first_semicolon  = iconName.indexOf(';', 11);

        if (first_semicolon > 11) {
            // iconPath is a string representing an inline image.
//            const QString &format = iconName.mid(11, first_semicolon - 11);

            int base64str_pos = iconName.indexOf("base64,", first_semicolon);

            if (base64str_pos > 0) {
                QPixmap pixmap;

                bool ok = pixmap.loadFromData(QByteArray::fromBase64(iconName.mid(base64str_pos + 7).toLatin1())/*, format.toLatin1().constData()*/);

                if (ok) {
                    d->icon = QIcon(pixmap);
                } else {
                    d->icon = QIcon::fromTheme("application-default-icon");
                }
            }
        }
    }

    if (d->icon.isNull())
        return DFileInfo::fileIcon();

    return d->icon;
}

QString DesktopFileInfo::fileName() const
{
    return getName();
}

void DesktopFileInfo::refresh()
{
    Q_D(DesktopFileInfo);

    DFileInfo::refresh();
    d->updateInfo(fileUrl());
}

QString DesktopFileInfo::iconName() const
{
    return getIconName();
}

QString DesktopFileInfo::fileDisplayName() const
{
    return getName();
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

void DesktopFileInfoPrivate::updateInfo(const DUrl &fileUrl)
{
    const QMap<QString, QVariant> &map = DesktopFileInfo::getDesktopFileInfo(fileUrl);

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
