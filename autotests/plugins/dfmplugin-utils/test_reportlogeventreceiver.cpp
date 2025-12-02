// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogeventreceiver.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogmanager.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-framework/dpf.h>
#include <dfm-framework/lifecycle/lifecycle.h>

#include <QApplication>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_ReportLogEventReceiver : public testing::Test
{
protected:
    void SetUp() override
    {
        receiver = new ReportLogEventReceiver();
    }

    void TearDown() override
    {
        delete receiver;
        receiver = nullptr;
        stub.clear();
    }

    ReportLogEventReceiver *receiver { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ReportLogEventReceiver, Constructor_CreatesObject)
{
    EXPECT_NE(receiver, nullptr);
}

TEST_F(UT_ReportLogEventReceiver, Constructor_WithParent_CreatesObject)
{
    QObject parent;
    ReportLogEventReceiver *recv = new ReportLogEventReceiver(&parent);

    EXPECT_NE(recv, nullptr);
    EXPECT_EQ(recv->parent(), &parent);
}

TEST_F(UT_ReportLogEventReceiver, commit_CallsManager)
{
    bool managerCalled = false;
    QString capturedType;

    stub.set_lamda(ADDR(ReportLogManager, commit),
                   [&managerCalled, &capturedType](ReportLogManager *, const QString &type, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                       capturedType = type;
                   });

    receiver->commit("TestType", QVariantMap());

    EXPECT_TRUE(managerCalled);
    EXPECT_EQ(capturedType, "TestType");
}

TEST_F(UT_ReportLogEventReceiver, commit_WithArgs_PassesArgs)
{
    QVariantMap capturedArgs;

    stub.set_lamda(ADDR(ReportLogManager, commit),
                   [&capturedArgs](ReportLogManager *, const QString &, const QVariantMap &args) {
                       __DBG_STUB_INVOKE__
                       capturedArgs = args;
                   });

    QVariantMap testArgs;
    testArgs["key1"] = "value1";
    testArgs["key2"] = 123;

    receiver->commit("TestType", testArgs);

    EXPECT_EQ(capturedArgs["key1"].toString(), "value1");
    EXPECT_EQ(capturedArgs["key2"].toInt(), 123);
}

TEST_F(UT_ReportLogEventReceiver, handleMenuData_CallsManager)
{
    bool managerCalled = false;
    QString capturedName;

    stub.set_lamda(ADDR(ReportLogManager, reportMenuData),
                   [&managerCalled, &capturedName](ReportLogManager *, const QString &name, const QList<QUrl> &) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                       capturedName = name;
                   });

    receiver->handleMenuData("TestMenu", QList<QUrl>());

    EXPECT_TRUE(managerCalled);
    EXPECT_EQ(capturedName, "TestMenu");
}

TEST_F(UT_ReportLogEventReceiver, handleMenuData_WithUrls_PassesUrls)
{
    QList<QUrl> capturedUrls;

    stub.set_lamda(ADDR(ReportLogManager, reportMenuData),
                   [&capturedUrls](ReportLogManager *, const QString &, const QList<QUrl> &urls) {
                       __DBG_STUB_INVOKE__
                       capturedUrls = urls;
                   });

    QList<QUrl> testUrls;
    testUrls << QUrl::fromLocalFile("/tmp/file1.txt");
    testUrls << QUrl::fromLocalFile("/tmp/file2.txt");

    receiver->handleMenuData("TestMenu", testUrls);

    EXPECT_EQ(capturedUrls.size(), 2);
}

TEST_F(UT_ReportLogEventReceiver, handleBlockMountData_CallsManager)
{
    bool managerCalled = false;

    stub.set_lamda(ADDR(ReportLogManager, reportBlockMountData),
                   [&managerCalled](ReportLogManager *, const QString &, bool) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                   });

    receiver->handleBlockMountData("/dev/sda1", true);

    EXPECT_TRUE(managerCalled);
}

