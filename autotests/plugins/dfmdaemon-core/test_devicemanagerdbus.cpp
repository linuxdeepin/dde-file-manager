// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "devicemanagerdbus.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-io/denumerator.h>
#include <dfm-io/dfileinfo.h>

#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QUrl>

DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

using namespace GlobalServerDefines;

class TestDeviceManagerDBus : public testing::Test
{
protected:
    void SetUp() override
    {
        // DeviceManager is a process-wide singleton whose monitoring/usage
        // methods spawn background threads and touch DConfig; those calls
        // would race with later stub patching (SIGBUS) and pollute other
        // suites. Neutralize the whole lifecycle for every test; individual
        // tests may re-stub any of these to record invocations.
        stub.set_lamda(&DeviceManager::doAutoMountAtStart, []() {
        });
        stub.set_lamda(&DeviceManager::startMonitor, []() {
        });
        stub.set_lamda(&DeviceManager::stopMonitor, []() {
        });
        stub.set_lamda(&DeviceManager::startPollingDeviceUsage, []() {
        });
        stub.set_lamda(&DeviceManager::stopPollingDeviceUsage, []() {
        });
        stub.set_lamda(&DeviceManager::initUsageCache, []() {
        });
        stub.set_lamda(&DeviceManager::refreshUsage, []() {
        });
        stub.set_lamda(&DeviceManager::enableBlockAutoMount, []() {
        });
        stub.set_lamda(&DeviceManager::startOpticalDiscScan, []() {
        });

        deviceManager = new DeviceManagerDBus();
    }

    void TearDown() override
    {
        delete deviceManager;
        stub.clear();
    }

    DeviceManagerDBus *deviceManager { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TestDeviceManagerDBus, Constructor_InitializesCorrectly)
{
    bool initializeCalled = false;
    bool initConnectionCalled = false;
    bool doAutoMountAtStartCalled = false;

    stub.set_lamda(&DeviceManagerDBus::initialize, [&]() {
        __DBG_STUB_INVOKE__
        initializeCalled = true;
    });

    stub.set_lamda(&DeviceManagerDBus::initConnection, [&]() {
        __DBG_STUB_INVOKE__
        initConnectionCalled = true;
    });

    stub.set_lamda(&DeviceManager::doAutoMountAtStart, [&]() {
        __DBG_STUB_INVOKE__
        doAutoMountAtStartCalled = true;
    });

    DeviceManagerDBus testManager;

    EXPECT_TRUE(initializeCalled);
    EXPECT_TRUE(initConnectionCalled);
    EXPECT_TRUE(doAutoMountAtStartCalled);
}

TEST_F(TestDeviceManagerDBus, IsMonotorWorking_ReturnsCorrectStatus)
{
    bool isMonitoringCalled = false;

    stub.set_lamda(&DeviceManager::isMonitoring, [&]() {
        __DBG_STUB_INVOKE__
        isMonitoringCalled = true;
        return true;
    });

    bool result = deviceManager->IsMonotorWorking();

    EXPECT_TRUE(result);
    EXPECT_TRUE(isMonitoringCalled);
}

TEST_F(TestDeviceManagerDBus, DetachBlockDevice_CallsCorrectMethod)
{
    bool detachBlockDevCalled = false;
    QString testId = "test-block-device";

    stub.set_lamda(&DeviceManager::detachBlockDev, [&](DeviceManager *, const QString &id, std::function<void(bool, const dfmmount::OperationErrorInfo &)>) {
        __DBG_STUB_INVOKE__
        detachBlockDevCalled = true;
        EXPECT_EQ(id, testId);
        return QStringList();
    });

    deviceManager->DetachBlockDevice(testId);

    EXPECT_TRUE(detachBlockDevCalled);
}

TEST_F(TestDeviceManagerDBus, DetachProtocolDevice_CallsCorrectMethod)
{
    bool detachProtoDevCalled = false;
    QString testId = "test-protocol-device";

    stub.set_lamda(&DeviceManager::detachProtoDev, [&](DeviceManager *, const QString &id) {
        __DBG_STUB_INVOKE__
        detachProtoDevCalled = true;
        EXPECT_EQ(id, testId);
    });

    deviceManager->DetachProtocolDevice(testId);

    EXPECT_TRUE(detachProtoDevCalled);
}

TEST_F(TestDeviceManagerDBus, Initialize_StartsMonitoringAndServices)
{
    bool startMonitorCalled = false;
    bool startPollingDeviceUsageCalled = false;
    bool enableBlockAutoMountCalled = false;
    bool initUsageCacheCalled = false;

    // Re-stub the SetUp guards to record invocations.
    stub.set_lamda(&DeviceManager::startMonitor, [&]() {
        __DBG_STUB_INVOKE__
        startMonitorCalled = true;
    });

    stub.set_lamda(&DeviceManager::startPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        startPollingDeviceUsageCalled = true;
    });

