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

//fix: 设置光盘容量属性
#include "../views/dfmopticalmediawidget.h"

#include "gvfsmountmanager.h"
#include "qdrive.h"
#include "qvolume.h"
#include "qmount.h"
#include "qdiskinfo.h"
#include "singleton.h"
#include "../app/define.h"
#include "../interfaces/dfileservices.h"
#include "../interfaces/dfmevent.h"
#include "deviceinfo/udisklistener.h"
#include "dialogs/dialogmanager.h"
#include "mountsecretdiskaskpassworddialog.h"
#include "app/filesignalmanager.h"
#include "shutil/fileutils.h"
#include "utils.h"

#include "networkmanager.h"
#include "dfmapplication.h"
#include "dabstractfilewatcher.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "mountaskpassworddialog.h"

#include <QThread>
#include <QApplication>
#include <QLoggingCategory>
#include <QTimer>
#include <QJsonArray>
#include <QProcess>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QRegularExpression>
#include <dgiofile.h>

#include <disomaster.h>

#include <views/windowmanager.h>

/*afc has no unix_device, so use uuid as unix_device*/

QMap<QString, QDrive> GvfsMountManager::Drives = {}; // key is unix-device
QMap<QString, QVolume> GvfsMountManager::Volumes = {}; // key is unix-device or uuid
QMap<QString, QMount> GvfsMountManager::Mounts = {}; // key is mount point root uri
QMap<QString, QDiskInfo> GvfsMountManager::DiskInfos = {};

QStringList GvfsMountManager::Drives_Keys = {};//key is unix-device
QStringList GvfsMountManager::Volumes_Drive_Keys = {}; // key is unix-device
QStringList GvfsMountManager::Volumes_No_Drive_Keys = {}; // key is unix-device or uuid

QStringList GvfsMountManager::NoVolumes_Mounts_Keys = {}; // key is mount point root uri
QStringList GvfsMountManager::Lsblk_Keys = {}; // key is got by lsblk

MountSecretDiskAskPasswordDialog *GvfsMountManager::mountSecretDiskAskPasswordDialog = nullptr;

bool GvfsMountManager::AskedPasswordWhileMountDisk = false;

QHash<GMountOperation *, QSharedPointer<DFMUrlBaseEvent>> GvfsMountManager::MountEventHash;
QHash<GMountOperation *, QSharedPointer<QTimer>> GvfsMountManager::MountTimerHash;
QHash<GMountOperation *, GCancellable *> GvfsMountManager::CancellHash;
QHash<GMountOperation *, QSharedPointer<QEventLoop>> GvfsMountManager::eventLoopHash;
QHash<GMountOperation *, bool> GvfsMountManager::AskingPasswordHash;
QHash<GMountOperation *, MountAskPasswordDialog *> GvfsMountManager::askPasswordDialogHash;
QHash<GMountOperation *, QJsonObject *> GvfsMountManager::SMBLoginObjHash;

//fix: 每次弹出光驱时需要删除临时缓存数据文件
QString GvfsMountManager::g_qVolumeId = "sr0";

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(mountManager, "gvfs.mountMgr")
#else
Q_LOGGING_CATEGORY(mountManager, "gvfs.mountMgr", QtInfoMsg)
#endif

GvfsMountManager::GvfsMountManager(QObject *parent) : QObject(parent)
{
    m_gVolumeMonitor = g_volume_monitor_get();
    qRegisterMetaType<QDrive>("QDrive");
}

void GvfsMountManager::initConnect()
{
    if (DFMGlobal::isRootUser()) {
        g_signal_connect(m_gVolumeMonitor, "mount-added", (GCallback)&GvfsMountManager::monitor_mount_added_root, nullptr);
        g_signal_connect(m_gVolumeMonitor, "mount-removed", (GCallback)&GvfsMountManager::monitor_mount_removed_root, nullptr);
    }
    //fix 28660 【文件管理器】【5.1.1.60-1】【服务器】root用户，卸载U盘后，无法重新挂载
    g_signal_connect(m_gVolumeMonitor, "mount-added", (GCallback)&GvfsMountManager::monitor_mount_added, nullptr);
    g_signal_connect(m_gVolumeMonitor, "mount-removed", (GCallback)&GvfsMountManager::monitor_mount_removed, nullptr);
    g_signal_connect(m_gVolumeMonitor, "mount-changed", (GCallback)&GvfsMountManager::monitor_mount_changed, nullptr);
    g_signal_connect(m_gVolumeMonitor, "volume-added", (GCallback)&GvfsMountManager::monitor_volume_added, nullptr);
    g_signal_connect(m_gVolumeMonitor, "volume-removed", (GCallback)&GvfsMountManager::monitor_volume_removed, nullptr);
    g_signal_connect(m_gVolumeMonitor, "volume-changed", (GCallback)&GvfsMountManager::monitor_volume_changed, nullptr);
//    }else{
//        g_signal_connect (m_gVolumeMonitor, "mount-added", (GCallback)&GvfsMountManager::monitor_mount_added, nullptr);
//        g_signal_connect (m_gVolumeMonitor, "mount-removed", (GCallback)&GvfsMountManager::monitor_mount_removed, nullptr);
//        g_signal_connect (m_gVolumeMonitor, "mount-changed", (GCallback)&GvfsMountManager::monitor_mount_changed, nullptr);
//        g_signal_connect (m_gVolumeMonitor, "volume-added", (GCallback)&GvfsMountManager::monitor_volume_added, nullptr);
//        g_signal_connect (m_gVolumeMonitor, "volume-removed", (GCallback)&GvfsMountManager::monitor_volume_removed, nullptr);
//        g_signal_connect (m_gVolumeMonitor, "volume-changed", (GCallback)&GvfsMountManager::monitor_volume_changed, nullptr);
//    }
    connect(this, &GvfsMountManager::loadDiskInfoFinished, deviceListener, &UDiskListener::update);
}

GvfsMountManager *GvfsMountManager::instance()
{
    return gvfsMountManager;
}

QStringList GvfsMountManager::getIconNames(GThemedIcon *icon)
{
    QStringList iconNames;
    char **names;
    char **iter;
    names = nullptr;
    g_object_get(icon, "names", &names, nullptr);
    for (iter = names; *iter; iter++) {
        iconNames.append(QString(*iter));
    }
    g_strfreev(names);
    return iconNames;
}

