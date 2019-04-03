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

#include "gvfsmountmanager.h"
#include "qdrive.h"
#include "qvolume.h"
#include "qmount.h"
#include "qdiskinfo.h"
#include "singleton.h"
#include "../app/define.h"
#include "../partman/partition.h"
#include "../partman/readusagemanager.h"
#include "../interfaces/dfileservices.h"
#include "../interfaces/dfmevent.h"
#include "deviceinfo/udisklistener.h"
#include "dialogs/dialogmanager.h"
#include "partman/command.h"
#include "mountsecretdiskaskpassworddialog.h"
#include "app/filesignalmanager.h"
#include "shutil/fileutils.h"

#include "networkmanager.h"
#include "dfmapplication.h"
#include "dabstractfilewatcher.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"

#include <QThread>
#include <QApplication>
#include <QLoggingCategory>

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

MountSecretDiskAskPasswordDialog* GvfsMountManager::mountSecretDiskAskPasswordDialog = nullptr;

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(mountManager, "gvfs.mountMgr")
#else
Q_LOGGING_CATEGORY(mountManager, "gvfs.mountMgr", QtInfoMsg)
#endif

GvfsMountManager::GvfsMountManager(QObject *parent) : QObject(parent)
{
    if (getDialogManager(false)) {
        connect(this, &GvfsMountManager::mount_added, dialogManager, &DialogManager::showNtfsWarningDialog);
    }

    m_gVolumeMonitor = g_volume_monitor_get();
    qRegisterMetaType<QDrive>("QDrive");

//    mount_mounted("smb://10.0.10.30/people");
//    unmount("smb://10.0.10.30/people");
//    mount_device("/dev/sda5");
//    unmount("file:///media/djf/34e594c1-341d-4961-9f85-822f366312da");
//    eject_device("/dev/sdb");
//    eject_mounted("file:///media/djf/Fat32");
}

void GvfsMountManager::initConnect()
{
    if (DFMGlobal::isRootUser()){
        g_signal_connect (m_gVolumeMonitor, "mount-added", (GCallback)&GvfsMountManager::monitor_mount_added_root, nullptr);
        g_signal_connect (m_gVolumeMonitor, "mount-removed", (GCallback)&GvfsMountManager::monitor_mount_removed_root, nullptr);
    }else{
        g_signal_connect (m_gVolumeMonitor, "drive-connected", (GCallback)&GvfsMountManager::monitor_drive_connected, nullptr);
        g_signal_connect (m_gVolumeMonitor, "drive-disconnected", (GCallback)&GvfsMountManager::monitor_drive_disconnected, nullptr);
        g_signal_connect (m_gVolumeMonitor, "mount-added", (GCallback)&GvfsMountManager::monitor_mount_added, nullptr);
        g_signal_connect (m_gVolumeMonitor, "mount-removed", (GCallback)&GvfsMountManager::monitor_mount_removed, nullptr);
        g_signal_connect (m_gVolumeMonitor, "mount-changed", (GCallback)&GvfsMountManager::monitor_mount_changed, nullptr);
        g_signal_connect (m_gVolumeMonitor, "volume-added", (GCallback)&GvfsMountManager::monitor_volume_added, nullptr);
        g_signal_connect (m_gVolumeMonitor, "volume-removed", (GCallback)&GvfsMountManager::monitor_volume_removed, nullptr);
        g_signal_connect (m_gVolumeMonitor, "volume-changed", (GCallback)&GvfsMountManager::monitor_volume_changed, nullptr);
    }
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
    g_object_get (icon, "names", &names, nullptr);
    for (iter = names; *iter; iter++){
        iconNames.append(QString(*iter));
    }
    g_strfreev (names);
    return iconNames;
}

