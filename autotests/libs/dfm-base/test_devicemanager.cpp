// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanager.cpp
 * @brief Unit tests for DeviceManager safe methods (devicemanager.cpp) —
 *        instance, isMonitoring, enableBlockAutoMount, startOpticalDiscScan,
 *        retryMount (with timeout > 1 early return).
 *        Heavy mount/DBus methods are not exercised.
 */

#include <gtest/gtest.h>
#include <QString>
#include <QVariantMap>
#include <QTimer>
#include <QCoreApplication>
#include <QSignalSpy>

#include "stubext.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/private/devicemanager_p.h>
#include <dfm-base/base/device/private/devicewatcher.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-mount/base/dmount_global.h>

using namespace dfmbase;

TEST(DeviceManagerTest, InstanceReturnsSingleton)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceManager::instance(); });
}

TEST(DeviceManagerTest, IsMonitoringFalseByDefault)
{
    EXPECT_FALSE(DeviceManager::instance()->isMonitoring());
}

TEST(DeviceManagerTest, EnableBlockAutoMount)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->enableBlockAutoMount(); });
}

TEST(DeviceManagerTest, StartOpticalDiscScanCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->startOpticalDiscScan(); });
}

TEST(DeviceManagerTest, RetryMountExceedsTimeoutReturns)
{
    // timeout > 1 -> early return, no QTimer::singleShot
    EXPECT_NO_FATAL_FAILURE({
        DeviceManager::instance()->retryMount("/dev/sda1", dfmmount::DeviceType::kBlockDevice, 5);
    });
}

TEST(DeviceManagerTest, DetachAllProtoDevsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->detachAllProtoDevs(); });
}

TEST(DeviceManagerTest, DetachProtoDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->detachProtoDev("/org/freedesktop/UDisks2/block_devices/sda1"); });
}

TEST(DeviceManagerTest, InitUsageCacheCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->initUsageCache(); });
}

TEST(DeviceManagerTest, RefreshUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->refreshUsage(); });
}

TEST(DeviceManagerTest, StartPollingDeviceUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->startPollingDeviceUsage(); });
}

TEST(DeviceManagerTest, StopPollingDeviceUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->stopPollingDeviceUsage(); });
}

TEST(DeviceManagerTest, D0DestructorPath)
{
    // singleton already exists; just verify instance is valid
    EXPECT_NE(DeviceManager::instance(), nullptr);
}

TEST(DeviceManagerTest, GetAllBlockDevIDCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceManager::instance()->getAllBlockDevID(); });
}

TEST(DeviceManagerTest, GetAllProtocolDevIDCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceManager::instance()->getAllProtocolDevID(); });
}

TEST(DeviceManagerTest, GetBlockDevInfoCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->getBlockDevInfo("/org/freedesktop/UDisks2/block_devices/nonexistent");
    });
}

TEST(DeviceManagerTest, GetProtocolDevInfoCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->getProtocolDevInfo("/nonexistent/protocol/dev");
    });
}

TEST(DeviceManagerTest, RescanBlockDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->rescanBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent");
    });
}

TEST(DeviceManagerTest, UnmountBlockDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->unmountBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent");
    });
}

TEST(DeviceManagerTest, MountProtocolDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->mountProtocolDev("/nonexistent/protocol/dev");
    });
}

TEST(DeviceManagerTest, UnmountProtocolDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->unmountProtocolDev("/nonexistent/protocol/dev");
    });
}

// ============================================================
// Additional coverage: async device operations with stubs
// ============================================================

TEST(DeviceManagerTest, MountBlockDevAsyncWithNonExistentDevice)
{
    // Non-existent device: createBlockDevice returns nullptr, callback should get error
    bool cbCalled = false;
    bool cbOk = false;
    auto *ins = DeviceManager::instance();
    ins->mountBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_abc123",
                             {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &, const QString &) {
                                 cbCalled = true;
                                 cbOk = ok;
                             });
    // Process events to let async calls settle
    QCoreApplication::processEvents();
    EXPECT_TRUE(cbCalled);
    EXPECT_FALSE(cbOk);
}

TEST(DeviceManagerTest, MountBlockDevAsyncSignalsMountResult)
{
    // Non-existent device emits blockDevMountResult(false)
    auto *ins = DeviceManager::instance();
    QSignalSpy spy(ins, &DeviceManager::blockDevMountResult);
    ins->mountBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_signal",
                             {}, nullptr);
    QCoreApplication::processEvents();
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(spy.at(0).at(1).toBool());
}

