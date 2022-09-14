// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include <QTimer>
#include <QXmlStreamReader>
#include <QDBusPendingReply>
#include "udiskdeviceinfo.h"


#define EventTypeVolumeAdded 1
#define EventTypeVolumeRemoved 2
#define EventTypeMountAdded 3
#define EventTypeMountRemoved 4

class UDiskDeviceInfo;
class Subscriber;

class DDiskManager;
class DBlockDevice;

class UDiskListener : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit UDiskListener(QObject *parent = nullptr);
    ~UDiskListener();
    UDiskDeviceInfoPointer getDevice(const QString &id);
    void addDevice(UDiskDeviceInfoPointer device);
    void removeDevice(UDiskDeviceInfoPointer device);
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;

    QString lastPart(const QString &path);
    UDiskDeviceInfoPointer hasDeviceInfo(const QString &id);

    void addSubscriber(Subscriber *sub);
    void removeSubscriber(Subscriber *sub);

    QMap<QString, UDiskDeviceInfoPointer> getAllDeviceInfos();
    QList<UDiskDeviceInfoPointer> getDeviceList();
    QList<UDiskDeviceInfoPointer> getMountList();

    bool isDeviceFolder(const QString &path) const;
    bool isInDeviceFolder(const QString &path) const;
    bool isInRemovableDeviceFolder(const QString &path) const;

    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    UDiskDeviceInfoPointer getDeviceByDevicePath(const QString &deveicePath);
    UDiskDeviceInfoPointer getDeviceByMountPoint(const QString &mountPoint);
    UDiskDeviceInfoPointer getDeviceByMountPointFilePath(const QString &filePath);
    UDiskDeviceInfoPointer getDeviceByPath(const QString &path);
    UDiskDeviceInfoPointer getDeviceByFilePath(const QString &path, const bool bshareuse = false);
    UDiskDeviceInfoPointer getDeviceByDeviceID(const QString &deviceID);

    void loadCustomVolumeLetters();
    QMap<QString, QString> getVolumeLetters();

    QMap<QString, UDiskDeviceInfoPointer> getMountedRemovableDiskDeviceInfos();

    QMap<QString, UDiskDeviceInfoPointer> getCanSendDisksByUrl(QString filepath);

    bool isMountedRemovableDiskExits();

    bool isFileFromDisc(const QString &filePath); // 文件是否来自光盘
    void appendHiddenDirs(const QString &path);
    QStringList hiddenDirs();
    bool isBlockFile(const QString &filePath); // 文件是否来自块设备
    bool isFromNativeDisk(const QString &uuid); // 是否为本地分区
    bool isFromNativeBlockDev(const QString &mntPath); // 挂载点的设备是否是本地的块设备

    int getCountOfMountedSmb(const QString &ip);//获取ip下smb挂载数量
    void setBatchedRemovingSmbMount(bool value);//设置批量卸载smb的标志
    bool isBatchedRemovingSmbMount();
    void clearLoginData();

private:
    void initDiskManager();
    void initConnect();

signals:
    void volumeAdded(UDiskDeviceInfoPointer device);
    void volumeRemoved(UDiskDeviceInfoPointer device);
    void volumeChanged(UDiskDeviceInfoPointer device);
    void mountAdded(UDiskDeviceInfoPointer device);
    void mountRemoved(UDiskDeviceInfoPointer device);

public slots:
    void update();
    void addMountDiskInfo(const QDiskInfo &diskInfo);
    void removeMountDiskInfo(const QDiskInfo &diskInfo);
    void addVolumeDiskInfo(const QDiskInfo &diskInfo);
    void removeVolumeDiskInfo(const QDiskInfo &diskInfo);
    void changeVolumeDiskInfo(const QDiskInfo &diskInfo);
    void mount(const QString &path);
    bool mountByUDisks(const QString &path);
    void unmount(const QString &path);
    void eject(const QString &path);
    void stopDrive(const QString &path);
    void forceUnmount(const QString &id);

private slots:
    void fileSystemDeviceIdLabelChanged(const QString &path);
    void insertFileSystemDevice(const QString dbusPath);
    void loopCheckCD();

private:
    DDiskManager *m_diskMgr = nullptr;
    QMap<QString, DBlockDevice *> m_fsDevMap;

    QList<UDiskDeviceInfoPointer> m_list;
    QList<UDiskDeviceInfoPointer> m_mountList;
    QMap<QString, UDiskDeviceInfoPointer> m_map;
    QMap<QString, QString> m_volumeLetters;

    QList<Subscriber *> m_subscribers;

    QTimer *m_diskTimer;
    int m_nCDRomCount = 0; // 光驱接入个数

    QStringList m_hiddenDirs; // feature: hide specified dirs of unremovable devices
    QStringList m_uuids; // from /etc/fstab
    bool m_isBatchedRemovingSmbMount = false;//是否批量卸载SMB
};

#endif // UDISKLISTENER_H