QDrive GvfsMountManager::gDriveToqDrive(GDrive *drive)
{
    QDrive qDrive;
    char *name;
    char **ids;
    GIcon *icon;

    name = g_drive_get_name(drive);
    qDrive.setName(QString(name));
    g_free(name);

    ids = g_drive_enumerate_identifiers(drive);
    if (ids && ids[0] != nullptr) {
        for (int i = 0; ids[i] != nullptr; i++) {
            char *id = g_drive_get_identifier(drive,
                                              ids[i]);
            if (QString(ids[i]) == "unix-device") {
                qDrive.setUnix_device(QString(id));
            }
            g_free(id);
        }
    }
    g_strfreev(ids);

    qDrive.setHas_volumes(g_drive_has_volumes(drive));
    qDrive.setCan_eject(g_drive_can_eject(drive));
    qDrive.setCan_start(g_drive_can_start(drive));
    qDrive.setCan_start_degraded(g_drive_can_start_degraded(drive));
    qDrive.setCan_poll_for_media(g_drive_can_poll_for_media(drive));
    qDrive.setCan_stop(g_drive_can_stop(drive));
    qDrive.setIs_removable(g_drive_is_removable(drive));
    qDrive.setStart_stop_type(g_drive_get_start_stop_type(drive));
    qDrive.setHas_media(g_drive_has_media(drive));
    qDrive.setIs_media_check_automatic(g_drive_is_media_check_automatic(drive));
    qDrive.setIs_media_removable(g_drive_is_media_removable(drive));

    icon = g_drive_get_icon(drive);
    if (icon) {
        if (G_IS_THEMED_ICON(icon)) {
            qDrive.setIcons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref(icon);
    }

    icon = g_drive_get_symbolic_icon(drive);
    if (icon) {
        if (G_IS_THEMED_ICON(icon)) {
            qDrive.setSymbolic_icons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref(icon);
    }

    return qDrive;
}

QVolume GvfsMountManager::gVolumeToqVolume(GVolume *gvolume)
{
    if (!gvolume)
        return QVolume();

    QVolume qVolume;
    char *name;
    char **ids;
    GIcon *icon;
    GFile *activation_root;

    name = g_volume_get_name(gvolume);
    qVolume.setName(QString(name));
    g_free(name);

    ids = g_volume_enumerate_identifiers(gvolume);
    if (ids && ids[0] != nullptr) {
        for (int i = 0; ids[i] != nullptr; i++) {
            char *id = g_volume_get_identifier(gvolume, ids[i]);
            if (QString(ids[i]) == G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) {
                qVolume.setUnix_device(QString(id));
            } else if (QString(ids[i]) == G_VOLUME_IDENTIFIER_KIND_LABEL) {
                qVolume.setLable(QString(id));
            } else if (QString(ids[i]) == G_VOLUME_IDENTIFIER_KIND_UUID) {
                qVolume.setUuid(QString(id));
            } else if (QString(ids[i]) == G_VOLUME_IDENTIFIER_KIND_NFS_MOUNT) {
                qVolume.setNfs_mount(QString(id));
            }

            if (qVolume.unix_device().isEmpty()) {
                qVolume.setUnix_device(qVolume.uuid());
            }

            g_free(id);
        }
    }
    g_strfreev(ids);

    qVolume.setCan_mount(g_volume_can_mount(gvolume));
    qVolume.setCan_eject(g_volume_can_eject(gvolume));
    qVolume.setShould_automount(g_volume_should_automount(gvolume));


    icon = g_volume_get_icon(gvolume);
    if (icon) {
        if (G_IS_THEMED_ICON(icon)) {
            qVolume.setIcons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref(icon);
    }

    icon = g_volume_get_symbolic_icon(gvolume);
    if (icon) {
        if (G_IS_THEMED_ICON(icon)) {
            qVolume.setSymbolic_icons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref(icon);
    }

    GMount *mount = g_volume_get_mount(gvolume);
    if (mount) {
        qVolume.setIsMounted(true);
        GFile *root = g_mount_get_root(mount);
        char *uri = g_file_get_uri(root);
        qVolume.setMounted_root_uri(QString(uri));

        g_object_unref(root);
        g_free(uri);
    }

    activation_root = g_volume_get_activation_root(gvolume);
    if (activation_root != nullptr) {
        char *action_root_uri = g_file_get_uri(activation_root);
        qVolume.setActivation_root_uri(QString(action_root_uri));
        g_object_unref(activation_root);
        g_free(action_root_uri);
    }

    GDrive *gDrive = g_volume_get_drive(gvolume);
    if (gDrive) {
        QDrive qDrive = gDriveToqDrive(gDrive);
        qVolume.setDrive_unix_device(QString(g_drive_get_identifier(gDrive, "unix-device")));
        qVolume.setDrive(qDrive);
    }

    return qVolume;
}

QMount GvfsMountManager::gMountToqMount(GMount *gmount)
{
    if(!gmount)
        return QMount();

    QMount qMount;
    char *name, *uri;
    GFile *root, *default_location;
    GIcon *icon;

    name = g_mount_get_name(gmount);
    qMount.setName(QString(name));
    g_free(name);

    root = g_mount_get_root(gmount);
    uri = g_file_get_uri(root);
    qMount.setMounted_root_uri(QString(uri));
    g_object_unref(root);
    g_free(uri);

    default_location = g_mount_get_default_location(gmount);
    if (default_location) {
        char *loc_uri = g_file_get_uri(default_location);
        qMount.setDefault_location(QString(loc_uri));
        g_free(loc_uri);
        g_object_unref(default_location);
    }

    icon = g_mount_get_icon(gmount);
    if (icon) {
        if (G_IS_THEMED_ICON(icon)) {
            qMount.setIcons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref(icon);
    }

    icon = g_mount_get_symbolic_icon(gmount);
    if (icon) {
        if (G_IS_THEMED_ICON(icon)) {
            qMount.setSymbolic_icons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref(icon);
    }

    qMount.setCan_unmount(g_mount_can_unmount(gmount));
    qMount.setCan_eject(g_mount_can_eject(gmount));
    qMount.setIs_shadowed(g_mount_is_shadowed(gmount));

    return qMount;
}

QDiskInfo GvfsMountManager::qVolumeToqDiskInfo(const QVolume &volume)
{
    QDiskInfo diskInfo;
    diskInfo.setId(volume.unix_device());

    diskInfo.setName(volume.name());

    diskInfo.setDrive_unix_device(volume.drive_unix_device());
    diskInfo.setUnix_device(volume.unix_device());
    diskInfo.setUuid(volume.uuid());
    diskInfo.setActivation_root_uri(volume.activation_root_uri());
    diskInfo.setMounted_root_uri(volume.mounted_root_uri());

    if (volume.icons().count() > 0) {
        diskInfo.setIconName(volume.icons().at(0));
    }

    diskInfo.setCan_mount(volume.can_mount());
    diskInfo.setCan_eject(volume.can_eject());

    if (!volume.mounted_root_uri().isEmpty()) {
        diskInfo.setCan_unmount(true);
    }

    if (diskInfo.iconName() == "phone-apple-iphone") {
        diskInfo.setType("iphone");
    } else if (diskInfo.iconName() == "phone" || diskInfo.iconName() == "multimedia-player") {
        diskInfo.setType("phone");
    } else if (diskInfo.iconName() == "camera-photo" || diskInfo.iconName() == "camera") {
        diskInfo.setType("camera");
    } else if (diskInfo.can_eject() && (diskInfo.iconName() == "drive-harddisk-usb" || diskInfo.iconName() == "drive-removable-media-usb")) {
        diskInfo.setType("removable");
        diskInfo.setIs_removable(true);
    } else if (isDVD(volume)) {
        diskInfo.setType("dvd");
    } else {
        diskInfo.setType("native");
    }

    diskInfo.setHas_volume(true);

    diskInfo.updateGvfsFileSystemInfo();


    if (Drives.contains(volume.drive_unix_device())) {
        const QDrive &drive = Drives.value(volume.drive_unix_device());
        if (drive.is_removable()) {
            diskInfo.setType("removable");
            diskInfo.setIs_removable(true);
        }
    }

    return diskInfo;
}

QDiskInfo GvfsMountManager::qMountToqDiskinfo(const QMount &mount, bool updateUsage)
{
    QDiskInfo diskInfo;
    diskInfo.setId(mount.mounted_root_uri());

    diskInfo.setName(mount.name());
//    diskInfo.setType();
    diskInfo.setUuid(mount.uuid());
    diskInfo.setMounted_root_uri(mount.mounted_root_uri());

    if (mount.icons().count() > 0) {
        diskInfo.setIconName(mount.icons().at(0));
    }

    diskInfo.setCan_unmount(mount.can_unmount());
    diskInfo.setCan_eject(mount.can_eject());
    diskInfo.setDefault_location(mount.default_location());

    if (diskInfo.mounted_root_uri().startsWith("smb://")) {
        diskInfo.setType("smb");
    } else if (diskInfo.iconName() == "drive-optical" || diskInfo.iconName().startsWith("CD")) {
        diskInfo.setType("dvd");
    } else {
        diskInfo.setType("network");
    }
    if (updateUsage) // 该函数的具体作用是通过 Gio 获取文件系统的用量信息，获取该用量信息在一些场景下非常迟滞例如当移除 iPhone 设备时；但在移除设备时，用量信息是无需有的，因此这里添加参数控制是否需要更新用量。默认更新
        diskInfo.updateGvfsFileSystemInfo();
    return diskInfo;
}

QVolume GvfsMountManager::getVolumeByMountedRootUri(const QString &mounted_root_uri)
{
    foreach (const QVolume &volume, Volumes) {
        if (volume.isMounted() && volume.mounted_root_uri() == mounted_root_uri) {
            return volume;
        }
    }
    return QVolume();
}

QVolume GvfsMountManager::getVolumeByUnixDevice(const QString &unix_device)
{
    foreach (const QVolume &volume, Volumes) {
        if (volume.unix_device() == unix_device) {
            return volume;
        }
    }
    return QVolume();
}

void GvfsMountManager::monitor_mount_added_root(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_mount_added_root==============================";
    QMount qMount = gMountToqMount(mount);
    qCDebug(mountManager()) << qMount;

    QDiskInfo diskInfo = qMountToqDiskinfo(qMount);
    if (qMount.can_unmount())
        diskInfo.setCan_unmount(true);
    if (qMount.can_eject())
        diskInfo.setCan_eject(true);
    DiskInfos.insert(diskInfo.id(), diskInfo);
    emit gvfsMountManager->volume_added(diskInfo);
}

void GvfsMountManager::monitor_mount_removed_root(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_mount_removed_root==============================";
    QMount qMount = gMountToqMount(mount);
    qCDebug(mountManager()) << qMount;

    QDiskInfo diskInfo = qMountToqDiskinfo(qMount, false);
    DiskInfos.remove(diskInfo.id());
    emit gvfsMountManager->volume_removed(diskInfo);
}

void GvfsMountManager::monitor_mount_added(GVolumeMonitor *volume_monitor, GMount *gmount)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_mount_added==============================";
    QMount qMount = gMountToqMount(gmount);
    GVolume *gvolume = g_mount_get_volume(gmount);
    QVolume qVolume = gVolumeToqVolume(gvolume);

    //fix: 探测光盘推进,弹出和挂载状态机标识
    if (qMount.icons().contains("media-optical")) { //CD/DVD
        QString volTag = getVolTag(gvolume);
        DFMOpticalMediaWidget::g_mapCdStatusInfo[volTag].bMntFlag = true;
        DFMOpticalMediaWidget::g_mapCdStatusInfo[volTag].bVolFlag = true;

        // 对于光盘来说，执行挂载之后，一定是能够获取到 dp 对象的，在 actionMount 函数以及 setRootUrl 函数中，均请求了光盘的 dp 属性
        DISOMasterNS::DeviceProperty dp = ISOMaster->getDevicePropertyCached(qVolume.unix_device());
        if (dp.avail == 0) {
            QString mpt = qMount.mounted_root_uri();
            mpt.remove(FILE_ROOT);
            mpt = mpt.startsWith("/") ? mpt : ("/" + mpt);
            QStorageInfo info(mpt);
            if (info.isValid()) {
                quint64 total = static_cast<quint64>(info.bytesTotal());
                DFMOpticalMediaWidget::g_mapCdStatusInfo[volTag].nTotal = total;
                DFMOpticalMediaWidget::g_mapCdStatusInfo[volTag].nUsage = total;
            }
        } else {
            DFMOpticalMediaWidget::g_mapCdStatusInfo[volTag].nTotal = dp.avail + dp.data;
            DFMOpticalMediaWidget::g_mapCdStatusInfo[volTag].nUsage = dp.data;
        }
    }

    qCDebug(mountManager()) << "===================" << qMount.mounted_root_uri() << gvolume << "=======================";
    qCDebug(mountManager()) << "===================" << qMount << "=======================";
    if (gvolume != nullptr) {
        QVolume qVolume = gVolumeToqVolume(gvolume);
        Volumes.insert(qVolume.unix_device(), qVolume);

        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);
        if (qMount.can_unmount()) {
            diskInfo.setCan_unmount(true);
        }
        if (qMount.can_eject()) {
            diskInfo.setCan_eject(true);
        }

        DiskInfos.insert(diskInfo.id(), diskInfo);
        emit gvfsMountManager->mount_added(diskInfo);
        qCDebug(mountManager()) << "========mount_added===========" << diskInfo;
    } else {
        // ignore afc first mounted event
        if (isIgnoreUnusedMounts(qMount)) {
            return;
        }

        if (!NoVolumes_Mounts_Keys.contains(qMount.mounted_root_uri())) {
            NoVolumes_Mounts_Keys.append(qMount.mounted_root_uri());

            QDiskInfo diskInfo = qMountToqDiskinfo(qMount);
            DiskInfos.insert(diskInfo.id(), diskInfo);
            emit gvfsMountManager->mount_added(diskInfo);
        }
    }

    // when remote connection is mounted, reload computer model.
    if (qMount.mounted_root_uri().startsWith("smb://")) {
        QScopedPointer<DGioFile> file(DGioFile::createFromUri(qMount.mounted_root_uri()));
        RemoteMountsStashManager::stashRemoteMount(file->path(), qMount.name());
    }

    Mounts.insert(qMount.mounted_root_uri(), qMount);
}

void GvfsMountManager::monitor_mount_removed(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_mount_removed==============================" ;
    QMount qMount = gMountToqMount(mount);

    //fix: 探测光盘推进,弹出和挂载状态机标识
    if (qMount.name().contains("CD/DVD") || qMount.name().contains("CD") || qMount.icons().contains("media-optical")) { //CD/DVD
        DFMOpticalMediaWidget::g_mapCdStatusInfo[getVolTag(mount)].bMntFlag = false;
    }

    qCDebug(mountManager()) << "===================" << qMount.mounted_root_uri() << "=======================";

    QVolume volume = getVolumeByMountedRootUri(qMount.mounted_root_uri());
    qCDebug(mountManager()) << volume.isValid() << volume;
    if (volume.isValid()) {
        volume.setIsMounted(false);
        volume.setMounted_root_uri("");
        Volumes.insert(volume.unix_device(), volume);
    } else {
        NoVolumes_Mounts_Keys.removeOne(qMount.mounted_root_uri());
    }

    bool removed = Mounts.remove(qMount.mounted_root_uri());
    if (removed) {

        if (volume.isValid()) {
            QDiskInfo diskInfo = qVolumeToqDiskInfo(volume);
            DiskInfos.insert(diskInfo.id(), diskInfo);
            diskInfo.setHas_volume(true);
            emit gvfsMountManager->mount_removed(diskInfo);
        } else {
            QDiskInfo diskInfo = qMountToqDiskinfo(qMount, false); // 在卸载设备的时候无需关心设备的用量信息
            bool diskInfoRemoved = DiskInfos.remove(diskInfo.id());
            if (diskInfoRemoved) {
                diskInfo.setHas_volume(false);
                emit gvfsMountManager->mount_removed(diskInfo);
            }
        }

        // 触发关闭标签的信号
        GFile *root = g_mount_get_root(mount);
        char *path = g_file_get_path(root);
        DUrl durl = DUrl::fromLocalFile(path);
        g_free(path);
        g_object_unref(root);
        emit fileSignalManager->requestCloseTab(durl);

        if (qMount.mounted_root_uri().startsWith("smb://")) {
            if (DFMApplication::genericAttribute(DFMApplication::GA_AlwaysShowOfflineRemoteConnections).toBool())
                emit DFMApplication::instance()->reloadComputerModel();
        }
    }
}

void GvfsMountManager::monitor_mount_changed(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    Q_UNUSED(mount)
//    qDebug() << "==============================monitor_mount_changed==============================" ;
    GVolume *volume = g_mount_get_volume(mount);
    if (volume != nullptr) {
//        qDebug() << "==============================changed removed==============================" ;

        QVolume qVolume = gVolumeToqVolume(volume);
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);

        bool isDVDChanged = isDVD(qVolume);
        if (isDVDChanged) {
            diskInfo.setType("dvd");
            qCDebug(mountManager()) << diskInfo;
            if (diskInfo.can_unmount()) {
                diskInfo.updateGvfsFileSystemInfo();
                emit gvfsMountManager->volume_changed(diskInfo);
            }
        }
    } else {
//        qDebug() << "==============================changed volume empty==============================" ;
    }
}

void GvfsMountManager::monitor_volume_added(GVolumeMonitor *volume_monitor, GVolume *volume)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_volume_added==============================" ;
    QVolume qVolume = gVolumeToqVolume(volume);

    qCDebug(mountManager()) << "===================" << qVolume.unix_device() << "=======================";

    //fix: 每次弹出光驱时需要删除临时缓存数据文件
    //id="/dev/sr1" -> tempId="sr1"
    GvfsMountManager::g_qVolumeId = qVolume.unix_device().mid(5);

    //fix: 探测光盘推进,弹出和挂载状态机标识
    if (qVolume.icons().contains("media-optical")) { //CD/DVD
        DFMOpticalMediaWidget::g_mapCdStatusInfo[getVolTag(volume)].bMntFlag = false;
        DFMOpticalMediaWidget::g_mapCdStatusInfo[getVolTag(volume)].bVolFlag = true;
        DFMOpticalMediaWidget::g_mapCdStatusInfo[getVolTag(volume)].bBurningOrErasing = false;
        //fix: 设置光盘容量属性
        DFMOpticalMediaWidget::setBurnCapacity(DFMOpticalMediaWidget::BCSA_BurnCapacityStatusAdd, getVolTag(volume));
    }

    GDrive *drive = g_volume_get_drive(volume);
    if (drive) {
        QDrive qDrive = gDriveToqDrive(drive);

        if (!qDrive.unix_device().isEmpty()) {
            Drives.insert(qDrive.unix_device(), qDrive);
            if (!Volumes_Drive_Keys.contains(qDrive.unix_device())) {
                Volumes_Drive_Keys.append(qDrive.unix_device());
            }
        }

        if (drive != nullptr) {
            qVolume.setDrive_unix_device(QString(g_drive_get_identifier(drive, "unix-device")));
        }
    }
    Volumes.insert(qVolume.unix_device(), qVolume);

    QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);

    if (diskInfo.type() == "iphone") {
        QRegularExpression express(QString("afc://%1(:[\\d]+)?/").arg(diskInfo.uuid()));
        auto match = express.match(diskInfo.activation_root_uri());

        if (!match.isValid()) {
            return;
        }
    }

    DiskInfos.insert(diskInfo.id(), diskInfo);
    emit gvfsMountManager->volume_added(diskInfo);
}

void GvfsMountManager::monitor_volume_removed(GVolumeMonitor *volume_monitor, GVolume *volume)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_volume_removed==============================" ;
    QVolume qVolume = gVolumeToqVolume(volume);

    qCDebug(mountManager()) << "===================" << qVolume.unix_device() << "=======================";

    //fix: 探测光盘推进,弹出和挂载状态机标识
    if (qVolume.name().contains("CD/DVD") || qVolume.name().contains("CD") || qVolume.icons().contains("media-optical")) { //CD/DVD
        DFMOpticalMediaWidget::g_mapCdStatusInfo[getVolTag(volume)].bVolFlag = false;
       // DFMOpticalMediaWidget::g_mapCdStatusInfo[getVolTag(volume)].bBurningOrErasing = false;
    }

    //fix: 每次弹出光驱时需要删除临时缓存数据文件
    if ((qVolume.activation_root_uri().contains("burn:///") && qVolume.unix_device().contains("")) || \
            (qVolume.activation_root_uri().contains("") || qVolume.unix_device().contains("/dev/sr"))) {
        //fix: 设置光盘容量属性
        DFMOpticalMediaWidget::g_mapCdStatusInfo[getVolTag(volume)].nTotal = 0;
        DFMOpticalMediaWidget::g_mapCdStatusInfo[getVolTag(volume)].nUsage = 0;;
        DFMOpticalMediaWidget::setBurnCapacity(DFMOpticalMediaWidget::BCSA_BurnCapacityStatusEjct, getVolTag(volume));
        emit fileSignalManager->requestUpdateComputerView();

        const static QString stagePrefix = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/"
                + qApp->organizationName() + "/" DISCBURN_STAGING "/";
        clearStageDir(stagePrefix + qVolume.unix_device().replace("/", "_"));
    }

    GDrive *drive = g_volume_get_drive(volume);
    if (drive) {
        QDrive qDrive = gDriveToqDrive(drive);

        if (!qDrive.unix_device().isEmpty()) {
            Drives.insert(qDrive.unix_device(), qDrive);
            Volumes_Drive_Keys.removeOne(qDrive.unix_device());
        }
    }

    bool removed = Volumes.remove(qVolume.unix_device());

    qCDebug(mountManager()) << removed << qVolume << qVolumeToqDiskInfo(qVolume);
    if (removed) {
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);
        qCDebug(mountManager()) << diskInfo;
        bool diskInfoRemoved = DiskInfos.remove(diskInfo.id());
        if (diskInfoRemoved) {
            emit gvfsMountManager->volume_removed(diskInfo);
        }
    } else {
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);
        emit gvfsMountManager->volume_removed(diskInfo);
    }
}

