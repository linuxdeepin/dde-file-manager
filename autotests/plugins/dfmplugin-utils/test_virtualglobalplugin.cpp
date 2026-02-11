// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/global/virtualglobalplugin.h"
#include "plugins/common/dfmplugin-utils/global/globaleventreceiver.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DPF_USE_NAMESPACE

class UT_VirtualGlobalPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(ADDR(GlobalEventReceiver, initEventConnect),
                       [](GlobalEventReceiver *) {
                           __DBG_STUB_INVOKE__
                       });

        plugin = new VirtualGlobalPlugin();
    }

    void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    VirtualGlobalPlugin *plugin { nullptr };
};

TEST_F(UT_VirtualGlobalPlugin, Constructor_EventReceiverInitialized)
{
    EXPECT_NE(plugin->eventReceiver.data(), nullptr);
}

TEST_F(UT_VirtualGlobalPlugin, initialize_CallsInitEventConnect)
{
    bool initEventConnectCalled = false;

    stub.set_lamda(ADDR(GlobalEventReceiver, initEventConnect),
                   [&initEventConnectCalled](GlobalEventReceiver *) {
                       __DBG_STUB_INVOKE__
                       initEventConnectCalled = true;
                   });

    plugin->initialize();

    EXPECT_TRUE(initEventConnectCalled);
}

TEST_F(UT_VirtualGlobalPlugin, start_ReturnsTrue)
{
    EXPECT_TRUE(plugin->start());
}

TEST_F(UT_VirtualGlobalPlugin, PluginLifecycle_InitializeThenStart)
{
    bool initCalled = false;

    stub.set_lamda(ADDR(GlobalEventReceiver, initEventConnect),
                   [&initCalled](GlobalEventReceiver *) {
                       __DBG_STUB_INVOKE__
                       initCalled = true;
                   });

    plugin->initialize();
    EXPECT_TRUE(initCalled);

    EXPECT_TRUE(plugin->start());
    EXPECT_NE(plugin->eventReceiver.data(), nullptr);
}
