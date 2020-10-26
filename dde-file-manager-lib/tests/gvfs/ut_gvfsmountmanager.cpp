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
#include "gvfs/gvfsmountmanager.h"
#include "gvfs/qdrive.h"
#include "gvfs/qvolume.h"
#include "gvfs/qdiskinfo.h"
#include "gvfs/qmount.h"

#include <gtest/gtest.h>

namespace {
class TestGvfsMountManager: public testing::Test
{
public:
    GvfsMountManager *m_manager = GvfsMountManager::instance();
    void SetUp() override
    {
        m_manager->startMonitor();
        m_manager->listDrives();
        m_manager->listMounts();
        m_manager->updateDiskInfos();
        m_manager->listMountsBylsblk();
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
    QDiskInfo info = GvfsMountManager::qVolumeToqDiskInfo(volume);
    EXPECT_STREQ(info.id().toStdString().c_str(), volume.unix_device().toStdString().c_str());
}

TEST_F(TestGvfsMountManager, qMountToqDiskinfo)
{
    QMount mount;
    mount.setMounted_root_uri("123");
    QDiskInfo info = GvfsMountManager::qMountToqDiskinfo(mount);
    EXPECT_STREQ(info.id().toStdString().c_str(), mount.mounted_root_uri().toStdString().c_str());
}

TEST_F(TestGvfsMountManager, mount)
{
    GvfsMountManager::mount("/dev/sda1");
}

TEST_F(TestGvfsMountManager, eject)
{
    GvfsMountManager::mount("/dev/sda1");
}