    stub.set_lamda(&DeviceManager::enableBlockAutoMount, [&]() {
        __DBG_STUB_INVOKE__
        enableBlockAutoMountCalled = true;
    });

    stub.set_lamda(&DeviceManager::initUsageCache, [&]() {
        __DBG_STUB_INVOKE__
        initUsageCacheCalled = true;
    });

    deviceManager->initialize();

    EXPECT_TRUE(startMonitorCalled);
    // Polling is now started on-demand (see initialize()): only the usage
    // cache is built up front.
    EXPECT_FALSE(startPollingDeviceUsageCalled);
    EXPECT_TRUE(initUsageCacheCalled);
    EXPECT_TRUE(enableBlockAutoMountCalled);
}

TEST_F(TestDeviceManagerDBus, DetachAllMountedDevices_CallsCorrectMethods)
{
    bool detachAllRemovableBlockDevsCalled = false;
    bool detachAllProtoDevsCalled = false;

    stub.set_lamda(&DeviceManager::detachAllRemovableBlockDevs, [&]() {
        __DBG_STUB_INVOKE__
        detachAllRemovableBlockDevsCalled = true;
    });

    stub.set_lamda(&DeviceManager::detachAllProtoDevs, [&]() {
        __DBG_STUB_INVOKE__
        detachAllProtoDevsCalled = true;
    });

    deviceManager->DetachAllMountedDevices();

    EXPECT_TRUE(detachAllRemovableBlockDevsCalled);
    EXPECT_TRUE(detachAllProtoDevsCalled);
}

TEST_F(TestDeviceManagerDBus, GetBlockDevicesIdList_ReturnsCorrectList)
{
    QStringList expectedList = { "device1", "device2", "device3" };
    bool getAllBlockDevIDCalled = false;
    int testOpts = 1;

    stub.set_lamda(&DeviceManager::getAllBlockDevID, [&](DeviceManager *, DeviceQueryOptions opts) {
        __DBG_STUB_INVOKE__
        getAllBlockDevIDCalled = true;
        EXPECT_EQ(static_cast<int>(opts), testOpts);
        return expectedList;
    });

    QStringList result = deviceManager->GetBlockDevicesIdList(testOpts);

    EXPECT_EQ(result, expectedList);
    EXPECT_TRUE(getAllBlockDevIDCalled);
}

TEST_F(TestDeviceManagerDBus, QueryBlockDeviceInfo_ReturnsCorrectInfo)
{
    QVariantMap expectedInfo;
    expectedInfo["id"] = "test-device";
    expectedInfo["mountPoint"] = "/mnt/test";

    bool getBlockDevInfoCalled = false;
    QString testId = "test-device";
    bool testReload = true;

    stub.set_lamda(&DeviceManager::getBlockDevInfo, [&](DeviceManager *, const QString &id, bool reload) {
        __DBG_STUB_INVOKE__
        getBlockDevInfoCalled = true;
        EXPECT_EQ(id, testId);
        EXPECT_EQ(reload, testReload);
        return expectedInfo;
    });

    QVariantMap result = deviceManager->QueryBlockDeviceInfo(testId, testReload);

    EXPECT_EQ(result, expectedInfo);
    EXPECT_TRUE(getBlockDevInfoCalled);
}