TEST_F(UT_ReportLogEventReceiver, handleBlockMountData_PassesCorrectParams)
{
    QString capturedId;
    bool capturedResult = false;

    stub.set_lamda(ADDR(ReportLogManager, reportBlockMountData),
                   [&capturedId, &capturedResult](ReportLogManager *, const QString &id, bool result) {
                       __DBG_STUB_INVOKE__
                       capturedId = id;
                       capturedResult = result;
                   });

    receiver->handleBlockMountData("/dev/sdb1", false);

    EXPECT_EQ(capturedId, "/dev/sdb1");
    EXPECT_FALSE(capturedResult);
}

TEST_F(UT_ReportLogEventReceiver, handleMountNetworkResult_CallsManager)
{
    bool managerCalled = false;

    stub.set_lamda(ADDR(ReportLogManager, reportNetworkMountData),
                   [&managerCalled](ReportLogManager *, bool, dfmmount::DeviceError, const QString &) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                   });

    receiver->handleMountNetworkResult("smb://server", true, dfmmount::DeviceError::kNoError, "");

    EXPECT_TRUE(managerCalled);
}

TEST_F(UT_ReportLogEventReceiver, handleMountNetworkResult_PassesCorrectParams)
{
    bool capturedRet = false;
    dfmmount::DeviceError capturedErr;
    QString capturedMsg;

    stub.set_lamda(ADDR(ReportLogManager, reportNetworkMountData),
                   [&capturedRet, &capturedErr, &capturedMsg](ReportLogManager *, bool ret, dfmmount::DeviceError err, const QString &msg) {
                       __DBG_STUB_INVOKE__
                       capturedRet = ret;
                       capturedErr = err;
                       capturedMsg = msg;
                   });

    receiver->handleMountNetworkResult("smb://server", false, dfmmount::DeviceError::kUserErrorFailed, "Connection failed");

    EXPECT_FALSE(capturedRet);
    EXPECT_EQ(capturedErr, dfmmount::DeviceError::kUserErrorFailed);
    EXPECT_EQ(capturedMsg, "Connection failed");
}

TEST_F(UT_ReportLogEventReceiver, handleDesktopStartupData_CallsManager)
{
    bool managerCalled = false;

    stub.set_lamda(ADDR(ReportLogManager, reportDesktopStartUp),
                   [&managerCalled](ReportLogManager *, const QString &, const QVariant &) {
                       __DBG_STUB_INVOKE__
                       managerCalled = true;
                   });

    receiver->handleDesktopStartupData("testKey", QVariant("testData"));

    EXPECT_TRUE(managerCalled);
}

TEST_F(UT_ReportLogEventReceiver, handleDesktopStartupData_PassesCorrectParams)
{
    QString capturedKey;
    QVariant capturedData;

    stub.set_lamda(ADDR(ReportLogManager, reportDesktopStartUp),
                   [&capturedKey, &capturedData](ReportLogManager *, const QString &key, const QVariant &data) {
                       __DBG_STUB_INVOKE__
                       capturedKey = key;
                       capturedData = data;
                   });

    receiver->handleDesktopStartupData("loadTime", QVariant(1500));

    EXPECT_EQ(capturedKey, "loadTime");
    EXPECT_EQ(capturedData.toInt(), 1500);
}

