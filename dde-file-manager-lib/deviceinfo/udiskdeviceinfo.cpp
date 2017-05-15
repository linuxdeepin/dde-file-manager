#include "udiskdeviceinfo.h"
#include "app/define.h"
#include "shutil/fileutils.h"
#include <QIcon>
#include "utils.h"
#include "durl.h"
#include "udisklistener.h"
#include "singleton.h"

#include <unistd.h>

UDiskDeviceInfo::UDiskDeviceInfo()
    : DFileInfo("", false)
{

}

UDiskDeviceInfo::UDiskDeviceInfo(UDiskDeviceInfoPointer info)
    : UDiskDeviceInfo()
{
    setDiskInfo(info->getDiskInfo());
}

UDiskDeviceInfo::UDiskDeviceInfo(const DUrl &url)
    : DFileInfo(url, false)
{

}

UDiskDeviceInfo::UDiskDeviceInfo(const QString &url)
    : DFileInfo(url, false)
{

}

UDiskDeviceInfo::~UDiskDeviceInfo()
{

}

void UDiskDeviceInfo::setDiskInfo(QDiskInfo diskInfo)
{
    m_diskInfo = diskInfo;
    DUrl url = getMountPointUrl();

    url.setQuery(getMountPoint());
    setUrl(url);
}

QDiskInfo UDiskDeviceInfo::getDiskInfo() const
{
    return m_diskInfo;
}

QString UDiskDeviceInfo::getId() const
{
    return m_diskInfo.id();
}

QString UDiskDeviceInfo::getName() const
{
    QString letter = deviceListener->getVolumeLetters().value(getId().toLower());
    if (!letter.isEmpty())
        return QString("%1 (%2:)").arg(m_diskInfo.name(), letter);
    return m_diskInfo.name();
}

QString UDiskDeviceInfo::getType() const
{
    return m_diskInfo.type();
}

QString UDiskDeviceInfo::getPath() const
{
    return m_diskInfo.unix_device();
}

QString UDiskDeviceInfo::getMountPoint() const
{
    return m_diskInfo.mounted_root_uri();
}

DUrl UDiskDeviceInfo::getMountPointUrl()
{
    if (!getId().isEmpty())
        return getMountPointUrl(m_diskInfo);
    else
        return DUrl();
}

DUrl UDiskDeviceInfo::getMountPointUrl(QDiskInfo &info)
{
    QString path = QString("/run/user/%1/gvfs").arg(getuid());

    QString mounted_root_uri = info.mounted_root_uri();
    DUrl MountPointUrl = DUrl(mounted_root_uri);

    if (mounted_root_uri == "/"){
        MountPointUrl = DUrl::fromLocalFile("/");
    }else{
        if ((info.type() != "native" &&
             info.type() != "removable" &&
             info.type() != "dvd") && !info.id_filesystem().isEmpty()){
            if (info.type() == "network"){
                MountPointUrl = DUrl::fromLocalFile(QString("%1/%2%3").arg(path, info.id_filesystem(), DUrl(info.default_location()).path()));
            }else{
                MountPointUrl = DUrl::fromLocalFile(QString("%1/%2").arg(path, info.id_filesystem()));
            }
        }
    }

//    info.setMounted_url(MountPointUrl);

    return MountPointUrl;
}

QString UDiskDeviceInfo::getIcon() const
{
    return m_diskInfo.iconName();
}

bool UDiskDeviceInfo::canEject() const
{
    return m_diskInfo.can_eject();
}

bool UDiskDeviceInfo::canUnmount() const
{
    return m_diskInfo.can_unmount();
}

qulonglong UDiskDeviceInfo::getFree()
{
    //when device is mounted, use QStorageInfo to get datas
    if(canUnmount()){
        if (getMediaType() == dvd || getMediaType() == native || getMediaType() == removable){
            return QStorageInfo(getMountPointUrl().toLocalFile()).bytesFree();
        }
    }
    return m_diskInfo.free();
}

qulonglong UDiskDeviceInfo::getTotal()
{
    if(canUnmount()){
        if (getMediaType() == dvd || getMediaType() == native || getMediaType() == removable){
            return QStorageInfo(getMountPointUrl().toLocalFile()).bytesTotal();
        }
    }
    return m_diskInfo.total();
}

qint64 UDiskDeviceInfo::size() const
{
    return m_diskInfo.total();
}

QString UDiskDeviceInfo::fileDisplayName() const
{
    QString displayName = getName();
    if (!displayName.isEmpty()){
        return displayName;
    }
    return FileUtils::formatSize(size());
}

