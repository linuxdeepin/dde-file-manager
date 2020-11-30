/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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

static bool inited = false;
namespace {
class TestGvfsMountManager: public testing::Test
{
public:
    GvfsMountManager *m_manager = GvfsMountManager::instance();
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
};
}

TEST_F(TestGvfsMountManager, qVolumeToqDiskInfo)
{
    QVolume volume;
    volume.setUnix_device("abc");
    volume.setUuid("10011");
    QDiskInfo info = GvfsMountManager::qVolumeToqDiskInfo(volume);
    EXPECT_STREQ(info.id().toStdString().c_str(), volume.unix_device().toStdString().c_str());
    EXPECT_STREQ(info.uuid().toStdString().c_str(), volume.uuid().toStdString().c_str());
}

TEST_F(TestGvfsMountManager, qMountToqDiskinfo)
{
    QMount mount;
    mount.setMounted_root_uri("123");
    QDiskInfo info = GvfsMountManager::qMountToqDiskinfo(mount);
    EXPECT_STREQ(info.id().toStdString().c_str(), mount.mounted_root_uri().toStdString().c_str());
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

    EXPECT_NE(nullptr, rvolume);
    EXPECT_NE(nullptr, rmount);
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
    QString rpath = "/dev/NOWAY";
    char* path = g_file_get_path(file);
    if(path != nullptr) {
        rpath = path;
    }
    GvfsMountManager::mount(rpath, true);
    GvfsMountManager::mount(rpath, false);
    GvfsMountManager::getDiskInfo(rpath, true);
    GvfsMountManager::getDiskInfo(rpath, false);

    GvfsMountManager::mount_mounted(diskInfo.mounted_root_uri(), true);
    GvfsMountManager::mount_mounted(diskInfo.mounted_root_uri(), false);
    GvfsMountManager::mount_device(diskInfo.unix_device(), true);
    GvfsMountManager::mount_device(diskInfo.unix_device(), false);
    GvfsMountManager::isDeviceCrypto_LUKS(diskInfo);

    GvfsMountManager::instance()->listMountsBylsblk();

    QVolume vol1 = GvfsMountManager::getVolumeByMountedRootUri(diskInfo.mounted_root_uri());

    QString unix_device = "/dev/sda";
    QVolume vol = GvfsMountManager::getVolumeByUnixDevice(unix_device);
    QString dev = GvfsMountManager::getDriveUnixDevice(unix_device);
}

TEST_F(TestGvfsMountManager, mount)
{
    GvfsMountManager::instance()->autoMountAllDisks();

    DFMUrlBaseEvent event(nullptr, DUrl("smb:///"));
    GvfsMountManager::mount_sync(event);
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

    GvfsMountManager::monitor_mount_removed(volume_monitor, mount);
    GvfsMountManager::monitor_mount_removed_root(volume_monitor, mount);

    GvfsMountManager::monitor_mount_changed(volume_monitor, mount);
}

TEST_F(TestGvfsMountManager, monitor_volume_changed)
{
    GVolumeMonitor *volume_monitor = GvfsMountManager::instance()->m_gVolumeMonitor;
    GVolume *volume = nullptr;
    GvfsMountManager::monitor_volume_added(volume_monitor, volume);
    GvfsMountManager::monitor_volume_removed(volume_monitor, volume);
    GvfsMountManager::monitor_volume_changed(volume_monitor, volume);
}

TEST_F(TestGvfsMountManager, printVolumeMounts)
{
    GvfsMountManager::printVolumeMounts();
}

#include <DDialog>
//DWIDGET_BEGIN_NAMESPACE
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
}

//DWIDGET_END_NAMESPACE

TEST_F(TestGvfsMountManager, eject_device)
{
    QString unix_device = "/dev/sdno";
    GvfsMountManager::eject_device(unix_device);
}

TEST_F(TestGvfsMountManager, eject_mounted)
{
    QString mounted_root_uri = "/mnt";
    GvfsMountManager::eject_mounted(mounted_root_uri);
}

TEST_F(TestGvfsMountManager, stop_device)
{
    QString drive_unix_device = "/dev/sdno";
    GvfsMountManager::stop_device(drive_unix_device);
}

//ACCESS_PRIVATE_FIELD(A, int, a);

