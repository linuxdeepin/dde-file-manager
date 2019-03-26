/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "udisklistener.h"
#include "fstab.h"
#include "dfileservices.h"
#include "dfmevent.h"
#include "dfmapplication.h"
#include "dfmsettings.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "controllers/subscriber.h"
#include "singleton.h"

#include "gvfs/gvfsmountmanager.h"
#include "udiskdeviceinfo.h"
#include "dblockdevice.h"
#include "ddiskmanager.h"
#include "shutil/fileutils.h"
#include "dialogs/dialogmanager.h"
#include "private/dabstractfilewatcher_p.h"

#include <QSettings>
#include <QProcess>
#include <QStorageInfo>
#include <QUrlQuery>


class UDiskFileWatcher;
class UDiskFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    UDiskFileWatcherPrivate(DAbstractFileWatcher *qq)
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

class UDiskFileWatcher : public DAbstractFileWatcher
{
public:
    explicit UDiskFileWatcher(const DUrl &url, QObject *parent = nullptr)
        : DAbstractFileWatcher(*new UDiskFileWatcherPrivate(this), url, parent)
    {

    }
};

UDiskListener::UDiskListener(QObject *parent):
    DAbstractFileController(parent)
{
    initDiskManager();
    initConnect();
    loadCustomVolumeLetters();

    fileService->setFileUrlHandler(DEVICE_SCHEME, "", this);
}

void UDiskListener::initDiskManager()
{
    m_diskMgr = new DDiskManager(this);
    m_diskMgr->setWatchChanges(true);
    QStringList blDevList = m_diskMgr->blockDevices();
    for (const QString &str : blDevList) {
        insertFileSystemDevice(str);
    }
}

void UDiskListener::initConnect()
{
    connect(m_diskMgr, &DDiskManager::fileSystemAdded, this, &UDiskListener::insertFileSystemDevice);
    connect(m_diskMgr, &DDiskManager::fileSystemRemoved, this, [this](const QString& path) {
        delete m_fsDevMap.take(path);
    });
    connect(gvfsMountManager, &GvfsMountManager::mount_added, this, &UDiskListener::addMountDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::mount_removed, this, &UDiskListener::removeMountDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::volume_added, this, &UDiskListener::addVolumeDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::volume_removed, this, &UDiskListener::removeVolumeDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::volume_changed, this, &UDiskListener::changeVolumeDiskInfo);
}

UDiskDeviceInfoPointer UDiskListener::getDevice(const QString &id)
{
    if (m_map.contains(id)) {
        return m_map[id];
    } else {
        return UDiskDeviceInfoPointer();
    }
}

void UDiskListener::addDevice(UDiskDeviceInfoPointer device)
{
    m_map.insert(device->getDiskInfo().id(), device);
    m_list.append(device);

    DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT),
                                      &DAbstractFileWatcher::subfileCreated,
                                      DUrl::fromDeviceId(device->getId()));
    emit volumeAdded(device);
}

void UDiskListener::removeDevice(UDiskDeviceInfoPointer device)
{
    m_list.removeOne(device);
    m_map.remove(device->getDiskInfo().id());

    DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT),
                                      &DAbstractFileWatcher::fileDeleted,
                                      DUrl::fromDeviceId(device->getId()));
}