UDiskDeviceInfo::MediaType UDiskDeviceInfo::getMediaType() const
{
    if(getType() == "native")
        return native;
    else if(getType() == "removable")
        return removable;
    else if(getType() == "network")
        return network;
    else if(getType() == "smb")
        return network;
    else if(getType() == "phone")
        return phone;
    else if(getType() == "iphone")
        return iphone;
    else if(getType() == "camera")
        return camera;
    else if(getType() == "dvd")
        return dvd;
    else
        return unknown;
}

QString UDiskDeviceInfo::deviceTypeDisplayName() const
{
    if(getType() == "native")
        return QObject::tr("Local disk");
    else if(getType() == "removable")
        return QObject::tr("Removable disk");
    else if(getType() == "network")
        return QObject::tr("Network shared directory");
    else if(getType() == "phone")
        return QObject::tr("Android mobile device");
    else if(getType() == "iphone")
        return QObject::tr("Apple mobile device");
    else if(getType() == "camera")
        return QObject::tr("Camera");
    else if(getType() == "dvd")
        return QObject::tr("Dvd");
    else
        return QObject::tr("Unknown device");
}

QString UDiskDeviceInfo::sizeDisplayName() const
{
    if (filesCount() <= 1){
        return QObject::tr("%1 item").arg(filesCount());
    }else{
        return QObject::tr("%1 items").arg(filesCount());
    }
}

int UDiskDeviceInfo::filesCount() const
{
    return FileUtils::filesCount(const_cast<UDiskDeviceInfo*>(this)->getMountPointUrl().toLocalFile());
}

bool UDiskDeviceInfo::isReadable() const
{
    return true;
}

bool UDiskDeviceInfo::isWritable() const
{
    return true;
}

bool UDiskDeviceInfo::canRename() const
{
    return false;
}

QIcon UDiskDeviceInfo::fileIcon() const
{
    return fileIcon(128, 128);
}

QIcon UDiskDeviceInfo::fileIcon(int width, int height) const
{
    if(getType() == "native")
        return QIcon(svgToPixmap(":/devices/images/device/drive-harddisk-256px.svg", width, height));
    else if(getType() == "removable")
        return QIcon(svgToPixmap(":/devices/images/device/drive-removable-media-usb-256px.svg", width, height));
    else if(getType() == "network")
        return QIcon(svgToPixmap(":/devices/images/device/drive-network-256px.svg", width, height));
    else if(getType() == "phone")
        return QIcon(svgToPixmap(":/devices/images/device/android-device-256px.svg", width, height));
    else if(getType() == "iphone")
        return QIcon(svgToPixmap(":/devices/images/device/ios-device-256px.svg", width, height));
    else if(getType() == "camera")
        return QIcon(svgToPixmap(":/devices/images/device/camera-256px.svg", width, height));
    else if(getType() == "dvd")
        return QIcon(svgToPixmap(":/devices/images/device/media-dvd-256px.svg", width, height));
    else
        return QIcon(svgToPixmap(":/devices/images/device/drive-harddisk-256px.svg", width, height));
}

bool UDiskDeviceInfo::isDir() const
{
    return true;
}

DUrl UDiskDeviceInfo::parentUrl() const
{
    return DUrl::fromComputerFile("/");
}

QVector<MenuAction> UDiskDeviceInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;

    if (type == SpaceArea)
        return actionKeys;

    qDebug() << const_cast<UDiskDeviceInfo*>(this)->getMountPointUrl();

    actionKeys.reserve(6);

    actionKeys << MenuAction::OpenDisk
               << MenuAction::OpenDiskInNewWindow
               << MenuAction::OpenDiskInNewTab
               << MenuAction::Separator;

    if(canEject()){
        actionKeys << MenuAction::Eject;
    }

    if (canUnmount()){
        actionKeys << MenuAction::Unmount;
    }else{
        actionKeys << MenuAction::Mount;
    }

    if(getMediaType() == removable)
        actionKeys << MenuAction::FormatDevice;

    if (getId().startsWith("smb://")
            || getId().startsWith("ftp://")
            || getId().startsWith("sftp://"))
        actionKeys << MenuAction::ForgetPassword;
    actionKeys << MenuAction::Separator << MenuAction::Property;

    return actionKeys;
}

QSet<MenuAction> UDiskDeviceInfo::disableMenuActionList() const
{
    QSet<MenuAction> actionKeys = DAbstractFileInfo::disableMenuActionList();

    if(DFMGlobal::isRootUser()){
        actionKeys << MenuAction::Unmount;
    }

    if (!canUnmount()){
        actionKeys << MenuAction::Property;
    }


    return actionKeys;
}

bool UDiskDeviceInfo::exists() const
{
    if (fileUrl().isComputerFile())
        return true;

    return true;
}

