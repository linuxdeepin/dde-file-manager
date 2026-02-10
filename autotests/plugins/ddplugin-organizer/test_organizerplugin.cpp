// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stubext.h>
#include "organizerplugin.h"
#include "framemanager.h"

#include <QDBusInterface>

#include <gtest/gtest.h>
#include <dfm-framework/dpf.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>


using namespace ddplugin_organizer;
DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_OrganizerPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        // mock QDBusInterface::call
        stub.set_lamda(ADDR(QDBusInterface, doCall), [] {
            __DBG_STUB_INVOKE__
            return QDBusMessage();
        });

        plugin = new OrganizerPlugin();
    }

    void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    OrganizerPlugin *plugin = nullptr;
};

TEST_F(UT_OrganizerPlugin, Constructor_Default_InitializesCorrectly)
{
    EXPECT_NE(plugin, nullptr);
    EXPECT_EQ(plugin->instance, nullptr);
}

TEST_F(UT_OrganizerPlugin, initialize_Always_CallsDConfigManager)
{
    bool addConfigCalled = false;
    // stub.set_lamda((void (DConfigManager::*)(const QString &, QString *))&DConfigManager::addConfig, [&addConfigCalled]() {
    //     __DBG_STUB_INVOKE__
    //     addConfigCalled = true;
    // });
    // Stub DConfigManager to avoid real configuration operations
    stub.set_lamda(&DConfigManager::addConfig, [&](DConfigManager *, const QString &, QString *) -> bool {
        __DBG_STUB_INVOKE__
        addConfigCalled = true;
        return true;
    });

    plugin->initialize();
    EXPECT_TRUE(addConfigCalled);
}

TEST_F(UT_OrganizerPlugin, start_WhenInitialized_ReturnsTrue)
{
    // Stub FrameManager methods to avoid actual initialization
    stub.set_lamda(&FrameManager::initialize, [](FrameManager *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    bool result = plugin->start();
    EXPECT_TRUE(result);
    EXPECT_NE(plugin->instance, nullptr);
}

TEST_F(UT_OrganizerPlugin, start_WhenFrameManagerInitFails_ReturnsFalse)
{
    // Stub FrameManager methods to simulate failure
    stub.set_lamda(&FrameManager::initialize, [](FrameManager *) {
        __DBG_STUB_INVOKE__
        return false;
    });
    
    bool result = plugin->start();
    EXPECT_FALSE(result);
}

TEST_F(UT_OrganizerPlugin, stop_Always_DeletesInstance)
{
    // First start to create instance
    stub.set_lamda(&FrameManager::initialize, [](FrameManager *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    plugin->start();
    EXPECT_NE(plugin->instance, nullptr);
    
    // Then stop should delete it
    plugin->stop();
    EXPECT_EQ(plugin->instance, nullptr);
}

TEST_F(UT_OrganizerPlugin, bindEvent_Always_ConnectsEventChannel)
{
    // Start plugin to create FrameManager instance
    stub.set_lamda(&FrameManager::initialize, [](FrameManager *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    plugin->start();
    
    bool connectCalled = false;
    // Mock EventChannelManager::connect
    // dpfSlotChannel->connect("ddplugin_organizer", "slot_Organizer_Enabled", instance, &FrameManager::organizerEnabled);
    typedef bool (dpf::EventChannelManager::*Connect)(const QString &, const QString &, FrameManager *, decltype(&FrameManager::organizerEnabled));
    auto conn = static_cast<Connect>(&dpf::EventChannelManager::connect);
    stub.set_lamda(conn, [&] {
        __DBG_STUB_INVOKE__
        connectCalled = true;
        return true;
    });
    
    plugin->bindEvent();
    EXPECT_TRUE(connectCalled);
}

TEST_F(UT_OrganizerPlugin, stop_WithoutStart_DoesNotCrash)
{
    // Call stop without calling start first
    EXPECT_NO_THROW(plugin->stop());
}

TEST_F(UT_OrganizerPlugin, start_CalledTwice_DeletesPreviousInstance)
{
    // Stub FrameManager methods
    stub.set_lamda(&FrameManager::initialize, [](FrameManager *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    // First start
    plugin->start();
    auto firstInstance = plugin->instance;
    EXPECT_NE(firstInstance, nullptr);
    
    // Second start should delete first instance and create new one
    plugin->start();
    EXPECT_NE(plugin->instance, nullptr);
    EXPECT_NE(plugin->instance, firstInstance);
}
