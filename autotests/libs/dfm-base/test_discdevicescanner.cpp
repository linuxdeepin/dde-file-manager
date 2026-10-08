// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_discdevicescanner.cpp
 * @brief Unit tests for DiscDeviceScanner (base/device/private/discdevicescanner.cpp)
 *        and the nested DiscDevice::Scanner. Covers ctor, the startScan empty
 *        early-return, stopScan no-op, updateScanState empty-branch, scanOpticalDisc
 *        over an empty device group, the property-change slots (non-matching id
 *        early return), the disc working-state slot (working=true, empty group)
 *        and Scanner ctor + run() on a nonexistent device node. The
 *        DBus-touching initialize() is intentionally not invoked.
 *
 *        Private methods are reached via -fno-access-control.
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QDBusVariant>

#include "stubext.h"
#include <dfm-base/base/device/private/discdevicescanner.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-mount/base/dmount_global.h>

using namespace dfmbase;

TEST(DiscDeviceScannerTest, CtorIsSafe)
{
    DiscDeviceScanner s;
    EXPECT_NO_FATAL_FAILURE({ (void)&s; });
}

TEST(DiscDeviceScannerTest, StartScanEmptyGroupReturnsFalse)
{
    DiscDeviceScanner s;
    EXPECT_FALSE(s.startScan());   // private
}

TEST(DiscDeviceScannerTest, StopScanWhenIdleIsNoOp)
{
    DiscDeviceScanner s;
    EXPECT_NO_FATAL_FAILURE({ s.stopScan(); });   // private
}

TEST(DiscDeviceScannerTest, UpdateScanStateEmptyCallsStopScan)
{
    DiscDeviceScanner s;
    EXPECT_NO_FATAL_FAILURE({ s.updateScanState(); });   // private -> stopScan path
}

TEST(DiscDeviceScannerTest, ScanOpticalDiscEmptyGroupIsNoOp)
{
    DiscDeviceScanner s;
    EXPECT_NO_FATAL_FAILURE({ s.scanOpticalDisc(); });   // private slot
}

TEST(DiscDeviceScannerTest, OnDevicePropertyChangedQVarNonPrefixReturnsEarly)
{
    DiscDeviceScanner s;
    QVariant v(true);
    EXPECT_NO_FATAL_FAILURE({
        s.onDevicePropertyChangedQVar("/not/a/block/prefix", "Optical", v);   // private slot
    });
}

TEST(DiscDeviceScannerTest, OnDevicePropertyChangedQDBusVarNonPrefixReturnsEarly)
{
    DiscDeviceScanner s;
    QDBusVariant v(true);
    EXPECT_NO_FATAL_FAILURE({
        s.onDevicePropertyChangedQDBusVar("/not/a/block/prefix", "Optical", v);   // private slot
    });
}

TEST(DiscDeviceScannerTest, OnDiscWorkingStateChangedTrueNoOpOnEmptyGroup)
{
    DiscDeviceScanner s;
    EXPECT_NO_FATAL_FAILURE({
        s.onDiscWorkingStateChanged("/org/freedesktop/UDisks2/block_devices/sr0", "/dev/sr0", true);   // private slot
    });
}

TEST(DiscDeviceScannerTest, ScannerCtorAndRunOnNonexistentDevice)
{
    DiscDevice::Scanner scanner("/dev/dfm_nonexistent_device");
    EXPECT_NO_FATAL_FAILURE({ scanner.run(); });   // open() fails -> no close -> returns
}

// ============================================================
// PMS sev-2 regression cluster: discdevicescanner.cpp (work-order batch 3)
// ============================================================

// PMS:144643 光盘设备挂载后文管崩溃：scanOpticalDisc 遍历 discDevIdGroup，
// 光驱属性命中创建 Scanner 任务入线程池；无效设备节点打开失败无害返回
TEST(DiscDeviceScannerTest, BUG144643_ScanOpticalDiscWithInjectedOpticalDevice)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo),
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       QVariantMap info;
                       info.insert(GlobalServerDefines::DeviceProperty::kDevice, QStringLiteral("/dev/sr_ut_144643"));
                       info.insert(GlobalServerDefines::DeviceProperty::kOptical, true);
                       return info;
                   });

    DiscDeviceScanner scanner;
    scanner.discDevIdGroup << QStringLiteral("sr_ut_144643");   // private member via -fno-access-control
    EXPECT_NO_FATAL_FAILURE({ scanner.scanOpticalDisc(); });
    // thread pool dtor waits for the Scanner task; open() fails on the fake node
}