bool UDiskListener::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    const DUrl &oldUrl = event->fromUrl();
    const DUrl &newUrl = event->toUrl();

    const QSharedPointer<DFMCreateFileInfoEvnet> e(new DFMCreateFileInfoEvnet(nullptr, oldUrl));
    const DAbstractFileInfoPointer &oldDevicePointer = UDiskListener::createFileInfo(e);

    DAbstractFileInfo* info = oldDevicePointer.data();
    UDiskDeviceInfo* udiskInfo = dynamic_cast<UDiskDeviceInfo*>(info);
    QString devicePath = udiskInfo->getPath();
    QUrlQuery query(newUrl);
    devicePath.replace("dev", "org/freedesktop/UDisks2/block_devices");
    QString newName = query.queryItemValue("new_name");
    DBlockDevice *partition = DDiskManager::createBlockDevice(devicePath, nullptr);

    if (!partition) {
        return false;
    }

    // set new label name
    partition->setLabel(newName, {});

    // check if we got error
    QDBusError err = DDiskManager::lastError();
    qDebug() << err.type();
    switch (err.type()) {
    case QDBusError::NoReply:
        // blumia: user doesn't do action and caused a dbus message reply timeout error (org.freedesktop.DBus.Error.NoReply).
        //         just return it and don't show any error message dialog.
        //         notice that don't be confused with another timeout-releated error type (org.freedesktop.DBus.Error.Timeout
        //         and org.freedesktop.DBus.Error.TimedOut), the `Timeout` one *possibly* means a socket ETIMEDOUT error
        //         and the `Timedout` one means certain timeout errors.
        // see also: https://dbus.freedesktop.org/doc/api/html/group__DBusProtocol.html
        //           https://www.freedesktop.org/wiki/Software/DBusBindingErrors/ (¯\_(ツ)_/¯)
        return false;
    case QDBusError::NoError:
        // blumia: when renamed, a DBus signal will be sent by udisks2, and it will trigger `fileSystemDeviceIdLabelChanged()`
        //         we don't need do rename here so just return true.
        return true;
    default:
        dialogManager->showErrorDialog(tr("Failed to rename the label"), err.message());
        return false;
    }

    return false;
}

void UDiskListener::update()
{
    QStringList keys;

    if (DFMGlobal::isRootUser()) {
        keys.append(GvfsMountManager::Lsblk_Keys);
    } else {
        keys.append(GvfsMountManager::Volumes_Drive_Keys);
        keys.append(GvfsMountManager::Volumes_No_Drive_Keys);
        keys.append(GvfsMountManager::NoVolumes_Mounts_Keys);
    }
    foreach (QString key, keys) {
        QDiskInfo diskInfo = GvfsMountManager::DiskInfos.value(key);
        if (diskInfo.isValid()) {
            addMountDiskInfo(diskInfo);
        }
    }
}


QString UDiskListener::lastPart(const QString &path)
{
    return path.split('/').last();
}


UDiskDeviceInfoPointer UDiskListener::hasDeviceInfo(const QString &id)
{
    return m_map.value(id);
}

void UDiskListener::addSubscriber(Subscriber *sub)
{
    if (!m_subscribers.contains(sub)) {
        m_subscribers.append(sub);
    }
}

void UDiskListener::removeSubscriber(Subscriber *sub)
{
    if (m_subscribers.contains(sub)) {
        m_subscribers.removeOne(sub);
    }
}

QMap<QString, UDiskDeviceInfoPointer> UDiskListener::getAllDeviceInfos()
{
    return m_map;
}

QList<UDiskDeviceInfoPointer> UDiskListener::getDeviceList()
{
    return m_list;
}

bool UDiskListener::isDeviceFolder(const QString &path) const
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info->getMountPointUrl().toLocalFile() == path) {
            return true;
        }
    }
    return false;
}

bool UDiskListener::isInDeviceFolder(const QString &path) const
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPointUrl().isEmpty()) {
            if (path.startsWith(info->getMountPointUrl().toLocalFile())) {
                return true;
            }
        }
    }
    return false;
}

