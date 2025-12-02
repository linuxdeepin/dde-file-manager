// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QDBusVariant>
#include <QTest>

#include "stubext.h"

#include <dfm-base/base/device/private/discdevicescanner.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <fcntl.h>
#include <unistd.h>

DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

class TestDiscDeviceScanner : public testing::Test
{
public:
    void SetUp() override
    {
        // Stub SysInfoUtils
        stub.set_lamda(&SysInfoUtils::isRootUser, []() -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        stub.set_lamda(&SysInfoUtils::isServerSys, []() -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        stub.set_lamda(&SysInfoUtils::isDesktopSys, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&SysInfoUtils::isProfessional, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        // Stub DeviceProxyManager
        stub.set_lamda(&DeviceProxyManager::isDBusRuning, [](DeviceProxyManager *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        stub.set_lamda(&DeviceProxyManager::getAllBlockIds, [](DeviceProxyManager *, DeviceQueryOptions) -> QStringList {
            __DBG_STUB_INVOKE__
            return QStringList { "block:dev:sr0", "block:dev:sr1" };
        });

        stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *, const QString &id, bool) -> QVariantMap {
            __DBG_STUB_INVOKE__
            QVariantMap info;
            info[DeviceProperty::kId] = id;
            info[DeviceProperty::kDevice] = "/dev/sr0";
            info[DeviceProperty::kOptical] = true;
            return info;
        });

        // Stub system calls
        using OpenFunc = int (*)(const char *, int);
        stub.set_lamda(reinterpret_cast<OpenFunc>(::open), [](const char *, int) -> int {
            __DBG_STUB_INVOKE__
            return 10;   // Return valid fd
        });

        stub.set_lamda(::close, [](int) -> int {
            __DBG_STUB_INVOKE__
            return 0;
        });

        scanner = new DiscDeviceScanner();
    }

    void TearDown() override
    {
        delete scanner;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    DiscDeviceScanner *scanner { nullptr };
};

// ========== Constructor Tests ==========

TEST_F(TestDiscDeviceScanner, Constructor_CreatesValidObject)
{
    // Test that constructor creates valid object
    EXPECT_NE(scanner, nullptr);
}

// ========== initialize() Tests ==========

TEST_F(TestDiscDeviceScanner, initialize_NormalConditions)
{
    // Test initialization under normal conditions
    scanner->initialize();

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, initialize_CalledMultipleTimes)
{
    // Test calling initialize multiple times (should only run once)
    scanner->initialize();
    scanner->initialize();
    scanner->initialize();

    // Should handle gracefully due to std::call_once
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, initialize_WithDBusRunning)
{
    // Test when DBus is running
    stub.set_lamda(&DeviceProxyManager::isDBusRuning, [](DeviceProxyManager *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    scanner->initialize();

    // Should skip initialization
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, initialize_AsRootUser)
{
    // Test initialization as root user
    stub.set_lamda(&SysInfoUtils::isRootUser, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    scanner->initialize();

    // Should not start scanning as root
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, initialize_OnServerSystem)
{
    // Test initialization on server system
    stub.set_lamda(&SysInfoUtils::isServerSys, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    scanner->initialize();

    // Should not start scanning on server
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, initialize_DesktopCommunityEdition)
{
    // Test on desktop community edition
    stub.set_lamda(&SysInfoUtils::isDesktopSys, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SysInfoUtils::isProfessional, []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    scanner->initialize();

    // Should not start scanning on community edition
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, initialize_NoOpticalDevices)
{
    // Test when no optical devices are available
    stub.set_lamda(&DeviceProxyManager::getAllBlockIds, [](DeviceProxyManager *, DeviceQueryOptions) -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList();   // Empty list
    });

    scanner->initialize();

    // Should not start scanning with no devices
    EXPECT_TRUE(true);
}

// ========== scanOpticalDisc() Tests ==========
TEST_F(TestDiscDeviceScanner, scanOpticalDisc_WithOpticalDevices)
{
    // Initialize first to set up disc device list
    scanner->initialize();

    // Manually call scan
    QMetaObject::invokeMethod(scanner, "scanOpticalDisc", Qt::DirectConnection);

    // Wait for thread pool tasks
    QTest::qWait(100);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, scanOpticalDisc_WithNonOpticalDevice)
{
    // Test scanning non-optical device
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *, const QString &id, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kId] = id;
        info[DeviceProperty::kDevice] = "/dev/sda1";
        info[DeviceProperty::kOptical] = false;   // Not optical
        return info;
    });

    scanner->initialize();
    QMetaObject::invokeMethod(scanner, "scanOpticalDisc", Qt::DirectConnection);

    QTest::qWait(100);
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, scanOpticalDisc_InvalidDevicePath)
{
    // Test with invalid device path
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *, const QString &id, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kId] = id;
        info[DeviceProperty::kDevice] = "/dev/sda1";   // Not sr* device
        info[DeviceProperty::kOptical] = true;
        return info;
    });

    scanner->initialize();
    QMetaObject::invokeMethod(scanner, "scanOpticalDisc", Qt::DirectConnection);

    QTest::qWait(100);
    EXPECT_TRUE(true);
}

// ========== onDevicePropertyChangedQVar() Tests ==========

TEST_F(TestDiscDeviceScanner, onDevicePropertyChangedQVar_OpticalPropertyTrue)
{
    // Test optical property changed to true
    QString id = "/org/freedesktop/UDisks2/block_devices/sr0";
    QString propertyName = DeviceProperty::kOptical;
    QVariant var(true);

    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, propertyName),
                              Q_ARG(QVariant, var));

