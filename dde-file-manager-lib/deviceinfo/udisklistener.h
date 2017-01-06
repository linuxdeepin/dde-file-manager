#ifndef UDISKLISTENER_H
#define UDISKLISTENER_H

#include "dabstractfilecontroller.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusObjectPath>
#include <QList>
#include <QMap>
#include <QDBusArgument>
#include <QXmlStreamReader>
#include <QDBusPendingReply>
#include "dbusinterface/diskmount_interface.h"
#include "udiskdeviceinfo.h"


#define EventTypeVolumeAdded 1
#define EventTypeVolumeRemoved 2
#define EventTypeMountAdded 3
#define EventTypeMountRemoved 4

class UDiskDeviceInfo;
class Subscriber;
class DeviceInfoManagerInterface;


class UDiskListener : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit UDiskListener(QObject *parent = 0);
    UDiskDeviceInfoPointer getDevice(const QString &id);
    void addDevice(UDiskDeviceInfoPointer device);
    void removeDevice(UDiskDeviceInfoPointer device);
    void update();
    QString lastPart(const QString &path);
    bool isSystemDisk(const QString &path) const;
    UDiskDeviceInfoPointer hasDeviceInfo(const QString &id);

    void addSubscriber(Subscriber* sub);
    void removeSubscriber(Subscriber* sub);

    QMap<QString, UDiskDeviceInfoPointer> getAllDeviceInfos();
    QList<UDiskDeviceInfoPointer> getDeviceList();

    bool isDeviceFolder(const QString &path) const;
    bool isInDeviceFolder(const QString &path) const;
    UDiskDeviceInfoPointer getDeviceByDevicePath(const QString &deveicePath);
    UDiskDeviceInfoPointer getDeviceByPath(const QString &path);
    UDiskDeviceInfoPointer getDeviceByFilePath(const QString &path);
    UDiskDeviceInfo::MediaType getDeviceMediaType(const QString &path);

    QString getVolumeConfPath();
    bool isVolumeConfExists();
    void loadCustomVolumeLetters();
    QMap<QString, QString> getVolumeLetters();

signals:
    void volumeAdded(UDiskDeviceInfoPointer device);
    void volumeRemoved(UDiskDeviceInfoPointer device);
    void mountAdded(UDiskDeviceInfoPointer device);
    void mountRemoved(UDiskDeviceInfoPointer device);
    void deviecInfoChanged(UDiskDeviceInfoPointer device);
    void requestDiskInfosFinihsed();

public slots:
    void mount(const QString &path);
    DiskInfo queryDisk(const QString& path);
    void unmount(const QString &path);
    void eject(const QString &path);
    void asyncRequestDiskInfos();
    void asyncRequestDiskInfosFinihsed(QDBusPendingCallWatcher *call);
    void changed(int in0, const QString &in1);
    void forceUnmount(const QString &id);

    bool requestAsycGetUsage(const QString &devicePath);
    void asyncRequestGetUsageFinihsed(QDBusPendingCallWatcher *call);
    void refreshAsycGetAllDeviceUsage();


private:
    QList<UDiskDeviceInfoPointer> m_list;
    QMap<QString, UDiskDeviceInfoPointer> m_map;
    QMap<QString, QString> m_volumeLetters;
    QList<QString> fstab;

    QList<Subscriber*> m_subscribers;

    DiskMountInterface* m_diskMountInterface = NULL;
    DeviceInfoManagerInterface* m_deviceInfoManagerInterface = NULL;
public:
    const QList<DAbstractFileInfoPointer> getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                     QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                     bool &accepted) const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
};

#endif // UDISKLISTENER_H