void GvfsMountManager::monitor_volume_changed(GVolumeMonitor *volume_monitor, GVolume *volume)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_volume_changed==============================" ;

    if (volume != nullptr) {
        qCDebug(mountManager()) << "==============================volume changed==============================" ;

        QVolume qVolume = gVolumeToqVolume(volume);
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);
        DiskInfos.insert(diskInfo.id(), diskInfo);
        qCDebug(mountManager()) << diskInfo;
        emit gvfsMountManager->volume_changed(diskInfo);
    } else {
        qCDebug(mountManager()) << "==============================changed volume empty==============================" ;
    }
}

GMountOperation *GvfsMountManager::new_mount_op(bool isDisk = true)
{
    GMountOperation *op;

    op = g_mount_operation_new();

    g_signal_connect(op, "ask_question", G_CALLBACK(ask_question_cb), NULL);
    g_signal_connect(op, "ask_password",
                     isDisk ? G_CALLBACK(ask_disk_password_cb) : G_CALLBACK(ask_password_cb),
                     nullptr);

    /* TODO: we *should* also connect to the "aborted" signal but since the
     *       main thread is blocked handling input we won't get that signal
     *       anyway...
     */

    return op;
}

static int requestAnswerDialog(WId parentWindowId, const QString &message, QStringList choices)
{
    DDialog askQuestionDialog(WindowManager::getWindowById(parentWindowId));

    askQuestionDialog.setMessage(message);
    askQuestionDialog.addButtons(choices);
    askQuestionDialog.setMaximumWidth(480);

    return askQuestionDialog.exec();
}