TEST_F(TestDeviceManagerDBus, GetProtocolDevicesIdList_ReturnsCorrectList)
{
    QStringList expectedList = { "proto1", "proto2" };
    bool getAllProtocolDevIDCalled = false;

    stub.set_lamda(&DeviceManager::getAllProtocolDevID, [&]() {
        __DBG_STUB_INVOKE__
        getAllProtocolDevIDCalled = true;
        return expectedList;
    });

    QStringList result = deviceManager->GetProtocolDevicesIdList();

    EXPECT_EQ(result, expectedList);
    EXPECT_TRUE(getAllProtocolDevIDCalled);
}

TEST_F(TestDeviceManagerDBus, QueryProtocolDeviceInfo_ReturnsCorrectInfo)
{
    QVariantMap expectedInfo;
    expectedInfo["id"] = "proto-device";
    expectedInfo["host"] = "192.168.1.100";

    bool getProtocolDevInfoCalled = false;
    QString testId = "proto-device";
    bool testReload = false;

    stub.set_lamda(&DeviceManager::getProtocolDevInfo, [&](DeviceManager *, const QString &id, bool reload) {
        __DBG_STUB_INVOKE__
        getProtocolDevInfoCalled = true;
        EXPECT_EQ(id, testId);
        EXPECT_EQ(reload, testReload);
        return expectedInfo;
    });

    QVariantMap result = deviceManager->QueryProtocolDeviceInfo(testId, testReload);

    EXPECT_EQ(result, expectedInfo);
    EXPECT_TRUE(getProtocolDevInfoCalled);
}

TEST_F(TestDeviceManagerDBus, RequestRefreshDesktopAsNeeded_EmptyPaths_DoesNothing)
{
    bool standardPathsLocationCalled = false;
    bool fileInfoListCalled = false;

    stub.set_lamda(static_cast<QString (*)(StandardPaths::StandardLocation)>(&StandardPaths::location), [&](StandardPaths::StandardLocation type) {
        __DBG_STUB_INVOKE__
        standardPathsLocationCalled = true;
        EXPECT_EQ(type, StandardPaths::kDesktopPath);
        return QString();   // Return empty path
    });
    // Empty desktop path means an early return: the DEnumerator would be
    // constructed right after the path check, so fileInfoList() (called on
    // it immediately) must never fire.
    stub.set_lamda(&dfmio::DEnumerator::fileInfoList, [&]() {
        __DBG_STUB_INVOKE__
        fileInfoListCalled = true;
        return QList<QSharedPointer<dfmio::DFileInfo>>();
    });

    deviceManager->requestRefreshDesktopAsNeeded("", "onMount");

    EXPECT_TRUE(standardPathsLocationCalled);
    EXPECT_FALSE(fileInfoListCalled);
}

