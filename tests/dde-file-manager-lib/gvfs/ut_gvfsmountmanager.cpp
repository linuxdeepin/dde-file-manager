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

#include <gtest/gtest.h>
#include "gvfs/qdrive.h"
#include "gvfs/qvolume.h"
#include "gvfs/qdiskinfo.h"
#include "gvfs/qmount.h"
#define private public
#define protected public
#include "gvfs/gvfsmountmanager.h"
#include "testhelper.h"
#include "stub.h"
#include "stubext.h"

#include <DDialog>

static bool inited = false;

namespace {

typedef int(*fptr)(Dtk::Widget::DDialog*);
fptr pDDialogExec = (fptr)(&Dtk::Widget::DDialog::exec);
Stub stub;

class TestGvfsMountManager: public testing::Test
{
public:
    GvfsMountManager *m_manager = GvfsMountManager::instance();
    static void SetUpTestCase()
    {
        int (*stub_DDialog_exec)(void) = [](void)->int{return Dtk::Widget::DDialog::Accepted;};
        stub.set(pDDialogExec, stub_DDialog_exec);
    }

    static void TearDownTestCase()
    {
        // 等待所有弹框处理完毕再还原打桩
        TestHelper::runInLoop([=](){
        }, 5000);

        stub.reset(pDDialogExec);
    }

    void SetUp() override
    {
        if(!inited)
        {
            m_manager->startMonitor();
            inited = true;
        }

    }

    void TearDown() override
    {
    }

    char errBuffer[2];
};
}

GMount *get_fisrt_usable_mount(void)
{
    //TODO get GMount for /dev/sr0
    GList *mounts = g_volume_monitor_get_mounts(GvfsMountManager::instance()->m_gVolumeMonitor);
    GVolume *rvolume = nullptr;
    GMount *rmount = nullptr;
    GDrive *rdrive = nullptr;

    GList *m = mounts;
    for (int c = 0; m != nullptr; m = m->next, c++) {
        GMount *mount = static_cast<GMount *>(m->data);
        rvolume = g_mount_get_volume(mount);
        if(rvolume != nullptr) {
            rdrive = g_volume_get_drive(rvolume);
            if(rdrive != nullptr) {
                //have {rdrive rvolume mount}
                rmount = mount;
                break;
            }
        }
    }
    g_list_free_full(mounts, g_object_unref);

    return rmount;
}

GVolume *get_fisrt_usable_volume(void)
{
    //TODO get GMount for /dev/sr0
    GList *mounts = g_volume_monitor_get_mounts(GvfsMountManager::instance()->m_gVolumeMonitor);
    GVolume *rvolume = nullptr;
    GMount *rmount = nullptr;
    GDrive *rdrive = nullptr;

    GList *m = mounts;
    for (int c = 0; m != nullptr; m = m->next, c++) {
        GMount *mount = static_cast<GMount *>(m->data);
        rvolume = g_mount_get_volume(mount);
        if(rvolume != nullptr) {
            rdrive = g_volume_get_drive(rvolume);
            if(rdrive != nullptr) {
                //have {rdrive rvolume mount}
                rmount = mount;
                break;
            }
        }
    }
    g_list_free_full(mounts, g_object_unref);

    return rvolume;
}