// blumia: This callback is mainly for sftp fingerprint identity dialog, but should works on any ask-question signal.
//         ref: https://www.freedesktop.org/software/gstreamer-sdk/data/docs/latest/gio/GMountOperation.html#GMountOperation-ask-question
void GvfsMountManager::ask_question_cb(GMountOperation *op, const char *message, const GStrv choices)
{
    char **ptr = choices;
    int choice;
    QStringList choiceList;

    QString oneMessage(message);
    qCDebug(mountManager()) << "ask_question_cb() message: " << message;

    while (*ptr) {
        QString oneOption = QString::asprintf("%s", *ptr++);
        qCDebug(mountManager()) << "ask_question_cb()  - option(s): " << oneOption;
        choiceList << oneOption;
    }

    QString newmsg = oneMessage;
    if (oneMessage.startsWith("Can’t verify the identity of") &&
            oneMessage.endsWith("If you want to be absolutely sure it is safe to continue, contact the system administrator.")) {
        QString arg1, arg2;
        QRegularExpression ovrex("“.*?”");
        auto ovrxm = ovrex.match(oneMessage);
        if (ovrxm.hasMatch()) {
            arg1 = ovrxm.captured(0);
            newmsg = newmsg.replace(arg1, "");
            auto ovr_xm = ovrex.match(newmsg);
            arg2 = ovr_xm.captured(0);
            // 修复文管文案问题
            newmsg = tr("Can’t verify the identity of %1.").arg(arg1) + '\n' +
                     tr("This happens when you log in to a computer the first time.") + '\n' +
                     tr("The identity sent by the remote computer is") + '\n' +
                     arg2 + '\n' +
                     tr("If you want to be absolutely sure it is safe to continue, contact the system administrator.");
        }
        newmsg = newmsg.replace("\\r\\n", "\n");
        qDebug() << newmsg;
    }

    if (MountEventHash.contains(op) && MountEventHash.value(op)) {
            choice = DThreadUtil::runInMainThread(requestAnswerDialog, MountEventHash.value(op)->windowId(), newmsg, choiceList);
        } else {
            choice = -1;
            qWarning() << "MountEventHash: bad choice.";
        }    qCDebug(mountManager()) << "ask_question_cb() user choice(start at 0): " << choice;

    // check if choose is invalid
    if (choice < 0 || choice >= choiceList.count()) {
        g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    g_mount_operation_set_choice(op, choice);
    g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);

    return;
}

static QJsonObject requestPasswordDialog(WId parentWindowId, bool showDomainLine, const QJsonObject &data,GMountOperation *op)
{
    //fix 22749 修复输入秘密错误了后，2到3次才弹提示框
    if (!GvfsMountManager::instance()->askPasswordDialogHash.contains(op)) {
        GvfsMountManager::instance()->askPasswordDialogHash.insert(op, new MountAskPasswordDialog(WindowManager::getWindowById(parentWindowId)));
    }
    GvfsMountManager::instance()->askPasswordDialogHash.value(op)->setLoginData(data);
    GvfsMountManager::instance()->askPasswordDialogHash.value(op)->setDomainLineVisible(showDomainLine);

    int ret = GvfsMountManager::instance()->askPasswordDialogHash.value(op)->exec();

    if (ret == DDialog::Accepted) {
        return GvfsMountManager::instance()->askPasswordDialogHash.value(op)->getLoginData();
    }

    return QJsonObject();
}

void GvfsMountManager::ask_password_cb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags)
{
    if (MountTimerHash.contains(op))
        MountTimerHash.value(op)->stop();
    //fix 22749 修复输入秘密错误了后，2到3次才弹提示框
    if (askPasswordDialogHash.value(op)) {
        askPasswordDialogHash.value(op)->deleteLater();
        askPasswordDialogHash.remove(op);
        if (AskingPasswordHash.value(op))
            DThreadUtil::runInMainThread(dialogManager, &DialogManager::showErrorDialog,
                                        tr("Mounting device error"), tr("Wrong username or password"));
        //fix bug 63796,是sftp和ftp时，后面不会弹窗，所以这里要弹提示
        if (!AskingPasswordHash.value(op) && MountEventHash.value(op) && MountEventHash.contains(op) && MountEventHash.value(op)->fileUrl().scheme() != SMB_SCHEME)
            DThreadUtil::runInMainThread(dialogManager, &DialogManager::showErrorDialog,
                                         tr("Mounting device error"), QString());
        return;
    }

    bool anonymous = g_mount_operation_get_anonymous(op);
    GPasswordSave passwordSave = g_mount_operation_get_password_save(op);

    const char *default_password = g_mount_operation_get_password(op);

    qCDebug(mountManager()) << "anonymous" << anonymous;
    qCDebug(mountManager()) << "message" << message;
    qCDebug(mountManager()) << "username" << default_user;
    qCDebug(mountManager()) << "domain" << default_domain;
    qCDebug(mountManager()) << "password" << default_password;
    qCDebug(mountManager()) << "GAskPasswordFlags" << flags;
    qCDebug(mountManager()) << "passwordSave" << passwordSave;

    QJsonObject obj;
    obj.insert("message", message);
    obj.insert("anonymous", anonymous);
    obj.insert("username", default_user);
    obj.insert("domain", default_domain);
    obj.insert("password", default_password);
    obj.insert("GAskPasswordFlags", flags);
    obj.insert("passwordSave", passwordSave);
    QJsonObject loginObj;
    if (MountTimerHash.contains(op) && MountEventHash.value(op)) {
        loginObj = DThreadUtil::runInMainThread(requestPasswordDialog,
                                                MountEventHash.value(op)->windowId(),
                                                MountEventHash.value(op)->fileUrl().isSMBFile(), obj, op);
    }

    if (!loginObj.isEmpty()) {
        anonymous = loginObj.value("anonymous").toBool();
        QString username = loginObj.value("username").toString();
        QString domain = loginObj.value("domain").toString();
        QString password = loginObj.value("password").toString();
        GPasswordSave passwordsaveFlag =  static_cast<GPasswordSave>(loginObj.value("passwordSave").toInt());

        SMBLoginObjHash.insert(op,new QJsonObject(loginObj));

        if ((flags & G_ASK_PASSWORD_ANONYMOUS_SUPPORTED) && anonymous) {
            g_mount_operation_set_anonymous(op, TRUE);
        } else {
            if (flags & G_ASK_PASSWORD_NEED_USERNAME) {
                g_mount_operation_set_username(op, username.toStdString().c_str());
            }

            if (flags & G_ASK_PASSWORD_NEED_DOMAIN) {
                g_mount_operation_set_domain(op, domain.toStdString().c_str());
            }

            if (flags & G_ASK_PASSWORD_NEED_PASSWORD) {
                g_mount_operation_set_password(op, password.toStdString().c_str());
            }

            if (flags & G_ASK_PASSWORD_SAVING_SUPPORTED) {
                g_mount_operation_set_password_save(op, passwordsaveFlag);
            }
        }

        /* Only try anonymous access once. */
        if (anonymous &&
                GPOINTER_TO_INT(g_object_get_data(G_OBJECT(op), "state")) == MOUNT_OP_ASKED) {
            g_object_set_data(G_OBJECT(op), "state", GINT_TO_POINTER(MOUNT_OP_ABORTED));
            g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
        } else {
            qCDebug(mountManager()) << "g_mount_operation_reply before";
//            g_object_set_data (G_OBJECT (op), "state", GINT_TO_POINTER (MOUNT_OP_ASKED));
            g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
            qCDebug(mountManager()) << "g_mount_operation_reply end";
        }
        AskingPasswordHash.insert(op,!anonymous);
        qCDebug(mountManager()) << "AskingPassword" << AskingPasswordHash.value(op);

    } else {
        qCDebug(mountManager()) << "cancel connect";
        AskingPasswordHash.insert(op,false);
        g_object_set_data(G_OBJECT(op), "state", GINT_TO_POINTER(MOUNT_OP_ABORTED));
        g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
    }
    if (MountTimerHash.contains(op))
        MountTimerHash.value(op)->start();
}

