// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanager_cov2.cpp
 * @brief Coverage for DeviceManagerPrivate methods (devicemanager.cpp):
 *        shouldAutoMountBlockDevice, shouldAutoMountBlockDeviceAtRuntime,
 *        mountAllBlockDev, isDaemonMountRunning, handleDlnfsMount,
 *        unmountStackedMount.
 *        Private members accessed via d-> (global -fno-access-control).
 */

#include <gtest/gtest.h>
#include <QString>
#include <QVariantMap>
#include <QCoreApplication>

#include "stubext.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/private/devicemanager_p.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

using namespace dfmbase;
using namespace GlobalServerDefines;

class DeviceManagerCov2Test : public testing::Test
{
protected:
    stub_ext::StubExt stub;
    void SetUp() override { stub.clear(); }
    void TearDown() override { stub.clear(); }
};

// ---- shouldAutoMountBlockDevice ----

TEST_F(DeviceManagerCov2Test, ShouldAutoMount_Encrypted_ReturnsFalse)
{
    QVariantMap info;
    info[DeviceProperty::kIsEncrypted] = true;
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_FALSE(d->shouldAutoMountBlockDevice("/dev/sda1", info));
}

TEST_F(DeviceManagerCov2Test, ShouldAutoMount_CryptoBacking_ReturnsFalse)
{
    QVariantMap info;
    info[DeviceProperty::kCryptoBackingDevice] = "/dev/mapper/luks";
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_FALSE(d->shouldAutoMountBlockDevice("/dev/sda1", info));
}

TEST_F(DeviceManagerCov2Test, ShouldAutoMount_HintIgnore_ReturnsFalse)
{
    QVariantMap info;
    info[DeviceProperty::kHintIgnore] = true;
    info[DeviceProperty::kHasFileSystem] = true;
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_FALSE(d->shouldAutoMountBlockDevice("/dev/sda1", info));
}

TEST_F(DeviceManagerCov2Test, ShouldAutoMount_LoopDevice_ReturnsFalse)
{
    QVariantMap info;
    info[DeviceProperty::kIsLoopDevice] = true;
    info[DeviceProperty::kHasFileSystem] = true;
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_FALSE(d->shouldAutoMountBlockDevice("/dev/loop0", info));
}

TEST_F(DeviceManagerCov2Test, ShouldAutoMount_NoFileSystem_ReturnsFalse)
{
    QVariantMap info;
    info[DeviceProperty::kHasFileSystem] = false;
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_FALSE(d->shouldAutoMountBlockDevice("/dev/sda1", info));
}

TEST_F(DeviceManagerCov2Test, ShouldAutoMount_OpticalDevice_ReturnsFalse)
{
    QVariantMap info;
    info[DeviceProperty::kHasFileSystem] = true;
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_FALSE(d->shouldAutoMountBlockDevice("/org/freedesktop/UDisks2/block_devices/sr0", info));
}

TEST_F(DeviceManagerCov2Test, ShouldAutoMount_NormalDevice_ReturnsTrue)
{
    QVariantMap info;
    info[DeviceProperty::kHasFileSystem] = true;
    info[DeviceProperty::kCryptoBackingDevice] = "/";
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_TRUE(d->shouldAutoMountBlockDevice("/org/freedesktop/UDisks2/block_devices/sda1", info));
}

// ---- shouldAutoMountBlockDeviceAtRuntime ----

TEST_F(DeviceManagerCov2Test, ShouldAutoMountAtRuntime_Encrypted_ReturnsFalse)
{
    QVariantMap info;
    info[DeviceProperty::kIsEncrypted] = true;
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_FALSE(d->shouldAutoMountBlockDeviceAtRuntime("/dev/sda1", info));
}

TEST_F(DeviceManagerCov2Test, ShouldAutoMountAtRuntime_NonRemovable_ReturnsFalse)
{
    QVariantMap info;
    info[DeviceProperty::kHasFileSystem] = true;
    info[DeviceProperty::kCryptoBackingDevice] = "/";
    info[DeviceProperty::kRemovable] = false;
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_FALSE(d->shouldAutoMountBlockDeviceAtRuntime("/dev/sda1", info));
}

TEST_F(DeviceManagerCov2Test, ShouldAutoMountAtRuntime_Removable_ReturnsTrue)
{
    QVariantMap info;
    info[DeviceProperty::kHasFileSystem] = true;
    info[DeviceProperty::kCryptoBackingDevice] = "/";
    info[DeviceProperty::kRemovable] = true;
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_TRUE(d->shouldAutoMountBlockDeviceAtRuntime("/dev/sda1", info));
}

// ---- mountAllBlockDev ----

TEST_F(DeviceManagerCov2Test, MountAllBlockDev_NoMountable_NoCrash)
{
    auto *d = DeviceManager::instance()->d.data();
    EXPECT_NO_FATAL_FAILURE({ d->mountAllBlockDev(); });
}

// ---- isDaemonMountRunning ----

TEST_F(DeviceManagerCov2Test, IsDaemonMountRunning_Callable_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManagerPrivate::isDaemonMountRunning();
    });
}

// ---- handleDlnfsMount ----

TEST_F(DeviceManagerCov2Test, HandleDlnfsMount_MountDisabled_NoCrash)
{
    stub.set_lamda(&DConfigManager::value, [](void *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });
    EXPECT_NO_FATAL_FAILURE({
        DeviceManagerPrivate::handleDlnfsMount("/mnt/test", true);
    });
}

TEST_F(DeviceManagerCov2Test, HandleDlnfsMount_Unmount_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        DeviceManagerPrivate::handleDlnfsMount("/mnt/test", false);
    });
}

// ---- unmountStackedMount ----

TEST_F(DeviceManagerCov2Test, UnmountStackedMount_NoOp_NoCrash)
{
    // All code inside #if 0 — effectively a no-op
    EXPECT_NO_FATAL_FAILURE({
        DeviceManagerPrivate::unmountStackedMount("/mnt/test");
    });
}