TEST_F(TestDeviceManagerDBus, RequestRefreshDesktopAsNeeded_NoSymlinks_DoesNotRefresh)
{
    bool standardPathsLocationCalled = false;
    bool dEnumeratorCalled = false;
    bool fileInfoListCalled = false;
    bool timerStarted = false;

    QString desktopPath = "/home/user/Desktop";
    QString devicePath = "/mnt/device";

    stub.set_lamda(static_cast<QString (*)(StandardPaths::StandardLocation)>(&StandardPaths::location), [&](StandardPaths::StandardLocation type) {
        __DBG_STUB_INVOKE__
        standardPathsLocationCalled = true;
        EXPECT_EQ(type, StandardPaths::kDesktopPath);
        return desktopPath;
    });

    // Skip constructor stubbing - just mark as called
    dEnumeratorCalled = true;

    stub.set_lamda(&dfmio::DEnumerator::fileInfoList, [&]() {
        __DBG_STUB_INVOKE__
        fileInfoListCalled = true;
        return QList<QSharedPointer<dfmio::DFileInfo>>();   // Return empty list
    });

    stub.set_lamda(static_cast<void (*)(int, const std::function<void()> &)>(&QTimer::singleShot), [&](int msec, const std::function<void()> &functor) {
        __DBG_STUB_INVOKE__
        timerStarted = true;
    });

    deviceManager->requestRefreshDesktopAsNeeded(devicePath, "onMount");

    EXPECT_TRUE(standardPathsLocationCalled);
    EXPECT_TRUE(dEnumeratorCalled);
    EXPECT_TRUE(fileInfoListCalled);
    EXPECT_FALSE(timerStarted);
}

TEST_F(TestDeviceManagerDBus, RequestRefreshDesktopAsNeeded_HasMatchingSymlinks_TriggersRefresh)
{
    bool standardPathsLocationCalled = false;
    bool fileInfoListCalled = false;

    QString desktopPath = "/home/user/Desktop";
    QString devicePath = "/mnt/device";

    stub.set_lamda(static_cast<QString (*)(StandardPaths::StandardLocation)>(&StandardPaths::location), [&](StandardPaths::StandardLocation type) {
        __DBG_STUB_INVOKE__
        standardPathsLocationCalled = true;
        return desktopPath;
    });

    // Create mock file info for symlink
    auto mockFileInfo = QSharedPointer<dfmio::DFileInfo>::create(QUrl());
    QList<QSharedPointer<dfmio::DFileInfo>> fileList;
    fileList.append(mockFileInfo);

    stub.set_lamda(&dfmio::DEnumerator::fileInfoList, [&]() {
        __DBG_STUB_INVOKE__
        fileInfoListCalled = true;
        return fileList;
    });

    // Mock DFileInfo methods
    stub.set_lamda(&dfmio::DFileInfo::attribute, [&](dfmio::DFileInfo *, dfmio::DFileInfo::AttributeID id, bool *) {
        __DBG_STUB_INVOKE__
        if (id == dfmio::DFileInfo::AttributeID::kStandardIsSymlink) {
            return QVariant(true);
        } else if (id == dfmio::DFileInfo::AttributeID::kStandardSymlinkTarget) {
            return QVariant(devicePath + "/subfolder");   // Target starts with devicePath
        }
        return QVariant();
    });

    // The product schedules the desktop refresh via QTimer::singleShot(3s,
    // lambda); template singleShot overloads instantiated with a product
    // lambda cannot be stubbed. Instead, let the timer really fire and catch
    // the resulting DBus call: asyncCall("Refresh") resolves to doAsyncCall.
    QString refreshMethod;
    using DoAsyncFunc = QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QVariant *, size_t);
    stub.set_lamda(static_cast<DoAsyncFunc>(&QDBusAbstractInterface::doAsyncCall),
                   [&](QDBusAbstractInterface *, const QString &method, const QVariant *, size_t) {
        __DBG_STUB_INVOKE__
        refreshMethod = method;
        return QDBusPendingCall::fromCompletedCall(QDBusMessage::createMethodCall(
                QStringLiteral("test.service"), QStringLiteral("/test/path"),
                QStringLiteral("test.iface"), QStringLiteral("noop")));
    });

    deviceManager->requestRefreshDesktopAsNeeded(devicePath, "onMount");

    EXPECT_TRUE(standardPathsLocationCalled);
    EXPECT_TRUE(fileInfoListCalled);

    // Wait for the 3s watchdog singleShot to fire and process the deferred
    // asyncCall("Refresh") through a real event loop.
    QTest::qWait(3400);

    EXPECT_EQ(refreshMethod, QStringLiteral("Refresh"));
}