bool UDiskListener::isInRemovableDeviceFolder(const QString &path) const
{
    QList<UDiskDeviceInfo::MediaType> mediaTypes = {UDiskDeviceInfo::removable,
                                                    UDiskDeviceInfo::iphone,
                                                    UDiskDeviceInfo::phone,
                                                    UDiskDeviceInfo::camera
                                                   };
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (mediaTypes.contains(info->getMediaType())) {
            if (!info->getMountPointUrl().isEmpty()) {
                if (path.startsWith(info->getMountPointUrl().toLocalFile())) {
                    return true;
                }
            }
        }
    }
    return false;
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByDevicePath(const QString &deveicePath)
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && info->getPath() == deveicePath) {
            return info;
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByMountPoint(const QString &mountPoint)
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPoint().isEmpty()) {
            if (DUrl(info->getMountPoint()) == DUrl(mountPoint)) {
                return info;
            }

            // TODO: this is a polyfill since getting GFileInfo from network devices and mounted
            //       devices (both provided by gio) will return different uri (case insensitive
            //       when getting uri from network device, or say, samba/smb uri).
            //       this should be fixed or report to upstream.
            if (mountPoint.startsWith("smb://")) {
                QString source = mountPoint;
                QString target = info->getMountPoint();

                if (source.endsWith("/")) {
                    source.chop(1);
                }

                source = QUrl::fromPercentEncoding(source.toUtf8());

                if (target.endsWith("/")) {
                    target.chop(1);
                }

                target = QUrl::fromPercentEncoding(target.toUtf8());

                if (QString::compare(source, target, Qt::CaseInsensitive) == 0) {
                    return info;
                }
            }
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByMountPointFilePath(const QString &filePath)
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPoint().isEmpty()) {
            bool flag = DUrl(info->getMountPoint()) == DUrl(filePath);
            if (!flag && filePath.startsWith(QString("%1").arg(info->getMountPointUrl().toLocalFile()))) {
                return info;
            }
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByPath(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPointUrl().isEmpty()) {
            bool flag = (DUrl::fromLocalFile(path) == info->getMountPointUrl());

            if (path.startsWith(info->getMountPointUrl().toLocalFile()) && flag) {
                return info;
            }
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByFilePath(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPointUrl().isEmpty()) {
            bool flag = (DUrl::fromLocalFile(path) == info->getMountPointUrl());
            if (!flag && path.startsWith(QString("%1").arg(info->getMountPointUrl().toLocalFile()))) {
                return info;
            }
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByDeviceID(const QString &deviceID)
{
    foreach (const UDiskDeviceInfoPointer &info, m_list) {
        if (info->getId() == deviceID) {
            return info;
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfoPointer UDiskListener::getDeviceByUUID(const QString &uuid)
{
    foreach (const UDiskDeviceInfoPointer &info, m_list) {
        if (info->getDiskInfo().uuid() == uuid) {
            return info;
        }
    }
    return UDiskDeviceInfoPointer();
}

UDiskDeviceInfo::MediaType UDiskListener::getDeviceMediaType(const QString &path)
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && info->getMountPointUrl().toLocalFile() == path) {
            return info->getMediaType();
        }
    }
    return UDiskDeviceInfo::unknown;
}

void UDiskListener::loadCustomVolumeLetters()
{
    const QSet<QString> &keys = DFMApplication::genericObtuselySetting()->keys("Disk/Volume");

    for (const QString &key : keys)
        m_volumeLetters.insert(key, DFMApplication::genericObtuselySetting()->value("Disk/Volume", key).toString());
}

QMap<QString, QString> UDiskListener::getVolumeLetters()
{
    return m_volumeLetters;
}

QMap<QString, UDiskDeviceInfoPointer> UDiskListener::getMountedRemovableDiskDeviceInfos()
{
    QMap<QString, UDiskDeviceInfoPointer> infos;
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info->getDiskInfo().is_removable() && info->getDiskInfo().can_unmount()) {
            infos.insert(info->getDiskInfo().id(), info);
        }
    }
    return infos;
}

QMap<QString, UDiskDeviceInfoPointer> UDiskListener::getCanSendDisksByUrl(QString filepath)
{
    QMap<QString, UDiskDeviceInfoPointer> infos;
    foreach (UDiskDeviceInfoPointer info, getMountedRemovableDiskDeviceInfos().values()) {
        if (getDeviceByFilePath(filepath) == info) {
            continue;
        }
        infos.insert(info->getDiskInfo().id(), info);
    }
    return infos;
}

bool UDiskListener::isMountedRemovableDiskExits()
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info->getDiskInfo().is_removable() && info->getDiskInfo().can_unmount()) {
            return true;
        }
    }
    return false;
}

