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

#include "ut_mock_stub_disk_gio.h"

#include "dattachedvfsdevice.h"

#include <QFileInfo>
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <DDBusSender>

#include "stub.h"
#include "stubext.h"
#include "ut_mock_stub_diskdevice.h"


DAttachedVfsDevice * mDummyVfsDevice = nullptr;

namespace  {
    class TestDAttachedVfsDevice : public testing::Test {
    public:

        void SetUp() override
        {
            if(mDummyVfsDevice != nullptr)
                return;

            Stub stub;
            stub.set(ADDR(DGioMount, createFromPath), createFromPath_gioMount_stub);
            mDummyVfsDevice = new DAttachedVfsDevice(getDummyMountPoint());
        }
        void TearDown() override
        {

        }

    public:

    };
}


TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_can_unmounted)
{
    bool (QFileInfo::*exists)() const = &QFileInfo::exists;

    Stub stub;
    stub.set(ADDR(DGioMount, canUnmount), canUnmount_stub);
    stub.set(g_file_find_enclosing_mount, g_file_find_enclosing_mount_stub);
    stub.set(g_mount_unmount_with_operation, g_mount_unmount_with_operation_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_TRUE(mDummyVfsDevice->isValid());
    EXPECT_TRUE(mDummyVfsDevice->detachable());

    mDummyVfsDevice->detach();
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_can_unmounted_nodevice)
{
    Stub stub;
    stub.set( ADDR(DGioMount, canUnmount), canUnmount_stub );
    stub.set( ADDR(QFileInfo, permission), permission_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_TRUE(mDummyVfsDevice->isValid());
    EXPECT_TRUE(mDummyVfsDevice->detachable());

    mDummyVfsDevice->detach();
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_cant_unmounted_file_nopermission)
{
    bool (QFileInfo::*exists)() const = &QFileInfo::exists;

    Stub stub;
    stub.set(ADDR(DGioMount, canUnmount), canUnmount_stub);
    stub.set(g_mount_unmount_with_operation, g_mount_unmount_with_operation_stub);
    stub.set(exists, exists_true_stub);
    stub.set(ADDR(QFileInfo, permission), permission_false_stub);
    stub.set(ADDR(QFileInfo, owner), owner_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_TRUE(mDummyVfsDevice->isValid());
    EXPECT_TRUE(mDummyVfsDevice->detachable());

    mDummyVfsDevice->detach();
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_cant_unmounted_file_UUIDOK)
{
    bool (QFileInfo::*exists)() const = &QFileInfo::exists;

    Stub stub;
    stub.set(ADDR(DGioMount, canUnmount), canUnmount_stub);
    stub.set(g_mount_unmount_with_operation, g_mount_unmount_with_operation_stub);
    stub.set(exists, exists_true_stub);
    stub.set(ADDR(QFileInfo, permission), permission_false_stub);
    stub.set(ADDR(QFileInfo, owner), owner_stub);
    stub.set(ADDR(QFileInfo, ownerId), getuid_stub);
    stub.set(getuid, getuid_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_TRUE(mDummyVfsDevice->isValid());
    EXPECT_TRUE(mDummyVfsDevice->detachable());

    mDummyVfsDevice->detach();
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_cant_unmounted_file_GroupIDOK)
{
    bool (QFileInfo::*exists)() const = &QFileInfo::exists;

    Stub stub;
    stub.set(ADDR(DGioMount, canUnmount), canUnmount_stub);
    stub.set(g_mount_unmount_with_operation, g_mount_unmount_with_operation_stub);
    stub.set(exists, exists_true_stub);
    stub.set(ADDR(QFileInfo, permission), permission_false_stub);
    stub.set(ADDR(QFileInfo, owner), owner_stub);
    stub.set(ADDR(QFileInfo, groupId), groupId_stub);
    stub.set(getgid, groupId_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_TRUE(mDummyVfsDevice->isValid());
    EXPECT_TRUE(mDummyVfsDevice->detachable());

    mDummyVfsDevice->detach();
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_can_getdisplayname)
{
    Stub stub;
    stub.set(ADDR(DGioMount, name), name_stub);
    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_EQ(mDummyVfsDevice->displayName(), gio_dummy_device_name);
}


TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_can_get_iconName)
{
    Stub stub;
    stub.set(ADDR(DGioMount, name), name_stub);
    stub.set(ADDR(DGioMount, themedIconNames), themedIconNames_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_EQ(mDummyVfsDevice->iconName(), gio_dummy_device_icon_name );
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_cannot_get_iconName)
{
    Stub stub;
    stub.set(ADDR(DGioMount, name), name_stub);
    stub.set(ADDR(DGioMount, themedIconNames), themedIconNames_empty_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_EQ(mDummyVfsDevice->iconName(), QStringLiteral("drive-network") );
}


TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_can_get_mountpointUrl)
{
    Stub stub;
    stub.set(ADDR(DGioMount, getRootFile), getRootFile_stub);
    stub.set(ADDR(DGioFile, path), getDummyMountPoint);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_EQ(mDummyVfsDevice->mountpointUrl(), QUrl::fromLocalFile(getDummyMountPoint() ) );
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_can_get_accessPointUrl)
{
    Stub stub;
    stub.set(ADDR(DGioMount, getRootFile), getRootFile_stub);
    stub.set(ADDR(DGioFile, path), getDummyMountPoint);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_EQ(mDummyVfsDevice->accessPointUrl(), QUrl::fromLocalFile(getDummyMountPoint() ) );
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_deviceUsageValid)
{
    Stub stub;
    stub.set(ADDR(DGioMount, name), name_stub);
    stub.set(ADDR(DGioMount, getRootFile), getRootFile_stub);
    stub.set(ADDR(DGioFileInfo, fileType), getFileType_stub);
    stub.set(ADDR(DGioFile, createFileInfo), createFileInfo_stub);
    stub.set(ADDR(DGioFile, createFileSystemInfo), createFileSystemInfo_stub);
    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    EXPECT_TRUE(mDummyVfsDevice->deviceUsageValid() );
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_can_getUsageValid)
{
    Stub stub;
    stub.set(ADDR(DGioMount, name), name_stub);
    stub.set(ADDR(DGioMount, getRootFile), getRootFile_stub);
    stub.set(ADDR(DGioFileInfo, fileType), getFileType_stub);
    stub.set(ADDR(DGioFile, createFileInfo), createFileInfo_stub);
    stub.set(ADDR(DGioFile, createFileSystemInfo), createFileSystemInfo_stub);
    stub.set(ADDR(DGioFileInfo, fsFreeBytes), fsFreeBytes_1KB_stub);
    stub.set(ADDR(DGioFileInfo, fsTotalBytes), fsTotalBytes_2KB_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    QPair<quint64, quint64> value = QPair<quint64, quint64>(1024, 2*1024);
    QPair<quint64, quint64> curValue = mDummyVfsDevice->deviceUsage();
    EXPECT_EQ(curValue, value );
}

TEST_F(TestDAttachedVfsDevice, dummy_vfsdev_can_not_getUsageValid)
{
    Stub stub;
    stub.set(ADDR(DGioMount, name), name_stub);
    stub.set(ADDR(DGioMount, getRootFile), getRootFile_stub);
    stub.set(ADDR(DGioFile, createFileInfo), createFileSystemInfo_return_null_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    QPair<quint64, quint64> value = QPair<quint64, quint64>(0, 0);
    QPair<quint64, quint64> curValue = mDummyVfsDevice->deviceUsage();
    EXPECT_EQ(curValue, value );
}
