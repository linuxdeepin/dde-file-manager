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
#include "deviceinfo/udiskdeviceinfo.h"
#include "gvfs/qdiskinfo.h"

#include <QIcon>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace {
class TestUDiskDeviceInfo: public testing::Test {
public:
    QDiskInfo m_diskInfo;
    UDiskDeviceInfo *m_devInfo {nullptr};
    void SetUp() override
    {
        m_devInfo = new UDiskDeviceInfo;
        m_diskInfo.setId("123");
        m_diskInfo.setName("abc");
        m_diskInfo.setType("dvd");
        m_diskInfo.setUnix_device("/deb/sdb");
        m_diskInfo.setUuid("abcd-efgh");
        m_diskInfo.setMounted_root_uri("/media/root/ntfs");
        m_diskInfo.setIconName(":computer");
        m_diskInfo.setCan_unmount(true);
        m_diskInfo.setCan_eject(true);
        m_diskInfo.setUsed(true);
        m_diskInfo.setTotal(65536);
        m_diskInfo.setFree(1024);
        m_diskInfo.setIsNativeCustom(true);
        m_diskInfo.setCan_mount(true);
        m_diskInfo.setRead_only(true);
        m_diskInfo.setActivation_root_uri("/");
        m_diskInfo.setIs_removable(true);
        m_diskInfo.setHas_volume(true);
        m_diskInfo.setId_filesystem("ntfs");
        m_diskInfo.setDefault_location("/");
        m_diskInfo.setDrive_unix_device("0");
        m_devInfo->setDiskInfo(m_diskInfo);
    }
    void TearDown() override
    {
        delete m_devInfo;
    }
};
}

TEST_F(TestUDiskDeviceInfo, getDiskInfo)
{
    QDiskInfo info(m_devInfo->getDiskInfo());
    EXPECT_STREQ(info.id().toStdString().c_str(), m_diskInfo.id().toStdString().c_str());
    EXPECT_STREQ(info.name().toStdString().c_str(), m_diskInfo.name().toStdString().c_str());
    EXPECT_STREQ(info.type().toStdString().c_str(), m_diskInfo.type().toStdString().c_str());
    EXPECT_STREQ(info.unix_device().toStdString().c_str(), m_diskInfo.unix_device().toStdString().c_str());
    EXPECT_STREQ(info.uuid().toStdString().c_str(), m_diskInfo.uuid().toStdString().c_str());
    EXPECT_STREQ(info.mounted_root_uri().toStdString().c_str(), m_diskInfo.mounted_root_uri().toStdString().c_str());
    EXPECT_STREQ(info.iconName().toStdString().c_str(), m_diskInfo.iconName().toStdString().c_str());
    EXPECT_TRUE(info.can_unmount());
    EXPECT_TRUE(info.can_eject());
    EXPECT_TRUE(info.used());
    EXPECT_EQ(info.total(), m_diskInfo.total());
    EXPECT_EQ(info.free(), m_diskInfo.free());
    EXPECT_TRUE(info.isNativeCustom());
    EXPECT_TRUE(info.can_mount());
    EXPECT_TRUE(info.read_only());
    EXPECT_EQ(info.activation_root_uri(), m_diskInfo.activation_root_uri());
    EXPECT_TRUE(info.is_removable());
    EXPECT_TRUE(info.has_volume());
    EXPECT_STREQ(info.id_filesystem().toStdString().c_str(), m_diskInfo.id_filesystem().toStdString().c_str());
    EXPECT_STREQ(info.default_location().toStdString().c_str(), m_diskInfo.default_location().toStdString().c_str());
    EXPECT_STREQ(info.drive_unix_device().toStdString().c_str(), m_diskInfo.drive_unix_device().toStdString().c_str());
}

TEST_F(TestUDiskDeviceInfo, single_interfaces)
{
    QDiskInfo info(m_devInfo->getDiskInfo());
    // single interfaces
    EXPECT_STREQ(info.id().toStdString().c_str(), m_devInfo->getId().toStdString().c_str());
    EXPECT_STREQ(info.name().toStdString().c_str(), m_devInfo->getName().toStdString().c_str());
    EXPECT_STREQ(info.type().toStdString().c_str(), m_devInfo->getType().toStdString().c_str());
    EXPECT_STREQ(info.unix_device().toStdString().c_str(), m_devInfo->getPath().toStdString().c_str());
    EXPECT_STREQ(info.mounted_root_uri().toStdString().c_str(), m_devInfo->getMountPoint().toStdString().c_str());
}

TEST_F(TestUDiskDeviceInfo, getMediaType)
{
    EXPECT_EQ(m_devInfo->getMediaType(), UDiskDeviceInfo::dvd);
}

TEST_F(TestUDiskDeviceInfo, extraProperties)
{
    QVariantHash hash(m_devInfo->extraProperties());
    EXPECT_STREQ(m_devInfo->getId().toStdString().c_str(), hash["deviceId"].toString().toStdString().c_str());
}

TEST_F(TestUDiskDeviceInfo, fileIcon)
{
    EXPECT_FALSE(m_devInfo->fileIcon().isNull());
}

TEST_F(TestUDiskDeviceInfo, deviceTypeDisplayName)
{
    EXPECT_STREQ("DVD", m_devInfo->deviceTypeDisplayName().toStdString().c_str());
}

TEST_F(TestUDiskDeviceInfo, menuActionList)
{
    EXPECT_FALSE(m_devInfo->menuActionList(DAbstractFileInfo::MultiFiles).isEmpty());
}

TEST_F(TestUDiskDeviceInfo, disableMenuActionList)
{
    EXPECT_FALSE(m_devInfo->disableMenuActionList().isEmpty());
}
