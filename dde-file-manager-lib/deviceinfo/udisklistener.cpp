#include "udisklistener.h"
#include "fstab.h"
#include "dfileservices.h"
#include "dfmevent.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "controllers/subscriber.h"
#include "singleton.h"

#include "gvfs/gvfsmountmanager.h"

#include <QSettings>
#include <QProcess>

UDiskListener::UDiskListener(QObject *parent):
    DAbstractFileController(parent)
{
    initConnect();
    loadCustomVolumeLetters();
}

void UDiskListener::initConnect()
{
    connect(gvfsMountManager, &GvfsMountManager::loadDiskInfoFinished, this, &UDiskListener::update);
    connect(gvfsMountManager, &GvfsMountManager::mount_added, this, &UDiskListener::addMountDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::mount_removed, this, &UDiskListener::removeMountDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::volume_added, this, &UDiskListener::addVolumeDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::volume_removed, this, &UDiskListener::removeVolumeDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::volume_changed, this, &UDiskListener::changeVolumeDiskInfo);
}

UDiskDeviceInfoPointer UDiskListener::getDevice(const QString &id)
{
    if (m_map.contains(id))
        return m_map[id];
    else
        return UDiskDeviceInfoPointer();
}

void UDiskListener::addDevice(UDiskDeviceInfoPointer device)
{
    m_map.insert(device->getDiskInfo().id(), device);
    m_list.append(device);
}

void UDiskListener::removeDevice(UDiskDeviceInfoPointer device)
{
    m_list.removeOne(device);
    m_map.remove(device->getDiskInfo().id());
}

void UDiskListener::update()
{
    QStringList keys;

    if (DFMGlobal::isRootUser()){
        keys.append(GvfsMountManager::Lsblk_Keys);
    }else{
        keys.append(GvfsMountManager::Volumes_Drive_Keys);
        keys.append(GvfsMountManager::Volumes_No_Drive_Keys);
        keys.append(GvfsMountManager::NoVolumes_Mounts_Keys);
    }
    foreach (QString key, keys) {
        QDiskInfo diskInfo = GvfsMountManager::DiskInfos.value(key);
        if (diskInfo.isValid()){
            addMountDiskInfo(diskInfo);
        }
    }
}


QString UDiskListener::lastPart(const QString &path)
{
    return path.split('/').last();
}


UDiskDeviceInfoPointer UDiskListener::hasDeviceInfo(const QString &id)
{
    return m_map.value(id);
}

void UDiskListener::addSubscriber(Subscriber *sub)
{
    if (!m_subscribers.contains(sub)){
        m_subscribers.append(sub);
    }
}

void UDiskListener::removeSubscriber(Subscriber *sub)
{
    if (m_subscribers.contains(sub)){
        m_subscribers.removeOne(sub);
    }
}

QMap<QString, UDiskDeviceInfoPointer> UDiskListener::getAllDeviceInfos()
{
    return m_map;
}

QList<UDiskDeviceInfoPointer> UDiskListener::getDeviceList()
{
    return m_list;
}

bool UDiskListener::isDeviceFolder(const QString &path) const
{
    for (int i = 0; i < m_list.size(); i++)
    {
       UDiskDeviceInfoPointer info = m_list.at(i);
        if (info->getMountPointUrl().toLocalFile() == path){
            return true;
        }
    }
    return false;
}

bool UDiskListener::isInDeviceFolder(const QString &path) const
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPointUrl().isEmpty()){
            if (path.startsWith(info->getMountPointUrl().toLocalFile())){
                return true;
            }
        }
    }
    return false;
}

