#include "udiskdeviceinfo.h"
#include "app/define.h"
#include "shutil/fileutils.h"
#include <QIcon>
#include "app/singleapplication.h"
#include "utils.h"
#include "durl.h"
#include "udisklistener.h"
#include "../widgets/singleton.h"

UDiskDeviceInfo::UDiskDeviceInfo()
    : DFileInfo("")
{

}

UDiskDeviceInfo::UDiskDeviceInfo(UDiskDeviceInfo *info)
    : UDiskDeviceInfo(info->getDiskInfo())
{

}

UDiskDeviceInfo::UDiskDeviceInfo(const DUrl &url)
    : DFileInfo(url)
{

}

UDiskDeviceInfo::UDiskDeviceInfo(const QString &url)
    : DFileInfo(url)
{

}

UDiskDeviceInfo::UDiskDeviceInfo(DiskInfo diskInfo)
    : DFileInfo(({DUrl url = getMountPointUrl(diskInfo); url.setQuery(diskInfo.ID); url;}))
{
    m_diskInfo = diskInfo;
}

UDiskDeviceInfo::~UDiskDeviceInfo()
{

}

void UDiskDeviceInfo::setDiskInfo(const DiskInfo &diskInfo)
{
    m_diskInfo = diskInfo;
    DUrl url = getMountPointUrl();

    url.setQuery(getId());
    setUrl(url);
}

DiskInfo UDiskDeviceInfo::getDiskInfo() const
{
    return m_diskInfo;
}

QString UDiskDeviceInfo::getId() const
{
    return m_diskInfo.ID;
}

QString UDiskDeviceInfo::getName() const
{
    QString letter = deviceListener->getVolumeLetters().value(getId().toLower());
    if (!letter.isEmpty())
        return QString("%1 (%2:)").arg(m_diskInfo.Name, letter);
    return m_diskInfo.Name;
}

QString UDiskDeviceInfo::getType() const
{
    return m_diskInfo.Type;
}

QString UDiskDeviceInfo::getPath() const
{
    return m_diskInfo.Path;
}

QString UDiskDeviceInfo::getMountPoint() const
{
    return m_diskInfo.MountPoint;
}

DUrl UDiskDeviceInfo::getMountPointUrl()
{
    return getMountPointUrl(m_diskInfo);
}

DUrl UDiskDeviceInfo::getMountPointUrl(DiskInfo &info)
{
    QString path = QString("/run/user/%1/gvfs").arg(SingleApplication::userId());

    if (info.MountPoint.startsWith(AFC_SCHEME)){
        info.MountPointUrl = DUrl::fromLocalFile(QString("%1/afc:host=%2").arg(path, info.ID));
    }else if (info.MountPoint.startsWith(MTP_SCHEME)){
        QStringList ids = info.MountPoint.split("/");
        QString key = QUrl::toPercentEncoding(ids[2]);
        info.MountPointUrl = DUrl::fromLocalFile(QString("%1/mtp:host=%2").arg(path, key));
    }else if (info.MountPoint.startsWith(SMB_SCHEME)){
        QString mountPath = QString("%1/smb-share:").arg(path);
        QStringList ids = info.MountPoint.split("/");
        QString share = ids[3];
        QString domain;
        QString user;
        QString ip;
        if (ids.at(2).contains(";")){
            domain = ids.at(2).split(";").at(0);
            QString userIps = ids.at(2).split(";").at(1);
            if (userIps.contains("@")){
                user = userIps.split("@").at(0);
                ip = userIps.split("@").at(1);
            }
        }else{
            QString userIps = ids.at(2);
            if (userIps.contains("@")){
                user = userIps.split("@").at(0);
                ip = userIps.split("@").at(1);
            }else{
                ip = userIps;
            }
        }

        if (!domain.isEmpty()){
            mountPath = QString("%1domain=%2,").arg(mountPath, domain);
        }
        if (!ip.isEmpty()){
            mountPath = QString("%1server=%2,").arg(mountPath, ip);
        }
        if (!share.isEmpty()){
            mountPath = QString("%1share=%2,").arg(mountPath, share);
        }
        if (!user.isEmpty()){
            mountPath = QString("%1user=%2,").arg(mountPath, user);
        }
//        qDebug() << domain << ip << share << user << mountPath << QUrl::fromPercentEncoding(mountPath.toLatin1());
        mountPath = QUrl::fromPercentEncoding(mountPath.toLatin1());
        info.MountPointUrl = DUrl::fromLocalFile(mountPath.left(mountPath.length()-1));
    }else if(info.MountPoint.startsWith(GPHOTO2_SCHEME)){
        QStringList ids = info.MountPoint.split("/");
        QString key = QUrl::toPercentEncoding(ids[2]);
        info.MountPointUrl = DUrl::fromLocalFile(QString("%1/gphoto2:host=%2").arg(path, key));
    }

    return info.MountPointUrl;
}

