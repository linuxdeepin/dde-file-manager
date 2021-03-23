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
        m_diskInfo.setUnix_device("/deb/sd*");
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

TEST_F(TestUDiskDeviceInfo, construct2)
{
    UDiskDeviceInfoPointer p(new UDiskDeviceInfo());
    EXPECT_NO_FATAL_FAILURE(UDiskDeviceInfo info(p));
}

TEST_F(TestUDiskDeviceInfo, construct3)
{
    QString path("/");
    EXPECT_NO_FATAL_FAILURE(UDiskDeviceInfo info(path));
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
    UDiskDeviceInfo dev;
    QDiskInfo disk;

    disk.setType("native");
    dev.setDiskInfo(disk);
    EXPECT_EQ(dev.getMediaType(), UDiskDeviceInfo::native);

    disk.setType("removable");
    dev.setDiskInfo(disk);
    EXPECT_EQ(dev.getMediaType(), UDiskDeviceInfo::removable);

    disk.setType("network");
    dev.setDiskInfo(disk);
    EXPECT_EQ(dev.getMediaType(), UDiskDeviceInfo::network);

    disk.setType("smb");
    dev.setDiskInfo(disk);
    EXPECT_EQ(dev.getMediaType(), UDiskDeviceInfo::network);

    disk.setType("phone");
    dev.setDiskInfo(disk);
    EXPECT_EQ(dev.getMediaType(), UDiskDeviceInfo::phone);

    disk.setType("iphone");
    dev.setDiskInfo(disk);
    EXPECT_EQ(dev.getMediaType(), UDiskDeviceInfo::iphone);

    disk.setType("camera");
    dev.setDiskInfo(disk);
    EXPECT_EQ(dev.getMediaType(), UDiskDeviceInfo::camera);

    disk.setType("dvd");
    dev.setDiskInfo(disk);
    EXPECT_EQ(dev.getMediaType(), UDiskDeviceInfo::dvd);

    disk.setType("");
    dev.setDiskInfo(disk);
    EXPECT_EQ(dev.getMediaType(), UDiskDeviceInfo::unknown);
}

TEST_F(TestUDiskDeviceInfo, extraProperties)
{
    QVariantHash hash(m_devInfo->extraProperties());
    EXPECT_STREQ(m_devInfo->getId().toStdString().c_str(), hash["deviceId"].toString().toStdString().c_str());
}

TEST_F(TestUDiskDeviceInfo, fileIcon)
{
    UDiskDeviceInfo dev;
    QDiskInfo disk;

    disk.setType("native");
    dev.setDiskInfo(disk);
    EXPECT_FALSE(dev.fileIcon().isNull());

    disk.setType("removable");
    dev.setDiskInfo(disk);
    EXPECT_FALSE(dev.fileIcon().isNull());

    disk.setType("network");
    dev.setDiskInfo(disk);
    EXPECT_FALSE(dev.fileIcon().isNull());

    disk.setType("phone");
    dev.setDiskInfo(disk);
    EXPECT_FALSE(dev.fileIcon().isNull());

    disk.setType("iphone");
    dev.setDiskInfo(disk);
    EXPECT_FALSE(dev.fileIcon().isNull());

    disk.setType("camera");
    dev.setDiskInfo(disk);
    EXPECT_FALSE(dev.fileIcon().isNull());

    disk.setType("dvd");
    dev.setDiskInfo(disk);
    EXPECT_FALSE(dev.fileIcon().isNull());

    disk.setType("");
    dev.setDiskInfo(disk);
    EXPECT_FALSE(dev.fileIcon().isNull());
}

TEST_F(TestUDiskDeviceInfo, deviceTypeDisplayName)
{
    UDiskDeviceInfo dev;
    QDiskInfo disk;

    disk.setType("native");
    dev.setDiskInfo(disk);
    EXPECT_STREQ(dev.deviceTypeDisplayName().toStdString().c_str(), "Local disk");

    disk.setType("removable");
    dev.setDiskInfo(disk);
    EXPECT_STREQ(dev.deviceTypeDisplayName().toStdString().c_str(), "Removable disk");

    disk.setType("network");
    dev.setDiskInfo(disk);
    EXPECT_STREQ(dev.deviceTypeDisplayName().toStdString().c_str(), "Network shared directory");

    disk.setType("phone");
    dev.setDiskInfo(disk);
    EXPECT_STREQ(dev.deviceTypeDisplayName().toStdString().c_str(), "Android mobile device");

    disk.setType("iphone");
    dev.setDiskInfo(disk);
    EXPECT_STREQ(dev.deviceTypeDisplayName().toStdString().c_str(), "Apple mobile device");

    disk.setType("camera");
    dev.setDiskInfo(disk);
    EXPECT_STREQ(dev.deviceTypeDisplayName().toStdString().c_str(), "Camera");

    disk.setType("dvd");
    dev.setDiskInfo(disk);
    EXPECT_STREQ(dev.deviceTypeDisplayName().toStdString().c_str(), "DVD");

    disk.setType("");
    dev.setDiskInfo(disk);
    EXPECT_STREQ(dev.deviceTypeDisplayName().toStdString().c_str(), "Unknown device");
}

TEST_F(TestUDiskDeviceInfo, menuActionList)
{
    EXPECT_FALSE(m_devInfo->menuActionList(DAbstractFileInfo::MultiFiles).isEmpty());
}

TEST_F(TestUDiskDeviceInfo, disableMenuActionList)
{
    EXPECT_FALSE(m_devInfo->disableMenuActionList().isEmpty());
}

TEST_F(TestUDiskDeviceInfo, canUnmount)
{
    EXPECT_TRUE(m_devInfo->canUnmount());
}

TEST_F(TestUDiskDeviceInfo, getTotal)
{
    m_diskInfo.setType("network");
    m_devInfo->setDiskInfo(m_diskInfo);
    EXPECT_EQ(65536, m_devInfo->getTotal());
}

TEST_F(TestUDiskDeviceInfo, size)
{
    EXPECT_EQ(65536, m_devInfo->size());
}

TEST_F(TestUDiskDeviceInfo, fileName)
{
    EXPECT_EQ(QString("abc"), m_devInfo->fileName());
}

TEST_F(TestUDiskDeviceInfo, fileDisplayName)
{
    UDiskDeviceInfo info;
    QDiskInfo disk;
    info.setDiskInfo(disk);
    EXPECT_FALSE(info.fileDisplayName().isEmpty());
}

TEST_F(TestUDiskDeviceInfo, getUrlByChildFileName)
{
    UDiskDeviceInfo info;
    QDiskInfo disk;
    disk.setUnix_device("/");
    info.setDiskInfo(disk);
    const DUrl &url = info.getUrlByChildFileName("tmp");
    EXPECT_FALSE(url.isValid());
}

TEST_F(TestUDiskDeviceInfo, optical)
{
    EXPECT_FALSE(m_devInfo->optical());
}

TEST_F(TestUDiskDeviceInfo, opticalBlank)
{
    EXPECT_FALSE(m_devInfo->opticalBlank());
}

TEST_F(TestUDiskDeviceInfo, opticalReuseable)
{
    EXPECT_FALSE(m_devInfo->opticalReuseable());
}

