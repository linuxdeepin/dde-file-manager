#ifndef UDISKLISTENER_H
#define UDISKLISTENER_H

#include "../../filemanager/controllers/abstractfilecontroller.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusObjectPath>
#include <QList>
#include <QMap>

class UDiskDeviceInfo;

class UDiskListener : public AbstractFileController
{
    Q_OBJECT
public:
    UDiskListener();
    void addDevice(UDiskDeviceInfo * device);
    void removeDevice(UDiskDeviceInfo * device);
public slots:
    void deviceAdded(const QDBusObjectPath &path);
    void deviceRemoved(const QDBusObjectPath &path);
    void deviceChanged(const QDBusObjectPath &path);
private:
    QList<UDiskDeviceInfo *> m_list;
    QMap<QString, UDiskDeviceInfo *> m_map;
};

#endif // UDISKLISTENER_H
