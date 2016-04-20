#include "udisklistener.h"
#include "udiskdeviceinfo.h"
#include "../../filemanager/app/global.h"
#include "fstab.h"

UDiskListener::UDiskListener()
{
    QDBusConnection system = QDBusConnection::systemBus();

    if (!system.isConnected())
        return;


    if (!QDBusInterface("org.freedesktop.UDisks2",
                           "/org/freedesktop/UDisks2",
                           "org.freedesktop.UDisks2", system).isValid())
    {
        qDebug() << "org.freedesktop.UDisks2 - not exists - ";
        return;
    }

    system.connect("org.freedesktop.UDisks2",
                       "/org/freedesktop/UDisks2",
                       "org.freedesktop.DBus.ObjectManager",
                       "InterfacesAdded",
                       this,
                       SLOT(interfacesAdded(QDBusObjectPath,QMap<QString,QVariant>)));

    system.connect("org.freedesktop.UDisks2",
                   "/org/freedesktop/UDisks2",
                   "org.freedesktop.DBus.ObjectManager",
                   "InterfacesRemoved",
                   this,
                   SLOT(interfacesRemoved(QDBusObjectPath, QStringList)));

    fileService->setFileUrlHandler(COMPUTER_SCHEME, "", this);

    readFstab();
    qDebug() << fstab;
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
    m_map.insert(device->uDiskPath(), device);
    m_list.append(device);
    connect(device, &UDiskDeviceInfo::changed, this, &UDiskListener::interfacesChanged);
}

void UDiskListener::removeDevice(UDiskDeviceInfo *device)
{
    m_list.removeOne(device);
    m_map.remove(device->uDiskPath());   
    device->deleteLater();
}

void UDiskListener::update()
{
    qDebug() << "update called";

    QList<QDBusObjectPath> paths;
    QDBusMessage call = QDBusMessage::createMethodCall("org.freedesktop.UDisks2",
                                                       "/org/freedesktop/UDisks2/block_devices",
                                                       "org.freedesktop.DBus.Introspectable",
                                                       "Introspect");
    QDBusPendingReply<QString> reply = QDBusConnection::systemBus().call(call);

    if (!reply.isValid())
    {
        qDebug() << "error: " << reply.error().name();
        return;
    }

    QXmlStreamReader xml(reply.value());
    while (!xml.atEnd())
    {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name().toString() == "node" )
        {
            QString name = xml.attributes().value("name").toString();
            if(!name.isEmpty())
                paths << QDBusObjectPath("/org/freedesktop/UDisks2/block_devices/" + name);
        }
    }

    foreach (QDBusObjectPath i, paths)
    {
        qDebug() << i.path();
        if (m_map.contains(i.path()))
        {
            UDiskDeviceInfo *device = getDevice(i);
            if (device)
                removeDevice(device);
        }
        else
        {
            UDiskDeviceInfo *device = new UDiskDeviceInfo(i);
            addDevice(device);
        }
    }
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


void UDiskListener::interfacesAdded(const QDBusObjectPath &path, const QMap<QString, QVariant> &interfaces)
{
    Q_UNUSED(interfaces);
    if(path.path().startsWith("/org/freedesktop/UDisks2/block_devices"))
    {
        UDiskDeviceInfo *device = getDevice(path);
        if (device)
            return;
        device = new UDiskDeviceInfo(path);
        addDevice(device);
        if(!device->fileSystem().isEmpty())
        {
            emit childrenAdded(DUrl::fromComputerFile(device->mountPath()));
            qDebug() << device->uDiskPath() << device->mountPath();
        }
    }
}

void UDiskListener::interfacesRemoved(const QDBusObjectPath &path, const QStringList &interfaces)
{
    Q_UNUSED(interfaces);
    if(path.path().startsWith("/org/freedesktop/UDisks2/block_devices"))
    {
        UDiskDeviceInfo *device = getDevice(path);
        if (device)
        {
            if(!device->fileSystem().isEmpty())
            {
                emit childrenRemoved(DUrl::fromComputerFile(device->mountPath()));
                qDebug() << device->uDiskPath() << device->mountPath();
            }
            removeDevice(device);
        }
    }
}

void UDiskListener::interfacesChanged()
{
    UDiskDeviceInfo * device = qobject_cast<UDiskDeviceInfo *>(sender());
    emit childrenUpdated(DUrl::fromComputerFile(device->mountPath()));
}

void UDiskListener::mount(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);
        if(!info->fileSystem().isEmpty() && path == info->uDiskPath())
        {
            info->mount();
            break;
        }
    }
}

void UDiskListener::unmount(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);
        if(!info->fileSystem().isEmpty() && path == info->uDiskPath())
        {
            info->unmount();
            break;
        }
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

const QList<AbstractFileInfo *> UDiskListener::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(filter)

    accepted = true;

    const QString &frav = fileUrl.fragment();

    if(!frav.isEmpty())
    {
        DUrl localUrl = DUrl::fromLocalFile(frav);

        QList<AbstractFileInfo*> list = fileService->getChildren(localUrl, filter);

        return list;
    }

    QList<AbstractFileInfo*> infolist;

    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);
        if(!info->fileSystem().isEmpty())
        {
            AbstractFileInfo *fileInfo = new UDiskDeviceInfo(info);
            infolist.append(fileInfo);
        }
    }

    return infolist;
}

AbstractFileInfo *UDiskListener::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    QString path = fileUrl.fragment();

    if(path.isEmpty())
        return new UDiskDeviceInfo(fileUrl);


    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);

        if(!info->fileSystem().isEmpty() && info->mountPath() == path)
        {
            AbstractFileInfo *fileInfo = new UDiskDeviceInfo(info);
            return fileInfo;
        }
    }

    return Q_NULLPTR;
}
