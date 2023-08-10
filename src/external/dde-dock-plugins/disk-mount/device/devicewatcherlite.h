// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEWATCHERLITE_H
#define DEVICEWATCHERLITE_H

#include <QObject>
#include <QMap>

namespace dfmmount {
class DBlockDevice;
class DProtocolDevice;
}

class DeviceWatcherLite : public QObject
{
    Q_OBJECT

public:
    enum OperateType { kUnmount,
                       kPowerOff,
                       kEject };

    static DeviceWatcherLite *instance();

    // TODO(xust) replace these functions with DeviceManager DBus interface.
    QStringList allMountedRemovableBlocks();
    QStringList allMountedProtocols();

    QSharedPointer<dfmmount::DBlockDevice> createBlockDevicePtr(const QString &id);
    QSharedPointer<dfmmount::DProtocolDevice> createProtocolDevicePtr(const QString &id);

    void detachBlockDevice(const QString &id);
    void detachProtocolDevice(const QString &id);
    void detachAllDevices();

Q_SIGNALS:
    void hintIgnoreChanged();
    void blockDriveAdded();
    void blockDriveRemoved();
    void blockDeviceMounted(const QString &id, const QString &mpt);
    void blockDeviceUnmounted(const QString &id);
    void blockFileSystemAdded(const QString &id);
    void blockFileSystemRemoved(const QString &id);
    void protocolDeviceMounted(const QString &id, const QString &mpt);
    void protocolDeviceUnmounted(const QString &id);

    void operationFailed(OperateType type);

private:
    void unmountStacked(const QString &mpt);
    void removeDevice(bool unmountDone, QSharedPointer<dfmmount::DBlockDevice> blk);
    bool isSiblingOfRoot(QSharedPointer<dfmmount::DBlockDevice> blkDev);
    enum SearchBy {
        kSearchByMountPoint,
        kSearchByDevice,
    };
    static QString getMountInfo(const QString &in, SearchBy what);

private:
    explicit DeviceWatcherLite(QObject *parent = nullptr);

    QMap<QString, QStringList> blksOfDrv;
};

#endif   // DEVICEWATCHERLITE_H