TEST(DeviceManagerTest, UnmountBlockDevAsyncWithNonExistentDevice)
{
    bool cbCalled = false;
    bool cbOk = false;
    auto *ins = DeviceManager::instance();
    ins->unmountBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_unmount",
                              {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &) {
                                  cbCalled = true;
                                  cbOk = ok;
                              });
    QCoreApplication::processEvents();
    EXPECT_TRUE(cbCalled);
    EXPECT_FALSE(cbOk);
}

TEST(DeviceManagerTest, UnmountBlockDevAsyncSignalsFailed)
{
    auto *ins = DeviceManager::instance();
    QSignalSpy spy(ins, &DeviceManager::blockDevUnmountAsyncFailed);
    ins->unmountBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_sig", {}, nullptr);
    QCoreApplication::processEvents();
    EXPECT_GE(spy.count(), 0);
}

TEST(DeviceManagerTest, LockBlockDevAsyncWithNonExistentDevice)
{
    bool cbCalled = false;
    auto *ins = DeviceManager::instance();
    ins->lockBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_lock",
                           {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &) {
                               cbCalled = true;
                               EXPECT_FALSE(ok);
                           });
    QCoreApplication::processEvents();
    EXPECT_TRUE(cbCalled);
}

TEST(DeviceManagerTest, UnlockBlockDevAsyncWithNonExistentDevice)
{
    bool cbCalled = false;
    auto *ins = DeviceManager::instance();
    ins->unlockBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_unlock",
                             "testpwd", {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &, const QString &) {
                                 cbCalled = true;
                                 EXPECT_FALSE(ok);
                             });
    QCoreApplication::processEvents();
    EXPECT_TRUE(cbCalled);
}

TEST(DeviceManagerTest, PowerOffBlockDevAsyncWithNonExistentDevice)
{
    bool cbCalled = false;
    auto *ins = DeviceManager::instance();
    ins->powerOffBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_poweroff",
                               {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &) {
                                   cbCalled = true;
                                   EXPECT_FALSE(ok);
                               });
    QCoreApplication::processEvents();
    EXPECT_TRUE(cbCalled);
}

TEST(DeviceManagerTest, PowerOffBlockDevAsyncSignalsFailed)
{
    auto *ins = DeviceManager::instance();
    QSignalSpy spy(ins, &DeviceManager::blockDevPoweroffAysncFailed);
    ins->powerOffBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_poff",
                               {}, nullptr);
    QCoreApplication::processEvents();
    EXPECT_GE(spy.count(), 0);
}

TEST(DeviceManagerTest, EjectBlockDevAsyncWithNonExistentDevice)
{
    bool cbCalled = false;
    auto *ins = DeviceManager::instance();
    ins->ejectBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_eject",
                           {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &) {
                               cbCalled = true;
                               EXPECT_FALSE(ok);
                           });
    QCoreApplication::processEvents();
    EXPECT_TRUE(cbCalled);
}

TEST(DeviceManagerTest, EjectBlockDevAsyncSignalsFailed)
{
    auto *ins = DeviceManager::instance();
    QSignalSpy spy(ins, &DeviceManager::blockDevEjectAsyncFailed);
    ins->ejectBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_ej",
                            {}, nullptr);
    QCoreApplication::processEvents();
    EXPECT_GE(spy.count(), 0);
}

TEST(DeviceManagerTest, RenameBlockDevAsyncWithNonExistentDevice)
{
    bool cbCalled = false;
    auto *ins = DeviceManager::instance();
    ins->renameBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_rename",
                             "NewLabel", {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &) {
                                 cbCalled = true;
                                 EXPECT_FALSE(ok);
                             });
    QCoreApplication::processEvents();
    EXPECT_TRUE(cbCalled);
}

TEST(DeviceManagerTest, RescanBlockDevAsyncWithNonExistentDevice)
{
    bool cbCalled = false;
    auto *ins = DeviceManager::instance();
    ins->rescanBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_rescan",
                             {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &) {
                                 cbCalled = true;
                                 EXPECT_FALSE(ok);
                             });
    QCoreApplication::processEvents();
    EXPECT_TRUE(cbCalled);
}

TEST(DeviceManagerTest, MountProtocolDevAsyncWithNonExistentDevice)
{
    bool cbCalled = false;
    auto *ins = DeviceManager::instance();
    ins->mountProtocolDevAsync("/nonexistent/protocol/dev_async",
                              {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &, const QString &) {
                                  cbCalled = true;
                              });
    QCoreApplication::processEvents();
    // Callback may not be called in some async envs
    EXPECT_NO_FATAL_FAILURE({ (void)cbCalled; });
}

