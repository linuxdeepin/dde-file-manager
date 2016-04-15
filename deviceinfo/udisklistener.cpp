#include "udisklistener.h"
#include "udiskdeviceinfo.h"
#include "../../filemanager/app/global.h"

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
                       "org.freedesktop.UDisks2",
                       "InterfacesAdded",
                       this,
                       SLOT(deviceAdded(QDBusObjectPath)));

    system.connect("org.freedesktop.UDisks2",
                   "/org/freedesktop/UDisks2",
                   "org.freedesktop.UDisks2",
                   "InterfacesRemoved",
                   this,
                   SLOT(deviceRemoved(QDBusObjectPath)));

    fileService->setFileUrlHandler(COMPUTER_SCHEME, "", this);
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
        if (m_map.contains(i.path()))
            deviceChanged(i);
        else
            deviceAdded(i);
    }
}


void UDiskListener::deviceAdded(const QDBusObjectPath &path)
{
    UDiskDeviceInfo *device = new UDiskDeviceInfo(path);
    addDevice(device);
}

void UDiskListener::deviceRemoved(const QDBusObjectPath &path)
{
    UDiskDeviceInfo *device = getDevice(path);
    if (device)
        removeDevice(device);
}

void UDiskListener::deviceChanged(const QDBusObjectPath &path)
{
    UDiskDeviceInfo *device = getDevice(path);
    if (!device)
        return;

    device->update();
}

const QList<AbstractFileInfo *> UDiskListener::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(filter)

    QList<AbstractFileInfo*> infolist;

    for (int i = 0; i < m_list.size(); i++)
    {
        UDiskDeviceInfo * info = m_list.at(i);
        AbstractFileInfo *fileInfo = new UDiskDeviceInfo(info);
        infolist.append(fileInfo);
    }

    accepted = true;

    return infolist;
}

AbstractFileInfo *UDiskListener::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return new UDiskDeviceInfo(fileUrl);
}