void UDiskListener::addMountDiskInfo(const QDiskInfo &diskInfo)
{
    qDebug() << diskInfo;
    UDiskDeviceInfoPointer device;
    if (m_map.value(diskInfo.id())) {
        device = m_map.value(diskInfo.id());
        device->setDiskInfo(diskInfo);
    } else {
        device = new UDiskDeviceInfo();
        device->setDiskInfo(diskInfo);
        addDevice(device);
    }

    if (!diskInfo.mounted_root_uri().isEmpty()) {
        DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT),
                                          &DAbstractFileWatcher::fileAttributeChanged,
                                          DUrl::fromDeviceId(device->getId()));
        emit mountAdded(device);
    }

    qDebug() << m_subscribers;
    foreach (Subscriber *sub, m_subscribers) {
        QString url = device->getMountPointUrl().toString();
        qDebug() << url;
        sub->doSubscriberAction(url);
    }

}

void UDiskListener::removeMountDiskInfo(const QDiskInfo &diskInfo)
{
    UDiskDeviceInfoPointer device;
    qDebug() << diskInfo;
    qDebug() << m_map.contains(diskInfo.id());
    qDebug() << m_map;
    if (m_map.value(diskInfo.id())) {
        device = m_map.value(diskInfo.id());
        qDebug() << diskInfo.has_volume();
        if (diskInfo.has_volume()) {
            device->setDiskInfo(diskInfo);
        } else {
            removeDevice(device);
        }

        DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT),
                                          &DAbstractFileWatcher::fileAttributeChanged,
                                          DUrl::fromDeviceId(device->getId()));
        emit mountRemoved(device);
    }
}

void UDiskListener::addVolumeDiskInfo(const QDiskInfo &diskInfo)
{
    if (diskInfo.id().isEmpty()) {
        return;
    }

    UDiskDeviceInfoPointer device;

    if (m_map.contains(diskInfo.id())) {
        device = m_map.value(diskInfo.id());
        device->setDiskInfo(diskInfo);

        emit volumeChanged(device);
    } else {
        device = new UDiskDeviceInfo();
        device->setDiskInfo(diskInfo);
        addDevice(device);
    }
}

void UDiskListener::removeVolumeDiskInfo(const QDiskInfo &diskInfo)
{
    UDiskDeviceInfoPointer device;
    qDebug() << diskInfo << m_list;
    if (m_map.contains(diskInfo.id())) {
        device = m_map.value(diskInfo.id());
//        removeDevice(device);
//        emit volumeRemoved(device);
    } else {
        foreach (UDiskDeviceInfoPointer d, getDeviceList()) {
            qDebug() << d->getDiskInfo();

            if (!(diskInfo.uuid().isEmpty() && d->getDiskInfo().uuid() == diskInfo.uuid())
                    || d->getDiskInfo().id() == diskInfo.id()) {
                device = d;
                break;
            }
        }
    }

    if (device) {
        qDebug() << device->getDiskInfo();
        removeDevice(device);
        emit volumeRemoved(device);
    }
}

void UDiskListener::changeVolumeDiskInfo(const QDiskInfo &diskInfo)
{
    UDiskDeviceInfoPointer device;
    qDebug() << diskInfo;
    qDebug() << m_map.value(diskInfo.id());

    if (diskInfo.id().isEmpty()) {
        return;
    }

    if (m_map.value(diskInfo.id())) {
        device = m_map.value(diskInfo.id());
    } else {
        foreach (UDiskDeviceInfoPointer d, getDeviceList()) {
            qDebug() << d->getDiskInfo().uuid() << diskInfo.uuid();
            if (d->getDiskInfo().uuid() == diskInfo.uuid()) {
                device = d;
                break;
            }
        }
    }
    if (device) {
        device->setDiskInfo(diskInfo);
        emit volumeChanged(device);
    }
}