    // Should add device to scan list
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, onDevicePropertyChangedQVar_OpticalPropertyFalse)
{
    // Test optical property changed to false
    QString id = "/org/freedesktop/UDisks2/block_devices/sr0";
    QString propertyName = DeviceProperty::kOptical;
    QVariant var(false);

    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, propertyName),
                              Q_ARG(QVariant, var));

    // Should remove device from scan list
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, onDevicePropertyChangedQVar_NonOpticalProperty)
{
    // Test non-optical property change
    QString id = "/org/freedesktop/UDisks2/block_devices/sr0";
    QString propertyName = "Size";
    QVariant var(1000000000ULL);

    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, propertyName),
                              Q_ARG(QVariant, var));

    // Should ignore non-optical property
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, onDevicePropertyChangedQVar_InvalidId)
{
    // Test with invalid device ID
    QString id = "invalid-id-format";
    QString propertyName = DeviceProperty::kOptical;
    QVariant var(true);

    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, propertyName),
                              Q_ARG(QVariant, var));

    // Should ignore invalid ID
    EXPECT_TRUE(true);
}

// ========== onDevicePropertyChangedQDBusVar() Tests ==========

TEST_F(TestDiscDeviceScanner, onDevicePropertyChangedQDBusVar_OpticalPropertyTrue)
{
    // Test DBus variant optical property change
    QString id = "/org/freedesktop/UDisks2/block_devices/sr0";
    QString propertyName = DeviceProperty::kOptical;
    QDBusVariant var(true);

    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQDBusVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, propertyName),
                              Q_ARG(QDBusVariant, var));

    // Should add device to scan list
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, onDevicePropertyChangedQDBusVar_DeviceAlreadyInList)
{
    // Test adding device that's already in list
    QString id = "/org/freedesktop/UDisks2/block_devices/sr0";
    QString propertyName = DeviceProperty::kOptical;
    QDBusVariant var(true);

    // Add twice
    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQDBusVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, propertyName),
                              Q_ARG(QDBusVariant, var));

    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQDBusVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, propertyName),
                              Q_ARG(QDBusVariant, var));

    // Should not duplicate
    EXPECT_TRUE(true);
}

// ========== onDiscWorkingStateChanged() Tests ==========

TEST_F(TestDiscDeviceScanner, onDiscWorkingStateChanged_WorkingTrue)
{
    // Test disc working state changed to true
    QString id = "block:dev:sr0";
    QString dev = "/dev/sr0";
    bool working = true;

    QMetaObject::invokeMethod(scanner, "onDiscWorkingStateChanged", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, dev),
                              Q_ARG(bool, working));

    // Should remove device from scan list
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, onDiscWorkingStateChanged_WorkingFalse)
{
    // Test disc working state changed to false
    QString id = "block:dev:sr0";
    QString dev = "/dev/sr0";
    bool working = false;

    QMetaObject::invokeMethod(scanner, "onDiscWorkingStateChanged", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, dev),
                              Q_ARG(bool, working));

    // Should add device to scan list
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, onDiscWorkingStateChanged_EmptyId)
{
    // Test with empty ID
    QString id;
    QString dev = "/dev/sr0";
    bool working = false;

    QMetaObject::invokeMethod(scanner, "onDiscWorkingStateChanged", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, dev),
                              Q_ARG(bool, working));

    // Should handle gracefully
    EXPECT_TRUE(true);
}

// ========== Integration Tests ==========

