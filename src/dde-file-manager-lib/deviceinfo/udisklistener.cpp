/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "disomaster.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "controllers/subscriber.h"
#include "singleton.h"

#include "views/dfmopticalmediawidget.h"
#include "gvfs/gvfsmountmanager.h"
#include "udiskdeviceinfo.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "ddiskmanager.h"
#include "shutil/fileutils.h"
#include "dialogs/dialogmanager.h"
#include "private/dabstractfilewatcher_p.h"
#include "app/define.h"
#include <linux/cdrom.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mntent.h>

#include <QSettings>
#include <QProcess>
#include <QStorageInfo>
#include <QUrlQuery>


class UDiskFileWatcher;
class UDiskFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit UDiskFileWatcherPrivate(DAbstractFileWatcher *qq)
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

UDiskListener::~UDiskListener()
{
    DFileService::unsetFileUrlHandler(this);
}

void UDiskListener::initDiskManager()
{
    m_diskMgr = new DDiskManager(this);
    m_diskTimer = new QTimer(this);
    m_diskMgr->setWatchChanges(true);
    QStringList blDevList = m_diskMgr->blockDevices({});
    for (const QString &str : blDevList) {
        insertFileSystemDevice(str);
    }

// 以下这段定时器代码可解决打开光驱访问文件后，物理弹出光驱，文管界面却没能卸载光驱设备的问题。
    connect(m_diskTimer, &QTimer::timeout, this, &UDiskListener::loopCheckCD);
}

void UDiskListener::initConnect()
{
    connect(m_diskMgr, &DDiskManager::fileSystemAdded, this, &UDiskListener::insertFileSystemDevice);
    connect(m_diskMgr, &DDiskManager::fileSystemRemoved, this, [this](const QString & path) {
        delete m_fsDevMap.take(path);
    });
    connect(gvfsMountManager, &GvfsMountManager::mount_added, this, &UDiskListener::addMountDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::mount_removed, this, &UDiskListener::removeMountDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::volume_added, this, &UDiskListener::addVolumeDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::volume_removed, this, &UDiskListener::removeVolumeDiskInfo);
    connect(gvfsMountManager, &GvfsMountManager::volume_changed, this, &UDiskListener::changeVolumeDiskInfo);

    connect(fileSignalManager, &FileSignalManager::stopCdScanTimer, this, [ = ](const QString & strDev) {
        Q_UNUSED(strDev)
        if (m_diskTimer->isActive()) {
            m_diskTimer->stop();
            qDebug() << "timer stop, curr cdrom:" << m_nCDRomCount;
        }
    });
    connect(fileSignalManager, &FileSignalManager::restartCdScanTimer, this, [ = ](const QString & strDev) {
        Q_UNUSED(strDev)
        if (m_nCDRomCount > 0 && !m_diskTimer->isActive()) {
            m_diskTimer->start(3000);
            qDebug() << "timer restart, curr cdrom:" << m_nCDRomCount;
        }
    });
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

    if (device->getDiskInfo().drive_unix_device().contains("/dev/sr")) {// 探测到有光驱设备接入，启动光驱设备检测定时器
        m_nCDRomCount++;
        if (!m_diskTimer)
            m_diskTimer = new QTimer;
        if (!m_diskTimer->isActive()) {
            m_diskTimer->start(3000);
            qDebug() << "timer start, curr cdrom:" << m_nCDRomCount;
        }
    }
    emit volumeAdded(device);
}

void UDiskListener::removeDevice(UDiskDeviceInfoPointer device)
{
    m_list.removeOne(device);
    m_map.remove(device->getDiskInfo().id());
    if (device->getDiskInfo().drive_unix_device().contains("/dev/sr")) {
        m_nCDRomCount--;
        if (m_nCDRomCount == 0) {
            m_diskTimer->stop();
            qDebug() << "timer stop, curr cdrom: " << m_nCDRomCount;
        }
    }

    DAbstractFileWatcher::ghostSignal(DUrl(DEVICE_ROOT),
                                      &DAbstractFileWatcher::fileDeleted,
                                      DUrl::fromDeviceId(device->getId()));
}

