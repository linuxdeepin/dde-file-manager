// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "devicemanagerdbus.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QDBusContext>
#include <QDBusMessage>
#include <QThread>

DFMBASE_USE_NAMESPACE

using namespace GlobalServerDefines;

class DeviceManagerDBusImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        // QDBusContext::message() dereferences the thread-local DBus
        // dispatch context and segfaults when no real DBus call is in
        // flight; return a static empty message instead. The per-test
        // QDBusMessage::service stub then controls the client name.
        using MessageFunc = const QDBusMessage & (QDBusContext::*)() const;
        stub.set_lamda(static_cast<MessageFunc>(&QDBusContext::message), [](QDBusContext *) -> const QDBusMessage & {
            static QDBusMessage msg;
            return msg;
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

// ---------------------------------------------------------------------------
// Usage monitoring (StartMonitoringUsage / StopMonitoringUsage)
// ---------------------------------------------------------------------------

TEST_F(DeviceManagerDBusImpl, StartMonitoringUsage_EmptyClient_Ignored)
{
    bool startPollingCalled = false;

    using ServiceFunc = QString (QDBusMessage::*)() const;
    stub.set_lamda(static_cast<ServiceFunc>(&QDBusMessage::service), [](QDBusMessage *) {
        __DBG_STUB_INVOKE__
        return QString();
    });

    stub.set_lamda(&DeviceManager::startPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        startPollingCalled = true;
    });

    deviceManager->StartMonitoringUsage();

    EXPECT_FALSE(startPollingCalled);
}

TEST_F(DeviceManagerDBusImpl, StartMonitoringUsage_AlreadyMonitoring_Ignored)
{
    bool startPollingCalled = false;

    using ServiceFunc = QString (QDBusMessage::*)() const;
    stub.set_lamda(static_cast<ServiceFunc>(&QDBusMessage::service), [](QDBusMessage *) {
        __DBG_STUB_INVOKE__
        return QStringLiteral("client1");
    });

    stub.set_lamda(&DeviceManager::startPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        startPollingCalled = true;
    });

    // Pre-populate the monitoring set so the client is already subscribed.
    deviceManager->m_monitoringClients.insert("client1");
    deviceManager->StartMonitoringUsage();

    EXPECT_FALSE(startPollingCalled);
}

TEST_F(DeviceManagerDBusImpl, StartMonitoringUsage_FirstClient_StartsPolling)
{
    bool startPollingCalled = false;

    using ServiceFunc = QString (QDBusMessage::*)() const;
    stub.set_lamda(static_cast<ServiceFunc>(&QDBusMessage::service), [](QDBusMessage *) {
        __DBG_STUB_INVOKE__
        return QStringLiteral("client1");
    });

    stub.set_lamda(&DeviceManager::startPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        startPollingCalled = true;
    });

    deviceManager->StartMonitoringUsage();

    EXPECT_TRUE(startPollingCalled);
    EXPECT_TRUE(deviceManager->m_monitoringClients.contains("client1"));
}

TEST_F(DeviceManagerDBusImpl, StartMonitoringUsage_SecondClient_KeepsPolling)
{
    int startPollingCallCount = 0;

    using ServiceFunc = QString (QDBusMessage::*)() const;
    stub.set_lamda(static_cast<ServiceFunc>(&QDBusMessage::service), [](QDBusMessage *) {
        __DBG_STUB_INVOKE__
        return QStringLiteral("client2");
    });

    stub.set_lamda(&DeviceManager::startPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        ++startPollingCallCount;
    });

    deviceManager->m_monitoringClients.insert("client1");
    deviceManager->StartMonitoringUsage();

    EXPECT_EQ(startPollingCallCount, 0);
    EXPECT_TRUE(deviceManager->m_monitoringClients.contains("client2"));
}

TEST_F(DeviceManagerDBusImpl, StopMonitoringUsage_EmptyClient_Ignored)
{
    bool stopPollingCalled = false;

    using ServiceFunc = QString (QDBusMessage::*)() const;
    stub.set_lamda(static_cast<ServiceFunc>(&QDBusMessage::service), [](QDBusMessage *) {
        __DBG_STUB_INVOKE__
        return QString();
    });

    stub.set_lamda(&DeviceManager::stopPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        stopPollingCalled = true;
    });

    deviceManager->StopMonitoringUsage();

    EXPECT_FALSE(stopPollingCalled);
}

TEST_F(DeviceManagerDBusImpl, StopMonitoringUsage_NotMonitoring_Ignored)
{
    bool stopPollingCalled = false;

    using ServiceFunc = QString (QDBusMessage::*)() const;
    stub.set_lamda(static_cast<ServiceFunc>(&QDBusMessage::service), [](QDBusMessage *) {
        __DBG_STUB_INVOKE__
        return QStringLiteral("client1");
    });

    stub.set_lamda(&DeviceManager::stopPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        stopPollingCalled = true;
    });

    deviceManager->StopMonitoringUsage();

    EXPECT_FALSE(stopPollingCalled);
}