void UDiskListener::mount(const QString &path)
{
    qDebug() << path;
    GvfsMountManager::mount(path);
}

void UDiskListener::unmount(const QString &path)
{
    qDebug() << path;
    GvfsMountManager::unmount(path);
}

void UDiskListener::eject(const QString &path)
{
    GvfsMountManager::eject(path);
}

void UDiskListener::stopDrive(const QString &path)
{
    GvfsMountManager::stop_device(path);
}

void UDiskListener::forceUnmount(const QString &id)
{
    qDebug() << id;
    if (m_map.contains(id)) {
        UDiskDeviceInfoPointer device = m_map.value(id);
        QStringList args;
        args << "mount" << "-f" ;
        if (device->canEject()) {
            args << "-e" << device->getMountPointUrl().toLocalFile();
        } else {
            args << "-u" << device->getMountPointUrl().toLocalFile();
        }
        bool reslut = QProcess::startDetached("gio", args);
        qDebug() << "gio mount" << args << reslut;
    }
}

void UDiskListener::fileSystemDeviceIdLabelChanged(const QString & labelName)
{
    DBlockDevice* blDev = qobject_cast<DBlockDevice*>(QObject::sender());
    DUrl oldUrl, newUrl;
    oldUrl.setScheme(DEVICE_SCHEME);
    oldUrl.setPath(QString::fromLatin1(blDev->device()));
    newUrl = oldUrl;
    QUrlQuery query;
    query.addQueryItem("new_name", labelName);
    newUrl.setQuery(query);
    DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT), &DAbstractFileWatcher::fileMoved, oldUrl, newUrl);
}

/*!
 * \brief Insert a block device to filesysem device map.
 *
 * Won't insert if a block device ( \a dbusPath ) is not a filesystem patition
 *
 * \param dbusPath
 */
void UDiskListener::insertFileSystemDevice(const QString dbusPath)
{
    DBlockDevice* blDev = DDiskManager::createBlockDevice(dbusPath);
    if (blDev->hasFileSystem()) {
        blDev->setWatchChanges(true);
        connect(blDev, &DBlockDevice::idLabelChanged, this, &UDiskListener::fileSystemDeviceIdLabelChanged);
        m_fsDevMap.insert(dbusPath, blDev);
    } else {
        delete blDev;
    }
}

const QList<DAbstractFileInfoPointer> UDiskListener::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    const QString &frav = event->url().fragment();

    if (!frav.isEmpty()) {
        const QList<DAbstractFileInfoPointer> &list = fileService->getChildren(event->sender(), DUrl::fromLocalFile(frav),
                event->nameFilters(), event->filters(), event->flags());

        return list;
    }

    QList<DAbstractFileInfoPointer> infolist;

    for (int i = 0; i < m_list.size(); i++) {
        DAbstractFileInfoPointer fileInfo(new UDiskDeviceInfo(DUrl::fromDeviceId(m_list.at(i)->getId())));
        infolist.append(fileInfo);
    }

    return infolist;
}

const DAbstractFileInfoPointer UDiskListener::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    const QString &deviceId = event->url().path();

    if (deviceId.isEmpty()) {
        return DAbstractFileInfoPointer();
    }


    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);

        if (info->getId() == deviceId) {
            DAbstractFileInfoPointer fileInfo(new UDiskDeviceInfo(info));
            return fileInfo;
        }
    }

    return DAbstractFileInfoPointer();
}

DAbstractFileWatcher *UDiskListener::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new UDiskFileWatcher(event->url());
}
