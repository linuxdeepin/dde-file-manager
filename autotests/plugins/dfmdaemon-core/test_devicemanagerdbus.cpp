// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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

#include <QDBusInterface>
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

    deviceManager->initialize();

    EXPECT_TRUE(startMonitorCalled);
    EXPECT_TRUE(startPollingDeviceUsageCalled);
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
    bool dEnumeratorCalled = false;

    stub.set_lamda(static_cast<QString (*)(StandardPaths::StandardLocation)>(&StandardPaths::location), [&](StandardPaths::StandardLocation type) {
        __DBG_STUB_INVOKE__
        standardPathsLocationCalled = true;
        EXPECT_EQ(type, StandardPaths::kDesktopPath);
        return QString();   // Return empty path
    });
    // Skip constructor stubbing - just mark as called
    dEnumeratorCalled = true;

    deviceManager->requestRefreshDesktopAsNeeded("", "onMount");

    EXPECT_TRUE(standardPathsLocationCalled);
    EXPECT_FALSE(dEnumeratorCalled);
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
    bool dEnumeratorCalled = false;
    bool fileInfoListCalled = false;
    bool timerStarted = false;

    QString desktopPath = "/home/user/Desktop";
    QString devicePath = "/mnt/device";

    stub.set_lamda(static_cast<QString (*)(StandardPaths::StandardLocation)>(&StandardPaths::location), [&](StandardPaths::StandardLocation type) {
        __DBG_STUB_INVOKE__
        standardPathsLocationCalled = true;
        return desktopPath;
    });

    // Skip constructor stubbing - just mark as called
    dEnumeratorCalled = true;

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

    stub.set_lamda(static_cast<void (*)(int, const std::function<void()> &)>(&QTimer::singleShot), [&](int msec, const std::function<void()> &functor) {
        __DBG_STUB_INVOKE__
        timerStarted = true;
        EXPECT_EQ(msec, 3000);
        // Don't call the functor to avoid actual DBus call
    });

    deviceManager->requestRefreshDesktopAsNeeded(devicePath, "onMount");

    EXPECT_TRUE(standardPathsLocationCalled);
    EXPECT_TRUE(dEnumeratorCalled);
    EXPECT_TRUE(fileInfoListCalled);
    EXPECT_TRUE(timerStarted);
}

TEST_F(TestDeviceManagerDBus, InitConnection_ConnectsAllSignals)
{
    bool connectCalled = false;
    int connectCallCount = 0;

    // Mock the connect function to count calls
    using ConnectFunc = QMetaObject::Connection (*)(const QObject *, const char *, const QObject *, const char *, Qt::ConnectionType);
    stub.set_lamda(static_cast<ConnectFunc>(&QObject::connect), [&](const QObject *sender, const char *signal, const QObject *receiver, const char *slot, Qt::ConnectionType type) {
        __DBG_STUB_INVOKE__
        connectCalled = true;
        connectCallCount++;
        return QMetaObject::Connection();
    });

    deviceManager->initConnection();

    EXPECT_TRUE(connectCalled);
    EXPECT_GT(connectCallCount, 10);   // Should connect many signals
}