QDrive GvfsMountManager::gDriveToqDrive(GDrive *drive)
{
    QDrive qDrive;
    char *name;
    char **ids;
    GIcon *icon;
    const gchar *sort_key;

    name = g_drive_get_name (drive);
    qDrive.setName(QString(name));
    g_free (name);

    ids = g_drive_enumerate_identifiers (drive);
    if (ids && ids[0] != nullptr){
        for (int i = 0; ids[i] != nullptr; i++){
            char *id = g_drive_get_identifier (drive,
                                               ids[i]);
            if (QString(ids[i]) == "unix-device"){
               qDrive.setUnix_device(QString(id));
            }
            g_free (id);
        }
    }
    g_strfreev (ids);

    qDrive.setHas_volumes(g_drive_has_volumes(drive));
    qDrive.setCan_eject(g_drive_can_eject(drive));
    qDrive.setCan_start(g_drive_can_start(drive));
    qDrive.setCan_start_degraded(g_drive_can_start_degraded(drive));
    qDrive.setCan_poll_for_media(g_drive_can_poll_for_media (drive));
    qDrive.setCan_stop(g_drive_can_stop (drive));
    qDrive.setIs_removable(g_drive_is_removable (drive));
    qDrive.setStart_stop_type(g_drive_get_start_stop_type (drive));
    qDrive.setHas_media(g_drive_has_media (drive));
    qDrive.setIs_media_check_automatic(g_drive_is_media_check_automatic(drive));
    qDrive.setIs_media_removable(g_drive_is_media_removable(drive));

    icon = g_drive_get_icon(drive);
    if (icon){
        if (G_IS_THEMED_ICON (icon)){
            qDrive.setIcons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref (icon);
    }

    icon = g_drive_get_symbolic_icon (drive);
    if (icon){
        if (G_IS_THEMED_ICON (icon)){
            qDrive.setSymbolic_icons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref (icon);
    }

    sort_key = g_drive_get_sort_key(drive);
    if (sort_key != nullptr){
        qDrive.setSort_key(QString(sort_key));
    }

    return qDrive;
}

QVolume GvfsMountManager::gVolumeToqVolume(GVolume *volume)
{
    QVolume qVolume;
    char *name;
    char **ids;
    GIcon *icon;
    const gchar *sort_key;

    GFile *root;
    GFile *activation_root;
    char *uri, * action_root_uri;

    name = g_volume_get_name(volume);
    qVolume.setName(QString(name));
    g_free (name);

    ids = g_volume_enumerate_identifiers (volume);
    if (ids && ids[0] != nullptr){
        for (int i = 0; ids[i] != nullptr; i++){
            char *id = g_volume_get_identifier(volume, ids[i]);
            if (QString(ids[i]) == G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE){
               qVolume.setUnix_device(QString(id));
            }else if (QString(ids[i]) == G_VOLUME_IDENTIFIER_KIND_LABEL){
                qVolume.setLable(QString(id));
            }else if (QString(ids[i]) == G_VOLUME_IDENTIFIER_KIND_UUID){
                qVolume.setUuid(QString(id));
            }else if (QString(ids[i]) == G_VOLUME_IDENTIFIER_KIND_NFS_MOUNT){
                qVolume.setNfs_mount(QString(id));
            }

            if (qVolume.unix_device().isEmpty()){
                qVolume.setUnix_device(qVolume.uuid());
            }

            g_free (id);
        }
    }
    g_strfreev (ids);

    qVolume.setCan_mount(g_volume_can_mount(volume));
    qVolume.setCan_eject(g_volume_can_eject(volume));
    qVolume.setShould_automount(g_volume_should_automount(volume));


    icon = g_volume_get_icon(volume);
    if (icon){
        if (G_IS_THEMED_ICON (icon)){
            qVolume.setIcons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref (icon);
    }

    icon = g_volume_get_symbolic_icon (volume);
    if (icon){
        if (G_IS_THEMED_ICON (icon)){
            qVolume.setSymbolic_icons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref (icon);
    }

    sort_key = g_volume_get_sort_key(volume);
    if (sort_key != nullptr){
        qVolume.setSort_key(QString(sort_key));
    }

    GMount *mount = g_volume_get_mount(volume);
    if (mount){
        qVolume.setIsMounted(true);
        root = g_mount_get_root (mount);
        uri = g_file_get_uri (root);
        qVolume.setMounted_root_uri(QString(uri));

        g_object_unref (root);
        g_free(uri);
    }

    activation_root = g_volume_get_activation_root(volume);
    if (activation_root != nullptr){
        action_root_uri = g_file_get_uri (activation_root);
        qVolume.setActivation_root_uri(QString(action_root_uri));
        g_object_unref (activation_root);
        g_free(action_root_uri);
    }

    GDrive *gDrive = g_volume_get_drive(volume);
    if (gDrive){
        QDrive qDrive = gDriveToqDrive(gDrive);
        qVolume.setDrive_unix_device(QString(g_drive_get_identifier(gDrive, "unix-device")));
        qVolume.setDrive(qDrive);
    }

    return qVolume;
}

QMount GvfsMountManager::gMountToqMount(GMount *mount)
{
    QMount qMount;
    char *name, *uri;
    GFile *root, *default_location;
    GIcon *icon;
    const gchar *sort_key;

    name = g_mount_get_name (mount);
    qMount.setName(QString(name));
    g_free (name);

    root = g_mount_get_root (mount);
    uri = g_file_get_uri (root);
    qMount.setMounted_root_uri(QString(uri));
    g_object_unref (root);
    g_free (uri);

    default_location = g_mount_get_default_location (mount);
    if (default_location){
        char *loc_uri = g_file_get_uri (default_location);
        qMount.setDefault_location(QString(loc_uri));
        g_free (loc_uri);
        g_object_unref (default_location);
    }

    icon = g_mount_get_icon (mount);
    if (icon){
        if (G_IS_THEMED_ICON (icon)){
            qMount.setIcons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref (icon);
    }

    icon = g_mount_get_symbolic_icon (mount);
    if (icon){
        if (G_IS_THEMED_ICON (icon)){
            qMount.setSymbolic_icons(getIconNames(G_THEMED_ICON(icon)));
        }
        g_object_unref (icon);
    }

    qMount.setCan_unmount(g_mount_can_unmount (mount));
    qMount.setCan_eject(g_mount_can_eject (mount));
    qMount.setIs_shadowed(g_mount_is_shadowed (mount));

    sort_key = g_mount_get_sort_key (mount);
    if (sort_key != nullptr){
        qMount.setSort_key(QString(sort_key));
    }

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

    if (volume.icons().count() > 0){
        diskInfo.setIconName(volume.icons().at(0));
    }

    diskInfo.setCan_mount(volume.can_mount());
    diskInfo.setCan_eject(volume.can_eject());

    if (!volume.mounted_root_uri().isEmpty()){
        diskInfo.setCan_unmount(true);
    }

    if (diskInfo.iconName() == "phone-apple-iphone"){
        diskInfo.setType("iphone");
    }else if (diskInfo.iconName() == "phone" || diskInfo.iconName() == "multimedia-player"){
        diskInfo.setType("phone");
    }else if (diskInfo.iconName() == "camera-photo" || diskInfo.iconName() == "camera"){
        diskInfo.setType("camera");
    }else if (diskInfo.can_eject() && (diskInfo.iconName() == "drive-harddisk-usb" || diskInfo.iconName() == "drive-removable-media-usb")){
        diskInfo.setType("removable");
        diskInfo.setIs_removable(true);
    }else if (isDVD(volume)){
        diskInfo.setType("dvd");
    }else{
        diskInfo.setType("native");
    }

    diskInfo.setHas_volume(true);

    diskInfo.updateGvfsFileSystemInfo();


    if (Drives.contains(volume.drive_unix_device())){
        const QDrive& drive = Drives.value(volume.drive_unix_device());
        if (drive.is_removable()){
            diskInfo.setType("removable");
            diskInfo.setIs_removable(true);
        }
    }

    return diskInfo;
}

QDiskInfo GvfsMountManager::qMountToqDiskinfo(const QMount &mount)
{
    QDiskInfo diskInfo;
    diskInfo.setId(mount.mounted_root_uri());

    diskInfo.setName(mount.name());
//    diskInfo.setType();
    diskInfo.setUuid(mount.uuid());
    diskInfo.setMounted_root_uri(mount.mounted_root_uri());

    if (mount.icons().count() > 0){
        diskInfo.setIconName(mount.icons().at(0));
    }

    diskInfo.setCan_unmount(mount.can_unmount());
    diskInfo.setCan_eject(mount.can_eject());
    diskInfo.setDefault_location(mount.default_location());

    if (diskInfo.mounted_root_uri().startsWith("smb://")){
        diskInfo.setType("smb");
    }else if (diskInfo.iconName() == "drive-optical" && diskInfo.iconName().startsWith("CD")){
        diskInfo.setType("dvd");
    }else{
        diskInfo.setType("network");
    }
    diskInfo.updateGvfsFileSystemInfo();
    return diskInfo;
}

QVolume GvfsMountManager::getVolumeByMountedRootUri(const QString &mounted_root_uri)
{
    foreach (const QVolume& volume, Volumes) {
        if (volume.isMounted() && volume.mounted_root_uri() == mounted_root_uri){
            return volume;
        }
    }
    return QVolume();
}

QVolume GvfsMountManager::getVolumeByUnixDevice(const QString &unix_device)
{
    foreach (const QVolume& volume, Volumes) {
        if (volume.unix_device() == unix_device){
            return volume;
        }
    }
    return QVolume();
}

void GvfsMountManager::monitor_drive_connected(GVolumeMonitor *volume_monitor, GDrive *drive)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_drive_connected==============================";
    QDrive qDrive = gDriveToqDrive(drive);
    qCDebug(mountManager()) << qDrive;
    emit gvfsMountManager->drive_connected(qDrive);
}

void GvfsMountManager::monitor_drive_disconnected(GVolumeMonitor *volume_monitor, GDrive *drive)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_drive_disconnected==============================";
    QDrive qDrive = gDriveToqDrive(drive);
    qCDebug(mountManager()) << qDrive;
    emit gvfsMountManager->drive_disconnected(qDrive);
}

void GvfsMountManager::monitor_mount_added_root(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_mount_added_root==============================";
    QMount qMount = gMountToqMount(mount);
    qCDebug(mountManager()) << qMount;

    foreach (QString key, DiskInfos.keys()) {
        QDiskInfo info = DiskInfos.value(key);
        if (info.mounted_root_uri() == qMount.mounted_root_uri()){
            emit gvfsMountManager->volume_added(info);
            return;
        }
    }


}

void GvfsMountManager::monitor_mount_removed_root(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_mount_removed_root==============================";
    QMount qMount = gMountToqMount(mount);
    qCDebug(mountManager()) << qMount;
    foreach (QString key, DiskInfos.keys()) {
        QDiskInfo info = DiskInfos.value(key);
        if (info.mounted_root_uri() == qMount.mounted_root_uri()){
            emit gvfsMountManager->volume_removed(info);
            return;
        }
    }
}


void GvfsMountManager::monitor_mount_added(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_mount_added==============================";
    QMount qMount = gMountToqMount(mount);

    GVolume *volume = g_mount_get_volume(mount);
    qCDebug(mountManager()) << "===================" << qMount.mounted_root_uri() << volume << "=======================";
    qCDebug(mountManager()) << "===================" << qMount << "=======================";
    if (volume != nullptr){
        QVolume qVolume = gVolumeToqVolume(volume);
        Volumes.insert(qVolume.unix_device(), qVolume);

        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);
        if (qMount.can_unmount()){
            diskInfo.setCan_unmount(true);
        }
        if (qMount.can_eject()){
            diskInfo.setCan_eject(true);
        }

        DiskInfos.insert(diskInfo.id(), diskInfo);
        emit gvfsMountManager->mount_added(diskInfo);
        qCDebug(mountManager()) << "========mount_added===========" << diskInfo;
    }else{
        // ignore afc first mounted event
        if (isIgnoreUnusedMounts(qMount)){
            return;
        }

        if (!NoVolumes_Mounts_Keys.contains(qMount.mounted_root_uri())){
            NoVolumes_Mounts_Keys.append(qMount.mounted_root_uri());

            QDiskInfo diskInfo = qMountToqDiskinfo(qMount);
            DiskInfos.insert(diskInfo.id(), diskInfo);
            emit gvfsMountManager->mount_added(diskInfo);
        }
    }

    Mounts.insert(qMount.mounted_root_uri(), qMount);
}

void GvfsMountManager::monitor_mount_removed(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_mount_removed==============================" ;
    QMount qMount = gMountToqMount(mount);

    qCDebug(mountManager()) << "===================" << qMount.mounted_root_uri() << "=======================";

    QVolume volume = getVolumeByMountedRootUri(qMount.mounted_root_uri());
    qCDebug(mountManager()) << volume.isValid() << volume;
    if (volume.isValid()){
        volume.setIsMounted(false);
        volume.setMounted_root_uri("");
        Volumes.insert(volume.unix_device(), volume);
    }else{
        NoVolumes_Mounts_Keys.removeOne(qMount.mounted_root_uri());
    }

    bool removed = Mounts.remove(qMount.mounted_root_uri());
    if (removed){

        if (volume.isValid()){
            QDiskInfo diskInfo = qVolumeToqDiskInfo(volume);
            DiskInfos.insert(diskInfo.id(), diskInfo);
            diskInfo.setHas_volume(true);
            emit gvfsMountManager->mount_removed(diskInfo);
        } else {
            QDiskInfo diskInfo = qMountToqDiskinfo(qMount);
            bool diskInfoRemoved = DiskInfos.remove(diskInfo.id());
            if (diskInfoRemoved){
                diskInfo.setHas_volume(false);
                emit gvfsMountManager->mount_removed(diskInfo);
            }
        }
    }
}

void GvfsMountManager::monitor_mount_changed(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    Q_UNUSED(mount)
//    qDebug() << "==============================monitor_mount_changed==============================" ;
    GVolume *volume = g_mount_get_volume(mount);
    if (volume != nullptr){
//        qDebug() << "==============================changed removed==============================" ;

        QVolume qVolume = gVolumeToqVolume(volume);
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);

        bool isDVDChanged = isDVD(qVolume);
        if (isDVDChanged){
            diskInfo.setType("dvd");
            qCDebug(mountManager()) << diskInfo;
            if (diskInfo.can_unmount()){
                diskInfo.updateGvfsFileSystemInfo();
                emit gvfsMountManager->volume_changed(diskInfo);
            }
        }
    }else{
//        qDebug() << "==============================changed volume empty==============================" ;
    }
}

void GvfsMountManager::monitor_volume_added(GVolumeMonitor *volume_monitor, GVolume *volume)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_volume_added==============================" ;
    QVolume qVolume = gVolumeToqVolume(volume);

    qCDebug(mountManager()) << "===================" << qVolume.unix_device() << "=======================";

    GDrive *drive = g_volume_get_drive(volume);
    if (drive) {
        QDrive qDrive = gDriveToqDrive(drive);

        if (!qDrive.unix_device().isEmpty()) {
            Drives.insert(qDrive.unix_device(), qDrive);
            if (!Volumes_Drive_Keys.contains(qDrive.unix_device())) {
                Volumes_Drive_Keys.append(qDrive.unix_device());
            }
        }

        if (drive != nullptr){
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

    GDrive *drive = g_volume_get_drive(volume);
    if (drive){
        QDrive qDrive = gDriveToqDrive(drive);

        if (!qDrive.unix_device().isEmpty()) {
            Drives.insert(qDrive.unix_device(), qDrive);
            Volumes_Drive_Keys.removeOne(qDrive.unix_device());
        }
    }

    bool removed = Volumes.remove(qVolume.unix_device());

    qCDebug(mountManager()) << removed << qVolume << qVolumeToqDiskInfo(qVolume);
    if (removed){
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);
        qCDebug(mountManager()) << diskInfo;
        bool diskInfoRemoved = DiskInfos.remove(diskInfo.id());
        if (diskInfoRemoved){
            emit gvfsMountManager->volume_removed(diskInfo);
        }
    }else{
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);
        emit gvfsMountManager->volume_removed(diskInfo);
    }
}

