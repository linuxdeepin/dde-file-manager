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
#include "../dbusinterface/diskmount_interface.h"
#include "udiskdeviceinfo.h"


#define EventTypeVolumeAdded 1
#define EventTypeVolumeRemoved 2
#define EventTypeMountAdded 3
#define EventTypeMountRemoved 4

class UDiskDeviceInfo;
class Subscriber;


class UDiskListener : public AbstractFileController
{
    Q_OBJECT
public:
    UDiskListener();
    UDiskDeviceInfo * getDevice(const QString &path);
    void addDevice(UDiskDeviceInfo * device);
    void removeDevice(UDiskDeviceInfo * device);
    void update();
    QString lastPart(const QString &path);
    bool isSystemDisk(const QString &path) const;
    UDiskDeviceInfo * hasDeviceInfo(const QString &id);

    void addSubscriber(Subscriber* sub);
    void removeSubscriber(Subscriber* sub);

    QMap<QString, UDiskDeviceInfo *> getAllDeviceInfos();
    QList<UDiskDeviceInfo *> getDeviceList();

    bool isDeviceFolder(const QString &path) const;
    bool isInDeviceFolder(const QString &path) const;
    UDiskDeviceInfo * getDeviceByPath(const QString &path);
    UDiskDeviceInfo::MediaType getDeviceMediaType(const QString &path);

signals:
    void volumeAdded(UDiskDeviceInfo * device);
    void volumeRemoved(UDiskDeviceInfo * device);
    void mountAdded(UDiskDeviceInfo * device);
    void mountRemoved(UDiskDeviceInfo * device);
public slots:
    void mount(const QString &path);
    DiskInfo queryDisk(const QString& path);
    void unmount(const QString &path);
    void eject(const QString &path);
    void asyncRequestDiskInfos();
    void asyncRequestDiskInfosFinihsed(QDBusPendingCallWatcher *call);
    void changed(int in0, const QString &in1);
private:
    void readFstab();
    QList<UDiskDeviceInfo *> m_list;
    QMap<QString, UDiskDeviceInfo *> m_map;
    QList<QString> fstab;

    QList<Subscriber*> m_subscribers;

    DiskMountInterface* m_diskMountInterface;
public:
    const QList<AbstractFileInfoPointer> getChildren(const DUrl &fileUrl, QDir::Filters filter, const FMEvent &event, bool &accepted) const;
    const AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const;
};

#endif // UDISKLISTENER_H