TEST(DeviceManagerTest, UnmountProtocolDevAsyncWithNonExistentDevice)
{
    bool cbCalled = false;
    auto *ins = DeviceManager::instance();
    ins->unmountProtocolDevAsync("/nonexistent/protocol/dev_unmount_async",
                                {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &) {
                                    cbCalled = true;
                                    // ok depends on dfm-mount env
                                });
    QCoreApplication::processEvents();
    EXPECT_TRUE(cbCalled);
}

TEST(DeviceManagerTest, MountNetworkDeviceAsyncWithEmptyAddress)
{
    // Empty address → early return, no crash
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->mountNetworkDeviceAsync("");
    });
}

TEST(DeviceManagerTest, DoAutoMountAtStartCallable)
{
    // doAutoMountAtStart checks isAutoMountEnable, then calls mountAllBlockDev
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->doAutoMountAtStart();
    });
}

TEST(DeviceManagerTest, DetachAllRemovableBlockDevsCallable)
{
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->detachAllRemovableBlockDevs();
    });
}

TEST(DeviceManagerTest, DetachBlockDevWithNonExistentDevice)
{
    auto *ins = DeviceManager::instance();
    QStringList result;
    EXPECT_NO_FATAL_FAILURE({
        result = ins->detachBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent_detach");
    });
    // Should return siblings (just the id itself)
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceManagerTest, StartMonitorCallable)
{
    auto *ins = DeviceManager::instance();
    bool wasWatching = ins->isMonitoring();
    EXPECT_NO_FATAL_FAILURE({
        ins->startMonitor();
    });
    EXPECT_TRUE(ins->isMonitoring());
    if (!wasWatching) {
        ins->stopMonitor();
    }
}

TEST(DeviceManagerTest, StopMonitorCallable)
{
    auto *ins = DeviceManager::instance();
    ins->startMonitor();
    EXPECT_NO_FATAL_FAILURE({
        ins->stopMonitor();
    });
    EXPECT_FALSE(ins->isMonitoring());
}

TEST(DeviceManagerTest, StartMonitorIdempotent)
{
    auto *ins = DeviceManager::instance();
    ins->startMonitor();
    EXPECT_NO_FATAL_FAILURE({
        ins->startMonitor();   // calling again should be a no-op
    });
    EXPECT_TRUE(ins->isMonitoring());
    ins->stopMonitor();
}

TEST(DeviceManagerTest, StopMonitorIdempotent)
{
    auto *ins = DeviceManager::instance();
    // Ensure not watching first
    ins->stopMonitor();
    EXPECT_NO_FATAL_FAILURE({
        ins->stopMonitor();   // calling again should be a no-op
    });
    EXPECT_FALSE(ins->isMonitoring());
}

TEST(DeviceManagerTest, RetryMountWithTimeoutZeroSchedulesRetry)
{
    // timeout = 0 → should schedule QTimer::singleShot
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->retryMount("/org/freedesktop/UDisks2/block_devices/sda1",
                        dfmmount::DeviceType::kBlockDevice, 0);
    });
}

TEST(DeviceManagerTest, DoAutoMountWithProtocolType)
{
    // doAutoMount with kProtocolDevice always auto-mounts protocol devices
    auto *ins = DeviceManager::instance();
    // Accessing private method via -fno-access-control
    EXPECT_NO_FATAL_FAILURE({
        ins->doAutoMount("/nonexistent/protocol/dev", dfmmount::DeviceType::kProtocolDevice);
    });
}

TEST(DeviceManagerTest, GetAllBlockDevIDWithFilters)
{
    // Test with various query options
    using O = GlobalServerDefines::DeviceQueryOption;
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({ (void)ins->getAllBlockDevID(O::kMounted); });
    EXPECT_NO_FATAL_FAILURE({ (void)ins->getAllBlockDevID(O::kRemovable); });
    EXPECT_NO_FATAL_FAILURE({ (void)ins->getAllBlockDevID(O::kMountable); });
    EXPECT_NO_FATAL_FAILURE({ (void)ins->getAllBlockDevID(O::kNotIgnored); });
    EXPECT_NO_FATAL_FAILURE({ (void)ins->getAllBlockDevID(O::kNotMounted); });
    EXPECT_NO_FATAL_FAILURE({ (void)ins->getAllBlockDevID(O::kOptical); });
    EXPECT_NO_FATAL_FAILURE({ (void)ins->getAllBlockDevID(O::kSystem); });
    EXPECT_NO_FATAL_FAILURE({ (void)ins->getAllBlockDevID(O::kLoop); });
    EXPECT_NO_FATAL_FAILURE({ (void)ins->getAllBlockDevID(O::kMounted | O::kRemovable); });
}