void GvfsMountManager::monitor_volume_changed(GVolumeMonitor *volume_monitor, GVolume *volume)
{
    Q_UNUSED(volume_monitor)
    qCDebug(mountManager()) << "==============================monitor_volume_changed==============================" ;

    if (volume != nullptr){
        qCDebug(mountManager()) << "==============================volume changed==============================" ;

        QVolume qVolume = gVolumeToqVolume(volume);
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);
//        Volumes.insert(qVolume.unix_device(), qVolume);
        DiskInfos.insert(diskInfo.id(), diskInfo);
        qCDebug(mountManager()) << diskInfo;
        emit gvfsMountManager->volume_changed(diskInfo);
    }else{
        qCDebug(mountManager()) << "==============================changed volume empty==============================" ;
    }
}

GMountOperation *GvfsMountManager::new_mount_op()
{
    GMountOperation *op;

    op = g_mount_operation_new ();

    g_signal_connect (op, "ask_password", G_CALLBACK (&GvfsMountManager::ask_password_cb), nullptr);

    /* TODO: we *should* also connect to the "aborted" signal but since the
     *       main thread is blocked handling input we won't get that signal
     *       anyway...
     */

    return op;
}

void GvfsMountManager::ask_password_cb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags)
{
    if (mountSecretDiskAskPasswordDialog){
        return;
    }

    g_print ("%s\n", message);

    bool anonymous = g_mount_operation_get_anonymous(op);
    GPasswordSave passwordSave = g_mount_operation_get_password_save(op);

    const char* default_password = g_mount_operation_get_password(op);

    qCDebug(mountManager()) << "anonymous" << anonymous;
    qCDebug(mountManager()) << "message" << message;
    qCDebug(mountManager()) << "username" << default_user;
    qCDebug(mountManager()) << "domain" << default_domain;
    qCDebug(mountManager()) << "password" << default_password;
    qCDebug(mountManager()) << "GAskPasswordFlags" << flags;
    qCDebug(mountManager()) << "passwordSave" << passwordSave;

    if (flags & G_ASK_PASSWORD_NEED_USERNAME)
    {
        g_mount_operation_set_username (op, default_user);
    }

    if (flags & G_ASK_PASSWORD_NEED_DOMAIN)
    {
        g_mount_operation_set_domain (op, default_domain);
    }

    if (flags & G_ASK_PASSWORD_NEED_PASSWORD)
    {
        QString tipMessage;
        QString m(message);
        QStringList messageList = m.split("\n");
        if (messageList.count() >= 2){
            tipMessage = messageList.at(1);
        }
        mountSecretDiskAskPasswordDialog = new MountSecretDiskAskPasswordDialog(tipMessage);
        int code = mountSecretDiskAskPasswordDialog->exec();
        QString p = mountSecretDiskAskPasswordDialog->password();
        if (code == 0){
            p.clear();
        }
        qCDebug(mountManager()) << "password is:" << p;
        std::string pstd = p.toStdString();
        g_mount_operation_set_password (op, pstd.c_str());
        mountSecretDiskAskPasswordDialog->deleteLater();
        mountSecretDiskAskPasswordDialog = nullptr;
    }

    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
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

QDiskInfo GvfsMountManager::getDiskInfo(const QString &path)
{
    QDiskInfo info;
    foreach (const QDiskInfo& diskInfo, DiskInfos.values()) {
        if (!path.isEmpty() && path == diskInfo.id()){
            info = diskInfo;
            break;
        }else if (!path.isEmpty() && diskInfo.mounted_root_uri() == path){
            info = diskInfo;
            break;
        }else if (!path.isEmpty() && path == diskInfo.unix_device()){
            info = diskInfo;
            break;
        }else if (!path.isEmpty() && path == diskInfo.uuid()){
            info = diskInfo;
            break;
        }
    }
    info.updateGvfsFileSystemInfo();
    return info;
}

bool GvfsMountManager::isDVD(const QVolume &volume)
{
    if (volume.drive().isValid() && volume.unix_device().startsWith("/dev/sr")){
        return true;
    }
    return false;
}

bool GvfsMountManager::isIgnoreUnusedMounts(const QMount &mount)
{
    /*the following protocol has two mounts event, ignore unused one*/
    if (mount.mounted_root_uri().startsWith("afc://") ||
        mount.mounted_root_uri().startsWith("mtp://") ||
        mount.mounted_root_uri().startsWith("gphoto2://")){
        return true;
    }
    return false;
}

QString GvfsMountManager::getDriveUnixDevice(const QString &unix_device)
{
    QString drive_unix_device;
    if (gvfsMountManager->DiskInfos.contains(unix_device)){
        drive_unix_device = gvfsMountManager->DiskInfos.value(unix_device).drive_unix_device();
    }
    return drive_unix_device;
}

bool GvfsMountManager::isDeviceCrypto_LUKS(const QDiskInfo &diskInfo)
{
    if (diskInfo.can_mount()){
        QString fstype = PartMan::Partition::getPartitionByDevicePath(diskInfo.unix_device()).fs();
        if (fstype == "crypto_LUKS" ){
            return true;
        }
    }
    return false;
}

void GvfsMountManager::startMonitor()
{
    if (DFMGlobal::isRootUser()){
        listMountsBylsblk();
    }else{
        listDrives();
        listVolumes();
        listMounts();
        updateDiskInfos();
    }
#ifdef DFM_MINIMUM
    qDebug() << "Don't auto mount disk";
#else
    if (qApp->applicationName() == QMAKE_TARGET && !DFMGlobal::IsFileManagerDiloagProcess){
        TIMER_SINGLESHOT_OBJECT(this, 1000, {
                                    this->autoMountAllDisks();
                                }, this)
    }
#endif
    initConnect();
    emit loadDiskInfoFinished();
}

void GvfsMountManager::listDrives()
{
    GList *drives;
    drives = g_volume_monitor_get_connected_drives (m_gVolumeMonitor);
    getDrives(drives);
    g_list_free_full (drives, g_object_unref);
}

void GvfsMountManager::getDrives(GList *drives)
{

    GList *volumes, *d, *v;
    char **ids;
    int c, j, k;
    GDrive *drive;
    for (c = 0, d = drives; d != nullptr; d = d->next, c++){
        drive = (GDrive *) d->data;
        QDrive qDrive = gDriveToqDrive(drive);

        if (!qDrive.unix_device().isEmpty()) {
            Drives.insert(qDrive.unix_device(), qDrive);
            Drives_Keys.append(qDrive.unix_device());
        }

        volumes = g_drive_get_volumes (drive);
        GVolume *volume;
        for (j = 0, v = volumes; v != nullptr; v = v->next, j++){
            volume = (GVolume *) v->data;

            ids = g_volume_enumerate_identifiers (volume);
            if (ids && ids[0] != nullptr){
                for (k = 0; ids[k] != nullptr; k++){
                    char *id = g_volume_get_identifier (volume,
                                                       ids[k]);
                    if (QString(ids[k]) == "unix-device"){
                        Volumes_Drive_Keys.append(QString(id));
                    }
                    g_free (id);
                }
            }
            g_strfreev (ids);
        }
        g_list_free_full (volumes, g_object_unref);
    }
}

void GvfsMountManager::listVolumes()
{
    GList *volumes;
    volumes = g_volume_monitor_get_volumes (m_gVolumeMonitor);
    getVolumes(volumes);
    g_list_free_full (volumes, g_object_unref);
}

void GvfsMountManager::getVolumes(GList *volumes)
{
    GList *v = volumes;
    GVolume *volume;

    for (int c = 0; v != nullptr; v = v->next, c++){
        volume = (GVolume *) v->data;
        QVolume qVolume = gVolumeToqVolume(volume);
        GDrive *drive = g_volume_get_drive(volume);
        if (drive != nullptr){
            qVolume.setDrive_unix_device(QString(g_drive_get_identifier(drive, "unix-device")));
        }else{
            if (!Volumes_No_Drive_Keys.contains(qVolume.unix_device())){
                Volumes_No_Drive_Keys.append(qVolume.unix_device());
            }
        }
        Volumes.insert(qVolume.unix_device(), qVolume);
    }
}

void GvfsMountManager::listMounts()
{
    GList *mounts;
    mounts = g_volume_monitor_get_mounts (m_gVolumeMonitor);
    getMounts(mounts);
    g_list_free_full (mounts, g_object_unref);
}

void GvfsMountManager::updateDiskInfos()
{
    Volumes_Drive_Keys.sort();
    foreach (QString key, Volumes_Drive_Keys) {
        if (Volumes.contains(key)){
            QVolume volume = Volumes.value(key);
            QDiskInfo diskInfo = qVolumeToqDiskInfo(volume);
            DiskInfos.insert(diskInfo.id(), diskInfo);
            qCDebug(mountManager()) << diskInfo;
        }
    }
    Volumes_No_Drive_Keys.sort();
    foreach (QString key, Volumes_No_Drive_Keys) {
        if (Volumes.contains(key)){
            QVolume volume = Volumes.value(key);
            QDiskInfo diskInfo = qVolumeToqDiskInfo(volume);
            if (diskInfo.type() == "iphone"){
                if (diskInfo.activation_root_uri() != QString("afc://%1/").arg(diskInfo.uuid())){
                    continue;
                }
            }
            DiskInfos.insert(diskInfo.id(), diskInfo);
            qCDebug(mountManager()) << diskInfo;
        }
    }
    NoVolumes_Mounts_Keys.sort();
    foreach (QString key, NoVolumes_Mounts_Keys) {
        if (Mounts.contains(key)){
            QMount mount = Mounts.value(key);
            QDiskInfo diskInfo = qMountToqDiskinfo(mount);
            DiskInfos.insert(diskInfo.id(), diskInfo);
            qCDebug(mountManager()) << diskInfo;
        }
    }
    qCDebug(mountManager()) << Mounts;
}

void GvfsMountManager::getMounts(GList *mounts)
{
    GList *m = mounts;
    GMount *mount;
    for (int c = 0; m != nullptr; m = m->next, c++){
        mount = (GMount *) m->data;
        QMount qMount = gMountToqMount(mount);
        Mounts.insert(qMount.mounted_root_uri(), qMount);
        GVolume* volume = g_mount_get_volume(mount);
        if (volume != nullptr){
            continue;
        }else{
            if (isIgnoreUnusedMounts(qMount)){
                continue;
            }
        }
        NoVolumes_Mounts_Keys.append(qMount.mounted_root_uri());
    }
}

void GvfsMountManager::listMountsBylsblk()
{
    PartMan::Partition p;
    QString output;
    QString err;
    bool status = PartMan::SpawnCmd("lsblk", {"-O", "-J", "-l"},
                  output, err);
    if(status){
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(output.toLocal8Bit(),&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject devObj = doc.object();
            foreach (QString key, devObj.keys()) {
                if (key == "blockdevices"){
                    QJsonArray objArray = devObj.value(key).toArray();
                    for(int i=0; i< objArray.count(); i++){


                        QJsonObject obj = objArray.at(i).toObject();

                        if (obj.contains("mountpoint")){
                            QString mountPoint = obj.value("mountpoint").toString();
                            if (mountPoint.isEmpty() || mountPoint=="/"){
                                continue;
                            }else{
                                p.setMountPoint(obj.value("mountpoint").toString());
                            }
                        }

                        if (obj.contains("name")){
                            p.setName(obj.value("name").toString());
                        }
                        if (obj.contains("fstype")){
                            p.setFs(obj.value("fstype").toString());
                        }
                        if (obj.contains("label")){
                            p.setLabel(obj.value("label").toString());
                        }
                        if (obj.contains("uuid")){
                            p.setUuid(obj.value("uuid").toString());
                        }
                        if(obj.contains("rm")){
                            QVariant data(obj.value("rm").toVariant());
                            p.setIsRemovable(data.toBool());
                        }

                        p.setPath(QString("/dev/%1").arg(p.name()));

                        if (!p.fs().isEmpty()){
                            PartMan::ReadUsageManager readUsageManager;
                            qlonglong freespace = 0;
                            qlonglong total = 0;
                            bool ret = readUsageManager.readUsage(p.path(), p.fs(), freespace, total);
                            if (ret){
                                p.setFreespace(freespace);
                                p.setTotal(total);
                            }
                        }


                        QDiskInfo diskInfo;

                        diskInfo.setName(p.name());
                        diskInfo.setUnix_device(p.path());
                        diskInfo.setUuid(p.uuid());
                        diskInfo.setId(p.path());
                        diskInfo.setFree(p.freespace());
                        diskInfo.setTotal(p.total());
                        diskInfo.setIs_removable(p.getIsRemovable());
                        diskInfo.setMounted_root_uri(QString("file://%1").arg(p.mountPoint()));
                        diskInfo.setCan_unmount(true);
                        diskInfo.setCan_mount(false);
                        diskInfo.setCan_eject(false);
                        if (diskInfo.is_removable()){
                            diskInfo.setType("removable");
                        }else{
                            diskInfo.setType("native");
                        }
                        Lsblk_Keys.append(p.path());
                        DiskInfos.insert(diskInfo.id(), diskInfo);
                    }
                }
            }
        }else{
            qCDebug(mountManager()) << error.errorString();
        }
    }else{
        qCDebug(mountManager()) << status << output << err;
    }
}

bool GvfsMountManager::errorCodeNeedSilent(int errorCode)
{
    switch(errorCode) {
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
             info.type() != "dvd") && !info.id_filesystem().isEmpty()){
            if (info.type() == "network"){
                MountPointUrl = DUrl::fromLocalFile(QString("%1/%2%3").arg(path, info.id_filesystem(), DUrl(info.default_location()).path()));
            }else{
                MountPointUrl = DUrl::fromLocalFile(QString("%1/%2").arg(path, info.id_filesystem()));
            }
        }
    }

    return MountPointUrl;
}

