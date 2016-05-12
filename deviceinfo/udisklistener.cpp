#include "udisklistener.h"

#include "../../filemanager/app/global.h"
#include "fstab.h"

UDiskListener::UDiskListener()
{
    fileService->setFileUrlHandler(COMPUTER_SCHEME, "", this);
    readFstab();
    m_diskMountInterface = new DiskMountInterface(DiskMountInterface::staticServerPath(),
                                                  DiskMountInterface::staticInterfacePath(),
                                                  QDBusConnection::sessionBus(), this);
    connect(m_diskMountInterface, &DiskMountInterface::Changed, this, &UDiskListener::changed);
}

UDiskDeviceInfo *UDiskListener::getDevice(const QDBusObjectPath &path) const
{
    if (m_map.contains(path.path()))
        return m_map[path.path()];
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

void UDiskListener::interfacesChanged()
{

}

void UDiskListener::mount(const QString &path)
{
    m_diskMountInterface->Mount(path);
}

void UDiskListener::unmount(const QString &path)
{
    m_diskMountInterface->Unmount(path);
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
            UDiskDeviceInfo *device = new UDiskDeviceInfo(info);
            addDevice(device);
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
    switch(in0)
    {
    case EventTypeVolumeAdded:
        //emit volumeAdded(in1);
        break;
    case EventTypeVolumeRemoved:
    {
        UDiskDeviceInfo * device = hasDeviceInfo(in1);
        if(device)
        {
            emit volumeRemoved(device);
            removeDevice(device);
        }
        break;
    }
    case EventTypeMountAdded:
    {
        if(hasDeviceInfo(in1) == NULL)
        {
            DiskInfo info = m_diskMountInterface->QueryDisk(in1);
            UDiskDeviceInfo *device = new UDiskDeviceInfo(info);
            addDevice(device);
            emit mountAdded(device);
        }
        break;
    }
    case EventTypeMountRemoved:
        //emit mountRemoved(in1);
        break;
    default:
        qDebug() << "Unknown event type.";
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

const QList<AbstractFileInfoPointer> UDiskListener::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(filter)

    accepted = true;

    const QString &frav = fileUrl.fragment();

    if(!frav.isEmpty())
    {
        DUrl localUrl = DUrl::fromLocalFile(frav);

        QList<AbstractFileInfoPointer> list = fileService->getChildren(localUrl, filter);

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