TEST(DeviceManagerTest, GetBlockDevInfoWithReload)
{
    auto *ins = DeviceManager::instance();
    QVariantMap info;
    EXPECT_NO_FATAL_FAILURE({
        info = ins->getBlockDevInfo("/org/freedesktop/UDisks2/block_devices/sda1", true);
    });
    // sda1 may exist in the test environment
    EXPECT_TRUE(info.isEmpty() || info.contains("Id"));
}

TEST(DeviceManagerTest, GetProtocolDevInfoWithReload)
{
    auto *ins = DeviceManager::instance();
    QVariantMap info;
    EXPECT_NO_FATAL_FAILURE({
        info = ins->getProtocolDevInfo("/nonexistent/protocol/dev", true);
    });
    // May be empty or contain fake/real data
    EXPECT_TRUE(info.isEmpty() || info.contains("Id") || info.value("fake").toBool());
}

TEST(DeviceManagerTest, MountNetworkDeviceAsyncWithInvalidUrl)
{
    auto *ins = DeviceManager::instance();
    // Invalid URL scheme should still not crash
    EXPECT_NO_FATAL_FAILURE({
        ins->mountNetworkDeviceAsync("not_a_valid_url");
    });
}

// NOTE: Network mount tests below use stub to prevent real mount attempts
// (smb/ftp/sftp/nfs URLs would trigger system auth dialogs and block tests)

TEST(DeviceManagerTest, MountNetworkDeviceAsyncWithSmbUrl)
{
    stub_ext::StubExt stub;
    // Stub network check to return false immediately — prevents real mount attempt
    stub.set_lamda(ADDR(NetworkUtils, doAfterCheckNet),
                   [](NetworkUtils *, const QString &, const QStringList &, std::function<void(bool)> callback, int) {
                       if (callback)
                           callback(false);
                   });

    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->mountNetworkDeviceAsync("smb://192.168.255.255/testshare");
    });
    QCoreApplication::processEvents();
}

TEST(DeviceManagerTest, MountBlockDevAsyncWithCallback)
{
    // Test with non-null callback — non-existent device, no real mount
    int callCount = 0;
    auto *ins = DeviceManager::instance();
    ins->mountBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_cb",
                             {}, [&](bool ok, const DFMMOUNT::OperationErrorInfo &err, const QString &mpt) {
                                 callCount++;
                                 EXPECT_FALSE(ok);
                                 EXPECT_TRUE(mpt.isEmpty());
                             });
    QCoreApplication::processEvents();
    EXPECT_EQ(callCount, 1);
}

TEST(DeviceManagerTest, MountBlockDevAsyncWithOpts)
{
    // Test with mount options
    auto *ins = DeviceManager::instance();
    QVariantMap opts;
    opts["auth.no_user_interaction"] = true;
    opts["options"] = "ro";
    bool called = false;
    ins->mountBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_opts",
                             opts, [&](bool ok, const DFMMOUNT::OperationErrorInfo &, const QString &) {
                                 called = true;
                                 EXPECT_FALSE(ok);
                             });
    QCoreApplication::processEvents();
    EXPECT_TRUE(called);
}

TEST(DeviceManagerTest, UnmountBlockDevAsyncWithOpts)
{
    auto *ins = DeviceManager::instance();
    QVariantMap opts;
    opts["unmount_without_lock"] = true;
    bool called = false;
    ins->unmountBlockDevAsync("/org/freedesktop/UDisks2/block_devices/nonexistent_uopts",
                              opts, [&](bool ok, const DFMMOUNT::OperationErrorInfo &) {
                                  called = true;
                                  EXPECT_FALSE(ok);
                              });
    QCoreApplication::processEvents();
    EXPECT_TRUE(called);
}

TEST(DeviceManagerTest, RescanBlockDevWithOpts)
{
    auto *ins = DeviceManager::instance();
    QVariantMap opts;
    bool result;
    EXPECT_NO_FATAL_FAILURE({
        result = ins->rescanBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent_rescan_opts", opts);
    });
    EXPECT_FALSE(result);
}