TEST_F(DeviceManagerDBusImpl, StopMonitoringUsage_LastClient_StopsPolling)
{
    bool stopPollingCalled = false;

    using ServiceFunc = QString (QDBusMessage::*)() const;
    stub.set_lamda(static_cast<ServiceFunc>(&QDBusMessage::service), [](QDBusMessage *) {
        __DBG_STUB_INVOKE__
        return QStringLiteral("client1");
    });

    stub.set_lamda(&DeviceManager::stopPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        stopPollingCalled = true;
    });

    deviceManager->m_monitoringClients.insert("client1");
    deviceManager->StopMonitoringUsage();

    EXPECT_TRUE(stopPollingCalled);
    EXPECT_FALSE(deviceManager->m_monitoringClients.contains("client1"));
}

TEST_F(DeviceManagerDBusImpl, StopMonitoringUsage_OtherClients_KeepsPolling)
{
    bool stopPollingCalled = false;

    using ServiceFunc = QString (QDBusMessage::*)() const;
    stub.set_lamda(static_cast<ServiceFunc>(&QDBusMessage::service), [](QDBusMessage *) {
        __DBG_STUB_INVOKE__
        return QStringLiteral("client1");
    });

    stub.set_lamda(&DeviceManager::stopPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        stopPollingCalled = true;
    });

    deviceManager->m_monitoringClients.insert("client1");
    deviceManager->m_monitoringClients.insert("client2");
    deviceManager->StopMonitoringUsage();

    EXPECT_FALSE(stopPollingCalled);
    EXPECT_FALSE(deviceManager->m_monitoringClients.contains("client1"));
    EXPECT_TRUE(deviceManager->m_monitoringClients.contains("client2"));
}

// ---------------------------------------------------------------------------
// RefreshDeviceUsage
// ---------------------------------------------------------------------------

TEST_F(DeviceManagerDBusImpl, RefreshDeviceUsage_Debounce_Ignored)
{
    bool refreshCalled = false;

    stub.set_lamda(&DeviceManager::refreshUsage, [&]() {
        __DBG_STUB_INVOKE__
        refreshCalled = true;
    });

    deviceManager->m_lastRefreshTimer.start();
    // Ensure elapsed is well below the 500ms debounce window.
    QThread::msleep(5);
    deviceManager->RefreshDeviceUsage();

    EXPECT_FALSE(refreshCalled);
}

TEST_F(DeviceManagerDBusImpl, RefreshDeviceUsage_Normal_Refreshes)
{
    bool refreshCalled = false;

    stub.set_lamda(&DeviceManager::refreshUsage, [&]() {
        __DBG_STUB_INVOKE__
        refreshCalled = true;
    });

    deviceManager->RefreshDeviceUsage();

    EXPECT_TRUE(refreshCalled);
    EXPECT_TRUE(deviceManager->m_lastRefreshTimer.isValid());
}

// ---------------------------------------------------------------------------
// onNameOwnerChanged
// ---------------------------------------------------------------------------

TEST_F(DeviceManagerDBusImpl, OnNameOwnerChanged_NewOwnerNotEmpty_Ignored)
{
    bool stopPollingCalled = false;

    stub.set_lamda(&DeviceManager::stopPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        stopPollingCalled = true;
    });

    deviceManager->m_monitoringClients.insert("client1");
    deviceManager->onNameOwnerChanged("client1", "oldOwner", "newOwner");

    EXPECT_FALSE(stopPollingCalled);
    EXPECT_TRUE(deviceManager->m_monitoringClients.contains("client1"));
}

TEST_F(DeviceManagerDBusImpl, OnNameOwnerChanged_NotSubscribed_Ignored)
{
    bool stopPollingCalled = false;

    stub.set_lamda(&DeviceManager::stopPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        stopPollingCalled = true;
    });

    deviceManager->onNameOwnerChanged("otherClient", "oldOwner", QString());

    EXPECT_FALSE(stopPollingCalled);
}

TEST_F(DeviceManagerDBusImpl, OnNameOwnerChanged_SubscribedClientDisconnected_StopsPollingWhenLast)
{
    bool stopPollingCalled = false;

    stub.set_lamda(&DeviceManager::stopPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        stopPollingCalled = true;
    });

    deviceManager->m_monitoringClients.insert("client1");
    deviceManager->onNameOwnerChanged("client1", "oldOwner", QString());

    EXPECT_TRUE(stopPollingCalled);
    EXPECT_FALSE(deviceManager->m_monitoringClients.contains("client1"));
}

TEST_F(DeviceManagerDBusImpl, OnNameOwnerChanged_SubscribedClientDisconnected_OtherClientsRemain)
{
    bool stopPollingCalled = false;

    stub.set_lamda(&DeviceManager::stopPollingDeviceUsage, [&]() {
        __DBG_STUB_INVOKE__
        stopPollingCalled = true;
    });

    deviceManager->m_monitoringClients.insert("client1");
    deviceManager->m_monitoringClients.insert("client2");
    deviceManager->onNameOwnerChanged("client1", "oldOwner", QString());

    EXPECT_FALSE(stopPollingCalled);
    EXPECT_FALSE(deviceManager->m_monitoringClients.contains("client1"));
    EXPECT_TRUE(deviceManager->m_monitoringClients.contains("client2"));
}