bool UDiskListener::isInRemovableDeviceFolder(const QString &path) const
{
    QList<UDiskDeviceInfo::MediaType> mediaTypes = {UDiskDeviceInfo::removable,
                                                    UDiskDeviceInfo::iphone,
                                                   UDiskDeviceInfo::phone,
                                                   UDiskDeviceInfo::camera};
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (mediaTypes.contains(info->getMediaType())){
            if (!info->getMountPointUrl().isEmpty()){
                if (path.startsWith(info->getMountPointUrl().toLocalFile())){
                    return true;
                }
            }
        }
    }
    return false;
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByDevicePath(const QString &deveicePath)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && info->getPath() == deveicePath){
           return info;
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByMountPoint(const QString &mountPoint)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPoint().isEmpty()){
            if (DUrl(info->getMountPoint()) == DUrl(mountPoint))
                return info;
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByMountPointFilePath(const QString &filePath)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPoint().isEmpty()){
            bool flag = DUrl(info->getMountPoint()) == DUrl(filePath);
            if (!flag && filePath.startsWith(QString("%1/").arg(info->getMountPointUrl().toLocalFile())))
                return info;
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByPath(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPointUrl().isEmpty()){
            bool flag = (DUrl::fromLocalFile(path) == info->getMountPointUrl());

            if (path.startsWith(info->getMountPointUrl().toLocalFile()) && flag){
                return info;
            }
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByFilePath(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPointUrl().isEmpty()){
            bool flag = (DUrl::fromLocalFile(path) == info->getMountPointUrl());

            if (!flag && path.startsWith(QString("%1/").arg(info->getMountPointUrl().toLocalFile()))){
                return info;
            }
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByDeviceID(const QString &deviceID)
{
    foreach (const UDiskDeviceInfoPointer& info, m_list) {
        if(info->getId() == deviceID)
            return info;
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfo::MediaType UDiskListener::getDeviceMediaType(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && info->getMountPointUrl().toLocalFile() == path){
            return info->getMediaType();
        }
    }
    return UDiskDeviceInfo::unknown;
}

QString UDiskListener::getVolumeConfPath()
{
    return "/etc/deepin/volume_letter.conf";
}

bool UDiskListener::isVolumeConfExists()
{
    if (QFile(getVolumeConfPath()).exists()){
        return true;
    }
    return false;
}

void UDiskListener::loadCustomVolumeLetters()
{
    if (isVolumeConfExists()){
        QSettings VolumeSettings(getVolumeConfPath(), QSettings::IniFormat);
        VolumeSettings.beginGroup("Volume");
        foreach (QString key, VolumeSettings.childKeys()) {
            m_volumeLetters.insert(key, VolumeSettings.value(key).toString());
        }
        VolumeSettings.endGroup();
    }
}

QMap<QString, QString> UDiskListener::getVolumeLetters()
{
    return m_volumeLetters;
}

void UDiskListener::addMountDiskInfo(const QDiskInfo &diskInfo)
{
    qDebug() << diskInfo;
    UDiskDeviceInfoPointer device;
    if(m_map.value(diskInfo.id())){
        device = m_map.value(diskInfo.id());
        device->setDiskInfo(diskInfo);
    }else{
        device = new UDiskDeviceInfo();
        device->setDiskInfo(diskInfo);
        addDevice(device);
    }

    emit mountAdded(device);

    qDebug() << m_subscribers;
    foreach (Subscriber* sub, m_subscribers) {
        QString url = device->getMountPointUrl().toString();
        qDebug() << url;
        sub->doSubscriberAction(url);
    }

}

void UDiskListener::removeMountDiskInfo(const QDiskInfo &diskInfo)
{
    UDiskDeviceInfoPointer device;
    qDebug() << diskInfo;
    qDebug() << m_map.contains(diskInfo.id());
    qDebug() << m_map;
    if (m_map.value(diskInfo.id())){
        device = m_map.value(diskInfo.id());
        qDebug() << device->getMediaType();
        if (diskInfo.has_volume()){
            device->setDiskInfo(diskInfo);
        }else{
            removeDevice(device);
        }
        emit mountRemoved(device);
    }
}

void UDiskListener::addVolumeDiskInfo(const QDiskInfo &diskInfo)
{
    UDiskDeviceInfoPointer device;
    if(m_map.value(diskInfo.id())){
        device = m_map.value(diskInfo.id());
        device->setDiskInfo(diskInfo);
    }else{
        device = new UDiskDeviceInfo();
        device->setDiskInfo(diskInfo);
        addDevice(device);
    }
    emit volumeAdded(device);
}

void UDiskListener::removeVolumeDiskInfo(const QDiskInfo &diskInfo)
{
    UDiskDeviceInfoPointer device;
    qDebug() << diskInfo << m_map.contains(diskInfo.id()) << m_map;
    if (m_map.value(diskInfo.id())){
        device = m_map.value(diskInfo.id());
        removeDevice(device);
        emit volumeRemoved(device);
    }
}

void UDiskListener::changeVolumeDiskInfo(const QDiskInfo &diskInfo)
{
    UDiskDeviceInfoPointer device;
    qDebug() << diskInfo;
    qDebug() << m_map.value(diskInfo.id());
    if(m_map.value(diskInfo.id())){
        device = m_map.value(diskInfo.id());
        device->setDiskInfo(diskInfo);
        emit volumeChanged(device);
    }
}

void UDiskListener::mount(const QString &path)
{
    qDebug() << path;
    GvfsMountManager::mount(path);
}

void UDiskListener::unmount(const QString &path)
{
    qDebug() << path;
    GvfsMountManager::unmount(path);
}

void UDiskListener::eject(const QString &path)
{
    GvfsMountManager::eject(path);
}

void UDiskListener::forceUnmount(const QString &id)
{
    qDebug() << id;
    if (m_map.contains(id)){
        UDiskDeviceInfoPointer device = m_map.value(id);
        QStringList args;
        args << "-f" ;
        if (device->canEject()){
            args << "-e" << device->getMountPointUrl().toLocalFile();
        }else{
            args << "-u" << device->getMountPointUrl().toLocalFile();
        }
        bool reslut = QProcess::startDetached("gvfs-mount", args);
        qDebug() << "gvfs-mount" << args << reslut;
    }
}

const QList<DAbstractFileInfoPointer> UDiskListener::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
   const QString &frav = event->url().fragment();

    if (!frav.isEmpty()) {
        const QList<DAbstractFileInfoPointer> &list = fileService->getChildren(event->sender(), DUrl::fromLocalFile(frav),
                                                                               event->nameFilters(), event->filters(), event->flags());

        return list;
    }

    QList<DAbstractFileInfoPointer> infolist;

    for (int i = 0; i < m_list.size(); i++)
    {
        DAbstractFileInfoPointer fileInfo(new UDiskDeviceInfo(m_list.at(i)));
        infolist.append(fileInfo);
    }

    return infolist;
}

const DAbstractFileInfoPointer UDiskListener::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    const QString &path = event->url().fragment();

    if(path.isEmpty())
        return DAbstractFileInfoPointer(new UDiskDeviceInfo(event->url()));


    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);

        if(info->getMountPointUrl().toLocalFile() == path)
        {
            DAbstractFileInfoPointer fileInfo(new UDiskDeviceInfo(info));
            return fileInfo;
        }
    }

    return DAbstractFileInfoPointer();
}