TEST_F(UT_ReportLogEventReceiver, bindEvents_ConnectsSignals)
{
    stub.set_lamda(ADDR(Event, eventType),
                   [](Event *, const QString &, const QString &) -> DPF_NAMESPACE::EventType {
                       __DBG_STUB_INVOKE__
                       return DPF_NAMESPACE::EventTypeScope::kInValid;
                   });

    typedef bool (EventDispatcherManager::*SubscribeFunc)(const QString &, const QString &, ReportLogEventReceiver *, void (ReportLogEventReceiver::*)(const QString &, const QVariantMap &));
    stub.set_lamda(static_cast<SubscribeFunc>(&EventDispatcherManager::subscribe),
                   [](EventDispatcherManager *, const QString &, const QString &, ReportLogEventReceiver *, void (ReportLogEventReceiver::*)(const QString &, const QVariantMap &)) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&LifeCycle::pluginMetaObj,
                   [] {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    receiver->bindEvents();
}

TEST_F(UT_ReportLogEventReceiver, lazyBindCommitEvent_PluginStarted_SubscribesDirectly)
{
    auto mockMeta = QSharedPointer<PluginMetaObject>::create();
    stub.set_lamda(&LifeCycle::pluginMetaObj,
                   [&mockMeta] {
                       __DBG_STUB_INVOKE__
                       return mockMeta;
                   });

    stub.set_lamda(ADDR(PluginMetaObject, pluginState),
                   [](PluginMetaObject *) -> PluginMetaObject::State {
                       __DBG_STUB_INVOKE__
                       return PluginMetaObject::kStarted;
                   });

    bool subscribeCalled = false;
    typedef bool (EventDispatcherManager::*SubscribeFunc)(const QString &, const QString &, ReportLogEventReceiver *, void (ReportLogEventReceiver::*)(const QString &, const QVariantMap &));
    stub.set_lamda(static_cast<SubscribeFunc>(&EventDispatcherManager::subscribe),
                   [&subscribeCalled](EventDispatcherManager *, const QString &, const QString &, ReportLogEventReceiver *, void (ReportLogEventReceiver::*)(const QString &, const QVariantMap &)) -> bool {
                       __DBG_STUB_INVOKE__
                       subscribeCalled = true;
                       return true;
                   });

    receiver->lazyBindCommitEvent("dfmplugin-search", "dfmplugin_search");

    EXPECT_TRUE(subscribeCalled);
}

TEST_F(UT_ReportLogEventReceiver, lazyBindCommitEvent_PluginNotStarted_ConnectsListener)
{
    stub.set_lamda(&LifeCycle::pluginMetaObj,
                   [] {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    receiver->lazyBindCommitEvent("dfmplugin-search", "dfmplugin_search");
}

TEST_F(UT_ReportLogEventReceiver, lazyBindMenuDataEvent_PluginStarted_SubscribesDirectly)
{
    auto mockMeta = QSharedPointer<PluginMetaObject>::create();
    stub.set_lamda(&LifeCycle::pluginMetaObj,
                   [&mockMeta] {
                       __DBG_STUB_INVOKE__
                       return mockMeta;
                   });

    stub.set_lamda(ADDR(PluginMetaObject, pluginState),
                   [](PluginMetaObject *) -> PluginMetaObject::State {
                       __DBG_STUB_INVOKE__
                       return PluginMetaObject::kStarted;
                   });

    bool subscribeCalled = false;
    typedef bool (EventDispatcherManager::*SubscribeFunc)(const QString &, const QString &, ReportLogEventReceiver *, void (ReportLogEventReceiver::*)(const QString &, const QList<QUrl> &));
    stub.set_lamda(static_cast<SubscribeFunc>(&EventDispatcherManager::subscribe),
                   [&subscribeCalled](EventDispatcherManager *, const QString &, const QString &, ReportLogEventReceiver *, void (ReportLogEventReceiver::*)(const QString &, const QList<QUrl> &)) -> bool {
                       __DBG_STUB_INVOKE__
                       subscribeCalled = true;
                       return true;
                   });

    receiver->lazyBindMenuDataEvent("dfmplugin-myshares", "dfmplugin_myshares");

    EXPECT_TRUE(subscribeCalled);
}

TEST_F(UT_ReportLogEventReceiver, lazyBindMenuDataEvent_PluginNotStarted_ConnectsListener)
{
    stub.set_lamda(&LifeCycle::pluginMetaObj,
                   [] {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    receiver->lazyBindMenuDataEvent("dfmplugin-myshares", "dfmplugin_myshares");
}

