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


#include "dfmdevicecontroller.h"

#include "dfmevent.h"
#include "dfmdeviceinfo.h"
#include "dabstractfilewatcher.h"
#include "private/dabstractfilewatcher_p.h"

#include <dblockdevice.h>
#include <dfmvfsdevice.h>
#include <QUrlQuery>

DFM_USE_NAMESPACE

class DFMDeviceWatcher;
class DFMDeviceWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit DFMDeviceWatcherPrivate(DAbstractFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override
    {
        started = true;
        return true;
    }

    bool stop() override
    {
        started = false;
        return true;
    }
};

class DFMDeviceWatcher : public DAbstractFileWatcher
{
public:
    explicit DFMDeviceWatcher(const DUrl &url, QObject *parent = nullptr)
        : DAbstractFileWatcher(*new DFMDeviceWatcherPrivate(this), url, parent)
    {

    }
};

//------------------------------------------

DFMDeviceController::DFMDeviceController(QObject *parent)
    : DAbstractFileController(parent)
{
    initDiskManager();
    initVfsManager();
//    initConnect();
//    loadCustomVolumeLetters();

//    fileService->setFileUrlHandler(DEVICE_SCHEME, "", this);
}

void DFMDeviceController::initDiskManager()
{
    m_diskMgr.reset(new DDiskManager(this));
    m_diskMgr->setWatchChanges(true);

    // "init" mounted filesystem device.
    QStringList blDevList = m_diskMgr->blockDevices({});
    for (const QString &str : blDevList) {
        fileSystemDeviceAdded(str);
    }

    // watch change, signals.
    connect(m_diskMgr.data(), &DDiskManager::fileSystemAdded, this, &DFMDeviceController::fileSystemDeviceAdded);
    connect(m_diskMgr.data(), &DDiskManager::fileSystemRemoved, this, &DFMDeviceController::fileSystemDeviceRemoved);
}

void DFMDeviceController::initVfsManager()
{
    m_vfsMgr.reset(new DFMVfsManager(this));

    QList<QUrl> vfsdevList = m_vfsMgr->getVfsList();
    for (const QUrl &url : vfsdevList) {
        virualFileSystemDeviceAttached(url);
    }

    connect(m_vfsMgr.data(), &DFMVfsManager::vfsAttached, this, &DFMDeviceController::virualFileSystemDeviceAttached);
    connect(m_vfsMgr.data(), &DFMVfsManager::vfsDetached, this, &DFMDeviceController::virualFileSystemDeviceDetached);
}

const QList<DAbstractFileInfoPointer> DFMDeviceController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    event->ignore();

    QList<DAbstractFileInfoPointer> list;

    // local
    for (const QString &key : m_fsDevMap.keys()) {
        list.append(DAbstractFileInfoPointer(new DFMDeviceInfo(DUrl::fromDeviceId(key))));
    }

    // vfs
    for (const QUrl &url : m_vfsDevSet) {
        list.append(DAbstractFileInfoPointer(new DFMDeviceInfo(DUrl(url))));
    }

    return list;
}

const DAbstractFileInfoPointer DFMDeviceController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    const QString &devicePath = event->url().path();

    if (devicePath.isEmpty()) {
        return DAbstractFileInfoPointer();
    }

    DAbstractFileInfoPointer fileInfo(new DFMDeviceInfo(event->url()));
    return fileInfo;
}

DAbstractFileWatcher *DFMDeviceController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new DFMDeviceWatcher(event->url());
}

void DFMDeviceController::mount(const QString &path)
{
    DUrl url = DUrl::fromUserInput(path);
    if (url.hasScheme(DEVICE_SCHEME)) {
        QString pathStr = url.path();
        if (!QUrl::fromUserInput(pathStr).scheme().isEmpty()) {
            m_vfsMgr->attach(QUrl::fromUserInput(pathStr));
        } else {
            QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(pathStr));
            blDev->mount({});
        }
    } else {
        // consider accept path like "/dev/sda1" ?
        // if not needed, we can only accept url with device:// scheme
    }
}

void DFMDeviceController::unmount(const QString &path)
{
    DUrl url = DUrl::fromUserInput(path);
    if (url.hasScheme(DEVICE_SCHEME)) {
        QString pathStr = url.path();
        if (!QUrl::fromUserInput(pathStr).scheme().isEmpty()) {
            QScopedPointer<DFMVfsDevice> vfsDev(DFMVfsDevice::create(QUrl::fromUserInput(pathStr)));
            if (vfsDev) {
                vfsDev->detachAsync();
            }
        } else {
            QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(pathStr));
            blDev->unmount({});
        }
    } else {
        // consider accept path like "/dev/sda1" ?
        // if not needed, we can only accept url with device:// scheme
    }
}

void DFMDeviceController::eject(const QString &path)
{
    Q_UNUSED(path)
    throw "Not implemented";
}

void DFMDeviceController::stopDrive(const QString &path)
{
    Q_UNUSED(path)
    throw "Not implemented";
}

void DFMDeviceController::forceUnmount(const QString &id)
{
    Q_UNUSED(id)
    throw "Not implemented";
}

/*!
 * \brief Insert a block device to filesysem device map.
 *
 * Won't insert if a block device ( \a dbusPath ) is not a filesystem patition
 */
void DFMDeviceController::fileSystemDeviceAdded(const QString dbusPath)
{
    DBlockDevice *blDev = DDiskManager::createBlockDevice(dbusPath);
    if (blDev->hasFileSystem()) {
        blDev->setWatchChanges(true);
        connect(blDev, &DBlockDevice::idLabelChanged, this, &DFMDeviceController::fileSystemDeviceIdLabelChanged);
        m_fsDevMap.insert(dbusPath, blDev);
        // now we use dbusPath as device identifier
        DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT),
                                          &DAbstractFileWatcher::subfileCreated,
                                          DUrl::fromDeviceId(dbusPath));
    } else {
        delete blDev;
    }
}

void DFMDeviceController::fileSystemDeviceRemoved(const QString dbusPath)
{
    delete m_fsDevMap.take(dbusPath);
    // now we use dbusPath as device identifier
    DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT),
                                      &DAbstractFileWatcher::subfileCreated,
                                      DUrl::fromDeviceId(dbusPath));
}

void DFMDeviceController::fileSystemDeviceIdLabelChanged(const QString &labelName)
{
    DBlockDevice *blDev = qobject_cast<DBlockDevice *>(QObject::sender());
    DUrl oldUrl, newUrl;
    oldUrl.setScheme(DEVICE_SCHEME);
    oldUrl.setPath(blDev->drive());
    newUrl = oldUrl;
    QUrlQuery query;
    query.addQueryItem("new_name", labelName);
    newUrl.setQuery(query);
    DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT), &DAbstractFileWatcher::fileMoved, oldUrl, newUrl);
}

void DFMDeviceController::virualFileSystemDeviceAttached(const QUrl &url)
{
    if (m_vfsDevSet.contains(url)) return;

    m_vfsDevSet.insert(url);
    // for vfs, device id is url.
    DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT),
                                      &DAbstractFileWatcher::subfileCreated,
                                      DUrl::fromDeviceId(url.toString()));
}

void DFMDeviceController::virualFileSystemDeviceDetached(const QUrl &url)
{
    m_vfsDevSet.remove(url);
    // for vfs, device id is url.
    DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT),
                                      &DAbstractFileWatcher::fileDeleted,
                                      DUrl::fromDeviceId(url.toString()));
}
