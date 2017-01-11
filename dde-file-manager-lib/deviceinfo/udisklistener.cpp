#include "udisklistener.h"
#include "fstab.h"
#include "dfileservices.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "controllers/subscriber.h"
#include "dbusinterface/deviceinfomanager_interface.h"
#include "widgets/singleton.h"

UDiskListener::UDiskListener(QObject *parent):
    DAbstractFileController(parent)
{
//    readFstab();
    m_diskMountInterface = new DiskMountInterface(DiskMountInterface::staticServerPath(),
                                                  DiskMountInterface::staticInterfacePath(),
                                                  QDBusConnection::sessionBus(), this);
    m_deviceInfoManagerInterface = new DeviceInfoManagerInterface(
                                                  "com.deepin.filemanager.daemon",
                                                  "/com/deepin/filemanager/daemon/DeviceInfoManager",
                                                                  QDBusConnection::systemBus(), this);
    connect(m_diskMountInterface, &DiskMountInterface::Changed, this, &UDiskListener::changed);
//    connect(m_diskMountInterface, &DiskMountInterface::Error, this, &UDiskListener::handleError);
    connect(m_diskMountInterface, &DiskMountInterface::Error,
            fileSignalManager, &FileSignalManager::showDiskErrorDialog);
    loadCustomVolumeLetters();
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
    m_map.insert(device->getDiskInfo().ID, device);
    m_list.append(device);
}

void UDiskListener::removeDevice(UDiskDeviceInfoPointer device)
{
    m_list.removeOne(device);
    m_map.remove(device->getDiskInfo().ID);
}

void UDiskListener::update()
{
    asyncRequestDiskInfos();
}



QString UDiskListener::lastPart(const QString &path)
{
    return path.split('/').last();
}