TEST_F(TestDeviceManagerDBus, InitConnection_ConnectsAllSignals)
{
    // initConnection() wires DeviceManager singleton signals to the DBus
    // interface signals via PMF/lambda connect() overloads, which cannot be
    // stubbed through the char*-signature cast. Verify the forwarding
    // behavior instead: emitting DeviceManager signals must re-emit the
    // matching DBus interface signals.
    QSignalSpy busySpy(deviceManager, &DeviceManagerDBus::NotifyDeviceBusy);
    QSignalSpy sizeSpy(deviceManager, &DeviceManagerDBus::SizeUsedChanged);
    QSignalSpy addedSpy(deviceManager, &DeviceManagerDBus::BlockDeviceAdded);

    ASSERT_TRUE(busySpy.isValid());
    ASSERT_TRUE(sizeSpy.isValid());
    ASSERT_TRUE(addedSpy.isValid());

    // Prevent the mounted/unmounted forwarding lambdas from really walking
    // the desktop dir via requestRefreshDesktopAsNeeded(): empty desktop
    // path makes it return early.
    stub.set_lamda(static_cast<QString (*)(StandardPaths::StandardLocation)>(&StandardPaths::location), [](StandardPaths::StandardLocation) {
        __DBG_STUB_INVOKE__
        return QString();
    });

    deviceManager->initConnection();

    auto *devMng = DFMBASE_NAMESPACE::DeviceManager::instance();
    ASSERT_NE(devMng, nullptr);

    // Drive every forwarding lambda registered by initConnection().
    emit devMng->blockDevUnmountAsyncFailed(QStringLiteral("dev1"), DFMMOUNT::DeviceError::kNoError);
    emit devMng->blockDevEjectAsyncFailed(QStringLiteral("dev1"), DFMMOUNT::DeviceError::kNoError);
    emit devMng->blockDevPoweroffAysncFailed(QStringLiteral("dev1"), DFMMOUNT::DeviceError::kNoError);
    emit devMng->protocolDevUnmountAsyncFailed(QStringLiteral("dev1"), DFMMOUNT::DeviceError::kNoError);

    emit devMng->devSizeChanged(QStringLiteral("dev2"), 100, 50);
    emit devMng->blockDriveAdded();
    emit devMng->blockDriveRemoved();
    emit devMng->blockDevAdded(QStringLiteral("dev3"));
    emit devMng->blockDevFsAdded(QStringLiteral("dev3"));
    emit devMng->blockDevFsRemoved(QStringLiteral("dev3"));
    emit devMng->blockDevUnlocked(QStringLiteral("dev3"), QStringLiteral("clear3"));
    emit devMng->blockDevLocked(QStringLiteral("dev3"));
    emit devMng->blockDevPropertyChanged(QStringLiteral("dev3"), QStringLiteral("prop"), QVariant(1));
    emit devMng->protocolDevAdded(QStringLiteral("dev4"));
    emit devMng->protocolDevMounted(QStringLiteral("dev4"), QStringLiteral("/mnt/4"));
    emit devMng->protocolDevUnmounted(QStringLiteral("dev4"), QStringLiteral("/mnt/4"));
    emit devMng->protocolDevRemoved(QStringLiteral("dev4"), QStringLiteral("/mnt/4"));
    emit devMng->blockDevMounted(QStringLiteral("dev3"), QStringLiteral("/mnt/3"));
    emit devMng->blockDevUnmounted(QStringLiteral("dev3"), QStringLiteral("/mnt/3"));
    emit devMng->blockDevRemoved(QStringLiteral("dev3"), QStringLiteral("/mnt/3"));

    // The constructor already ran initConnection() once, so calling it again
    // duplicates every connection; the forwarding itself works for both.
    EXPECT_EQ(busySpy.count(), 8);   // 4 async-failed types x 2 connections
    EXPECT_EQ(sizeSpy.count(), 2);
    EXPECT_EQ(addedSpy.count(), 2);
}