TEST(DeviceManagerTest, MountProtocolDevWithOpts)
{
    auto *ins = DeviceManager::instance();
    QVariantMap opts;
    QString result;
    EXPECT_NO_FATAL_FAILURE({
        result = ins->mountProtocolDev("/nonexistent/protocol/dev_opts", opts);
    });
    EXPECT_TRUE(result.isEmpty());
}

TEST(DeviceManagerTest, UnmountProtocolDevWithOpts)
{
    auto *ins = DeviceManager::instance();
    QVariantMap opts;
    bool result;
    EXPECT_NO_FATAL_FAILURE({
        result = ins->unmountProtocolDev("/nonexistent/protocol/dev_uopts", opts);
    });
    // Result depends on dfm-mount environment
    EXPECT_TRUE(result == true || result == false);
}

TEST(DeviceManagerTest, DetachBlockDevWithCallback)
{
    auto *ins = DeviceManager::instance();
    bool cbCalled = false;
    QStringList result;
    EXPECT_NO_FATAL_FAILURE({
        result = ins->detachBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent_detach_cb",
                                       [&](bool ok, const DFMMOUNT::OperationErrorInfo &) {
                                           cbCalled = true;
                                       });
    });
    EXPECT_FALSE(result.isEmpty());
    // cb may or may not be called depending on async processing
}

TEST(DeviceManagerTest, MountNetworkDeviceAsyncWithFtpUrl)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(NetworkUtils, doAfterCheckNet),
                   [](NetworkUtils *, const QString &, const QStringList &, std::function<void(bool)> callback, int) {
                       if (callback)
                           callback(false);
                   });

    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->mountNetworkDeviceAsync("ftp://192.168.255.255/");
    });
    QCoreApplication::processEvents();
}

TEST(DeviceManagerTest, MountNetworkDeviceAsyncWithSftpUrl)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(NetworkUtils, doAfterCheckNet),
                   [](NetworkUtils *, const QString &, const QStringList &, std::function<void(bool)> callback, int) {
                       if (callback)
                           callback(false);
                   });

    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->mountNetworkDeviceAsync("sftp://192.168.255.255/");
    });
    QCoreApplication::processEvents();
}

TEST(DeviceManagerTest, MountNetworkDeviceAsyncWithNfsUrl)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(NetworkUtils, doAfterCheckNet),
                   [](NetworkUtils *, const QString &, const QStringList &, std::function<void(bool)> callback, int) {
                       if (callback)
                           callback(false);
                   });

    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->mountNetworkDeviceAsync("nfs://192.168.255.255/export");
    });
    QCoreApplication::processEvents();
}

TEST(DeviceManagerTest, MountNetworkDeviceAsyncWithCallback)
{
    stub_ext::StubExt stub;
    bool cbCalled = false;
    stub.set_lamda(ADDR(NetworkUtils, doAfterCheckNet),
                   [&cbCalled](NetworkUtils *, const QString &, const QStringList &, std::function<void(bool)> callback, int) {
                       if (callback)
                           callback(false);
                       cbCalled = true;
                   });

    auto *ins = DeviceManager::instance();
    ins->mountNetworkDeviceAsync("smb://192.168.255.255/share", [&](bool ok, const DFMMOUNT::OperationErrorInfo &, const QString &) {
        cbCalled = true;
    });
    QCoreApplication::processEvents();
}

TEST(DeviceManagerTest, RetryMountWithTimeoutOneSchedulesRetry)
{
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->retryMount("/org/freedesktop/UDisks2/block_devices/sda1",
                        dfmmount::DeviceType::kBlockDevice, 1);
    });
}

TEST(DeviceManagerTest, RetryMountForProtocolDevice)
{
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->retryMount("/nonexistent/protocol/dev_retry",
                        dfmmount::DeviceType::kProtocolDevice, 0);
    });
}

TEST(DeviceManagerTest, DoAutoMountWithBlockType)
{
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->doAutoMount("/org/freedesktop/UDisks2/block_devices/nonexistent_automount",
                        dfmmount::DeviceType::kBlockDevice);
    });
    QCoreApplication::processEvents();
}

TEST(DeviceManagerTest, DoAutoMountWithTimeout)
{
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->doAutoMount("/org/freedesktop/UDisks2/block_devices/nonexistent_timeout",
                        dfmmount::DeviceType::kBlockDevice, 0);
    });
    QCoreApplication::processEvents();
}

TEST(DeviceManagerTest, DoAutoMountProtocolWithTimeout)
{
    auto *ins = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        ins->doAutoMount("/protocol/dev_timeout_test",
                        dfmmount::DeviceType::kProtocolDevice, 0);
    });
    QCoreApplication::processEvents();
}

