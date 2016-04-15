#ifndef UDISKLISTENER_H
#define UDISKLISTENER_H

#include "../../filemanager/controllers/abstractfilecontroller.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusObjectPath>
#include <QList>
#include <QMap>
#include <QDBusArgument>
#include <QXmlStreamReader>
#include <QDBusPendingReply>

class UDiskDeviceInfo;

class UDiskListener : public AbstractFileController
{
    Q_OBJECT
public:
    UDiskListener();
    UDiskDeviceInfo * getDevice(const QDBusObjectPath &path) const;
    void addDevice(UDiskDeviceInfo * device);
    void removeDevice(UDiskDeviceInfo * device);
    void update();
public slots:
    void deviceAdded(const QDBusObjectPath &path);
    void deviceRemoved(const QDBusObjectPath &path);
    void deviceChanged(const QDBusObjectPath &path);
private:
    QList<UDiskDeviceInfo *> m_list;
    QMap<QString, UDiskDeviceInfo *> m_map;

public:
    const QList<AbstractFileInfo *> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const;
    AbstractFileInfo *createFileInfo(const DUrl &fileUrl, bool &accepted) const;
};

#endif // UDISKLISTENER_H