bool UDiskListener::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    const DUrl &oldUrl = event->fromUrl();
    const DUrl &newUrl = event->toUrl();

    const QSharedPointer<DFMCreateFileInfoEvent> e(new DFMCreateFileInfoEvent(nullptr, oldUrl));
    const DAbstractFileInfoPointer &oldDevicePointer = UDiskListener::createFileInfo(e);

    DAbstractFileInfo *info = oldDevicePointer.data();
    UDiskDeviceInfo *udiskInfo = dynamic_cast<UDiskDeviceInfo *>(info);
    QString devicePath;
    if (udiskInfo)
        devicePath = udiskInfo->getDBusPath();
    QUrlQuery query(newUrl);
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

QList<UDiskDeviceInfoPointer> UDiskListener::getMountList()
{
    return m_mountList;
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
        if (info && !info->getMountPointUrl().toLocalFile().isEmpty()) {
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
// fix task 29259 ,在判断空白光驱时就返回，是没有找到要用的UDiskDeviceInfoPointer，需要找的info在m_list中的位置在空白光驱之后，所以就不能直接返回
// 使用bshareuse，是在判断可以共享时，根据路径取得UDiskDeviceInfoPointer,找到空白光驱就跳过，查询完m_list所有的元素
UDiskDeviceInfoPointer UDiskListener::getDeviceByFilePath(const QString &path, const bool bshareuse)
{
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        if (info && !info->getMountPointUrl().isEmpty()) {

            bool t_ok = info->getId().contains("/dev/sr");
            Q_UNUSED(t_ok)
            //获取空白光盘路径有问题，fix
            if (!bshareuse && (info->getMountPointUrl().toString() == "burn:///" || info->getId().contains("/dev/sr"))) {
                return UDiskDeviceInfoPointer();
            }
            //获取空白光盘路径有问题，fix
            // fix task 29259增加一个判断info->getMountPointUrl()，因为有可能空光盘的挂载点是空
            bool flag = (DUrl::fromLocalFile(path) == info->getMountPointUrl());
            if (!flag && !info->getMountPointUrl().toLocalFile().isEmpty() && path.startsWith(QString("%1").arg(info->getMountPointUrl().toLocalFile()))) {
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

bool UDiskListener::isFileFromDisc(const QString &filePath)
{
    const QMap<QString, UDiskDeviceInfoPointer> &&devices = getMountedRemovableDiskDeviceInfos();
    foreach (auto d, devices) {
        // mountPath will be '/' if device is a blank disk
        QString mountPath = d->getMountPointUrl().path();
        if (d->optical() && !d->opticalBlank() && mountPath != "/"
                && !mountPath.isEmpty() && filePath.startsWith(mountPath)) {
            qInfo() << filePath << "is belong to disc, mount point" << mountPath;
            return true;
        }
    }
    return false;
}

void UDiskListener::appendHiddenDirs(const QString &path)
{
    if (!m_hiddenDirs.contains(path))
        m_hiddenDirs.append(path);
}

QStringList UDiskListener::hiddenDirs()
{
    return m_hiddenDirs;
}

bool UDiskListener::isBlockFile(const QString &filePath)
{
    const QMap<QString, UDiskDeviceInfoPointer> &&devices = getMountedRemovableDiskDeviceInfos();
    foreach (auto d, devices) {
        // mountPath will be '/' if device is a blank disk
        // always return true if mountPath is '/'
        if (d->optical() && d->opticalBlank())
            continue;

        QString mountPath = d->getMountPointUrl().path();
        if (!mountPath.isEmpty() && mountPath != "/" && filePath.startsWith(mountPath)) {
            return true;
        }
    }
    return false;
}

bool UDiskListener::isFromNativeDisk(const QString &uuid)
{
    bool ret = false;
    static std::once_flag flag;

    // 线程安全: getmntent 不是可重入函数, call_once 能保证线程安全
    // 性能: /etc/fstab 不是频繁变化的配置文件, 且配置后一般会重启才生效, 所以不必多次读取
    std::call_once(flag, [this]() {
        FILE *f = nullptr;
        f = setmntent(_PATH_FSTAB, "r");
        if (f) {
            struct mntent *m = nullptr;
            while ((m = getmntent(f))) {
                QString name(m->mnt_fsname);
                QStringList group(name.split("="));
                if (name.startsWith("UUID", Qt::CaseInsensitive) && group.size() == 2) {
                    m_uuids.append(group.at(1));
                }
            }
            endmntent(f);
        }
    });

    if (m_uuids.contains(uuid))
        ret = true;

    return ret;
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
        m_mountList.append(device);
        emit mountAdded(device);
    }

    qDebug() << m_subscribers;
    for (Subscriber *sub : m_subscribers) {
        QString url = device->getMountPointUrl().toString();
        QStringList devList = DDiskManager::resolveDeviceNode(device->getId(), {});
        if (!devList.isEmpty()) {
            QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(devList.first()));
            QScopedPointer<DDiskDevice> drive(DDiskManager::createDiskDevice(blkdev->drive()));
            if (drive->optical()) {
                url = DUrl::fromBurnFile(device->getId() + "/" + BURN_SEG_ONDISC + "/").toString();
            }
            qDebug() << url;
            sub->doSubscriberAction(url);
        } else {
            if (DFMGlobal::isOpenAsAdmin())
                sub->doSubscriberAction(url);
        }
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
        ISOMaster->nullifyDevicePropertyCache(QString(diskInfo.drive_unix_device()));
        m_mountList.removeOne(device);
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

    ISOMaster->nullifyDevicePropertyCache(QString(diskInfo.drive_unix_device()));

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

    // sp3 feature 35 光盘正在加载时，光标移动到光盘图标（左侧tab和计算机页面item），光标显示繁忙状态，加载完成后显示为正常指针
    if (diskInfo.id().contains("sr") && !diskInfo.mounted_root_uri().isEmpty()) { // 光盘已挂载上后挂载路径有值
        const QString &volTag = diskInfo.id().mid(5);
        if (DFMOpticalMediaWidget::g_mapCdStatusInfo.contains(volTag)) {
            DFMOpticalMediaWidget::g_mapCdStatusInfo[volTag].bLoading = false;
            DFileService::instance()->setCursorBusyState(false); // 上面一行代码用于设置光标在移动时的判定条件，如果没有移动不会触发更新，因此这里要手动刷新一次状态
        }
    }
}

void UDiskListener::mount(const QString &path)
{
    qDebug() << path;
    // main.cpp : handleEnvOfOpenAsAdmin 已经解决 gio root 用户下的挂载问题
    GvfsMountManager::mount(path);
#if 0
    // The gio's method cannot mount block device, use the UDisks's method replace it.(bug 42690)
    if (DFMGlobal::isOpenAsAdmin() && mountByUDisks(path)) {
        return;
    } else {
        GvfsMountManager::mount(path);
    }
#endif
}

bool UDiskListener::mountByUDisks(const QString &path)
{
    const QStringList &rootDeviceNode = DDiskManager::resolveDeviceNode(path, {});
    if (rootDeviceNode.isEmpty()) {
        return false;
    }

    const QString &udiskspath = rootDeviceNode.first();
    QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(udiskspath));
    if (blkdev) {
        const QString &mountedPath = blkdev->mount({});
        qDebug() << "mounted path by udisks:" << mountedPath;
        return mountedPath.isEmpty() ? false : true;
    }

    return false;
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

void UDiskListener::fileSystemDeviceIdLabelChanged(const QString &labelName)
{
    DBlockDevice *blDev = qobject_cast<DBlockDevice *>(QObject::sender());
    DUrl oldUrl, newUrl;
    oldUrl.setScheme(DEVICE_SCHEME);
    if (blDev)
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
    DBlockDevice *blDev = DDiskManager::createBlockDevice(dbusPath);
    if (blDev->hasFileSystem()) {
        blDev->setWatchChanges(true);
        connect(blDev, &DBlockDevice::idLabelChanged, this, &UDiskListener::fileSystemDeviceIdLabelChanged);
        m_fsDevMap.insert(dbusPath, blDev);
    } else {
        delete blDev;
    }
}

void UDiskListener::loopCheckCD()
{
    // 服务器版本会刷系统日志
    if (DFMGlobal::isServerSys())
        return;
    for (int i = 0; i < m_list.size(); i++) {
        UDiskDeviceInfoPointer info = m_list.at(i);
        //qDebug() << "UDiskDeviceInfoPointer" << info->getDiskInfo().drive_unix_device();
        QString t_device = info->getDiskInfo().drive_unix_device();

        //监测光驱托盘是否被弹出
        if (t_device.contains("/dev/sr")) {

            /* only try to open read/write if not root, since it doesn't seem
             * to make a difference for root and can have negative side-effects
             */
            if (geteuid()) {
                QByteArray devBytes(t_device.toLatin1());
                int t_cdromfd = open(devBytes.data(), O_RDWR | O_NONBLOCK);
                if (t_cdromfd != -1) {
                    close(t_cdromfd);
                }
            }
        }
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

const DAbstractFileInfoPointer UDiskListener::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
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
