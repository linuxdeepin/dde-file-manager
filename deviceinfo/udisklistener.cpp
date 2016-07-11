#include "udisklistener.h"

#include "../../filemanager/app/global.h"
#include "../../filemanager/app/filesignalmanager.h"
#include "fstab.h"
#include "../controllers/subscriber.h"

UDiskListener::UDiskListener()
{
    fileService->setFileUrlHandler(COMPUTER_SCHEME, "", this);

//    readFstab();
    m_diskMountInterface = new DiskMountInterface(DiskMountInterface::staticServerPath(),
                                                  DiskMountInterface::staticInterfacePath(),
                                                  QDBusConnection::sessionBus(), this);
    connect(m_diskMountInterface, &DiskMountInterface::Changed, this, &UDiskListener::changed);
    connect(m_diskMountInterface, &DiskMountInterface::Error,
            fileSignalManager, &FileSignalManager::showDiskErrorDialog);
}

UDiskDeviceInfo *UDiskListener::getDevice(const QString &path)
{
    if (m_map.contains(path))
        return m_map[path];
    else
        return NULL;
}

void UDiskListener::addDevice(UDiskDeviceInfo *device)
{
    m_map.insert(device->getDiskInfo().ID, device);
    m_list.append(device);
}

void UDiskListener::removeDevice(UDiskDeviceInfo *device)
{
    m_list.removeOne(device);
    m_map.remove(device->getDiskInfo().ID);
    delete device;
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

UDiskDeviceInfo *UDiskListener::hasDeviceInfo(const QString &id)
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

QMap<QString, UDiskDeviceInfo *> UDiskListener::getAllDeviceInfos()
{
    return m_map;
}

QList<UDiskDeviceInfo *> UDiskListener::getDeviceList()
{
    return m_list;
}

bool UDiskListener::isDeviceFolder(const QString &path) const
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);
        if (info->getMountPoint() == path){
            return true;
        }
    }
    return false;
}

bool UDiskListener::isInDeviceFolder(const QString &path) const
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);
        if (!info->getMountPoint().isEmpty()){
            if (path.startsWith(info->getMountPoint())){
                return true;
            }
        }
    }
    return false;
}

UDiskDeviceInfo *UDiskListener::getDeviceByPath(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);
        if (!info->getMountPoint().isEmpty()){
            bool flag = DUrl(path) == DUrl(info->getMountPoint());
            if (path.startsWith(info->getMountPoint()) && flag){
                return info;
            }
        }
    }
    return NULL;
}

UDiskDeviceInfo::MediaType UDiskListener::getDeviceMediaType(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);
        if (info->getMountPoint() == path){
            return info->getMediaType();
        }
    }
    return UDiskDeviceInfo::unknown;
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
            qDebug() << info;
            UDiskDeviceInfo *device;
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
        }
    }else{
        qCritical() << reply.error().message();
    }
    call->deleteLater();
}

void UDiskListener::changed(int in0, const QString &in1)
{
    qDebug() << in0 << in1;

    UDiskDeviceInfo *device = hasDeviceInfo(in1);
    DiskInfo info = m_diskMountInterface->QueryDisk(in1);

    qDebug() << device << info;
    if(device == NULL && !info.ID.isEmpty())
    {
        device = new UDiskDeviceInfo(info);
        addDevice(device);
        emit mountAdded(device);
    }else if (device && !info.ID.isEmpty())
    {
        device->setDiskInfo(info);
        if (info.CanUnmount){
            emit mountAdded(device);
        }else{
            emit mountRemoved(device);
        }
        qDebug() << m_subscribers;
        foreach (Subscriber* sub, m_subscribers) {
            QString url = DUrl::fromLocalFile(device->getMountPoint()).toString();
            qDebug() << url;
            sub->doSubscriberAction(url);
        }

    }else if (device && info.ID.isEmpty()){
        emit volumeRemoved(device);
        removeDevice(device);
    }
}

void UDiskListener::readFstab()
{
    setfsent();
    struct fstab * fs = getfsent();
    while(fs != NULL)
    {
        fstab.append(fs->fs_file);
        fs = getfsent();
    }
    qDebug() << "read fstab";
    endfsent();
}

const QList<AbstractFileInfoPointer> UDiskListener::getChildren(const DUrl &fileUrl, QDir::Filters filter, const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(filter)

    accepted = true;

    const QString &frav = fileUrl.fragment();

    if(!frav.isEmpty())
    {
        DUrl localUrl = DUrl::fromLocalFile(frav);

        QList<AbstractFileInfoPointer> list = fileService->getChildren(localUrl, filter, event);

        return list;
    }

    QList<AbstractFileInfoPointer> infolist;

    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);
        AbstractFileInfoPointer fileInfo(new UDiskDeviceInfo(info));
        infolist.append(fileInfo);
    }

    return infolist;
}

const AbstractFileInfoPointer UDiskListener::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    QString path = fileUrl.fragment();

    if(path.isEmpty())
        return AbstractFileInfoPointer(new UDiskDeviceInfo(fileUrl));


    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);

        if(info->getMountPoint() == path)
        {
            AbstractFileInfoPointer fileInfo(new UDiskDeviceInfo(info));
            return fileInfo;
        }
    }

    return AbstractFileInfoPointer();
}