void GvfsMountManager::ask_disk_password_cb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags)
{
    if (mountSecretDiskAskPasswordDialog) {
        return;
    }

    AskedPasswordWhileMountDisk = true;

    g_print("%s\n", message);

    bool anonymous = g_mount_operation_get_anonymous(op);
    GPasswordSave passwordSave = g_mount_operation_get_password_save(op);

    const char *default_password = g_mount_operation_get_password(op);

    qCDebug(mountManager()) << "anonymous" << anonymous;
    qCDebug(mountManager()) << "message" << message;
    qCDebug(mountManager()) << "username" << default_user;
    qCDebug(mountManager()) << "domain" << default_domain;
    qCDebug(mountManager()) << "password" << default_password;
    qCDebug(mountManager()) << "GAskPasswordFlags" << flags;
    qCDebug(mountManager()) << "passwordSave" << passwordSave;

    if (flags & G_ASK_PASSWORD_NEED_USERNAME) {
        g_mount_operation_set_username(op, default_user);
    }

    if (flags & G_ASK_PASSWORD_NEED_DOMAIN) {
        g_mount_operation_set_domain(op, default_domain);
    }

    if (flags & G_ASK_PASSWORD_NEED_PASSWORD) {
        QString tipMessage;
        QString m(message);
        QStringList messageList = m.split("\n");
        if (messageList.count() >= 2) {
            tipMessage = messageList.at(1);
        }
        mountSecretDiskAskPasswordDialog = new MountSecretDiskAskPasswordDialog(tipMessage);
        int code = mountSecretDiskAskPasswordDialog->exec();
        QString p = mountSecretDiskAskPasswordDialog->password();
        if (code == 0) {
            p.clear();
        }
        qCDebug(mountManager()) << "password is:" << p;
        std::string pstd = p.toStdString();
        g_mount_operation_set_password(op, pstd.c_str());
        mountSecretDiskAskPasswordDialog->deleteLater();
        mountSecretDiskAskPasswordDialog = nullptr;
    }

    g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
}

void GvfsMountManager::printVolumeMounts()
{
    qDebug() << QThread::currentThread() << qApp->thread();
    qDebug() << "==============================";
    qDebug() << "Volumes:   " << Volumes;
    qDebug() << "Mounts:   " << Mounts;
    qDebug() << "==============================";
    qDebug() << "Volumes_Drive_Keys" << Volumes_Drive_Keys;
    qDebug() << "Volumes_No_Drive_Keys" << Volumes_No_Drive_Keys;
    qDebug() << "NoVolumes_Mounts_Keys:   " << NoVolumes_Mounts_Keys;
    qDebug() << "==============================";
}

QDiskInfo GvfsMountManager::getDiskInfo(const QString &path, bool bupdate)
{
    QDiskInfo info;
    foreach (const QDiskInfo &diskInfo, DiskInfos.values()) {
        if (!path.isEmpty() && path == diskInfo.id()) {
            info = diskInfo;
            break;
        } else if (!path.isEmpty() && diskInfo.mounted_root_uri() == path) {
            info = diskInfo;
            break;
        } else if (!path.isEmpty() && path == diskInfo.unix_device()) {
            info = diskInfo;
            break;
        } else if (!path.isEmpty() && path == diskInfo.uuid()) {
            info = diskInfo;
            break;
        }
    }
    if (!info.isValid()) {
        qDebug() << "获取磁盘信息失败";
        dialogManager->showFormatDialog(path);
    }
    if (bupdate) {
        info.updateGvfsFileSystemInfo();
    }
    return info;
}

bool GvfsMountManager::isDVD(const QVolume &volume)
{
    if (volume.drive().isValid() && volume.unix_device().startsWith("/dev/sr")) {
        return true;
    }
    return false;
}

bool GvfsMountManager::isIgnoreUnusedMounts(const QMount &mount)
{
    /*the following protocol has two mounts event, ignore unused one*/
    if (mount.mounted_root_uri().startsWith("mtp://") || mount.mounted_root_uri().startsWith("gphoto2://")) {
        return true;
    }
    return false;
}

QString GvfsMountManager::getDriveUnixDevice(const QString &unix_device)
{
    QString drive_unix_device;
    if (gvfsMountManager->DiskInfos.contains(unix_device)) {
        drive_unix_device = gvfsMountManager->DiskInfos.value(unix_device).drive_unix_device();
    }
    return drive_unix_device;
}

bool GvfsMountManager::isDeviceCrypto_LUKS(const QDiskInfo &diskInfo)
{
    if (diskInfo.can_mount()) {
        QStringList &&udiskspaths = DDiskManager::resolveDeviceNode(diskInfo.unix_device(), {});
        if (udiskspaths.isEmpty()) return false;
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(udiskspaths.first()));
        QString fstype = blk->idType();
        if (fstype == "crypto_LUKS") {
            return true;
        }
    }
    return false;
}

void GvfsMountManager::startMonitor()
{
    if (DFMGlobal::isRootUser()) {
        listMountsBylsblk();
    }
    listVolumes();
    listMounts();
    listDrives();
    updateDiskInfos(); //磁盘信息root用户也需要刷新,否则会出现root用户只能挂载不能卸载的情况
#ifdef DFM_MINIMUM
    qDebug() << "Don't auto mount disk";
#else
//    if (qApp->applicationName() == QMAKE_TARGET && !DFMGlobal::IsFileManagerDiloagProcess){
//        TIMER_SINGLESHOT_OBJECT(this, 1000, {
//                                    this->autoMountAllDisks();
//                                }, this)
//    }
#endif
    initConnect();
    emit loadDiskInfoFinished();
}

void GvfsMountManager::listDrives()
{
    GList *drives = g_volume_monitor_get_connected_drives(m_gVolumeMonitor);

    GList *volumes, *d, *v;
    char **ids;
    int c, j, k;
    for (c = 0, d = drives; d != nullptr; d = d->next, c++) {
        GDrive *drive = static_cast<GDrive *>(d->data);
        QDrive qDrive = gDriveToqDrive(drive);

        if (!qDrive.unix_device().isEmpty()) {
            Drives.insert(qDrive.unix_device(), qDrive);
            Drives_Keys.append(qDrive.unix_device());
        }

        volumes = g_drive_get_volumes(drive);
        for (j = 0, v = volumes; v != nullptr; v = v->next, j++) {
            GVolume *volume = static_cast<GVolume *>(v->data);

            ids = g_volume_enumerate_identifiers(volume);
            if (ids && ids[0] != nullptr) {
                for (k = 0; ids[k] != nullptr; k++) {
                    char *id = g_volume_get_identifier(volume,
                                                       ids[k]);
                    if (QString(ids[k]) == "unix-device") {
                        Volumes_Drive_Keys.append(QString(id));
                    }
                    g_free(id);
                }
            }
            g_strfreev(ids);
        }
        g_list_free_full(volumes, g_object_unref);
    }

    g_list_free_full(drives, g_object_unref);
}

void GvfsMountManager::listVolumes()
{
    GList *volumes = g_volume_monitor_get_volumes(m_gVolumeMonitor);

    GList *v;
    int c;
    for (c = 0, v = volumes; v != nullptr; v = v->next, c++) {
        GVolume *volume = static_cast<GVolume *>(v->data);
        QVolume qVolume = gVolumeToqVolume(volume);
        GDrive *drive = g_volume_get_drive(volume);
        if (drive != nullptr) {
            qVolume.setDrive_unix_device(QString(g_drive_get_identifier(drive, "unix-device")));
        } else {
            if (!Volumes_No_Drive_Keys.contains(qVolume.unix_device())) {
                Volumes_No_Drive_Keys.append(qVolume.unix_device());
            }
        }
        Volumes.insert(qVolume.unix_device(), qVolume);
    }

    g_list_free_full(volumes, g_object_unref);
}

void GvfsMountManager::listMounts()
{
    GList *mounts = g_volume_monitor_get_mounts(m_gVolumeMonitor);

    GList *m;
    int c;
    for (c = 0, m = mounts; m != nullptr; m = m->next, c++) {
        GMount *mount = static_cast<GMount *>(m->data);
        QMount qMount = gMountToqMount(mount);
        Mounts.insert(qMount.mounted_root_uri(), qMount);
        GVolume *volume = g_mount_get_volume(mount);
        if (volume != nullptr) {
            continue;
        } else {
            if (isIgnoreUnusedMounts(qMount)) {
                continue;
            }
        }
        NoVolumes_Mounts_Keys.append(qMount.mounted_root_uri());
    }

    g_list_free_full(mounts, g_object_unref);
}

void GvfsMountManager::updateDiskInfos()
{
    Volumes_Drive_Keys.sort();
    foreach (QString key, Volumes_Drive_Keys) {
        if (Volumes.contains(key)) {
            QVolume volume = Volumes.value(key);
            QDiskInfo diskInfo = qVolumeToqDiskInfo(volume);
            DiskInfos.insert(diskInfo.id(), diskInfo);
            qCDebug(mountManager()) << diskInfo;
        }
    }
    Volumes_No_Drive_Keys.sort();
    foreach (QString key, Volumes_No_Drive_Keys) {
        if (Volumes.contains(key)) {
            QVolume volume = Volumes.value(key);
            QDiskInfo diskInfo = qVolumeToqDiskInfo(volume);
            if (diskInfo.type() == "iphone") {
                if (diskInfo.activation_root_uri() != QString("afc://%1/").arg(diskInfo.uuid())) {
                    continue;
                }
            }
            DiskInfos.insert(diskInfo.id(), diskInfo);
            qCDebug(mountManager()) << diskInfo;
        }
    }
    NoVolumes_Mounts_Keys.sort();
    foreach (QString key, NoVolumes_Mounts_Keys) {
        if (Mounts.contains(key)) {
            QMount mount = Mounts.value(key);
            QDiskInfo diskInfo = qMountToqDiskinfo(mount);
            DiskInfos.insert(diskInfo.id(), diskInfo);
            qCDebug(mountManager()) << diskInfo;
        }
    }
    qCDebug(mountManager()) << Mounts;
}

