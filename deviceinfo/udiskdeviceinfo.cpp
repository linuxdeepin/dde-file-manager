#include "udiskdeviceinfo.h"
#include "../filemanager/app/global.h"
#include "../filemanager/shutil/fileutils.h"
#include <QIcon>
#include "../app/singleapplication.h"


UDiskDeviceInfo::UDiskDeviceInfo()
{

}

UDiskDeviceInfo::UDiskDeviceInfo(UDiskDeviceInfo *info)
    : AbstractFileInfo(DUrl::fromComputerFile("/"))
{
    m_diskInfo = info->getDiskInfo();
    data->url.setFragment(info->getMountPoint());
}

UDiskDeviceInfo::UDiskDeviceInfo(const DUrl &url)
    : AbstractFileInfo(url)
{

}

UDiskDeviceInfo::UDiskDeviceInfo(const QString &url)
    : AbstractFileInfo(url)
{

}

UDiskDeviceInfo::UDiskDeviceInfo(const DiskInfo &diskInfo)
{
    m_diskInfo = diskInfo;
    data->url.setFragment(getMountPoint());
}

UDiskDeviceInfo::~UDiskDeviceInfo()
{

}

void UDiskDeviceInfo::setDiskInfo(const DiskInfo &diskInfo)
{
    m_diskInfo = diskInfo;
}

DiskInfo UDiskDeviceInfo::getDiskInfo()
{
    return m_diskInfo;
}

QString UDiskDeviceInfo::getId()
{
    return m_diskInfo.ID;
}

QString UDiskDeviceInfo::getName()
{
    return m_diskInfo.Name;
}

QString UDiskDeviceInfo::getType() const
{
    return m_diskInfo.Type;
}

QString UDiskDeviceInfo::getPath()
{
    return m_diskInfo.Path;
}

QString UDiskDeviceInfo::getMountPoint()
{
    qDebug() << m_diskInfo;

    QString path = QString("/run/user/%1/gvfs").arg(SingleApplication::UserID);

    if (m_diskInfo.MountPoint.startsWith("afc://")){
        m_diskInfo.MountPoint = QString("%1/afc:host=%2").arg(path, m_diskInfo.ID);
    }else if (m_diskInfo.MountPoint.startsWith("mtp://")){
        QStringList ids = m_diskInfo.MountPoint.split("/");
        QString key = QUrl::toPercentEncoding(ids[2]);
        m_diskInfo.MountPoint = QString("%1/mtp:host=%2").arg(path, key);
    }else if (m_diskInfo.MountPoint.startsWith("smb://")){
        QStringList ids = m_diskInfo.MountPoint.split("/");
        m_diskInfo.MountPoint = QString("%1/smb-share:server=%2,share=%3").arg(path, ids[2], ids[3]);
    }
    return m_diskInfo.MountPoint.replace("file://", "");
}

QString UDiskDeviceInfo::getIcon()
{
    return m_diskInfo.Icon;
}

bool UDiskDeviceInfo::canEject()
{
    return m_diskInfo.CanEject;
}

bool UDiskDeviceInfo::canUnmount()
{
    return m_diskInfo.CanUnmount;
}

qulonglong UDiskDeviceInfo::getUsed()
{
    return m_diskInfo.Used;
}

qulonglong UDiskDeviceInfo::getTotal()
{
    return m_diskInfo.Total;
}

qint64 UDiskDeviceInfo::size()
{
    return m_diskInfo.Total;
}

QString UDiskDeviceInfo::displayName() const
{
    if (!m_diskInfo.Name.isEmpty()){
        return m_diskInfo.Name;
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
    else
        return unknown;
}

bool UDiskDeviceInfo::isCanRename() const
{
    return false;
}

QIcon UDiskDeviceInfo::fileIcon() const
{
    return QIcon(":/icons/images/icons/disk_normal_22px.svg");
}

bool UDiskDeviceInfo::isDir() const
{
    return true;
}

DUrl UDiskDeviceInfo::parentUrl() const
{
    return DUrl::fromComputerFile("/");
}

QVector<MenuAction> UDiskDeviceInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    Q_UNUSED(type);

    QVector<MenuAction> actionKeys;

    actionKeys.reserve(6);

    actionKeys << MenuAction::OpenDisk << MenuAction::OpenDiskInNewWindow
               << MenuAction::Separator;

    if(m_diskInfo.CanEject){
        actionKeys << MenuAction::Eject;
    }

    if (m_diskInfo.CanUnmount){
        actionKeys << MenuAction::Unmount;
    }else{
        actionKeys << MenuAction::Mount;
    }

    actionKeys << MenuAction::Separator << MenuAction::Property;

    return actionKeys;
}