TEST_F(TestGvfsMountManager, gVolumeToqVolume)
{
    GVolume* vol = get_fisrt_usable_volume();
    char **(*stub_g_volume_enumerate_identifiers)(GVolume *) = [](GVolume *)->char **{
        char **info_table = (char**)malloc(5 * sizeof(char*));

        info_table[0] = (char *)malloc(strlen(G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) + 1);
        info_table[1] = (char *)malloc(strlen(G_VOLUME_IDENTIFIER_KIND_LABEL) + 1);
        info_table[2] = (char *)malloc(strlen(G_VOLUME_IDENTIFIER_KIND_UUID) + 1);
        info_table[3] = (char *)malloc(strlen(G_VOLUME_IDENTIFIER_KIND_NFS_MOUNT) + 1);
        info_table[4] = nullptr;

        memset(info_table[0], 0, strlen(G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE) + 1);
        memset(info_table[0], 0, strlen(G_VOLUME_IDENTIFIER_KIND_LABEL) + 1);
        memset(info_table[0], 0, strlen(G_VOLUME_IDENTIFIER_KIND_UUID) + 1);
        memset(info_table[0], 0, strlen(G_VOLUME_IDENTIFIER_KIND_NFS_MOUNT) + 1);

        strcpy(info_table[0], G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        strcpy(info_table[1], G_VOLUME_IDENTIFIER_KIND_LABEL);
        strcpy(info_table[2], G_VOLUME_IDENTIFIER_KIND_UUID);
        strcpy(info_table[3], G_VOLUME_IDENTIFIER_KIND_NFS_MOUNT);

        return info_table;
    };

    char *(*stub_g_volume_get_identifier) (GVolume *volume, const char *kind) = [](GVolume *volume, const char *kind)->char* {
        char *id = (char *)malloc(strlen("id") + 1);
        memset(id, 0, strlen("id") + 1);
        strcpy(id, "id");
        return id;
    };

    stub.set(g_volume_enumerate_identifiers, stub_g_volume_enumerate_identifiers);
    GvfsMountManager::gVolumeToqVolume(vol);
    stub.reset(g_volume_enumerate_identifiers);
}

TEST_F(TestGvfsMountManager, qVolumeToqDiskInfo)
{
    QVolume volume;
    volume.setUnix_device("abc");
    volume.setUuid("10011");
    QDiskInfo info = GvfsMountManager::qVolumeToqDiskInfo(volume);
    EXPECT_STREQ(info.id().toStdString().c_str(), volume.unix_device().toStdString().c_str());
    EXPECT_STREQ(info.uuid().toStdString().c_str(), volume.uuid().toStdString().c_str());

    QString(*stub_iconName_iphone)(void*) = [](void*)->QString{return "phone-apple-iphone";};
    stub.set(&QDiskInfo::iconName, stub_iconName_iphone);
    info = GvfsMountManager::qVolumeToqDiskInfo(volume);
    stub.reset(&QDiskInfo::iconName);

    QString(*stub_iconName_camera)(void*) = [](void*)->QString{return "camera-photo";};
    stub.set(&QDiskInfo::iconName, stub_iconName_camera);
    info = GvfsMountManager::qVolumeToqDiskInfo(volume);
    stub.reset(&QDiskInfo::iconName);

    stub_ext::StubExt stubx;
    stubx.set_lamda(&QDiskInfo::iconName, [](void*)->QString{return "multimedia-player";});
    stubx.set_lamda(&QDrive::is_removable, [](void*)->bool{return true;});
    info = GvfsMountManager::qVolumeToqDiskInfo(volume);
    stubx.reset(&QDrive::is_removable);
    stubx.reset(&QDiskInfo::iconName);

    bool(*stub_can_eject)(void*) = [](void*)->bool{return true;};
    QString(*stub_iconName_harddisk)(void*) = [](void*)->QString{return "drive-harddisk-usb";};
    stub.set(&QDiskInfo::can_eject, stub_can_eject);
    stub.set(&QDiskInfo::iconName, stub_iconName_harddisk);
    info = GvfsMountManager::qVolumeToqDiskInfo(volume);
    stub.reset(&QDiskInfo::can_eject);
    stub.reset(&QDiskInfo::iconName);
}

TEST_F(TestGvfsMountManager, qMountToqDiskinfo)
{
    QMount mount;
    mount.setMounted_root_uri("123");
    QDiskInfo info = GvfsMountManager::qMountToqDiskinfo(mount);
    EXPECT_STREQ(info.id().toStdString().c_str(), mount.mounted_root_uri().toStdString().c_str());

    QString(*stub_mounted_root_uri)(void*) = [](void*)->QString{return "smb://";};
    stub.set(&QDiskInfo::mounted_root_uri, stub_mounted_root_uri);
    info = GvfsMountManager::qMountToqDiskinfo(mount);
    stub.reset(&QDiskInfo::mounted_root_uri);

    QString(*stub_iconName)(void*) = [](void*)->QString{return "drive-optical";};
    stub.set(&QDiskInfo::iconName, stub_iconName);
    info = GvfsMountManager::qMountToqDiskinfo(mount);
    stub.reset(&QDiskInfo::iconName);
}

GMount *getMounofDevSr0()
{
    //TODO get GMount for /dev/sr0
    GList *mounts = g_volume_monitor_get_mounts(GvfsMountManager::instance()->m_gVolumeMonitor);
    GVolume *rvolume = nullptr;
    GMount *rmount = nullptr;
    GDrive *rdrive = nullptr;

    GList *m = mounts;
    for (int c = 0; m != nullptr; m = m->next, c++) {
        GMount *mount = static_cast<GMount *>(m->data);

        char* mname = g_mount_get_name(mount);
        //printf("---mname  %s\n",   mname);
        rvolume = g_mount_get_volume(mount);
        if(rvolume != nullptr) {
            char* dname = g_volume_get_name(rvolume);
            //printf("---dname  %s\n",   dname);
            rdrive = g_volume_get_drive(rvolume);
            if(rdrive != nullptr) {
                char* drvname = g_drive_get_name(rdrive);
                //printf("---drvname  %s\n",   drvname);
            }
            rmount = mount;
            //break;
        }
    }
    g_list_free_full(mounts, g_object_unref);

    return rmount;
}


TEST_F(TestGvfsMountManager, volumeMonitor)
{
    GList *mounts = g_volume_monitor_get_mounts(GvfsMountManager::instance()->m_gVolumeMonitor);
    GVolume *rvolume = nullptr;
    GMount *rmount = nullptr;

    GList *m = mounts;
    for (int c = 0; m != nullptr; m = m->next, c++) {
        GMount *mount = static_cast<GMount *>(m->data);
        rvolume = g_mount_get_volume(mount);
        if(rvolume != nullptr) {
            rmount = mount;
            break;
        }
    }
    g_list_free_full(mounts, g_object_unref);

    if(rmount != nullptr) {
        QString info2 = GvfsMountManager::getVolTag(rmount);
    }

    if(rvolume != nullptr) {
        QString info1 = GvfsMountManager::getVolTag(rvolume);
    }

    if(rmount == nullptr || rvolume == nullptr) {
        return;
    }

    QMount qMount = GvfsMountManager::gMountToqMount(rmount);
    QDiskInfo diskInfo = GvfsMountManager::qMountToqDiskinfo(qMount);

    GvfsMountManager::mount(diskInfo,true);
    GvfsMountManager::mount(diskInfo,false);

    GFile *file = g_mount_get_root (rmount);
    static QString rpath = "/dev/NOWAY";
    char* path = g_file_get_path(file);
    if(path != nullptr) {
        rpath = path;
    }
    GvfsMountManager::mount(rpath, true);
    GvfsMountManager::mount(rpath, false);
    GvfsMountManager::getDiskInfo(rpath, true);
    GvfsMountManager::getDiskInfo(rpath, false);

    QString (QDiskInfo::*mounted_root_uri)() const = &QDiskInfo::mounted_root_uri;
    QString (QDiskInfo::*activation_root_uri)() const = &QDiskInfo::activation_root_uri;

    stub_ext::StubExt stubx;
    stubx.set_lamda(&QDiskInfo::mounted_root_uri, [](void*)->QString{return "";});
    stubx.set_lamda(&QDiskInfo::activation_root_uri, [](void*)->QString{return "notempty";});
    GvfsMountManager::mount(rpath, false);
    stubx.reset(&QDiskInfo::mounted_root_uri);
    stubx.reset(&QDiskInfo::activation_root_uri);


    QString(*stub_id)(void*) = [](void*)->QString{return rpath;};
    stub.set(&QDiskInfo::id, stub_id);
    GvfsMountManager::getDiskInfo(rpath, false);
    stub.reset(&QDiskInfo::id);

    QString(*stub_mounted_root_uri)(void*) = [](void*)->QString{return rpath;};
    stub.set(&QDiskInfo::mounted_root_uri, stub_mounted_root_uri);
    GvfsMountManager::getDiskInfo(rpath, false);
    stub.reset(&QDiskInfo::mounted_root_uri);

    QString(*stub_unix_device)(void*) = [](void*)->QString{return rpath;};
    stub.set(&QDiskInfo::unix_device, stub_unix_device);
    GvfsMountManager::getDiskInfo(rpath, false);
    stub.reset(&QDiskInfo::unix_device);

    QString(*stub_uuid)(void*) = [](void*)->QString{return rpath;};
    stub.set(&QDiskInfo::uuid, stub_uuid);
    GvfsMountManager::getDiskInfo(rpath, false);
    stub.reset(&QDiskInfo::uuid);

    GvfsMountManager::mount_mounted(diskInfo.mounted_root_uri(), true);
    GvfsMountManager::mount_mounted(diskInfo.mounted_root_uri(), false);
    GvfsMountManager::mount_device(diskInfo.unix_device(), true);
    GvfsMountManager::mount_device(diskInfo.unix_device(), false);
    GvfsMountManager::isDeviceCrypto_LUKS(diskInfo);

    bool (*stub_can_mount)(void*) = [](void*)->bool{return true;};
    stub.set(&QDiskInfo::can_mount, stub_can_mount);
    GvfsMountManager::isDeviceCrypto_LUKS(diskInfo);
    stub.reset(&QDiskInfo::can_mount);

    rmount = get_fisrt_usable_mount();
    qMount = GvfsMountManager::gMountToqMount(rmount);
    diskInfo = GvfsMountManager::qMountToqDiskinfo(qMount);

    GvfsMountManager::instance()->listMountsBylsblk();

    QVolume vol1 = GvfsMountManager::getVolumeByMountedRootUri(diskInfo.mounted_root_uri());

    QString unix_device = "/dev/sda";
    QVolume vol = GvfsMountManager::getVolumeByUnixDevice(unix_device);
    QString dev = GvfsMountManager::getDriveUnixDevice(unix_device);

    QString(*stub_unix_device_2)(void*) = [](void*)->QString{return "/dev/sda";};
    stub.set(&QVolume::unix_device, stub_unix_device_2);
    vol = GvfsMountManager::getVolumeByUnixDevice(unix_device);
    stub.reset(&QVolume::unix_device);
}

TEST_F(TestGvfsMountManager, mount)
{
    // 阻塞CI
    // GvfsMountManager::instance()->autoMountAllDisks();

    // DFMUrlBaseEvent event(nullptr, DUrl("smb:///"));
    // GvfsMountManager::mount_sync(event);

    // stub_ext::StubExt stu;
    // stu.set_lamda(&QDiskInfo::can_eject, [](){return true;});
    // stu.set_lamda(&QDiskInfo::mounted_root_uri, [](){return "smb:///";});
    // GvfsMountManager::mount_sync(event);
    // stu.reset(&QDiskInfo::can_eject);
    // stu.reset(&QDiskInfo::mounted_root_uri);

    // stu.set_lamda(&QDiskInfo::can_eject, [](){return true;});
    // stu.set_lamda(&QDiskInfo::unix_device, [](){return "smb:///";});
    // GvfsMountManager::mount_sync(event);
    // stu.reset(&QDiskInfo::can_eject);
    // stu.reset(&QDiskInfo::unix_device);
}

TEST_F(TestGvfsMountManager, eject)
{
    GvfsMountManager::eject("/dev/sdno");
}

TEST_F(TestGvfsMountManager, monitor_mount_changed)
{
    GVolumeMonitor *volume_monitor = GvfsMountManager::instance()->m_gVolumeMonitor;
    GMount *mount = nullptr;
    //g_signal_emit_by_name (volume_monitor, "mount-added", mount);
    GvfsMountManager::monitor_mount_added(volume_monitor, mount);
    GvfsMountManager::monitor_mount_added_root(volume_monitor, mount);

    {
        Stub st;
        bool (*stub_isIgnoreUnusedMounts)(const QMount &) = [](const QMount &)->bool{return true;};
        st.set(&GvfsMountManager::isIgnoreUnusedMounts, stub_isIgnoreUnusedMounts);
        GvfsMountManager::monitor_mount_added(volume_monitor, mount);
    }

    {
        Stub st;
        static GVolume* v = get_fisrt_usable_volume();
        GVolume*(*stub_g_mount_get_volume)(GMount*) = [](GMount*)->GVolume*{ return v;};
        st.set(g_mount_get_volume, stub_g_mount_get_volume);
        GvfsMountManager::monitor_mount_added(volume_monitor, mount);
    }

    {
        Stub st;
        bool(*stub_can_unmount)(void*) = [](void*)->bool{return true;};
        bool(*stub_can_eject)(void*) = [](void*)->bool{return true;};
        st.set(&QMount::can_unmount, stub_can_unmount);
        st.set(&QMount::can_eject, stub_can_eject);
        GvfsMountManager::monitor_mount_added(volume_monitor, mount);
        GvfsMountManager::monitor_mount_added_root(volume_monitor, mount);
    }

//    GvfsMountManager::monitor_mount_removed(volume_monitor, mount);
//    GvfsMountManager::monitor_mount_removed_root(volume_monitor, mount);

    {
        Stub st;
        bool(*stub_isValid)(void*) = [](void*)->bool{return true;};
        QString(*stub_name)(void*) = [](void*)->QString{return "CD";};
        st.set(&QVolume::isValid, stub_isValid);
        st.set(&QMount::name, stub_name);
//        GvfsMountManager::monitor_mount_removed(volume_monitor, mount);
//        GvfsMountManager::monitor_mount_removed_root(volume_monitor, mount);
    }

    {
        Stub st;
        bool(*stub_isDVD)(void*, const QVolume &volume) = [](void*, const QVolume &volume)->bool{return true;};
        st.set(&GvfsMountManager::isDVD, stub_isDVD);
        GMount* mt = get_fisrt_usable_mount();
//        GvfsMountManager::monitor_mount_changed(volume_monitor, mt);
    }
}

TEST_F(TestGvfsMountManager, monitor_volume_changed)
{
    GVolumeMonitor *volume_monitor = GvfsMountManager::instance()->m_gVolumeMonitor;
    GVolume *volume = nullptr;
    GvfsMountManager::monitor_volume_added(volume_monitor, volume);
    GvfsMountManager::monitor_volume_removed(volume_monitor, volume);
    GvfsMountManager::monitor_volume_changed(volume_monitor, volume);

    bool(*stub_isValid)(void*) = [](void*)->bool{return true;};
    QString(*stub_name)(void*) = [](void*)->QString{return "CD";};
    stub.set(&QVolume::isValid, stub_isValid);
    stub.set(&QVolume::name, stub_name);
    GvfsMountManager::monitor_volume_removed(volume_monitor, volume);
    stub.reset(&QVolume::isValid);
    stub.reset(&QVolume::name);


    volume = get_fisrt_usable_volume();
    GvfsMountManager::monitor_volume_added(volume_monitor, volume);
    GvfsMountManager::monitor_volume_changed(volume_monitor, volume);

    stub_ext::StubExt stubx;
    stubx.set_lamda(&QVolume::icons,  [](void*)->QStringList{
        QStringList list;
        list << "media-optical";
        return list;
    });
    GvfsMountManager::monitor_volume_added(volume_monitor, volume);
    stubx.reset(&QVolume::icons);
}

TEST_F(TestGvfsMountManager, printVolumeMounts)
{
    GvfsMountManager::printVolumeMounts();
}

#include <QFileInfo>

bool foo_stub_bool(void* obj)
{
     QFileInfo info;
     info.exists();
     info.absoluteFilePath();
    //bool QFileInfo::exists() const
    return true;
}

#include <gio/gioenums.h>
TEST_F(TestGvfsMountManager, mount_device)
{
    QString unix_device = "/dev/sdno";
    bool silent = false;
    GvfsMountManager::mount_device(unix_device, silent);
    GvfsMountManager::unmount(unix_device);

    QDiskInfo diskInfo;
    diskInfo.setCan_mount(true);
    diskInfo.setMounted_root_uri("smb:///");
    GvfsMountManager::unmount(diskInfo);

    GvfsMountManager::unmount_mounted("smb:///");

    //TODO
    bool (QFileInfo::*exists)() const = &QFileInfo::exists;
    uint (QFileInfo::*ownerId)() const = &QFileInfo::ownerId;
    uint (QFileInfo::*groupId)() const = &QFileInfo::groupId;
    bool (QFileInfo::*permission)(QFile::Permissions) const = &QFileInfo::permission;
    QString (QFileInfo::*absoluteFilePath)() const = &QFileInfo::absoluteFilePath;

    stub_ext::StubExt stubx;
    stubx.set_lamda(exists, [](void*)->bool{return true;});
    stubx.set_lamda(getuid, []()->__uid_t{return 1000;});
    stubx.set_lamda(ownerId, [](void*)->__uid_t{return 1000;});
    stubx.set_lamda(permission, [](void*, QFile::Permissions)->bool{return false;});
    stubx.set_lamda(absoluteFilePath, []()->QString{return "/media/";});
    GvfsMountManager::unmount_mounted("smb:///");
    stubx.reset(exists);
    stubx.reset(getuid);
    stubx.reset(ownerId);
    stubx.reset(permission);
    stubx.reset(absoluteFilePath);


    stubx.set_lamda(exists, [](void*)->bool{return true;});
    stubx.set_lamda(getuid, []()->__uid_t{return 1000;});
    stubx.set_lamda(ownerId, [](void*)->__uid_t{return 1001;});
    stubx.set_lamda(groupId, [](void*)->__uid_t{return 1000;});
    stubx.set_lamda(permission, [](void*, QFile::Permissions)->bool{return false;});
    GvfsMountManager::unmount_mounted("smb:///");
    stubx.reset(exists);
    stubx.reset(getuid);
    stubx.reset(ownerId);
    stubx.reset(groupId);
    stubx.reset(permission);

    stubx.set_lamda(exists, [](void*)->bool{return true;});
    stubx.set_lamda(permission, [](void*, QFile::Permissions)->bool{return false;});
    stubx.set_lamda(getuid, []()->__uid_t{return 1000;});
    stubx.set_lamda(ownerId, [](void*)->__uid_t{return 1001;});
    stubx.set_lamda(groupId, [](void*)->__uid_t{return 1002;});
    GvfsMountManager::unmount_mounted("smb:///");
    stubx.reset(exists);
    stubx.reset(getuid);
    stubx.reset(ownerId);
    stubx.reset(groupId);
    stubx.reset(permission);

    //g_file_find_enclosing_mount
    stubx.set_lamda(g_file_find_enclosing_mount, [this](GFile                      *file,
                    GCancellable               *cancellable,
                    GError                    **error)->GMount*{

        GMount* mt = get_fisrt_usable_mount();
        GError *gerror = g_slice_new(GError);
        gerror->message = g_strdup("123");
        *error = gerror;
        return mt;
    });

    stubx.set_lamda(g_mount_unmount_with_operation, [](GMount              *mount,
                    GMountUnmountFlags   flags,
                    GMountOperation     *mount_operation,
                    GCancellable        *cancellable,
                    GAsyncReadyCallback  callback,
                    gpointer             user_data){
           });
    stubx.set_lamda(g_object_unref, [](gpointer p){});
    GvfsMountManager::unmount_mounted("smb:///");
    stubx.reset(g_file_find_enclosing_mount);
    stubx.reset(g_mount_unmount_with_operation);
    stubx.reset(g_object_unref);
}

TEST_F(TestGvfsMountManager, eject_device)
{
    QString unix_device = "/dev/sdno";
    GvfsMountManager::eject_device(unix_device);
}

GMount *new_mount(void)
{
    //GMount *mount = (GMount*) malloc (1024);//要崩溃
    GObject* obj = (GObject*)g_object_new(G_TYPE_MOUNT, nullptr);
    GMount* mount = G_MOUNT(obj);
    //TODO 这里返回的nullptr 未达到预期要求
    return mount;
}

GMount *g_file_find_enclosing_mount_stub (GFile *file,
                             GCancellable *cancellable,
                             GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    GMount* mount =  new_mount();
    return mount;
}

TEST_F(TestGvfsMountManager, eject_mounted)
{
    QString mounted_root_uri = "/mnt";
    GvfsMountManager::eject_mounted(mounted_root_uri);

    mounted_root_uri = "file:///";
    //stub.set(g_file_find_enclosing_mount, g_file_find_enclosing_mount_stub);
    GvfsMountManager::eject_mounted(mounted_root_uri);
    //stub.reset(g_file_find_enclosing_mount);


    stub_ext::StubExt stubx;
    //g_file_find_enclosing_mount
    stubx.set_lamda(g_file_find_enclosing_mount, [this](GFile                      *file,
                    GCancellable               *cancellable,
                    GError                    **error)->GMount*{

        GMount* mt = get_fisrt_usable_mount();

        GError *gerror = g_slice_new(GError);
        gerror->message = g_strdup("123");
        *error = gerror;
        return mt;
    });

    stubx.set_lamda(g_mount_unmount_with_operation, [](GMount              *mount,
                    GMountUnmountFlags   flags,
                    GMountOperation     *mount_operation,
                    GCancellable        *cancellable,
                    GAsyncReadyCallback  callback,
                    gpointer             user_data){
           });

    stubx.set_lamda(g_mount_eject_with_operation, [](GMount              *mount,
                    GMountUnmountFlags   flags,
                    GMountOperation     *mount_operation,
                    GCancellable        *cancellable,
                    GAsyncReadyCallback  callback,
                    gpointer             user_data){
           });
    stubx.set_lamda(g_object_unref, [](gpointer p){});
    GvfsMountManager::unmount_mounted("smb:///");
    stubx.reset(g_file_find_enclosing_mount);
    stubx.reset(g_mount_eject_with_operation);
    stubx.reset(g_object_unref);
}

TEST_F(TestGvfsMountManager, eject_by_diskInfo)
{
    QDiskInfo diskInfo;
    diskInfo.setMounted_root_uri("no:///");
    diskInfo.setUnix_device("/dev/no");
    GvfsMountManager::eject(diskInfo);

    diskInfo.setMounted_root_uri("");
    GvfsMountManager::eject(diskInfo);
}

gboolean g_volume_eject_with_operation_finish_stub (GVolume *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return true;
};

gboolean g_volume_eject_with_operation_finish_false_stub (GVolume *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return false;
};

gboolean g_mount_eject_with_operation_finish_stub (GMount *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return true;
};

gboolean g_mount_eject_with_operation_finish_false_stub (GMount *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return false;
};


gboolean g_volume_mount_finish_stub (GVolume *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR_FAILED,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return true;
};

gboolean g_volume_mount_finish_false_stub (GVolume *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return false;
};


gboolean g_mount_unmount_with_operation_finish_stub (GMount *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR_FAILED,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return true;
};

gboolean g_mount_unmount_with_operation_finish_false_stub (GMount *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return false;
};

gboolean g_drive_stop_finish_stub (GDrive *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR_FAILED,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return true;
};

gboolean g_drive_stop_finish_false_stub (GDrive *v, GAsyncResult  *result, GError **error)
{
    *error = g_error_new_literal (G_IO_ERROR,
                                    G_IO_ERROR_INVALID_ARGUMENT,
                                    "The given address is empty");
    return false;
};
TEST_F(TestGvfsMountManager, eject_with_device_file_cb)
{
    GObject *object = nullptr;
    GAsyncResult *res = nullptr;
    gpointer user_data = nullptr;
    stub.set(g_volume_eject_with_operation_finish, g_volume_eject_with_operation_finish_stub);
    GvfsMountManager::eject_with_device_file_cb(object, res, user_data);
    stub.reset(g_volume_eject_with_operation_finish);

    stub.set(g_volume_eject_with_operation_finish, g_volume_eject_with_operation_finish_false_stub);
    GvfsMountManager::eject_with_device_file_cb(object, res, user_data);
    stub.reset(g_volume_eject_with_operation_finish);
}

TEST_F(TestGvfsMountManager, eject_with_mounted_file_cb)
{
    //stub g_volume_eject_with_operation_finish

    GObject *object = nullptr;
    GAsyncResult *res = nullptr;
    gpointer user_data = nullptr;

    stub.set(g_mount_eject_with_operation_finish, g_mount_eject_with_operation_finish_stub);
    GvfsMountManager::eject_with_mounted_file_cb(object, res, user_data);
    stub.reset(g_mount_eject_with_operation_finish);

    stub.set(g_mount_eject_with_operation_finish, g_mount_eject_with_operation_finish_false_stub);
    GvfsMountManager::eject_with_mounted_file_cb(object, res, user_data);
    stub.reset(g_mount_eject_with_operation_finish);
}

TEST_F(TestGvfsMountManager, stop_device)
{
    QString drive_unix_device = "/dev/sdno";
    GvfsMountManager::stop_device(drive_unix_device);
}

#include <QStringList>
TEST_F(TestGvfsMountManager, ask_question_cb)
{
    GMountOperation *op = nullptr;
    char *message = "Can’t verify the identity of ?one? If you want to be absolutely sure it is safe to continue, contact the system administrator.";
    GStrv choices = g_strsplit("aa,bb,cc", ",", 2);
    GvfsMountManager::ask_question_cb(op, message, choices);

     choices = g_strsplit("", ",", 2);
     message = "Can’t verify the identity of “.*?” If you want to be absolutely sure it is safe to continue, contact the system administrator.";
     GvfsMountManager::ask_question_cb(op, message, choices);
}

QJsonObject stub_SecretManager_getLoginData(void*, const QString &id)
{
    QJsonObject obj;
    obj.insert("message", "message");
    obj.insert("anonymous", "anonymous");
    obj.insert("username", "default_user");
    obj.insert("domain", "default_domain");
    obj.insert("password", "default_password");
    obj.insert("GAskPasswordFlags", "flags");
    obj.insert("passwordSave", "passwordSave");
    return obj;
}

#include "addr_any.h"

TEST_F(TestGvfsMountManager, ask_disk_password_cb)
{
    GMountOperation *op = nullptr;
    char *message = "Can’t verify the identity of ?one? If you want to be absolutely sure it is safe to continue, contact the system administrator.";
    char *default_user = "utuser";
    char *default_domain = "default_domain";

    GAskPasswordFlags flags = G_ASK_PASSWORD_NEED_DOMAIN;
    GvfsMountManager::ask_disk_password_cb(op, message, default_user, default_domain, flags);

    flags = G_ASK_PASSWORD_NEED_USERNAME;
    GvfsMountManager::ask_disk_password_cb(op, message, default_user, default_domain, flags);

    flags = G_ASK_PASSWORD_NEED_PASSWORD;
    GvfsMountManager::ask_disk_password_cb(op, message, default_user, default_domain, flags);
}

#include "gvfs/secretmanager.h"
TEST_F(TestGvfsMountManager, ask_password_cb)
{
    GMountOperation *op = nullptr;
    char *message = "Can’t verify the identity of ?one? If you want to be absolutely sure it is safe to continue, contact the system administrator.";
    char *default_user = "utuser";
    char *default_domain = "default_domain";
    GAskPasswordFlags flags = G_ASK_PASSWORD_TCRYPT;
    GvfsMountManager::ask_password_cb(op, message, default_user, default_domain, flags);

    flags = G_ASK_PASSWORD_ANONYMOUS_SUPPORTED;
    GvfsMountManager::ask_password_cb(op, message, default_user, default_domain, flags);

    flags = G_ASK_PASSWORD_SAVING_SUPPORTED;
    GvfsMountManager::ask_password_cb(op, message, default_user, default_domain, flags);

    flags = G_ASK_PASSWORD_NEED_DOMAIN;
    GvfsMountManager::ask_password_cb(op, message, default_user, default_domain, flags);

    flags = G_ASK_PASSWORD_NEED_USERNAME;
    GvfsMountManager::ask_password_cb(op, message, default_user, default_domain, flags);

    flags = G_ASK_PASSWORD_NEED_PASSWORD;
    GvfsMountManager::ask_password_cb(op, message, default_user, default_domain, flags);

    //TODO requestPasswordDialog
    stub.set(&SecretManager::getLoginData, stub_SecretManager_getLoginData);
    GvfsMountManager::ask_password_cb(op, message, default_user, default_domain, flags);
    stub.set(&SecretManager::getLoginData, stub_SecretManager_getLoginData);
}


TEST_F(TestGvfsMountManager, mount_with_device_file_cb)
{
    GObject *object = nullptr;
    GAsyncResult *res = nullptr;
    gpointer user_data = nullptr;
    stub.set(g_volume_mount_finish, g_volume_mount_finish_stub);
    GvfsMountManager::mount_with_device_file_cb(object, res, user_data);
    stub.reset(g_volume_mount_finish);

    stub.set(g_volume_mount_finish, g_volume_mount_finish_false_stub);
    GvfsMountManager::mount_with_device_file_cb(object, res, user_data);
    stub.reset(g_volume_mount_finish);
}

TEST_F(TestGvfsMountManager, unmount_done_cb)
{
    GObject *object = nullptr;
    GAsyncResult *res = nullptr;
    gpointer user_data = nullptr;
    stub.set(g_mount_unmount_with_operation_finish, g_mount_unmount_with_operation_finish_stub);
    GvfsMountManager::unmount_done_cb(object, res, user_data);
    stub.reset(g_mount_unmount_with_operation_finish);

    stub.set(g_mount_unmount_with_operation_finish, g_mount_unmount_with_operation_finish_false_stub);
    GvfsMountManager::unmount_done_cb(object, res, user_data);
    stub.reset(g_mount_unmount_with_operation_finish);

    stub_ext::StubExt stubx;
    user_data = strdup("/test/");
    stubx.set_lamda(g_mount_unmount_with_operation_finish, [](GMount          *mount,
                    GAsyncResult        *result,
                    GError             **error)->bool{return true;});
    stubx.set_lamda(g_file_get_uri, [](GFile  *file)->char *{
        char* p = (char*)g_malloc(strlen("notfile://") + 1);
        memset(p , 0, strlen("notfile://") + 1);
        strcpy(p, "notfile://");
        return p;});
    GvfsMountManager::unmount_done_cb(object, res, user_data);
    stubx.reset(g_mount_unmount_with_operation_finish);
    stubx.reset(g_file_get_uri);
}

TEST_F(TestGvfsMountManager, stop_with_device_file_cb)
{
    GObject *object = nullptr;
    GAsyncResult *res = nullptr;
    gpointer user_data = nullptr;
    stub.set(g_drive_stop_finish, g_drive_stop_finish_stub);
    GvfsMountManager::stop_with_device_file_cb(object, res, user_data);
    stub.reset(g_mount_unmount_with_operation_finish);

    stub.set(g_drive_stop_finish, g_drive_stop_finish_false_stub);
    GvfsMountManager::stop_with_device_file_cb(object, res, user_data);
    stub.reset(g_mount_unmount_with_operation_finish);
}
#ifndef __arm__
TEST_F(TestGvfsMountManager, initConnect)
{
    stub_ext::StubExt stu;
    stu.set_lamda(&DFMGlobal::isRootUser, []{return true;});
    m_manager->initConnect();
    stu.reset(&DFMGlobal::isRootUser);
}
#endif