void GvfsMountManager::listMountsBylsblk()
{
    QString output;
    QString err;
    QProcess p;
    p.setProgram("lsblk");
    p.setArguments({"-OJlb"});
    p.start();
    p.waitForFinished(-1);
    output = p.readAllStandardOutput();
    err = p.readAllStandardError();
    bool status = p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0;
    if (status) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(output.toLocal8Bit(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject devObj = doc.object();
            foreach (QString key, devObj.keys()) {
                if (key == "blockdevices") {
                    QJsonArray objArray = devObj.value(key).toArray();
                    for (int i = 0; i < objArray.count(); i++) {
                        QJsonObject obj = objArray.at(i).toObject();
                        QDiskInfo diskInfo;

                        if (obj.contains("mountpoint")) {
                            QString mountPoint = obj.value("mountpoint").toString();
                            if (mountPoint.isEmpty() || mountPoint == "/") {
                                continue;
                            } else {
                                diskInfo.setMounted_root_uri(QString("file://%1").arg(obj.value("mountpoint").toString()));
                            }
                        }

                        if (obj.contains("name")) {
                            diskInfo.setName(obj.value("name").toString());
                        }
                        if (obj.contains("path")) {
                            diskInfo.setUnix_device(obj.value("path").toString());
                            diskInfo.setId(diskInfo.unix_device());
                        }
                        if (obj.contains("fstype")) {
                            diskInfo.setId_filesystem(obj.value("fstype").toString());
                        }
                        if (obj.contains("label")) {
                            //diskInfo.setName(obj.value("label").toString());
                        }
                        if (obj.contains("uuid")) {
                            diskInfo.setUuid(obj.value("uuid").toString());
                        }
                        if (obj.contains("rm")) {
                            QVariant data(obj.value("rm").toVariant());
                            diskInfo.setIs_removable(data.toBool());
                        }
                        if (obj.contains("fsavail") && obj.value("fsavail").type() != QJsonValue::Type::Null) {
                            diskInfo.setFree(obj.value("fsavail").toVariant().toULongLong());
                        }
                        if (obj.contains("fssize") && obj.value("fssize").type() != QJsonValue::Type::Null) {
                            diskInfo.setTotal(obj.value("fssize").toVariant().toULongLong());
                        } else if (obj.contains("size") && obj.value("size").type() != QJsonValue::Type::Null) {
                            diskInfo.setTotal(obj.value("size").toVariant().toULongLong());
                        }

                        diskInfo.setCan_unmount(true);
                        diskInfo.setCan_mount(false);
                        diskInfo.setCan_eject(false);
                        if (diskInfo.is_removable()) {
                            diskInfo.setType("removable");
                        } else {
                            diskInfo.setType("native");
                        }
                        Lsblk_Keys.append(diskInfo.unix_device());
                        DiskInfos.insert(diskInfo.id(), diskInfo);
                    }
                }
            }
        } else {
            qCDebug(mountManager()) << error.errorString();
        }
    } else {
        qCDebug(mountManager()) << status << output << err;
    }
}

bool GvfsMountManager::errorCodeNeedSilent(int errorCode)
{
    switch (errorCode) {
    case G_IO_ERROR_NOT_SUPPORTED:
    case G_IO_ERROR_DBUS_ERROR: // to avoid: An operation is already pending.
        return true;
    default:
        break;
    }

    return false;
}

/*
 * get real mount url from  mounted_root_uri of info
 * smb://10.0.12.150/share -> file:///run/user/1000/gvfs/smb-share:server=10.0.12.150,share=share
*/
DUrl GvfsMountManager::getRealMountUrl(const QDiskInfo &info)
{
    QString path = QString("/run/user/%1/gvfs").arg(getuid());

    QString mounted_root_uri = info.mounted_root_uri();
    DUrl MountPointUrl = DUrl(mounted_root_uri);

    // blumia: It's possible that the device is not managed by gvfs so the real mount url won't be under
    //         gvfs mount path, try `sshfs` mount something...
    // blumia: We assume `mounted_root_uri()` can get the correct mount path, and only return it directly
    //         if it is a local file path.
    if (MountPointUrl.isLocalFile()) {
        return MountPointUrl;
    }

    if (mounted_root_uri == "/") {
        MountPointUrl = DUrl::fromLocalFile("/");
    } else {
        if ((info.type() != "native" &&
                info.type() != "removable" &&
                info.type() != "dvd") && !info.id_filesystem().isEmpty()) {
            if (info.type() == "network") {
                MountPointUrl = DUrl::fromLocalFile(QString("%1/%2%3").arg(path, info.id_filesystem(), DUrl(info.default_location()).path()));
            } else {
                MountPointUrl = DUrl::fromLocalFile(QString("%1/%2").arg(path, info.id_filesystem()));
            }
        }
    }

    return MountPointUrl;
}

QString GvfsMountManager::getVolTag(GMount *m)
{
    if (!m) return QString();
    QMount qMount = gMountToqMount(m);
    QVolume volume = getVolumeByMountedRootUri(qMount.mounted_root_uri());
    return volume.unix_device().mid(5);
}

QString GvfsMountManager::getVolTag(GVolume *v)
{
    if (!v) return QString();
    QVolume qVolume = gVolumeToqVolume(v);
    return qVolume.unix_device().mid(5);
}

void GvfsMountManager::cancellMountSync(GMountOperation *op)
{
    if (CancellHash.contains(op)) {
        GCancellable * cancell = CancellHash.value(op);
        CancellHash.remove(op);
        if (cancell) {
            g_cancellable_cancel(cancell);
            //84859 g_cancellable_get_fd或g_cancellable_make_pollfd 后才能调用g_cancellable_release_fd
            //g_cancellable_release_fd(cancell);
            g_object_unref(cancell);
        }
    }
}

void GvfsMountManager::autoMountAllDisks()
{
    // check if we are in live system, don't do auto mount if we are in live system.
    static QMap<QString, QString> cmdline = FileUtils::getKernelParameters();
    if (cmdline.value("boot", "") == QStringLiteral("live")) {
        return;
    }

    if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_AutoMount).toBool()) {
        foreach (const QDiskInfo &diskInfo, DiskInfos.values()) {
            if (diskInfo.can_mount()) {
                if (isDeviceCrypto_LUKS(diskInfo))
                    continue;
                mount(diskInfo, true);
            }
        }
    }
}

void GvfsMountManager::mount(const QString &path, bool silent)
{
    QDiskInfo diskInfo = getDiskInfo(path);
    mount(diskInfo, silent);
}

void GvfsMountManager::mount(const QDiskInfo &diskInfo, bool silent)
{
    if (!diskInfo.mounted_root_uri().isEmpty()) {
        mount_mounted(diskInfo.mounted_root_uri(), silent);
    } else if (!diskInfo.activation_root_uri().isEmpty()) {
        mount_mounted(diskInfo.activation_root_uri(), silent);
    } else {
        mount_device(diskInfo.unix_device(), silent);
    }
}

MountStatus GvfsMountManager::mount_sync(const DFMUrlBaseEvent &event)
{
    DUrl fileUrl = event.fileUrl();
    // fix bug 100864 获取真实的网络挂载点
    static const QRegExp rxPath(R"((^/[^/]+))");
    if (rxPath.indexIn(fileUrl.path()) != -1) {
        qInfo() << rxPath.cap(1);
        fileUrl.setPath(rxPath.cap(1));
    }
    GFile *file = g_file_new_for_uri(fileUrl.toString().toUtf8().constData());

    if (file == nullptr)
        return MOUNT_FAILED;

    GMountOperation *op = new_mount_op(false);


    QSharedPointer<DFMUrlBaseEvent> evettemp(new DFMUrlBaseEvent(event));

    MountEventHash.insert(op, evettemp);
    QSharedPointer<QEventLoop> event_loop(new QEventLoop);
    eventLoopHash.insert(op, event_loop);

    QSharedPointer<QTimer> timer(new QTimer);
    MountTimerHash.insert(op,timer);
    timer->connect(timer.data(), &QTimer::timeout, timer.data(),[=](){
        cancellMountSync(op);
        qInfo() << "mount_sync time out !!!!!!";
    });
    timer->setInterval(10000);

    GCancellable *cancell = g_cancellable_new();
    if (cancell)
        CancellHash.insert(op,cancell);

    g_file_mount_enclosing_volume(file, static_cast<GMountMountFlags>(0),
                                  op, cancell, mount_done_cb, op);
    timer->start();
    int ret = event_loop->exec();
    if (MountTimerHash.value(op))
        MountTimerHash.value(op)->stop();
    if (askPasswordDialogHash.value(op))
        askPasswordDialogHash.value(op)->deleteLater();
    MountEventHash.remove(op);
    MountTimerHash.remove(op);
    if (CancellHash.value(op)) {
        g_object_unref(CancellHash.value(op));
    }
    CancellHash.remove(op);
    eventLoopHash.remove(op);
    AskingPasswordHash.remove(op);
    askPasswordDialogHash.remove(op);
    if (SMBLoginObjHash.value(op))
        delete SMBLoginObjHash.value(op);
    SMBLoginObjHash.remove(op);

    g_object_unref(file);
    g_object_unref(op);

    return static_cast<MountStatus>(ret);
}

