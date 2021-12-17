/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "dattachedudisks2device.h"

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <QStorageInfo>
#include <QPair>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "stub.h"
#include "ut_mock_stub_diskdevice.h"

namespace  {
    class TestDAttachedUdisks2Device : public testing::Test {
    public:

        void SetUp() override
        {
            Stub stub;
            stub.set(ADDR(DBlockDevice,mountPoints), mountPoints_stub);

            mDummyblkDevice = DDiskManager::createBlockDevice(QDir::homePath());
            mUdisks2Device = new DAttachedUdisks2Device(mDummyblkDevice);
        }
        void TearDown() override
        {
            delete mDummyblkDevice;
            delete mUdisks2Device;
        }

    public:
        DBlockDevice* mDummyblkDevice = nullptr;
        DAttachedUdisks2Device* mUdisks2Device = nullptr;
    };
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_is_valid)
{
    EXPECT_TRUE(mUdisks2Device->isValid());
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_not_detachable)
{
    EXPECT_FALSE(mUdisks2Device->detachable());
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_no_mounturl)
{
    QUrl url = mUdisks2Device->mountpointUrl();
    EXPECT_TRUE(url.path().contains( getDummyMountPoint() ));
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_sharedptr_not_null)
{
    ASSERT_TRUE(mUdisks2Device->blockDevice());
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_isnotoptical_accessPointUrl)
{
    QUrl accessUrl = mUdisks2Device->accessPointUrl();

    EXPECT_EQ(accessUrl.path(), getDummyMountPoint());
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_isoptical_accessPointUrl)
{
    Stub stub;
    stub.set(ADDR(DDiskDevice,optical), optical_stub);
    stub.set(ADDR(DBlockDevice,device), device_stub);

    QUrl accessUrl = mUdisks2Device->accessPointUrl();

    EXPECT_TRUE(accessUrl.path().contains(BLK_DEVICE_PATH));
}

TEST_F(TestDAttachedUdisks2Device, dummy_optical_device_detach)
{
    Stub stub;
    stub.set(ADDR(DDiskDevice,optical), optical_stub);
    stub.set(ADDR(DDiskDevice,ejectable), ejectable_stub);
    stub.set(ADDR(DDiskDevice,lastError), nolastError_stub);
    mUdisks2Device->detach();
    sleep(nTimeout_udisk);
}

TEST_F(TestDAttachedUdisks2Device, dummy_optical_device_notbe_detached)
{
    Stub stub;
    stub.set(ADDR(DDiskDevice,optical), optical_stub);
    stub.set(ADDR(DDiskDevice,ejectable), ejectable_stub);
    stub.set(ADDR(DDiskDevice,lastError), lastError_stub);
    mUdisks2Device->detach();
    sleep(nTimeout_udisk);
}

TEST_F(TestDAttachedUdisks2Device, dummy_block_device_detach)
{
    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);
    stub.set(ADDR(DDiskDevice,ejectable), ejectable_stub);
    stub.set(ADDR(DDiskDevice,canPowerOff), canPowerOff_stub);
    stub.set(ADDR(DDiskDevice,lastError), nolastError_stub);
    mUdisks2Device->detach();
    sleep(nTimeout_udisk);
}

TEST_F(TestDAttachedUdisks2Device, dummy_block_device_notbe_detached)
{
    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);
    stub.set(ADDR(DDiskDevice,ejectable), ejectable_stub);
    stub.set(ADDR(DDiskDevice,canPowerOff), canPowerOff_stub);
    stub.set(ADDR(DDiskDevice,lastError), lastError_stub);
    mUdisks2Device->detach();
    sleep(nTimeout_udisk);
}

TEST_F(TestDAttachedUdisks2Device, dummy_blkdevice_name_isempty)
{
    Stub stub;
    stub.set(ADDR(DBlockDevice,isValid), isValid_true_stub);
    stub.set(ADDR(DBlockDevice,idLabel), idLabel_empty_stub);
    stub.set(ADDR(DBlockDevice,size), size_1KB_stub);
    QString name = mUdisks2Device->displayName();
    EXPECT_TRUE(name.contains("1 KB"));
}

TEST_F(TestDAttachedUdisks2Device, dummy_blkdevice_name_notEmpty)
{
    Stub stub;
    stub.set(ADDR(DBlockDevice,isValid), isValid_true_stub);
    stub.set(ADDR(DBlockDevice,idLabel), idLabel_dummy_stub);

    QString name = mUdisks2Device->displayName();
    EXPECT_TRUE(name.contains("dummy"));
}

TEST_F(TestDAttachedUdisks2Device, dummy_blkdevice_not_Valid_storage_is_ok)
{
    Stub stub;
    stub.set(ADDR(DBlockDevice,isValid), isValid_false_stub);
    stub.set(ADDR(QStorageInfo,isValid), isValid_true_stub);
    stub.set(ADDR(QStorageInfo,bytesTotal), size_1KB_stub);

    QString name = mUdisks2Device->displayName();
    EXPECT_TRUE(name.contains("1 KB"));
}


TEST_F(TestDAttachedUdisks2Device, dummy_deviceUsage_can_get)
{
    Stub stub;
    stub.set(ADDR(QStorageInfo,isValid), isValid_true_stub);
    stub.set(ADDR(DBlockDevice, size), size_2KB_stub);
    stub.set(ADDR(QStorageInfo,bytesAvailable), size_1KB_stub);

    QPair<quint64, quint64> useage = mUdisks2Device->deviceUsage();
    QPair<quint64, quint64> values = QPair<quint64, quint64>(1024, 2*1024);
    EXPECT_EQ(useage, values);
}

TEST_F(TestDAttachedUdisks2Device, dummy_deviceUsage_cannot_get)
{
    Stub stub;
    stub.set(ADDR(QStorageInfo,isValid), isValid_false_stub);

    QPair<quint64, quint64> useage = mUdisks2Device->deviceUsage();
    QPair<quint64, quint64> values = QPair<quint64, quint64>(0, 0);
    EXPECT_EQ(useage, values);
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_usage_valid)
{
    Stub stub;
    stub.set(ADDR(QStorageInfo,isValid), isValid_true_stub);

    EXPECT_TRUE(mUdisks2Device->deviceUsageValid());
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_get_default_iconname)
{
    QString name = mUdisks2Device->iconName();
    EXPECT_TRUE(name.contains("drive-harddisk"));
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_isoptical_iconname)
{
    Stub stub;
    stub.set(ADDR(DDiskDevice,optical), optical_stub);

    QString name = mUdisks2Device->iconName();
    EXPECT_TRUE(name.contains("media-optical"));
}

TEST_F(TestDAttachedUdisks2Device, dummy_device_removable_iconname)
{
    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);

    QString name = mUdisks2Device->iconName();
    EXPECT_TRUE(name.contains("drive-removable-media-usb"));
}