QString UDiskDeviceInfo::getIcon() const
{
    return m_diskInfo.Icon;
}

bool UDiskDeviceInfo::canEject() const
{
    return m_diskInfo.CanEject;
}

bool UDiskDeviceInfo::canUnmount() const
{
    return m_diskInfo.CanUnmount;
}

qulonglong UDiskDeviceInfo::getFree()
{
    if (getType() == "dvd"){
        return QStorageInfo(getMountPointUrl().toLocalFile()).bytesFree();
    }
    if (m_diskInfo.Total == 0){
        return QStorageInfo(getMountPointUrl().toLocalFile()).bytesFree();
    }
    return (m_diskInfo.Total - m_diskInfo.Used) * 1024;
}

qulonglong UDiskDeviceInfo::getUsed() const
{
    return m_diskInfo.Used * 1024;
}

qulonglong UDiskDeviceInfo::getTotal()
{
    if (getType() == "dvd"){
        return QStorageInfo(getMountPointUrl().toLocalFile()).bytesTotal();
    }
    if (m_diskInfo.Total == 0){
        return QStorageInfo(getMountPointUrl().toLocalFile()).bytesTotal();
    }
    return m_diskInfo.Total * 1024;
}

qint64 UDiskDeviceInfo::size() const
{
    return m_diskInfo.Total * 1024;
}

QString UDiskDeviceInfo::fileDisplayName() const
{
    QString displayName = getName();
    if (!displayName.isEmpty()){
        return displayName;
    }
    return FileUtils::formatSize(m_diskInfo.Total * 1024);
}

UDiskDeviceInfo::MediaType UDiskDeviceInfo::getMediaType() const
{
    if(getType() == "native")
        return native;
    else if(getType() == "removable")
        return removable;
    else if(getType() == "network")
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

bool UDiskDeviceInfo::isCanRename() const
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
        return QIcon(svgToPixmap(":/devices/images/device/drive-harddisk.svg", width, height));
    else if(getType() == "removable")
        return QIcon(svgToPixmap(":/devices/images/device/drive-removable-media-usb.svg", width, height));
    else if(getType() == "network")
        return QIcon(svgToPixmap(":/devices/images/device/drive-network.svg", width, height));
    else if(getType() == "phone")
        return QIcon(svgToPixmap(":/devices/images/device/android-device.svg", width, height));
    else if(getType() == "iphone")
        return QIcon(svgToPixmap(":/devices/images/device/ios-device.svg", width, height));
    else if(getType() == "camera")
        return QIcon(svgToPixmap(":/devices/images/device/camera.svg", width, height));
    else if(getType() == "dvd")
        return QIcon(svgToPixmap(":/devices/images/device/media-dvd.svg", width, height));
    else
        return QIcon(svgToPixmap(":/devices/images/device/drive-harddisk.svg", width, height));
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
               << MenuAction::OpenInNewTab
               << MenuAction::Separator;

    if(m_diskInfo.CanEject){
        actionKeys << MenuAction::Eject;
    }

    if (m_diskInfo.CanUnmount){
        actionKeys << MenuAction::Unmount;
    }else{
        actionKeys << MenuAction::Mount;
    }

    if (m_diskInfo.ID.startsWith("smb://"))
        actionKeys << MenuAction::ForgetPassword;
    actionKeys << MenuAction::Separator << MenuAction::Property;

    return actionKeys;
}

QSet<MenuAction> UDiskDeviceInfo::disableMenuActionList() const
{
    QSet<MenuAction> actionKeys = DAbstractFileInfo::disableMenuActionList();
    if (!m_diskInfo.CanUnmount){
        actionKeys << MenuAction::Property;
    }
    return actionKeys;
}

bool UDiskDeviceInfo::exists() const
{
    if (fileUrl().isComputerFile())
        return true;

    return false;
}