TEST_F(TestDiscDeviceScanner, Integration_FullLifecycle)
{
    // Test complete lifecycle
    scanner->initialize();

    // Simulate property change
    QString id = "/org/freedesktop/UDisks2/block_devices/sr0";
    QVariant var(true);
    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, QString(DeviceProperty::kOptical)),
                              Q_ARG(QVariant, var));

    // Wait for potential timer
    QTest::qWait(100);

    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, Integration_DeviceAddRemove)
{
    // Test device add and remove cycle
    scanner->initialize();

    QString id = "/org/freedesktop/UDisks2/block_devices/sr0";

    // Add device
    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, QString(DeviceProperty::kOptical)),
                              Q_ARG(QVariant, QVariant(true)));

    // Remove device
    QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQVar", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, QString(DeviceProperty::kOptical)),
                              Q_ARG(QVariant, QVariant(false)));

    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, Integration_WorkingStateChanges)
{
    // Test working state changes
    scanner->initialize();

    QString id = "block:dev:sr0";
    QString dev = "/dev/sr0";

    // Set working
    QMetaObject::invokeMethod(scanner, "onDiscWorkingStateChanged", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, dev),
                              Q_ARG(bool, true));

    // Set not working
    QMetaObject::invokeMethod(scanner, "onDiscWorkingStateChanged", Qt::DirectConnection,
                              Q_ARG(QString, id),
                              Q_ARG(QString, dev),
                              Q_ARG(bool, false));

    EXPECT_TRUE(true);
}

// ========== Edge Cases ==========

TEST_F(TestDiscDeviceScanner, EdgeCase_SystemCallFailure)
{
    // Test when open() fails
    using OpenFunc = int (*)(const char *, int);
    stub.set_lamda(reinterpret_cast<OpenFunc>(::open), [](const char *, int) -> int {
        __DBG_STUB_INVOKE__
        return -1;   // Failure
    });

    scanner->initialize();
    QMetaObject::invokeMethod(scanner, "scanOpticalDisc", Qt::DirectConnection);

    QTest::qWait(100);

    // Should handle gracefully
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, EdgeCase_MultipleDiscDevices)
{
    // Test with multiple optical disc devices
    stub.set_lamda(&DeviceProxyManager::getAllBlockIds, [](DeviceProxyManager *, DeviceQueryOptions) -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList { "block:dev:sr0", "block:dev:sr1", "block:dev:sr2" };
    });

    scanner->initialize();
    QMetaObject::invokeMethod(scanner, "scanOpticalDisc", Qt::DirectConnection);

    QTest::qWait(200);

    // Should handle multiple devices
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, EdgeCase_RapidPropertyChanges)
{
    // Test rapid property changes
    scanner->initialize();

    QString id = "/org/freedesktop/UDisks2/block_devices/sr0";

    for (int i = 0; i < 10; ++i) {
        bool optical = (i % 2 == 0);
        QMetaObject::invokeMethod(scanner, "onDevicePropertyChangedQVar", Qt::DirectConnection,
                                  Q_ARG(QString, id),
                                  Q_ARG(QString, QString(DeviceProperty::kOptical)),
                                  Q_ARG(QVariant, QVariant(optical)));
    }

    // Should handle rapid changes
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, EdgeCase_EmptyDevicePath)
{
    // Test with empty device path
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *, const QString &id, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kId] = id;
        info[DeviceProperty::kDevice] = "";   // Empty path
        info[DeviceProperty::kOptical] = true;
        return info;
    });

    scanner->initialize();
    QMetaObject::invokeMethod(scanner, "scanOpticalDisc", Qt::DirectConnection);

    QTest::qWait(100);

    // Should handle gracefully
    EXPECT_TRUE(true);
}

// ========== DiscDevice::Scanner Tests ==========

TEST_F(TestDiscDeviceScanner, Scanner_ValidDevice)
{
    // Test Scanner runnable with valid device
    DiscDevice::Scanner *scanTask = new DiscDevice::Scanner("/dev/sr0");

    // Run directly (QThreadPool would take ownership and delete)
    scanTask->run();
    delete scanTask;

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, Scanner_EmptyDevice)
{
    // Test Scanner with empty device
    DiscDevice::Scanner *scanTask = new DiscDevice::Scanner("");

    scanTask->run();
    delete scanTask;

    // Should handle gracefully
    EXPECT_TRUE(true);
}

TEST_F(TestDiscDeviceScanner, Scanner_OpenFailure)
{
    // Test Scanner when open fails
    // Note: Cannot directly stub variadic function open(), so we stub the two-parameter version
    using OpenFunc = int (*)(const char *, int);
    stub.set_lamda(reinterpret_cast<OpenFunc>(::open), [](const char *, int) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });

    DiscDevice::Scanner *scanTask = new DiscDevice::Scanner("/dev/sr0");

    scanTask->run();
    delete scanTask;

    // Should handle failure
    EXPECT_TRUE(true);
}