void GvfsMountManager::mount_done_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    //fix 22749 修复输入秘密错误了后，2到3次才弹提示框
    bool bshow = true;
    GMountOperation *op = static_cast<GMountOperation *>(user_data);
    if (MountTimerHash.contains(op))
        MountTimerHash.value(op)->stop();

    if (askPasswordDialogHash.value(op)) {
        askPasswordDialogHash.value(op)->deleteLater();
        askPasswordDialogHash.remove(op);
    } else {
        bshow = false;
    }
    gboolean succeeded;
    GError *error = nullptr;

    MountStatus status = MOUNT_FAILED;

    succeeded = g_file_mount_enclosing_volume_finish(G_FILE(object), res, &error);

    DUrl rootUri = DUrl(g_file_get_uri(G_FILE (object)));

    if (!succeeded) {
        Q_ASSERT(error->domain == G_IO_ERROR);
        emit fileSignalManager->requestRemoveSmbUrl(rootUri);
        bool showWarnDlg = false;

        switch (error->code) {
        case G_IO_ERROR_FAILED:
            if (AskingPasswordHash.value(op)) {
                status = MOUNT_PASSWORD_WRONG;
            } else {
                showWarnDlg = true;
            }
            break;

        case G_IO_ERROR_FAILED_HANDLED: // Operation failed and a helper program has already interacted with the user. Do not display any error dialog.
            status = MOUNT_CANCEL;
            break;

        default:
            showWarnDlg = true;
            break;
        }

//        if (showWarnDlg) {
//            DThreadUtil::runInMainThread(dialogManager, &DialogManager::showErrorDialog,
//                                         tr("Mounting device error"), QString(error->message));
//        }
        if (showWarnDlg) {
            DThreadUtil::runInMainThread(dialogManager, &DialogManager::showErrorDialog,
                                         tr("Mounting device error"), QString());
            qInfo() << "Mounting device error: " << QString(error->message);
        } else {
            //fix 22749 修复输入秘密错误了后，2到3次才弹提示框
            if (status == MOUNT_PASSWORD_WRONG && bshow) {
                DThreadUtil::runInMainThread(dialogManager, &DialogManager::showErrorDialog,
                                             tr("Mounting device error"), tr("Wrong username or password"));
            } else if (status == MOUNT_CANCEL && bshow) {
                DThreadUtil::runInMainThread(dialogManager, &DialogManager::showErrorDialog,
                                             tr("Mounting device error"), tr(error->message));
            }

        }
        qCDebug(mountManager()) << "g_file_mount_enclosing_volume_finish" << succeeded << error;
        qCDebug(mountManager()) << "username" << g_mount_operation_get_username(op) << error->message;
    } else {
        qCDebug(mountManager()) << "g_file_mount_enclosing_volume_finish" << succeeded << AskingPasswordHash.value(op);
        if (AskingPasswordHash.value(op) && SMBLoginObjHash.value(op)) {
            SMBLoginObjHash.value(op)->insert("id", rootUri.toString());
            if (SMBLoginObjHash.value(op)->value("passwordSave").toInt() == 2) {
                SMBLoginObjHash.value(op)->remove("password");
                emit fileSignalManager->requsetCacheLoginData(*SMBLoginObjHash.value(op));
            }
            delete SMBLoginObjHash.value(op);
            SMBLoginObjHash.remove(op);
        } else {
            qCDebug(mountManager()) << "username" << g_mount_operation_get_username(op);
        }

        status = MOUNT_SUCCESS;
    }

    AskingPasswordHash.remove(op);

    if (eventLoopHash.value(op)) {
        eventLoopHash.value(op)->exit(status);
    }

    emit fileSignalManager->requestChooseSmbMountedFile(*MountEventHash.value(op));
    MountEventHash.remove(op);
}

void GvfsMountManager::mount_mounted(const QString &mounted_root_uri, bool silent)
{
    std::string file_uri = mounted_root_uri.toStdString();
    const char *f = file_uri.data();
    GFile *file;
    file = g_file_new_for_uri(f);
    if (file == nullptr)
        return;
    GMountOperation *op;
    op = new_mount_op();
    g_file_mount_enclosing_volume(file, G_MOUNT_MOUNT_NONE, op, nullptr, &GvfsMountManager::mount_with_mounted_uri_done, silent ? &silent : nullptr);
    g_object_unref(file);
}

void GvfsMountManager::mount_with_mounted_uri_done(GObject *object, GAsyncResult *res, gpointer user_data)
{
    gboolean succeeded;
    GError *error = nullptr;

    succeeded = g_file_mount_enclosing_volume_finish(G_FILE(object), res, &error);

    if (!succeeded) {
        qCDebug(mountManager()) << "Error mounting location: " << error->message << error->code;
        if (!user_data && !errorCodeNeedSilent(error->code)) {
            fileSignalManager->requestShowErrorDialog(QString::fromLocal8Bit(error->message), QString(" "));
        }
    }
}

void GvfsMountManager::mount_device(const QString &unix_device, bool silent)
{
    if (unix_device.isEmpty())
        return;
    std::string file_uri = unix_device.toStdString();
    const char *device_file = file_uri.data();

    GVolumeMonitor *volume_monitor;
    GList *volumes;
    GList *l;
    int outstanding_mounts = 0;
    volume_monitor = g_volume_monitor_get();

    volumes = g_volume_monitor_get_volumes(volume_monitor);
    for (l = volumes; l != nullptr; l = l->next) {
        GVolume *volume = G_VOLUME(l->data);

        if (g_strcmp0(g_volume_get_identifier(volume,
                                              G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE), device_file) == 0) {
            GMountOperation *op;

            op = new_mount_op();

            g_volume_mount(volume,
                           G_MOUNT_MOUNT_NONE,
                           op,
                           nullptr,
                           &GvfsMountManager::mount_with_device_file_cb,
                           silent ? &silent : nullptr);

            outstanding_mounts++;
        }
    }
    g_list_free_full(volumes, g_object_unref);

    if (outstanding_mounts == 0) {
        qDebug() << "No volume for device file" << device_file;
        return;
    }
    g_object_unref(volume_monitor);
}

bool GvfsMountManager::try_to_get_mounted_point(GVolume *volume)
{
    if(volume == nullptr)
        return false;

    GMount *mount = nullptr;
    GFile *root = nullptr;
    char *mount_path = nullptr;
    mount = g_volume_get_mount(volume);
    root = g_mount_get_root(mount);
    mount_path = g_file_get_path(root);
    bool ret = (mount_path != nullptr);
    qCDebug(mountManager()) << "Mounted" << g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) << "at" << mount_path;

    g_object_unref(mount);
    g_object_unref(root);
    g_free(mount_path);
    return ret;
}

void GvfsMountManager::mount_with_device_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    GVolume *volume;
    gboolean succeeded;
    GError *error = nullptr;

    volume = G_VOLUME(object);

    succeeded = g_volume_mount_finish(volume, res, &error);
    QVolume qVolume = gVolumeToqVolume(volume);

    // 返回false 的情况，如果有挂载点也算成功，此时要看看 gio 相关流程是否存在bug
    if (!succeeded && !try_to_get_mounted_point(volume)) {
        qCDebug(mountManager()) << "Error mounting: " << g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE)
                                << error->message << user_data << error->code;

        //! 下面这样做只是为了字符串翻译，因为错误信息是底层返回的
        QString err = QString::fromLocal8Bit(error->message);
        bool format = true; // 根据错误的种类, 选择是否需要格式化, 这里仅对于错误 G_IO_ERROR_DBUS_ERROR 不处理, 后续可能会扩充
        switch (error->code) {
        case G_IO_ERROR_FAILED:
            err = QString(tr("No key available to unlock device"));
            break;
        case G_IO_ERROR_DBUS_ERROR:
            format = false;
            break;
        default:
            break;
        }
        if (AskedPasswordWhileMountDisk) { // 显示过密码框的设备，说明该设备可解锁，但密码不一定正确或取消了，不需要提示用户格式化
            if (!user_data && !errorCodeNeedSilent(error->code)) {
                fileSignalManager->requestShowErrorDialog(err, QString(" "));
            }
        } else {
            if (format) {
                dialogManager->showFormatDialog(qVolume.drive_unix_device());
            }
        }
    }
    AskedPasswordWhileMountDisk = false;
}

void GvfsMountManager::unmount(const QDiskInfo &diskInfo)
{
    if (diskInfo.can_unmount()  && !diskInfo.mounted_root_uri().isEmpty()) {
        unmount_mounted(diskInfo.mounted_root_uri());
    }
}

void GvfsMountManager::unmount(const QString &id)
{
    QDiskInfo diskInfo = getDiskInfo(id, false);
    unmount(diskInfo);
}

