#include "udiskdeviceinfo.h"
#include "../filemanager/app/global.h"
#include "../filemanager/shutil/fileutils.h"
#include <QIcon>

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


QString UDiskDeviceInfo::getMountPoint() const
{
    DUrl url(m_diskInfo.MountPoint);
    return url.toLocalFile();
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

QVector<AbstractFileInfo::MenuAction> UDiskDeviceInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    Q_UNUSED(type);

    QVector<MenuAction> actionKeys;

    actionKeys.reserve(6);

    actionKeys << MenuAction::Open << MenuAction::OpenInNewWindow
               << MenuAction::Separator;

    switch(getMediaType())
    {
        case native:
        {
            if(m_diskInfo.CanEject || m_diskInfo.CanUnmount)
                actionKeys << MenuAction::Unmount;
            else
                actionKeys << MenuAction::Mount;
            break;
        }
        case removable:
        {
            if(m_diskInfo.CanEject || m_diskInfo.CanUnmount)
                actionKeys << MenuAction::Eject;
            else
                actionKeys << MenuAction::Mount;
            break;
        }
        case network:
        {
            if(m_diskInfo.CanEject || m_diskInfo.CanUnmount)
                actionKeys << MenuAction::Unmount;
            else
                actionKeys << MenuAction::Mount;
            break;
        }
        default:
            break;
    }

    actionKeys << MenuAction::Separator << MenuAction::Property;

    return actionKeys;
}

