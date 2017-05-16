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
    void initConnect();
    UDiskDeviceInfoPointer getDevice(const QString &id);
    void addDevice(UDiskDeviceInfoPointer device);
    void removeDevice(UDiskDeviceInfoPointer device);

    QString lastPart(const QString &path);
    UDiskDeviceInfoPointer hasDeviceInfo(const QString &id);

    void addSubscriber(Subscriber* sub);
    void removeSubscriber(Subscriber* sub);

    QMap<QString, UDiskDeviceInfoPointer> getAllDeviceInfos();
    QList<UDiskDeviceInfoPointer> getDeviceList();

    bool isDeviceFolder(const QString &path) const;
    bool isInDeviceFolder(const QString &path) const;
    bool isInRemovableDeviceFolder(const QString &path) const;
    UDiskDeviceInfoPointer getDeviceByDevicePath(const QString &deveicePath);
    UDiskDeviceInfoPointer getDeviceByMountPoint(const QString &mountPoint);
    UDiskDeviceInfoPointer getDeviceByMountPointFilePath(const QString &filePath);
    UDiskDeviceInfoPointer getDeviceByPath(const QString &path);
    UDiskDeviceInfoPointer getDeviceByFilePath(const QString &path);
    UDiskDeviceInfoPointer getDeviceByDeviceID(const QString &deviceID);
    UDiskDeviceInfo::MediaType getDeviceMediaType(const QString &path);

    QString getVolumeConfPath();
    bool isVolumeConfExists();
    void loadCustomVolumeLetters();
    QMap<QString, QString> getVolumeLetters();

signals:
    void volumeAdded(UDiskDeviceInfoPointer device);
    void volumeRemoved(UDiskDeviceInfoPointer device);
    void volumeChanged(UDiskDeviceInfoPointer device);
    void mountAdded(UDiskDeviceInfoPointer device);
    void mountRemoved(UDiskDeviceInfoPointer device);

public slots:
    void update();
    void addMountDiskInfo(const QDiskInfo& diskInfo);
    void removeMountDiskInfo(const QDiskInfo& diskInfo);
    void addVolumeDiskInfo(const QDiskInfo& diskInfo);
    void removeVolumeDiskInfo(const QDiskInfo& diskInfo);
    void changeVolumeDiskInfo(const QDiskInfo& diskInfo);
    void mount(const QString &path);
    void unmount(const QString &path);
    void eject(const QString &path);
    void forceUnmount(const QString &id);

private:
    QList<UDiskDeviceInfoPointer> m_list;
    QMap<QString, UDiskDeviceInfoPointer> m_map;
    QMap<QString, QString> m_volumeLetters;

    QList<Subscriber*> m_subscribers;

public:
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;
};

#endif // UDISKLISTENER_H
