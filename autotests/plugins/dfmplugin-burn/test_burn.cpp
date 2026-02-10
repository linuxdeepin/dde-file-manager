// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/burn.h"
#include "plugins/common/dfmplugin-burn/utils/discstatemanager.h"
#include "plugins/common/dfmplugin-burn/utils/burnhelper.h"
#include "plugins/common/dfmplugin-burn/utils/burnsignalmanager.h"
#include "plugins/common/dfmplugin-burn/events/burneventreceiver.h"

#include <dfm-framework/dpf.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/device/deviceutils.h>

#include <QUrl>
#include <QSet>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_Burn : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        plugin = new Burn();
    }

    virtual void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    Burn *plugin = nullptr;
};

TEST_F(UT_Burn, initialize)
{
    bool bindEventsCalled = false;
    stub.set_lamda(ADDR(Burn, bindEvents), [&bindEventsCalled] {
        __DBG_STUB_INVOKE__
        bindEventsCalled = true;
    });

    plugin->initialize();
    EXPECT_TRUE(bindEventsCalled);
}

TEST_F(UT_Burn, initialize_BurnDisabled)
{
    bool initializeManagerCalled = false;

    stub.set_lamda(ADDR(DiscStateManager, initilaize), [&initializeManagerCalled] {
        __DBG_STUB_INVOKE__
        initializeManagerCalled = true;
    });

    stub.set_lamda(ADDR(DiscStateManager, instance), [] {
        __DBG_STUB_INVOKE__
        static DiscStateManager manager;
        return &manager;
    });

    stub.set_lamda(ADDR(BurnEventReceiver, instance), [] {
        __DBG_STUB_INVOKE__
        static BurnEventReceiver receiver;
        return &receiver;
    });

    stub.set_lamda(ADDR(BurnHelper, isBurnEnabled), [] {
        __DBG_STUB_INVOKE__
        return false;   // Burn disabled
    });

    plugin->initialize();

    EXPECT_FALSE(initializeManagerCalled);   // Should not initialize when disabled
}

TEST_F(UT_Burn, start)
{
    bool bindSceneCalled = false;

    stub.set_lamda(ADDR(Burn, bindScene), [&bindSceneCalled] {
        __DBG_STUB_INVOKE__
        bindSceneCalled = true;
    });

    bool result = plugin->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(bindSceneCalled);
}

TEST_F(UT_Burn, bindScene)
{
    plugin->bindScene("WorkspaceScene");
}

TEST_F(UT_Burn, bindSceneOnAdded)
{
    plugin->bindSceneOnAdded("WorkspaceScene");
}

TEST_F(UT_Burn, bindSceneOnAdded_NotInWaitList)
{
    bool bindSceneCalled = false;

    stub.set_lamda(ADDR(Burn, bindScene), [&bindSceneCalled] {
        __DBG_STUB_INVOKE__
        bindSceneCalled = true;
    });

    plugin->bindSceneOnAdded("UnknownScene");

    EXPECT_FALSE(bindSceneCalled);   // Should not bind unknown scenes
}

TEST_F(UT_Burn, bindEvents)
{
    plugin->bindEvents();
}

TEST_F(UT_Burn, changeUrlEventFilter_BurnUrl)
{
    QUrl burnUrl;
    burnUrl.setScheme("burn");
    burnUrl.setPath("/dev/sr0/disc_files/");
    stub.set_lamda(&DeviceUtils::isWorkingOpticalDiscDev, []() {
        return true;
    });

    bool result = plugin->changeUrlEventFilter(12345, burnUrl);

    EXPECT_TRUE(result);   // Should handle burn URLs
}

TEST_F(UT_Burn, changeUrlEventFilter_NonBurnUrl)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/test.txt");

    bool result = plugin->changeUrlEventFilter(12345, fileUrl);

    EXPECT_FALSE(result);   // Should not handle non-burn URLs
}

TEST_F(UT_Burn, onPersistenceDataChanged_BurnEnable_True)
{
    plugin->onPersistenceDataChanged("BurnState", "burnEnable", QVariant(true));
}

TEST_F(UT_Burn, onPersistenceDataChanged_BurnEnable_False)
{
    bool initializeCalled = false;

    stub.set_lamda(VADDR(Burn, initialize), [&initializeCalled] {
        __DBG_STUB_INVOKE__
        initializeCalled = true;
    });

    plugin->onPersistenceDataChanged("BurnState", "burnEnable", QVariant(false));

    EXPECT_FALSE(initializeCalled);   // Should not initialize when disabled
}

TEST_F(UT_Burn, onPersistenceDataChanged_OtherGroup)
{
    bool initializeCalled = false;

    stub.set_lamda(VADDR(Burn, initialize), [&initializeCalled] {
        __DBG_STUB_INVOKE__
        initializeCalled = true;
    });

    plugin->onPersistenceDataChanged("OtherGroup", "burnEnable", QVariant(true));

    EXPECT_FALSE(initializeCalled);   // Should not respond to other groups
}

TEST_F(UT_Burn, onPersistenceDataChanged_OtherKey)
{
    bool initializeCalled = false;

    stub.set_lamda(VADDR(Burn, initialize), [&initializeCalled] {
        __DBG_STUB_INVOKE__
        initializeCalled = true;
    });

    plugin->onPersistenceDataChanged("BurnState", "otherKey", QVariant(true));

    EXPECT_FALSE(initializeCalled);   // Should not respond to other keys
}

TEST_F(UT_Burn, Constructor)
{
    Burn burnPlugin;

    // Should construct without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_Burn, bindEvents_EventSubscription)
{
    plugin->bindEvents();
}

TEST_F(UT_Burn, start_ReturnValue)
{
    stub.set_lamda(ADDR(Burn, bindScene), [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(ADDR(Burn, bindEvents), [] {
        __DBG_STUB_INVOKE__
    });

    bool result = plugin->start();

    EXPECT_TRUE(result);   // Should always return true
}

TEST_F(UT_Burn, changeUrlEventFilter_EmptyUrl)
{
    QUrl emptyUrl;

    bool result = plugin->changeUrlEventFilter(12345, emptyUrl);

    EXPECT_FALSE(result);   // Should not handle empty URLs
}
