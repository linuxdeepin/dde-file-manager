// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanager_pms.cpp
 * @brief PMS sev-2 regression tests for devicemanager.cpp.
 *
 * Bug -> case mapping (fix commit verified via `git show`):
 *   - PMS:225891 (1ea0b75c) block devices without a filesystem must never
 *     be auto-mounted. The original "no-fs" guard lives on in
 *     DeviceManagerPrivate::shouldAutoMountBlockDevice() which also skips
 *     encrypted/ignored/loop/optical devices.
 *
 * Note: the drive auto-mount path is guarded by UDisks-backed queries;
 * the predicate itself is a pure function of (id, info) so it can be
 * exercised without DBus or real hardware.
 */

#include <gtest/gtest.h>

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/private/devicemanager_p.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QVariantMap>

using namespace dfmbase;
using namespace GlobalServerDefines;

class UT_DeviceManagerPms : public testing::Test
{
protected:
    void SetUp() override
    {
        m = DeviceManager::instance();
        ASSERT_NE(m, nullptr);
        d = m->d.data();   // private d-ptr, accessible via -fno-access-control
        ASSERT_NE(d, nullptr);
    }

    DeviceManager *m = nullptr;
    DeviceManagerPrivate *d = nullptr;
};

// PMS:225891 插入无文件系统的设备（裸盘/损坏分区）时仍尝试挂载导致报错弹窗：
// 无文件系统的块设备必须被自动挂载前置检查拒绝
TEST_F(UT_DeviceManagerPms, BUG225891_BlockDeviceWithoutFilesystemSkipped)
{
    // Arrange — a plain block device that carries no filesystem.
    QVariantMap info;
    info.insert(DeviceProperty::kHasFileSystem, false);
    info.insert(DeviceProperty::kRemovable, true);

    // Act / Assert
    EXPECT_FALSE(d->shouldAutoMountBlockDevice(QStringLiteral("/org/freedesktop/UDisks2/block_devices/ut225891"), info));
    // The runtime variant applies the same rule.
    EXPECT_FALSE(d->shouldAutoMountBlockDeviceAtRuntime(QStringLiteral("/org/freedesktop/UDisks2/block_devices/ut225891"), info));

    // Missing kHasFileSystem key must not be treated as "has filesystem".
    QVariantMap noKey;
    EXPECT_FALSE(d->shouldAutoMountBlockDevice(QStringLiteral("/org/freedesktop/UDisks2/block_devices/ut225891b"), noKey));
}

// PMS:225891 修复不得误伤：可挂载的普通可移动块设备仍允许自动挂载
TEST_F(UT_DeviceManagerPms, BUG225891_MountableRemovableBlockDeviceAllowed)
{
    // Arrange — an ordinary, mountable, non-encrypted removable device.
    // Note: the current contract requires an explicit crypto backing value
    // of "/" to prove the device is not an encrypted container.
    QVariantMap info;
    info.insert(DeviceProperty::kHasFileSystem, true);
    info.insert(DeviceProperty::kRemovable, true);
    info.insert(DeviceProperty::kIsEncrypted, false);
    info.insert(DeviceProperty::kCryptoBackingDevice, QStringLiteral("/"));
    info.insert(DeviceProperty::kIsLoopDevice, false);
    info.insert(DeviceProperty::kHintIgnore, false);

    // Act / Assert
    EXPECT_TRUE(d->shouldAutoMountBlockDevice(QStringLiteral("/org/freedesktop/UDisks2/block_devices/sdb1"), info));
    EXPECT_TRUE(d->shouldAutoMountBlockDeviceAtRuntime(QStringLiteral("/org/freedesktop/UDisks2/block_devices/sdb1"), info));
}

// PMS:225891 自动挂载前置检查同样要排除加密盘/忽略盘/loop/光驱，防止误挂载
TEST_F(UT_DeviceManagerPms, BUG225891_EncryptedIgnoredLoopAndOpticalSkipped)
{
    // Encrypted device
    QVariantMap encrypted;
    encrypted.insert(DeviceProperty::kHasFileSystem, true);
    encrypted.insert(DeviceProperty::kIsEncrypted, true);
    EXPECT_FALSE(d->shouldAutoMountBlockDevice(QStringLiteral("/org/freedesktop/UDisks2/block_devices/dm0"), encrypted));

    // Encrypted via crypto backing device (LUKS container not unlocked)
    QVariantMap luks;
    luks.insert(DeviceProperty::kHasFileSystem, true);
    luks.insert(DeviceProperty::kCryptoBackingDevice, QStringLiteral("/dev/sda5"));
    EXPECT_FALSE(d->shouldAutoMountBlockDevice(QStringLiteral("/org/freedesktop/UDisks2/block_devices/luks1"), luks));

    // Device blacklisted via hint ignore
    QVariantMap ignored;
    ignored.insert(DeviceProperty::kHasFileSystem, true);
    ignored.insert(DeviceProperty::kHintIgnore, true);
    EXPECT_FALSE(d->shouldAutoMountBlockDevice(QStringLiteral("/org/freedesktop/UDisks2/block_devices/sdz9"), ignored));

    // Loop device
    QVariantMap loop;
    loop.insert(DeviceProperty::kHasFileSystem, true);
    loop.insert(DeviceProperty::kIsLoopDevice, true);
    EXPECT_FALSE(d->shouldAutoMountBlockDevice(QStringLiteral("/org/freedesktop/UDisks2/block_devices/loop0"), loop));

    // Optical drive (sr prefix)
    QVariantMap optical;
    optical.insert(DeviceProperty::kHasFileSystem, true);
    optical.insert(DeviceProperty::kCryptoBackingDevice, QStringLiteral("/"));
    EXPECT_FALSE(d->shouldAutoMountBlockDevice(QStringLiteral("/org/freedesktop/UDisks2/block_devices/sr0"), optical));

    // Runtime auto-mount additionally requires a removable device.
    QVariantMap internal;
    internal.insert(DeviceProperty::kHasFileSystem, true);
    internal.insert(DeviceProperty::kRemovable, false);
    EXPECT_FALSE(d->shouldAutoMountBlockDeviceAtRuntime(QStringLiteral("/org/freedesktop/UDisks2/block_devices/nvme0n1p2"), internal));
}
