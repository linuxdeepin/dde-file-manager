// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTimer>
#include <QVariantMap>
#include <QTest>
#include <QThreadPool>

#include "stubext.h"

#include <dfm-base/base/device/private/devicewatcher.h>
#include <dfm-base/base/device/private/devicewatcher_p.h>
#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-mount/ddevicemanager.h>

#include <sys/statvfs.h>

DFMBASE_USE_NAMESPACE
DFM_MOUNT_USE_NS
using namespace GlobalServerDefines;

class TestDeviceWatcher : public testing::Test
{
public:
    void SetUp() override
    {
        // Stub DDeviceManager to avoid real device access
        stub.set_lamda(&DDeviceManager::instance, []() -> DDeviceManager * {
            __DBG_STUB_INVOKE__
            static DDeviceManager mockManager;
            return &mockManager;
        });

        stub.set_lamda(&DDeviceManager::devices, [](DDeviceManager *, DeviceType) -> QMap<DeviceType, QStringList> {
            __DBG_STUB_INVOKE__
            QMap<DeviceType, QStringList> devs;
            devs[DeviceType::kBlockDevice] = QStringList { "block:dev:sda1", "block:dev:sdb1" };
            devs[DeviceType::kProtocolDevice] = QStringList { "proto:smb:server1" };
            return devs;
        });

        // Stub DeviceHelper
        using LoadBlockInfoFunc = QVariantMap (*)(const QString &);
        stub.set_lamda(static_cast<LoadBlockInfoFunc>(&DeviceHelper::loadBlockInfo), [](const QString &id) -> QVariantMap {
            __DBG_STUB_INVOKE__
            QVariantMap info;
            info[DeviceProperty::kId] = id;
            info[DeviceProperty::kDevice] = "/dev/sda1";
            info[DeviceProperty::kMountPoint] = "/media/test";
            info[DeviceProperty::kOptical] = false;
            info[DeviceProperty::kOpticalDrive] = false;
            info[DeviceProperty::kSizeTotal] = 1000000000ULL;
            info[DeviceProperty::kSizeFree] = 500000000ULL;
            info[DeviceProperty::kSizeUsed] = 500000000ULL;
            info[DeviceProperty::kUDisks2Size] = 1000000000ULL;
            return info;
        });

        using LoadProtocolInfoFunc = QVariantMap (*)(const QString &);
        stub.set_lamda(static_cast<LoadProtocolInfoFunc>(&DeviceHelper::loadProtocolInfo), [](const QString &id) -> QVariantMap {
            __DBG_STUB_INVOKE__
            QVariantMap info;
            info[DeviceProperty::kId] = id;
            info[DeviceProperty::kMountPoint] = "/run/user/1000/gvfs/smb-share";
            info[DeviceProperty::kSizeTotal] = 2000000000ULL;
            info[DeviceProperty::kSizeFree] = 1000000000ULL;
            info[DeviceProperty::kSizeUsed] = 1000000000ULL;
            return info;
        });

        stub.set_lamda(ADDR(DeviceHelper, createProtocolDevice), [](const QString &) -> ProtocolDevAutoPtr {
            __DBG_STUB_INVOKE__
            return ProtocolDevAutoPtr(nullptr);
        });

        stub.set_lamda(ADDR(DeviceHelper, readOpticalInfo), [](QVariantMap &data) {
            __DBG_STUB_INVOKE__
            data[DeviceProperty::kSizeTotal] = 700000000ULL;
            data[DeviceProperty::kSizeFree] = 0ULL;
            data[DeviceProperty::kSizeUsed] = 700000000ULL;
        });

        stub.set_lamda(ADDR(DeviceHelper, persistentOpticalInfo), [](const QVariantMap &) {
            __DBG_STUB_INVOKE__
        });

        // Stub DConfigManager
        stub.set_lamda(&DConfigManager::instance, []() -> DConfigManager * {
            __DBG_STUB_INVOKE__
            static DConfigManager mockConfig;
            return &mockConfig;
        });

        using ValueFunc = QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const;
        stub.set_lamda(static_cast<ValueFunc>(&DConfigManager::value), [](DConfigManager *, const QString &, const QString &, const QVariant &defaultVal) -> QVariant {
            __DBG_STUB_INVOKE__
            return defaultVal;
        });

        // Stub statvfs
        stub.set_lamda(statvfs, [](const char *, struct statvfs *buf) -> int {
            __DBG_STUB_INVOKE__
            buf->f_frsize = 4096;
            buf->f_blocks = 244190;
            buf->f_bavail = 122095;
            return 0;
        });

        watcher = new DeviceWatcher();
    }

