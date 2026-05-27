// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "openwith/openwithhelper.h"
#include "plugins/common/dfmplugin-utils/openwith/virtualopenwithplugin.h"
#include "plugins/common/dfmplugin-utils/openwith/openwitheventreceiver.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DPF_USE_NAMESPACE

class UT_VirtualOpenWithPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        plugin = new VirtualOpenWithPlugin();
    }

    void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    VirtualOpenWithPlugin *plugin { nullptr };
};

TEST_F(UT_VirtualOpenWithPlugin, Constructor_EventReceiverInitialized)
{
    EXPECT_NE(plugin->eventReceiver.data(), nullptr);
}

TEST_F(UT_VirtualOpenWithPlugin, initialize_CallsInitEventConnect)
{
    bool initEventConnectCalled = false;

    stub.set_lamda(ADDR(OpenWithEventReceiver, initEventConnect),
                   [&initEventConnectCalled](OpenWithEventReceiver *) {
                       __DBG_STUB_INVOKE__
                       initEventConnectCalled = true;
                   });

    plugin->initialize();

    EXPECT_TRUE(initEventConnectCalled);
}

TEST_F(UT_VirtualOpenWithPlugin, start_PropertyDialogInitialized_RegsView)
{
    bool regViewCalled = false;

    auto mockMetaObj = QSharedPointer<PluginMetaObject>(new PluginMetaObject);
    stub.set_lamda(&LifeCycle::pluginMetaObj,
                   [&mockMetaObj] {
                       __DBG_STUB_INVOKE__
                       return mockMetaObj;
                   });

    stub.set_lamda(&PluginMetaObject::pluginState,
                   [](PluginMetaObject *) -> PluginMetaObject::State {
                       __DBG_STUB_INVOKE__
                       return PluginMetaObject::kStarted;
                   });

    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, decltype(OpenWithHelper::createOpenWithWidget), const char(&)[8], int &&);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push),
                   [&regViewCalled] {
                       __DBG_STUB_INVOKE__
                       regViewCalled = true;
                       return QVariant();
                   });

    bool result = plugin->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(regViewCalled);
}

TEST_F(UT_VirtualOpenWithPlugin, start_PropertyDialogNotInitialized_ConnectsListener)
{
    bool listenerConnected = false;

    stub.set_lamda(&LifeCycle::pluginMetaObj,
                   [] {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    stub.set_lamda(ADDR(Listener, instance),
                   [&listenerConnected]() -> Listener * {
                       __DBG_STUB_INVOKE__
                       listenerConnected = true;
                       static Listener listener;
                       return &listener;
                   });

    bool result = plugin->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(listenerConnected);
}

TEST_F(UT_VirtualOpenWithPlugin, regViewToPropertyDialog_PushesSlot)
{
    bool pushCalled = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, decltype(OpenWithHelper::createOpenWithWidget), const char(&)[8], int &&);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push),
                   [&pushCalled] {
                       __DBG_STUB_INVOKE__
                       pushCalled = true;
                       return QVariant();
                   });

    plugin->regViewToPropertyDialog();
    EXPECT_TRUE(pushCalled);
}
