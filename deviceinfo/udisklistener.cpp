#include "udisklistener.h"

UDiskListener::UDiskListener()
{
    QDBusConnection system = QDBusConnection::systemBus();

    if (!system.isConnected())
        return;

    if (!QDBusInterface("org.freedesktop.UDisks",
                           "/org/freedesktop/UDisks",
                           "org.freedesktop.UDisks", system).isValid())
    {
        qDebug() << "org.freedesktop.UDisks - not exists - ";
        return;
    }

    system.connect("org.freedesktop.UDisks",
                       "/org/freedesktop/UDisks",
                       "org.freedesktop.UDisks",
                       "DeviceAdded",
                       this,
                       SLOT(dbusDeviceAdded(QDBusObjectPath)));

    system.connect("org.freedesktop.UDisks",
                   "/org/freedesktop/UDisks",
                   "org.freedesktop.UDisks",
                   "DeviceRemoved",
                   this,
                   SLOT(dbusDeviceRemoved(QDBusObjectPath)));

    system.connect("org.freedesktop.UDisks",
                   "/org/freedesktop/UDisks",
                   "org.freedesktop.UDisks",
                   "DeviceChanged",
                   this,
                   SLOT(dbusDeviceChanged(QDBusObjectPath)));
}

void UDiskListener::addDevice(UDiskDeviceInfo *device)
{

}

void UDiskListener::removeDevice(UDiskDeviceInfo *device)
{

}

void UDiskListener::deviceAdded(const QDBusObjectPath &path)
{

}

void UDiskListener::deviceRemoved(const QDBusObjectPath &path)
{

}

void UDiskListener::deviceChanged(const QDBusObjectPath &path)
{

}
