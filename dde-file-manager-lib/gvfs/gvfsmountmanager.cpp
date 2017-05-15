#include "gvfsmountmanager.h"
#include "qdrive.h"
#include "qvolume.h"
#include "qmount.h"
#include "qdiskinfo.h"
#include "singleton.h"
#include "../app/define.h"
#include "../partman/partition.h"
#include "../partman/readusagemanager.h"
#include "../interfaces/dfmsetting.h"
#include "../interfaces/dfileservices.h"
#include "../interfaces/dfmevent.h"
#include "partman/command.h"

#include <QThread>
#include <QApplication>

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

GvfsMountManager::GvfsMountManager(QObject *parent) : QObject(parent)
{
    m_gVolumeMonitor = g_volume_monitor_get();

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
        g_signal_connect (m_gVolumeMonitor, "mount-added", (GCallback)&GvfsMountManager::monitor_mount_added_root, NULL);
        g_signal_connect (m_gVolumeMonitor, "mount-removed", (GCallback)&GvfsMountManager::monitor_mount_removed_root, NULL);
    }else{
        g_signal_connect (m_gVolumeMonitor, "mount-added", (GCallback)&GvfsMountManager::monitor_mount_added, NULL);
        g_signal_connect (m_gVolumeMonitor, "mount-removed", (GCallback)&GvfsMountManager::monitor_mount_removed, NULL);
        g_signal_connect (m_gVolumeMonitor, "mount-changed", (GCallback)&GvfsMountManager::monitor_mount_changed, NULL);
        g_signal_connect (m_gVolumeMonitor, "volume-added", (GCallback)&GvfsMountManager::monitor_volume_added, NULL);
        g_signal_connect (m_gVolumeMonitor, "volume-removed", (GCallback)&GvfsMountManager::monitor_volume_removed, NULL);
    }
    //    g_signal_connect (m_gVolumeMonitor, "volume-changed", (GCallback)&GvfsMountManager::monitor_volume_changed, NULL);
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
    names = NULL;
    g_object_get (icon, "names", &names, NULL);
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
    if (ids && ids[0] != NULL){
        for (int i = 0; ids[i] != NULL; i++){
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
    if (sort_key != NULL){
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
    if (ids && ids[0] != NULL){
        for (int i = 0; ids[i] != NULL; i++){
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
    if (sort_key != NULL){
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
    if (activation_root != NULL){
        action_root_uri = g_file_get_uri (activation_root);
        qVolume.setActivation_root_uri(QString(action_root_uri));
        g_object_unref (activation_root);
        g_free(action_root_uri);
    }

    GDrive *gDrive = g_volume_get_drive(volume);
    if (gDrive){
        QDrive qDrive = gDriveToqDrive(gDrive);
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
    if (sort_key != NULL){
        qMount.setSort_key(QString(sort_key));
    }

    return qMount;
}

QDiskInfo GvfsMountManager::qVolumeToqDiskInfo(const QVolume &volume)
{
    QDiskInfo diskInfo;
    diskInfo.setId(volume.unix_device());

    diskInfo.setName(volume.name());

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

    PartMan::Partition partion = PartMan::Partition::getPartitionByDevicePath(volume.unix_device());
    if (diskInfo.iconName() == "phone-apple-iphone"){
        diskInfo.setType("iphone");
    }else if (diskInfo.iconName() == "phone"){
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

void GvfsMountManager::monitor_mount_added_root(GVolumeMonitor *volume_monitor, GMount *mount)
{
    Q_UNUSED(volume_monitor)
    qDebug() << "==============================monitor_mount_added_root==============================";
    QMount qMount = gMountToqMount(mount);
    qDebug() << qMount;

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
    qDebug() << "==============================monitor_mount_removed_root==============================";
    QMount qMount = gMountToqMount(mount);
    qDebug() << qMount;
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
    qDebug() << "==============================monitor_mount_added==============================";
    QMount qMount = gMountToqMount(mount);

    GVolume *volume = g_mount_get_volume(mount);
    qDebug() << "===================" << qMount.mounted_root_uri() << volume << "=======================";
    qDebug() << "===================" << qMount << "=======================";
    if (volume != NULL){
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
        qDebug() << "========mount_added===========" << diskInfo;
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
    qDebug() << "==============================monitor_mount_removed==============================" ;
    QMount qMount = gMountToqMount(mount);

    qDebug() << "===================" << qMount.mounted_root_uri() << "=======================";

    QVolume volume = getVolumeByMountedRootUri(qMount.mounted_root_uri());
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
        }else{
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
    qDebug() << "==============================monitor_mount_changed==============================" ;
    GVolume *volume = g_mount_get_volume(mount);
    if (volume != NULL){
        qDebug() << "==============================changed removed==============================" ;

        QVolume qVolume = gVolumeToqVolume(volume);
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);

        bool isDVDChanged = isDVD(qVolume);
        if (isDVDChanged){
            diskInfo.setType("dvd");
            qDebug() << diskInfo;
            if (diskInfo.can_unmount()){
                diskInfo.updateGvfsFileSystemInfo();
                emit gvfsMountManager->volume_changed(diskInfo);
            }
        }
    }else{
        qDebug() << "==============================changed volume empty==============================" ;

    }
}

void GvfsMountManager::monitor_volume_added(GVolumeMonitor *volume_monitor, GVolume *volume)
{
    Q_UNUSED(volume_monitor)
    qDebug() << "==============================monitor_volume_added==============================" ;
    QVolume qVolume = gVolumeToqVolume(volume);

    qDebug() << "===================" << qVolume.unix_device() << "=======================";

    Volumes.insert(qVolume.unix_device(), qVolume);

    GDrive *drive = g_volume_get_drive(volume);
    if (drive){
        QDrive qDrive = gDriveToqDrive(drive);
        Drives.insert(qDrive.unix_device(), qDrive);
        if (!Volumes_Drive_Keys.contains(qDrive.unix_device())){
            Volumes_Drive_Keys.append(qDrive.unix_device());
        }
    }

    QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);

    if (diskInfo.type() == "iphone"){
        if (diskInfo.activation_root_uri() != QString("afc://%1/").arg(diskInfo.uuid())){
            return;
        }
    }

    DiskInfos.insert(diskInfo.id(), diskInfo);
    emit gvfsMountManager->volume_added(diskInfo);
}

void GvfsMountManager::monitor_volume_removed(GVolumeMonitor *volume_monitor, GVolume *volume)
{
    Q_UNUSED(volume_monitor)
    qDebug() << "==============================monitor_volume_removed==============================" ;
    QVolume qVolume = gVolumeToqVolume(volume);

    qDebug() << "===================" << qVolume.unix_device() << "=======================";

    GDrive *drive = g_volume_get_drive(volume);
    if (drive){
        QDrive qDrive = gDriveToqDrive(drive);
        Drives.insert(qDrive.unix_device(), qDrive);
        Volumes_Drive_Keys.removeOne(qDrive.unix_device());
    }

    bool removed = Volumes.remove(qVolume.unix_device());
    if (removed){
        QDiskInfo diskInfo = qVolumeToqDiskInfo(qVolume);
        bool diskInfoRemoved = DiskInfos.remove(diskInfo.id());
        if (diskInfoRemoved){
            emit gvfsMountManager->volume_removed(diskInfo);
        }
    }
}

void GvfsMountManager::monitor_volume_changed(GVolumeMonitor *volume_monitor, GVolume *volume)
{
    Q_UNUSED(volume_monitor)
    Q_UNUSED(volume)
    qDebug() << "==============================monitor_volume_changed==============================" ;
}

GMountOperation *GvfsMountManager::new_mount_op()
{
    GMountOperation *op;

    op = g_mount_operation_new ();

    g_signal_connect (op, "ask_password", G_CALLBACK (&GvfsMountManager::ask_password_cb), NULL);

    /* TODO: we *should* also connect to the "aborted" signal but since the
     *       main thread is blocked handling input we won't get that signal
     *       anyway...
     */

    return op;
}

void GvfsMountManager::ask_password_cb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags)
{
    char *s;
    g_print ("%s\n", message);

    if (flags & G_ASK_PASSWORD_NEED_USERNAME)
    {
        s = "default_user";
        g_mount_operation_set_username (op, s);
        g_free (s);
    }

    if (flags & G_ASK_PASSWORD_NEED_DOMAIN)
    {
        s = "default_domain";
        g_mount_operation_set_domain (op, s);
        g_free (s);
    }

    if (flags & G_ASK_PASSWORD_NEED_PASSWORD)
    {
        s = "";
        g_mount_operation_set_password (op, s);
        g_free (s);
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
    for (c = 0, d = drives; d != NULL; d = d->next, c++){
        drive = (GDrive *) d->data;
        QDrive qDrive = gDriveToqDrive(drive);


        Drives.insert(qDrive.unix_device(), qDrive);
        Drives_Keys.append(qDrive.unix_device());

        volumes = g_drive_get_volumes (drive);
        GVolume *volume;
        for (j = 0, v = volumes; v != NULL; v = v->next, j++){
            volume = (GVolume *) v->data;

            ids = g_volume_enumerate_identifiers (volume);
            if (ids && ids[0] != NULL){
                for (k = 0; ids[k] != NULL; k++){
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

    for (int c = 0; v != NULL; v = v->next, c++){
        volume = (GVolume *) v->data;
        QVolume qVolume = gVolumeToqVolume(volume);

        Volumes.insert(qVolume.unix_device(), qVolume);

        GDrive *drive = g_volume_get_drive(volume);
        if (drive != NULL){
            continue;
        }else{
            if (!Volumes_No_Drive_Keys.contains(qVolume.unix_device())){
                Volumes_No_Drive_Keys.append(qVolume.unix_device());
            }
        }
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
            qDebug() << diskInfo;
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
            qDebug() << diskInfo;
        }
    }
    NoVolumes_Mounts_Keys.sort();
    foreach (QString key, NoVolumes_Mounts_Keys) {
        if (Mounts.contains(key)){
            QMount mount = Mounts.value(key);
            QDiskInfo diskInfo = qMountToqDiskinfo(mount);
            DiskInfos.insert(diskInfo.id(), diskInfo);
            qDebug() << diskInfo;
        }
    }
    qDebug() << Mounts;
}

void GvfsMountManager::getMounts(GList *mounts)
{
    GList *m = mounts;
    GMount *mount;
    for (int c = 0; m != NULL; m = m->next, c++){
        mount = (GMount *) m->data;
        QMount qMount = gMountToqMount(mount);
        Mounts.insert(qMount.mounted_root_uri(), qMount);
        GVolume* volume = g_mount_get_volume(mount);
        if (volume != NULL){
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
                            QString data = obj.value("rm").toString();
                            if(data == "1")
                                p.setIsRemovable(true);
                            else
                                p.setIsRemovable(false);
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
            qDebug() << error.errorString();
        }
    }else{
        qDebug() << status << output << err;
    }
}

bool GvfsMountManager::getAutoMountSwitch() const
{
    return m_autoMountSwitch;
}

void GvfsMountManager::setAutoMountSwitch(bool autoMountSwitch)
{
    m_autoMountSwitch = autoMountSwitch;
}

void GvfsMountManager::mount(const QString &path)
{
    QDiskInfo diskInfo = getDiskInfo(path);
    mount(diskInfo);
}

void GvfsMountManager::mount(const QDiskInfo &diskInfo)
{
    if (!diskInfo.mounted_root_uri().isEmpty()){
        mount_mounted(diskInfo.mounted_root_uri());
    }else if(!diskInfo.activation_root_uri().isEmpty()) {
        mount_mounted(diskInfo.activation_root_uri());
    }else{
        mount_device(diskInfo.unix_device());
    }
}

void GvfsMountManager::mount_mounted(const QString &mounted_root_uri)
{
    std::string file_uri = mounted_root_uri.toStdString();
    const char *f = file_uri.data();
    GFile *file;
    file = g_file_new_for_uri(f);
    if (file == NULL)
        return;
    GMountOperation* op;
    op = new_mount_op ();
    g_file_mount_enclosing_volume (file, G_MOUNT_MOUNT_NONE, op, NULL, &GvfsMountManager::mount_with_mounted_uri_done, op);
    g_object_unref (file);
}

void GvfsMountManager::mount_with_mounted_uri_done(GObject *object, GAsyncResult *res, gpointer user_data)
{
    gboolean succeeded;
    GError *error = NULL;

    succeeded = g_file_mount_enclosing_volume_finish (G_FILE (object), res, &error);

    if (!succeeded)
    {
        qDebug() << "Error mounting location: "<< error->message;
    }
}

void GvfsMountManager::mount_device(const QString &unix_device)
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
    for (l = volumes; l != NULL; l = l->next)
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
                          NULL,
                          &GvfsMountManager::mount_with_device_file_cb,
                          op);

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

void GvfsMountManager::mount_with_device_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    GVolume *volume;
    gboolean succeeded;
    GError *error = NULL;

    volume = G_VOLUME (object);

    succeeded = g_volume_mount_finish (volume, res, &error);

    if (!succeeded)
    {
          qDebug() << "Error mounting :" << g_volume_get_identifier (volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) << error->message;
    }else{
          GMount *mount;
          GFile *root;
          char *mount_path;
          mount = g_volume_get_mount (volume);
          root = g_mount_get_root (mount);
          mount_path = g_file_get_path (root);
          qDebug() << "Mounted" << g_volume_get_identifier (volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) << "at" << mount_path;
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
    std::string file_uri = mounted_root_uri.toStdString();
    GFile *file;
    file = g_file_new_for_uri(file_uri.data());
    if (file == NULL)
        return;

    GMount *mount;
    GError *error = NULL;
    GMountOperation *mount_op;

    mount = g_file_find_enclosing_mount (file, NULL, &error);
    if (mount == NULL){
      qDebug() << "Error finding enclosing mount:" << error->message;
      return;
    }

    mount_op = new_mount_op ();
    g_mount_unmount_with_operation (mount, G_MOUNT_UNMOUNT_NONE, mount_op, NULL, &GvfsMountManager::unmount_done_cb, NULL);
    g_object_unref (mount_op);

    g_object_unref (file);
}


void GvfsMountManager::unmount_done_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    gboolean succeeded;
    GError *error = NULL;

    succeeded = g_mount_unmount_with_operation_finish (G_MOUNT (object), res, &error);

    g_object_unref (G_MOUNT (object));

    if (!succeeded)
    {
        qDebug() << "Error unmounting mount: " << error->message;
    }
}

void GvfsMountManager::eject(const QString &path)
{
    foreach (const QDiskInfo& diskInfo, DiskInfos.values()) {
        if (!path.isEmpty() && diskInfo.can_eject() && diskInfo.mounted_root_uri() == path){
            eject_mounted(diskInfo.mounted_root_uri());
            return;
        }else if (!path.isEmpty() && diskInfo.can_eject() && path == diskInfo.unix_device()){
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
    for (l = volumes; l != NULL; l = l->next)
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
                          NULL,
                          &GvfsMountManager::eject_with_device_file_cb,
                          op);

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

void GvfsMountManager::eject_with_device_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    GVolume *volume;
    gboolean succeeded;
    GError *error = NULL;

    volume = G_VOLUME (object);

    succeeded = g_volume_eject_with_operation_finish (volume, res, &error);

    if (!succeeded)
    {
        qDebug() << "Error eject :" << g_volume_get_identifier (volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) << error->message;
    }else{
        qDebug() << "eject" <<  g_volume_get_identifier (volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) << "succeeded";
    }
}

void GvfsMountManager::eject_mounted(const QString &mounted_root_uri)
{
    if (mounted_root_uri.isEmpty())
        return;

    std::string file_uri = mounted_root_uri.toStdString();
    GFile *file;
    file = g_file_new_for_uri(file_uri.data());
    if (file == NULL)
        return;

    GMount *mount;
    GError *error = NULL;
    GMountOperation *mount_op;

    if (file == NULL)
        return;

    mount = g_file_find_enclosing_mount (file, NULL, &error);
    if (mount == NULL)
    {
      qDebug() << "Error finding enclosing mount:" << error->message;
      return;
    }

    mount_op = new_mount_op ();
    g_mount_eject_with_operation (mount, G_MOUNT_UNMOUNT_NONE, mount_op, NULL, &GvfsMountManager::eject_with_mounted_file_cb, NULL);
    g_object_unref (mount_op);

}

void GvfsMountManager::eject_with_mounted_file_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data)
    GMount* mount;
    gboolean succeeded;
    GError *error = NULL;

    mount = G_MOUNT (object);
    succeeded = g_mount_eject_with_operation_finish (G_MOUNT (object), res, &error);

    g_object_unref (G_MOUNT (object));

    if (!succeeded)
    {
        qDebug() << "Error ejecting mount:" << error->message;
    }else{
        qDebug() << "eject" << g_mount_get_name (mount)  << "succeeded";
    }
}