void GvfsMountManager::autoMountAllDisks()
{
    // check if we are in live system, don't do auto mount if we are in live system.
    static QMap<QString, QString> cmdline = FileUtils::getKernelParameters();
    if (cmdline.value("boot", "") == QStringLiteral("live")) {
        return;
    }

    if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_AutoMount).toBool()) {
        foreach (const QDiskInfo& diskInfo, DiskInfos.values()) {
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
    }else if(!diskInfo.activation_root_uri().isEmpty()) {
        mount_mounted(diskInfo.activation_root_uri(), silent);
    }else{
        mount_device(diskInfo.unix_device(), silent);
    }
}

void GvfsMountManager::mount_mounted(const QString &mounted_root_uri, bool silent)
{
    std::string file_uri = mounted_root_uri.toStdString();
    const char *f = file_uri.data();
    GFile *file;
    file = g_file_new_for_uri(f);
    if (file == nullptr)
        return;
    GMountOperation* op;
    op = new_mount_op ();
    g_file_mount_enclosing_volume (file, G_MOUNT_MOUNT_NONE, op, nullptr, &GvfsMountManager::mount_with_mounted_uri_done, silent ? &silent : nullptr);
    g_object_unref (file);
}

void GvfsMountManager::mount_with_mounted_uri_done(GObject *object, GAsyncResult *res, gpointer silent)
{
    gboolean succeeded;
    GError *error = nullptr;

    succeeded = g_file_mount_enclosing_volume_finish (G_FILE (object), res, &error);

    if (!succeeded)
    {
        qCDebug(mountManager()) << "Error mounting location: " << error->message << error->code;
        if (!silent && !errorCodeNeedSilent(error->code)) {
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

    volumes = g_volume_monitor_get_volumes (volume_monitor);
    for (l = volumes; l != nullptr; l = l->next)
    {
        GVolume *volume = G_VOLUME (l->data);

        if (g_strcmp0 (g_volume_get_identifier (volume,
                                              G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE), device_file) == 0)
        {
            GMountOperation *op;

            op = new_mount_op ();

            g_volume_mount (volume,
                          G_MOUNT_MOUNT_NONE,
                          op,
                          nullptr,
                          &GvfsMountManager::mount_with_device_file_cb,
                          silent ? &silent : nullptr);

            outstanding_mounts++;
        }
    }
    g_list_free_full (volumes, g_object_unref);

    if (outstanding_mounts == 0)
    {
      qDebug() << "No volume for device file" << device_file;
      return;
    }
    g_object_unref (volume_monitor);
}

void GvfsMountManager::mount_with_device_file_cb(GObject *object, GAsyncResult *res, gpointer silent)
{
    GVolume *volume;
    gboolean succeeded;
    GError *error = nullptr;

    volume = G_VOLUME (object);

    succeeded = g_volume_mount_finish (volume, res, &error);

    if (!succeeded) {
        qCDebug(mountManager()) << "Error mounting: " << g_volume_get_identifier (volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE)
                                << error->message << silent << error->code;
        if (!silent && !errorCodeNeedSilent(error->code)) {
            fileSignalManager->requestShowErrorDialog(QString::fromLocal8Bit(error->message), QString(" "));
        }
    } else {
        GMount *mount;
        GFile *root;
        char *mount_path;
        mount = g_volume_get_mount (volume);
        root = g_mount_get_root (mount);
        mount_path = g_file_get_path (root);
        qCDebug(mountManager()) << "Mounted" << g_volume_get_identifier (volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) << "at" << mount_path;
        g_object_unref (mount);
        g_object_unref (root);
        g_free (mount_path);
    }
}

void GvfsMountManager::unmount(const QDiskInfo &diskInfo)
{
    if (diskInfo.can_unmount()  && !diskInfo.mounted_root_uri().isEmpty()){
        unmount_mounted(diskInfo.mounted_root_uri());
    }
}

void GvfsMountManager::unmount(const QString &id)
{
    QDiskInfo diskInfo = getDiskInfo(id);
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

    mount = g_file_find_enclosing_mount (file, nullptr, &error);
    if (mount == nullptr) {
        bool no_permission = false;

        QFileInfo fileInfo(QUrl(mounted_root_uri).toLocalFile());

        while (!fileInfo.exists() && fileInfo.fileName() != QDir::rootPath() && !fileInfo.absolutePath().isEmpty()) {
            fileInfo.setFile(fileInfo.absolutePath());
        }

        if (fileInfo.exists()) {
            if (getuid() == fileInfo.ownerId()) {
                if (!fileInfo.permission(QFile::ReadOwner | QFile::ExeOwner))
                    no_permission = true;
            } else if (getgid() == fileInfo.groupId()) {
                if (!fileInfo.permission(QFile::ReadGroup | QFile::ExeGroup))
                    no_permission = true;
            } else if (!fileInfo.permission(QFile::ReadOther | QFile::ExeOther)) {
                no_permission = true;
            }
        }

        if (no_permission) {
            QString user_name = fileInfo.owner();

            if (fileInfo.absoluteFilePath().startsWith("/media/")) {
                user_name = fileInfo.baseName();
            }

            DDialog error_dilaog(tr("The disk is mounted by user \"%1\", you cannot unmount it.").arg(user_name), QString(" "));

            error_dilaog.setIcon(QIcon::fromTheme("dialog-error"), QSize(64, 64));
            error_dilaog.addButton(tr("Confirm"), true, DDialog::ButtonRecommend);
            error_dilaog.setModal(true);
            error_dilaog.exec();

            return;
        }

        DDialog error_dilaog(tr("Cannot find the mounting device"), QString(error->message));

        error_dilaog.setIcon(QIcon::fromTheme("dialog-error"), QSize(64, 64));
        error_dilaog.addButton(tr("Confirm"), true, DDialog::ButtonRecommend);
        error_dilaog.setModal(true);
        error_dilaog.exec();

        return;
    }

    bool is_netwrok_url = mounted_root_uri.startsWith("smb://") || mounted_root_uri.startsWith("sftp://") || mounted_root_uri.startsWith("ftp://");
    char *local_mount_point = is_netwrok_url ? g_file_get_path(file) : nullptr;

    mount_op = new_mount_op ();
    g_mount_unmount_with_operation (mount, G_MOUNT_UNMOUNT_NONE, mount_op, nullptr, &GvfsMountManager::unmount_done_cb, local_mount_point);
    g_object_unref (mount_op);

    g_object_unref (file);
}


void GvfsMountManager::unmount_done_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    gboolean succeeded;
    GError *error = nullptr;

    succeeded = g_mount_unmount_with_operation_finish (G_MOUNT (object), res, &error);

    if (!succeeded) {
        DDialog error_dilaog(tr("Cannot unmount the device"), QString(error->message));

        error_dilaog.setIcon(QIcon::fromTheme("dialog-error"), QSize(64, 64));
        error_dilaog.addButton(tr("Confirm"), true, DDialog::ButtonRecommend);
        error_dilaog.setModal(true);
        error_dilaog.exec();
    } else {
        char *local_mount_point = reinterpret_cast<char*>(user_data);

        if (local_mount_point) {
            // 由于卸载gvfs设备时不会触发文件系统的inotify, 所以此处手动模拟文件夹被移除的信号
            const DUrl url = DUrl::fromLocalFile(local_mount_point);
            DAbstractFileWatcher::ghostSignal(url.parentUrl(), &DAbstractFileWatcher::fileDeleted, url);

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

            for (const DUrl &url : dirty_list)
                NetworkManager::NetworkNodes.remove(url);
        }

        g_free(root_uri);
        g_object_unref(root_file);
    }

    g_object_unref (G_MOUNT (object));
}

void GvfsMountManager::eject(const QString &path)
{
    foreach (const QDiskInfo& diskInfo, DiskInfos.values()) {
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
    if (!diskInfo.mounted_root_uri().isEmpty()){
        eject_mounted(diskInfo.mounted_root_uri());
    }else{
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

    volumes = g_volume_monitor_get_volumes (volume_monitor);
    for (l = volumes; l != nullptr; l = l->next)
    {
        GVolume *volume = G_VOLUME (l->data);

        if (g_strcmp0 (g_volume_get_identifier (volume,
                                              G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE), device_file) == 0)
        {
            GMountOperation *op;

            op = new_mount_op ();

            g_volume_eject_with_operation (volume,
                          G_MOUNT_UNMOUNT_NONE,
                          op,
                          nullptr,
                          &GvfsMountManager::eject_with_device_file_cb,
                          op);

            outstanding_mounts++;
        }
    }
    g_list_free_full (volumes, g_object_unref);

    if (outstanding_mounts == 0) {
        qCDebug(mountManager()) << "No volume for device file: " << device_file;
        return;
    }
    g_object_unref (volume_monitor);
}

void GvfsMountManager::eject_with_device_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    GVolume *volume;
    gboolean succeeded;
    GError *error = nullptr;

    volume = G_VOLUME (object);

    succeeded = g_volume_eject_with_operation_finish (volume, res, &error);

    if (!succeeded) {
        DDialog error_dilaog(tr("Cannot eject the device \"%1\"").arg(g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE)),
                             QString(error->message));

        error_dilaog.setIcon(QIcon::fromTheme("dialog-error"), QSize(64, 64));
        error_dilaog.addButton(tr("Confirm"), true, DDialog::ButtonRecommend);
        error_dilaog.setModal(true);
        error_dilaog.exec();
    } else {
        qCDebug(mountManager()) << "eject" <<  g_volume_get_identifier (volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) << "succeeded";
    }
}

void GvfsMountManager::eject_mounted(const QString &mounted_root_uri)
{
    if (mounted_root_uri.isEmpty())
        return;

    std::string file_uri = mounted_root_uri.toStdString();
    GFile *file;
    file = g_file_new_for_uri(file_uri.data());
    if (file == nullptr)
        return;

    GMount *mount;
    GError *error = nullptr;
    GMountOperation *mount_op;

    if (file == nullptr)
        return;

    mount = g_file_find_enclosing_mount (file, nullptr, &error);
    if (mount == nullptr) {
        DDialog error_dilaog(tr("Cannot find the mounting device"), QString(error->message));

        error_dilaog.setIcon(QIcon::fromTheme("dialog-error"), QSize(64, 64));
        error_dilaog.addButton(tr("Confirm"), true, DDialog::ButtonRecommend);
        error_dilaog.setModal(true);
        error_dilaog.exec();

        return;
    }

    mount_op = new_mount_op ();
    g_mount_eject_with_operation (mount, G_MOUNT_UNMOUNT_NONE, mount_op, nullptr, &GvfsMountManager::eject_with_mounted_file_cb, nullptr);
    g_object_unref (mount_op);

}

void GvfsMountManager::eject_with_mounted_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    GMount* mount;
    gboolean succeeded;
    GError *error = nullptr;

    mount = G_MOUNT (object);
    succeeded = g_mount_eject_with_operation_finish (G_MOUNT (object), res, &error);

    g_object_unref (G_MOUNT (object));

    if (!succeeded) {
        qDebug() << "Error ejecting mount:" << error->message;
    } else {
        qCDebug(mountManager()) << "eject" << g_mount_get_name (mount)  << "succeeded";
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

    drives = g_volume_monitor_get_connected_drives( volume_monitor);
    GDrive *drive;
    for (i = 0, d = drives; d != nullptr; d = d->next, i++){
        drive = (GDrive *) d->data;

        if (g_strcmp0 (g_drive_get_identifier(drive, "unix-device"), device_file) == 0)
        {
            GMountOperation *op;

            op = new_mount_op ();

            g_drive_stop (drive,
                          G_MOUNT_UNMOUNT_NONE,
                          op,
                          nullptr,
                          &GvfsMountManager::stop_with_device_file_cb,
                          op);
        }

    }
    g_list_free_full (drives, g_object_unref);
    g_object_unref (volume_monitor);
}

void GvfsMountManager::stop_with_device_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    GDrive *drive;
    gboolean succeeded;
    GError *error = nullptr;

    drive = G_DRIVE (object);

    succeeded = g_drive_stop_finish (drive, res, &error);

    if (!succeeded) {
        qDebug() << "Error remove disk:" << g_drive_get_identifier (drive, "unix-device") << error->message;
    } else {
        qCDebug(mountManager()) << "Safely remove disk" <<  g_drive_get_identifier (drive, "unix-device") << "succeeded";
    }
}