void GvfsMountManager::unmount_mounted(const QString &mounted_root_uri)
{
    if (mounted_root_uri.isEmpty())
        return;

    GFile *file;
    file = g_file_new_for_uri(QFile::encodeName(mounted_root_uri));
    if (file == nullptr)
        return;

    GMount *mount;
    GError *error = nullptr;
    GMountOperation *mount_op;

    mount = g_file_find_enclosing_mount(file, nullptr, &error);
    if (mount == nullptr) {
        bool no_permission = false;

        QFileInfo info(QUrl(mounted_root_uri).toLocalFile());

        while (!info.exists() && info.fileName() != QDir::rootPath() && !info.absolutePath().isEmpty()) {
            info.setFile(info.absolutePath());
        }

        if (info.exists()) {
            if (getuid() == info.ownerId()) {
                if (!info.permission(QFile::ReadOwner | QFile::ExeOwner))
                    no_permission = true;
            } else if (getgid() == info.groupId()) {
                if (!info.permission(QFile::ReadGroup | QFile::ExeGroup))
                    no_permission = true;
            } else if (!info.permission(QFile::ReadOther | QFile::ExeOther)) {
                no_permission = true;
            }
        }

        if (no_permission) {
            QString user_name = info.owner();

            if (info.absoluteFilePath().startsWith("/media/")) {
                user_name = info.baseName();
            }

            DDialog error_dilaog(tr("The disk is mounted by user \"%1\", you cannot unmount it.").arg(user_name), QString(" "));

            error_dilaog.setIcon(QIcon::fromTheme("dialog-error"));
            error_dilaog.addButton(tr("Confirm","button"), true, DDialog::ButtonRecommend);
            error_dilaog.setModal(true);
            error_dilaog.exec();
            return;
        }

        DDialog error_dilaog(tr("Cannot find the mounted device"), QString(error->message));

        g_error_free(error);

        error_dilaog.setIcon(QIcon::fromTheme("dialog-error"));
        error_dilaog.addButton(tr("Confirm","button"), true, DDialog::ButtonRecommend);
        error_dilaog.setModal(true);
        error_dilaog.exec();
        return;
    }

    g_error_free(error);
    char *local_mount_point = g_file_get_path(file);

    mount_op = new_mount_op();
    g_mount_unmount_with_operation(mount, G_MOUNT_UNMOUNT_NONE, mount_op, nullptr, &GvfsMountManager::unmount_done_cb, local_mount_point);
    g_object_unref(mount_op);
    g_object_unref(file);
}


void GvfsMountManager::unmount_done_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    gboolean succeeded;
    GError *error = nullptr;

    succeeded = g_mount_unmount_with_operation_finish(G_MOUNT(object), res, &error);

    if (!succeeded) {
        DDialog error_dilaog(tr("Cannot unmount the device"), QString(error->message));

        error_dilaog.setIcon(QIcon::fromTheme("dialog-error"));
        error_dilaog.addButton(tr("Confirm","button"), true, DDialog::ButtonRecommend);
        error_dilaog.setModal(true);
        error_dilaog.exec();
    } else {
        char *local_mount_point = reinterpret_cast<char *>(user_data);

        if (local_mount_point) {
            // 由于卸载gvfs设备时不会触发文件系统的inotify, 所以此处手动模拟文件夹被移除的信号
            const DUrl durl = DUrl::fromLocalFile(local_mount_point);
            DAbstractFileWatcher::ghostSignal(durl.parentUrl(), &DAbstractFileWatcher::fileDeleted, durl);

            g_free(local_mount_point);
        }

        // 从缓存的Network节点中清理数据
        GFile *root_file = g_mount_get_root(G_MOUNT(object));
        char *root_uri = g_file_get_uri(root_file);
        const QString &root_url = QFile::decodeName(root_uri);

        if (root_uri && !root_url.startsWith("file://")) {
            DUrlList dirty_list;

            for (auto begin = NetworkManager::NetworkNodes.keyBegin(); begin != NetworkManager::NetworkNodes.keyEnd(); ++begin) {
                if (begin->toString().startsWith(root_url.endsWith("/") ? root_url.left(root_url.size() - 1) : root_url)) {
                    dirty_list << *begin;
                }
            }

            for (const DUrl &durl : dirty_list)
                NetworkManager::NetworkNodes.remove(durl);
        }

        if (root_uri) g_free(root_uri);
        if (root_file) g_object_unref(root_file);
    }

    if (G_IS_MOUNT(object)) g_object_unref(G_MOUNT(object));
}

void GvfsMountManager::eject(const QString &path)
{
    foreach (const QDiskInfo &diskInfo, DiskInfos.values()) {
        if (!path.isEmpty() && diskInfo.can_eject() && diskInfo.mounted_root_uri() == path) {
            eject_mounted(diskInfo.mounted_root_uri());
            return;
        } else if (!path.isEmpty() && diskInfo.can_eject() && path == diskInfo.unix_device()) {
            eject_device(diskInfo.unix_device());
            return;
        }
    }
}

void GvfsMountManager::eject(const QDiskInfo &diskInfo)
{
    if (!diskInfo.mounted_root_uri().isEmpty()) {
        eject_mounted(diskInfo.mounted_root_uri());
    } else {
        eject_device(diskInfo.unix_device());
    }
}

void GvfsMountManager::eject_device(const QString &unix_device)
{
    if (unix_device.isEmpty())
        return;
    std::string file_uri = unix_device.toStdString();
    const char *device_file = file_uri.data();

    GVolumeMonitor *volume_monitor;
    GList *volumes;
    GList *l;
    int outstanding_mounts = 0;
    volume_monitor = g_volume_monitor_get();

    volumes = g_volume_monitor_get_volumes(volume_monitor);
    for (l = volumes; l != nullptr; l = l->next) {
        GVolume *volume = G_VOLUME(l->data);

        if (g_strcmp0(g_volume_get_identifier(volume,
                                              G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE), device_file) == 0) {
            GMountOperation *op;

            op = new_mount_op();

            g_volume_eject_with_operation(volume,
                                          G_MOUNT_UNMOUNT_NONE,
                                          op,
                                          nullptr,
                                          &GvfsMountManager::eject_with_device_file_cb,
                                          op);

            outstanding_mounts++;
        }
    }
    g_list_free_full(volumes, g_object_unref);

    if (outstanding_mounts == 0) {
        qCDebug(mountManager()) << "No volume for device file: " << device_file;
        return;
    }
    g_object_unref(volume_monitor);
}

void GvfsMountManager::eject_with_device_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    GVolume *volume;
    gboolean succeeded;
    GError *error = nullptr;

    volume = G_VOLUME(object);

    succeeded = g_volume_eject_with_operation_finish(volume, res, &error);

    if (!succeeded) {
        DDialog error_dilaog(tr("Cannot eject the device \"%1\"").arg(g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE)),
                             QString(error->message));

        error_dilaog.setIcon(QIcon::fromTheme("dialog-error"));
        error_dilaog.addButton(tr("Confirm","button"), true, DDialog::ButtonRecommend);
        error_dilaog.setModal(true);
        error_dilaog.exec();
    } else {
        qCDebug(mountManager()) << "eject" <<  g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) << "succeeded";
    }
}

void GvfsMountManager::eject_mounted(const QString &mounted_root_uri)
{
    if (mounted_root_uri.isEmpty())
        return;

    std::string file_uri = mounted_root_uri.toStdString();
    GFile *file = g_file_new_for_uri(file_uri.data());
    if (file == nullptr)
        return;

    GError *error = nullptr;
    GMount *mount = g_file_find_enclosing_mount(file, nullptr, &error);
    if (mount == nullptr) {
        DDialog error_dilaog(tr("Cannot find the mounted device"), QString(error->message));
        g_error_free(error);
        error_dilaog.setIcon(QIcon::fromTheme("dialog-error"));
        error_dilaog.addButton(tr("Confirm","button"), true, DDialog::ButtonRecommend);
        error_dilaog.setModal(true);
        error_dilaog.exec();
        return;
    }
    g_error_free(error);
    GMountOperation *mount_op = new_mount_op();
    g_mount_eject_with_operation(mount, G_MOUNT_UNMOUNT_NONE, mount_op, nullptr, &GvfsMountManager::eject_with_mounted_file_cb, nullptr);
    g_object_unref(mount_op);

}

void GvfsMountManager::eject_with_mounted_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    GMount *mount;
    gboolean succeeded;
    GError *error = nullptr;

    mount = G_MOUNT(object);
    succeeded = g_mount_eject_with_operation_finish(G_MOUNT(object), res, &error);

    g_object_unref(G_MOUNT(object));

    if (!succeeded) {
        qDebug() << "Error ejecting mount:" << error->message;
    } else {
        qCDebug(mountManager()) << "eject" << g_mount_get_name(mount)  << "succeeded";
    }
}

void GvfsMountManager::stop_device(const QString &drive_unix_device)
{
    if (drive_unix_device.isEmpty())
        return;
    std::string file_uri = drive_unix_device.toStdString();
    const char *device_file = file_uri.data();

    GVolumeMonitor *volume_monitor;
    GList *drives, *d;
    int i;
    volume_monitor = g_volume_monitor_get();

    drives = g_volume_monitor_get_connected_drives(volume_monitor);
    for (i = 0, d = drives; d != nullptr; d = d->next, i++) {
        GDrive *drive = static_cast<GDrive *>(d->data);

        if (g_strcmp0(g_drive_get_identifier(drive, "unix-device"), device_file) == 0) {
            GMountOperation *op;

            op = new_mount_op();

            g_drive_stop(drive,
                         G_MOUNT_UNMOUNT_NONE,
                         op,
                         nullptr,
                         &GvfsMountManager::stop_with_device_file_cb,
                         op);
        }

    }
    g_list_free_full(drives, g_object_unref);
    g_object_unref(volume_monitor);
}

void GvfsMountManager::stop_with_device_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    GDrive *drive;
    gboolean succeeded;
    GError *error = nullptr;

    drive = G_DRIVE(object);

    succeeded = g_drive_stop_finish(drive, res, &error);

    if (!succeeded) {
        qDebug() << "Error remove disk:" << g_drive_get_identifier(drive, "unix-device") << error->message;
    } else {
        qCDebug(mountManager()) << "Safely remove disk" <<  g_drive_get_identifier(drive, "unix-device") << "succeeded";
    }
}
