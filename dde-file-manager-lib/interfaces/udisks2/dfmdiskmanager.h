/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DFMDISKMANAGER_H
#define DFMDISKMANAGER_H

#include <dfmglobal.h>

#include <QObject>
#include <QMap>
#include <QDBusError>

DFM_BEGIN_NAMESPACE

class DFMBlockDevice;
class DFMBlockPartition;
class DFMDiskDevice;
class DFMDiskManagerPrivate;
class DFMDiskManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMDiskManager)

    Q_PROPERTY(bool watchChanges READ watchChanges WRITE setWatchChanges)

public:
    explicit DFMDiskManager(QObject *parent = nullptr);
    ~DFMDiskManager();

    QStringList blockDevices() const;
    QStringList diskDevices() const;

    bool watchChanges() const;

    static QString objectPrintable(const QObject *object);
    static DFMBlockDevice *createBlockDevice(const QString &path, QObject *parent = nullptr);
    static DFMBlockPartition *createBlockPartition(const QString &path, QObject *parent = nullptr);
    static DFMDiskDevice *createDiskDevice(const QString &path, QObject *parent = nullptr);

    static QDBusError lastError();

public Q_SLOTS:
    void setWatchChanges(bool watchChanges);

Q_SIGNALS:
    void blockDeviceAdded(const QString &path);
    void blockDeviceRemoved(const QString &path);
    void diskDeviceAdded(const QString &path);
    void diskDeviceRemoved(const QString &path);
    void fileSystemAdded(const QString &blockDevicePath);
    void fileSystemRemoved(const QString &blockDevicePath);
    void mountAdded(const QString &blockDevicePath, const QByteArray &mountPoint);
    void mountRemoved(const QString &blockDevicePath, const QByteArray &mountPoint);
    void mountPointsChanged(const QString &blockDevicePath, const QByteArrayList &oldMountPoints, const QByteArrayList &newMountPoints);

private:
    QScopedPointer<DFMDiskManagerPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_ptr, void _q_onInterfacesAdded(const QDBusObjectPath &, const QMap<QString, QVariantMap> &))
    Q_PRIVATE_SLOT(d_ptr, void _q_onInterfacesRemoved(const QDBusObjectPath &, const QStringList &))
    Q_PRIVATE_SLOT(d_ptr, void _q_onPropertiesChanged(const QString &, const QVariantMap &, const QDBusMessage &))
};

DFM_END_NAMESPACE

#endif // DFMDISKMANAGER_H
