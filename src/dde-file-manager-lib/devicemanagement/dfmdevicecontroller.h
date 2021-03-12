/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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


#ifndef DFMDEVICECONTROLLER_H
#define DFMDEVICECONTROLLER_H

#include <dabstractfilecontroller.h>
#include <ddiskmanager.h>
#include <dfmvfsmanager.h>

DFM_BEGIN_NAMESPACE
class DFMVfsDevice;
DFM_END_NAMESPACE

class DFMDeviceController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit DFMDeviceController(QObject *parent = nullptr);

    void initDiskManager();
    void initVfsManager();

    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

signals:
    void vfsMountAdded(const QUrl &url);
    void vfsMountRemoved(const QUrl &url);

public slots:
    void mount(const QString &path);
    void unmount(const QString &path);
    void eject(const QString &path);
    void stopDrive(const QString &path);
    void forceUnmount(const QString &id);

private slots:
    // filesystem device managed by udisks2
    void fileSystemDeviceAdded(const QString dbusPath);
    void fileSystemDeviceRemoved(const QString dbusPath);
    void fileSystemDeviceIdLabelChanged(const QString &labelName);
    // virtual filesystem (from network location) managed by gio
    void virualFileSystemDeviceAttached(const QUrl &url);
    void virualFileSystemDeviceDetached(const QUrl &url);

private:
    QScopedPointer<DDiskManager> m_diskMgr;
    QScopedPointer<DFM_NAMESPACE::DFMVfsManager> m_vfsMgr;

    QMap<QString, DBlockDevice*> m_fsDevMap; // key is udisks2 device dbus path
    QSet<QUrl> m_vfsDevSet; // value is device scheme url with path as deviceId, eg. device:mtp://%5Busb%3A003,004%5D/
};

#endif // DFMDEVICECONTROLLER_H