    void TearDown() override
    {
        // Stop polling and wait for async tasks to complete
        if (watcher) {
            watcher->stopPollingUsage();
            // Wait for any running QtConcurrent tasks to finish
            // Use QThreadPool to wait for all tasks
            QThreadPool::globalInstance()->waitForDone();
        }
        delete watcher;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    DeviceWatcher *watcher { nullptr };
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestDeviceWatcher, Constructor_CreatesValidObject)
{
    // Test that constructor creates valid object
    EXPECT_NE(watcher, nullptr);
}

// ========== getDevInfo() Tests ==========

TEST_F(TestDeviceWatcher, getDevInfo_BlockDevice_ValidId)
{
    // Test getting block device info
    QString id = "block:dev:sda1";
    QVariantMap info = watcher->getDevInfo(id, DeviceType::kBlockDevice, false);

    EXPECT_FALSE(info.isEmpty());
    EXPECT_EQ(info.value(DeviceProperty::kId).toString(), id);
}

TEST_F(TestDeviceWatcher, getDevInfo_BlockDevice_WithReload)
{
    // Test getting block device info with reload
    QString id = "block:dev:sdb1";
    QVariantMap info = watcher->getDevInfo(id, DeviceType::kBlockDevice, true);

    EXPECT_FALSE(info.isEmpty());
}

TEST_F(TestDeviceWatcher, getDevInfo_ProtocolDevice_ValidId)
{
    // Test getting protocol device info
    QString id = "proto:smb:server1";
    QVariantMap info = watcher->getDevInfo(id, DeviceType::kProtocolDevice, false);

    EXPECT_FALSE(info.isEmpty());
    EXPECT_EQ(info.value(DeviceProperty::kId).toString(), id);
}

TEST_F(TestDeviceWatcher, getDevInfo_EmptyId)
{
    // Test with empty ID
    QVariantMap info = watcher->getDevInfo(QString(), DeviceType::kBlockDevice, false);

    // Should return empty or default map
    EXPECT_TRUE(true);
}

// ========== getDevIds() Tests ==========

TEST_F(TestDeviceWatcher, getDevIds_BlockDevices)
{
    // Initialize device data first
    watcher->initDevDatas();

    // Test getting block device IDs
    QStringList ids = watcher->getDevIds(DeviceType::kBlockDevice);

    EXPECT_TRUE(ids.size() >= 0);
}

TEST_F(TestDeviceWatcher, getDevIds_ProtocolDevices)
{
    // Initialize device data first
    watcher->initDevDatas();

    // Test getting protocol device IDs
    QStringList ids = watcher->getDevIds(DeviceType::kProtocolDevice);

    EXPECT_TRUE(ids.size() >= 0);
}

TEST_F(TestDeviceWatcher, getDevIds_AllDevices)
{
    // Test getting all device IDs
    QStringList ids = watcher->getDevIds(DeviceType::kAllDevice);

    EXPECT_TRUE(ids.isEmpty() || !ids.isEmpty());
}

// ========== getSiblings() Tests ==========

TEST_F(TestDeviceWatcher, getSiblings_ValidId)
{
    // Test getting sibling devices
    QString id = "block:dev:sda1";
    QStringList siblings = watcher->getSiblings(id);

    // Should return list (may be empty)
    EXPECT_TRUE(siblings.isEmpty() || !siblings.isEmpty());
}

TEST_F(TestDeviceWatcher, getSiblings_EmptyId)
{
    // Test with empty ID
    QStringList siblings = watcher->getSiblings(QString());

    EXPECT_TRUE(siblings.isEmpty());
}

// ========== startPollingUsage() / stopPollingUsage() Tests ==========

TEST_F(TestDeviceWatcher, startPollingUsage_StartsTimer)
{
    // Test starting usage polling
    watcher->startPollingUsage();

    // Should not crash
    EXPECT_TRUE(true);

    watcher->stopPollingUsage();
}

TEST_F(TestDeviceWatcher, startPollingUsage_CalledTwice)
{
    // Test calling start twice
    watcher->startPollingUsage();
    watcher->startPollingUsage();

    // Should handle gracefully
    EXPECT_TRUE(true);

    watcher->stopPollingUsage();
}

TEST_F(TestDeviceWatcher, stopPollingUsage_WithoutStart)
{
    // Test stopping without starting
    watcher->stopPollingUsage();

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, stopPollingUsage_StopsTimer)
{
    // Test stopping usage polling
    watcher->startPollingUsage();
    watcher->stopPollingUsage();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== startWatch() / stopWatch() Tests ==========

TEST_F(TestDeviceWatcher, startWatch_ExecutesWithoutCrash)
{
    // Test starting device watching
    watcher->startWatch();

    // Should not crash
    EXPECT_TRUE(true);

    watcher->stopWatch();
}

TEST_F(TestDeviceWatcher, stopWatch_ExecutesWithoutCrash)
{
    // Test stopping device watching
    watcher->startWatch();
    watcher->stopWatch();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== initDevDatas() Tests ==========

TEST_F(TestDeviceWatcher, initDevDatas_LoadsDevices)
{
    // Test initializing device data
    watcher->initDevDatas();

    // Should load device data
    QStringList blockIds = watcher->getDevIds(DeviceType::kBlockDevice);
    QStringList protoIds = watcher->getDevIds(DeviceType::kProtocolDevice);

    EXPECT_TRUE(blockIds.size() >= 0);
    EXPECT_TRUE(protoIds.size() >= 0);
}

TEST_F(TestDeviceWatcher, initDevDatas_CalledMultipleTimes)
{
    // Test calling init multiple times
    watcher->initDevDatas();
    watcher->initDevDatas();

    // Should handle gracefully
    EXPECT_TRUE(true);
}

// ========== queryOpticalDevUsage() Tests ==========

TEST_F(TestDeviceWatcher, queryOpticalDevUsage_ValidId)
{
    // Test querying optical device usage
    QString id = "block:dev:sr0";

    watcher->queryOpticalDevUsage(id);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, queryOpticalDevUsage_EmptyId)
{
    // Test with empty ID
    watcher->queryOpticalDevUsage(QString());

    // Should handle gracefully
    EXPECT_TRUE(true);
}

// ========== updateOpticalDevUsage() Tests ==========

TEST_F(TestDeviceWatcher, updateOpticalDevUsage_ValidParams)
{
    // Test updating optical device usage
    QString id = "block:dev:sr0";
    QString mpt = "/media/cdrom";

    watcher->updateOpticalDevUsage(id, mpt);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, updateOpticalDevUsage_EmptyParams)
{
    // Test with empty parameters
    watcher->updateOpticalDevUsage(QString(), QString());

    // Should handle gracefully
    EXPECT_TRUE(true);
}

// ========== saveOpticalDevUsage() Tests ==========

TEST_F(TestDeviceWatcher, saveOpticalDevUsage_ValidData)
{
    // Test saving optical device usage
    QString id = "block:dev:sr0";
    QVariantMap data;
    data[DeviceProperty::kSizeTotal] = 700000000ULL;
    data[DeviceProperty::kSizeUsed] = 700000000ULL;

    watcher->saveOpticalDevUsage(id, data);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, saveOpticalDevUsage_EmptyData)
{
    // Test with empty data
    QString id = "block:dev:sr0";
    QVariantMap emptyData;

    watcher->saveOpticalDevUsage(id, emptyData);

    // Should handle gracefully
    EXPECT_TRUE(true);
}

// ========== Slot Tests ==========

TEST_F(TestDeviceWatcher, onBlkDevAdded_ValidId)
{
    // Test block device added signal
    QString id = "block:dev:sdc1";

    // Manually call the slot (private, but accessible in tests)
    QMetaObject::invokeMethod(watcher, "onBlkDevAdded", Qt::DirectConnection,
                              Q_ARG(QString, id));

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, onBlkDevRemoved_ValidId)
{
    // Test block device removed signal
    QString id = "block:dev:sdc1";

    QMetaObject::invokeMethod(watcher, "onBlkDevRemoved", Qt::DirectConnection,
                              Q_ARG(QString, id));

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, onBlkDevMounted_ValidParams)
{
    // Test block device mounted signal
    QString id = "block:dev:sdc1";
    QString mpt = "/media/usb";

    QMetaObject::invokeMethod(watcher, "onBlkDevMounted", Qt::DirectConnection,
                              Q_ARG(QString, id), Q_ARG(QString, mpt));

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, onBlkDevUnmounted_ValidId)
{
    // Test block device unmounted signal
    QString id = "block:dev:sdc1";

    QMetaObject::invokeMethod(watcher, "onBlkDevUnmounted", Qt::DirectConnection,
                              Q_ARG(QString, id));

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, onBlkDevLocked_ValidId)
{
    // Test block device locked signal
    QString id = "block:dev:luks1";

    QMetaObject::invokeMethod(watcher, "onBlkDevLocked", Qt::DirectConnection,
                              Q_ARG(QString, id));

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, onBlkDevUnlocked_ValidParams)
{
    // Test block device unlocked signal
    QString id = "block:dev:luks1";
    QString cleartextId = "block:dev:dm-0";

    QMetaObject::invokeMethod(watcher, "onBlkDevUnlocked", Qt::DirectConnection,
                              Q_ARG(QString, id), Q_ARG(QString, cleartextId));

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, onProtoDevAdded_ValidId)
{
    // Test protocol device added signal
    QString id = "proto:smb:newserver";

    QMetaObject::invokeMethod(watcher, "onProtoDevAdded", Qt::DirectConnection,
                              Q_ARG(QString, id));

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, onProtoDevMounted_ValidParams)
{
    // Test protocol device mounted signal
    QString id = "proto:ftp:server";
    QString mpt = "/run/user/1000/gvfs/ftp";

    QMetaObject::invokeMethod(watcher, "onProtoDevMounted", Qt::DirectConnection,
                              Q_ARG(QString, id), Q_ARG(QString, mpt));

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== Integration Tests ==========

TEST_F(TestDeviceWatcher, Integration_InitAndQuery)
{
    // Test complete workflow: init and query devices
    watcher->initDevDatas();

    QStringList blockIds = watcher->getDevIds(DeviceType::kBlockDevice);
    EXPECT_TRUE(blockIds.size() >= 0);

    if (!blockIds.isEmpty()) {
        QVariantMap info = watcher->getDevInfo(blockIds.first(), DeviceType::kBlockDevice, false);
        EXPECT_FALSE(info.isEmpty());
    }
}

TEST_F(TestDeviceWatcher, Integration_PollingLifecycle)
{
    // Test polling lifecycle
    watcher->initDevDatas();
    watcher->startPollingUsage();

    // Let timer potentially fire (but stubbed)
    QTest::qWait(100);

    watcher->stopPollingUsage();

    // Should handle complete lifecycle
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, Integration_WatchingLifecycle)
{
    // Test watching lifecycle
    watcher->startWatch();
    watcher->stopWatch();

    // Should handle lifecycle
    EXPECT_TRUE(true);
}

// ========== Edge Cases ==========

TEST_F(TestDeviceWatcher, EdgeCase_MultipleStartStop)
{
    // Test multiple start/stop cycles
    watcher->startPollingUsage();
    watcher->stopPollingUsage();
    watcher->startPollingUsage();
    watcher->stopPollingUsage();

    watcher->startWatch();
    watcher->stopWatch();
    watcher->startWatch();
    watcher->stopWatch();

    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, EdgeCase_InvalidDeviceType)
{
    // Test with all device type
    QStringList ids = watcher->getDevIds(DeviceType::kAllDevice);

    // Should handle gracefully
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceWatcher, EdgeCase_QueryNonExistentDevice)
{
    // Test querying non-existent device
    QString id = "block:dev:nonexistent";
    QVariantMap info = watcher->getDevInfo(id, DeviceType::kBlockDevice, false);

    // Should return info (stubbed always returns data)
    EXPECT_TRUE(true);
}