bool UDiskListener::isSystemDisk(const QString &path) const
{
    if(fstab.contains(path))
        return true;
    else
        return false;
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

            if (path.startsWith(info->getMountPointUrl().toLocalFile()) && !flag){
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

void UDiskListener::mount(const QString &path)
{
    QDBusPendingReply<> reply = m_diskMountInterface->Mount(path);
    reply.waitForFinished();
    if (reply.isValid() && reply.isFinished()){
        qDebug() << "mount" << path << "successed";
    }
}

DiskInfo UDiskListener::queryDisk(const QString &path)
{
    DiskInfo info = m_diskMountInterface->QueryDisk(path);
    return info;
}

void UDiskListener::unmount(const QString &path)
{
    m_diskMountInterface->Unmount(path);
}

void UDiskListener::eject(const QString &path)
{
    m_diskMountInterface->Eject(path);
}

void UDiskListener::asyncRequestDiskInfos()
{
    QDBusPendingReply<DiskInfoList> reply = m_diskMountInterface->ListDisk();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                        this, SLOT(asyncRequestDiskInfosFinihsed(QDBusPendingCallWatcher*)));
}

void UDiskListener::asyncRequestDiskInfosFinihsed(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<DiskInfoList> reply = *call;
    if (!reply.isError()){
        DiskInfoList diskinfos = qdbus_cast<DiskInfoList>(reply.argumentAt(0));
        foreach(DiskInfo info, diskinfos)
        {
            if (info.Icon == "drive-optical" && info.Name.startsWith("CD")){
                info.Type = "dvd";
            }

            UDiskDeviceInfoPointer device;
            if(m_map.value(info.ID))
            {
                device = m_map.value(info.ID);
                device->setDiskInfo(info);
            }
            else
            {
                device = new UDiskDeviceInfo(info);
                addDevice(device);
            }
            mountAdded(device);

            if (device->getMediaType() == UDiskDeviceInfo::native ||
                    device->getMediaType() == UDiskDeviceInfo::removable){
                QString devicePath = info.Path;
                if(!devicePath.isEmpty())
                    deviceListener->requestAsycGetUsage(devicePath);
            }
        }
    }else{
        qCritical() << reply.error().message();
    }
    call->deleteLater();
    emit requestDiskInfosFinihsed();
}

void UDiskListener::changed(int in0, const QString &in1)
{
    qDebug() << in0 << in1;

    UDiskDeviceInfoPointer device = hasDeviceInfo(in1);
    DiskInfo info = m_diskMountInterface->QueryDisk(in1);

    if (info.Icon == "drive-optical" && info.Name.startsWith("CD")){
        info.Type = "dvd";
    }

    qDebug() << device << info;
    if(!device && !info.ID.isEmpty())
    {
        device = new UDiskDeviceInfo(info);
        addDevice(device);
        emit mountAdded(device);
    }else if (device && !info.ID.isEmpty())
    {
        DUrl oldMountUrl = device->getMountPointUrl();
        bool oldCanUnmount = device->getDiskInfo().CanUnmount;
        device->setDiskInfo(info);
        if (oldCanUnmount != info.CanUnmount){
            if (info.CanUnmount){
                emit mountAdded(device);
            }else{
                info.MountPointUrl = oldMountUrl;
                device->setDiskInfo(info);
                emit mountRemoved(device);
            }

            qDebug() << m_subscribers;
            foreach (Subscriber* sub, m_subscribers) {
                QString url = device->getMountPointUrl().toString();
                qDebug() << url;
                sub->doSubscriberAction(url);
            }
        }

    }else if (device && info.ID.isEmpty()){
        emit volumeRemoved(device);
        removeDevice(device);
    }
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

bool UDiskListener::requestAsycGetUsage(const QString &devicePath)
{
    QDBusPendingReply<bool, qlonglong, qlonglong> reply = m_deviceInfoManagerInterface->readUsage(devicePath);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    watcher->setProperty("devicePath", devicePath);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(asyncRequestGetUsageFinihsed(QDBusPendingCallWatcher*)));
}

void UDiskListener::asyncRequestGetUsageFinihsed(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<bool, qlonglong, qlonglong> reply = *call;
    if (!reply.isError()){
        QString devicePath = call->property("devicePath").toString();
        bool ret = qdbus_cast<bool>(reply.argumentAt(0));
        qlonglong freeSpace  = qdbus_cast<qlonglong>(reply.argumentAt(1));
        qlonglong total = qdbus_cast<qlonglong>(reply.argumentAt(2));
        if (ret){
            UDiskDeviceInfoPointer device = getDeviceByDevicePath(devicePath);
            if (device){
                DiskInfo info = device->getDiskInfo();
                info.Total = total;
                info.Free = freeSpace;
                info.Used = total - freeSpace;
                device->setDiskInfo(info);
                emit deviecInfoChanged(device);
            }
        }

    }else{
        qCritical() << reply.error().message();
    }
    call->deleteLater();
}

void UDiskListener::refreshAsycGetAllDeviceUsage()
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (!info->getPath().isEmpty()){
            requestAsycGetUsage(info->getPath());
        }
    }
}


const QList<DAbstractFileInfoPointer> UDiskListener::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                                QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                                bool &accepted) const
{
    Q_UNUSED(filters)

    accepted = true;

    const QString &frav = fileUrl.fragment();

    if(!frav.isEmpty())
    {
        DUrl localUrl = DUrl::fromLocalFile(frav);

        QList<DAbstractFileInfoPointer> list = fileService->getChildren(localUrl, nameFilters, filters, flags);

        return list;
    }

    QList<DAbstractFileInfoPointer> infolist;

    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfoPointer info = m_list.at(i);
        DAbstractFileInfoPointer fileInfo(new UDiskDeviceInfo(info));
        infolist.append(fileInfo);
    }

    return infolist;
}

const DAbstractFileInfoPointer UDiskListener::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    QString path = fileUrl.fragment();

    if(path.isEmpty())
        return DAbstractFileInfoPointer(new UDiskDeviceInfo(fileUrl));


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
