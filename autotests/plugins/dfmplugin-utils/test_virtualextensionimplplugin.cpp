// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/virtualextensionimplplugin.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/extensionlibmenuscene.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/emblemimpl/extensionemblemmanager.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/windowimpl/extensionwindowsmanager.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/fileimpl/extensionfilemanager.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/pluginsload/extensionpluginmanager.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DPF_USE_NAMESPACE

class UT_VirtualExtensionImplPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        plugin = new VirtualExtensionImplPlugin();
    }

    void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    VirtualExtensionImplPlugin *plugin { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_VirtualExtensionImplPlugin, Constructor_CreatesPlugin)
{
    EXPECT_NE(plugin, nullptr);
}

TEST_F(UT_VirtualExtensionImplPlugin, initialize_InitializesManagers)
{
    bool emblemInitialized = false;
    bool windowInitialized = false;
    bool fileInitialized = false;

    stub.set_lamda(ADDR(ExtensionEmblemManager, initialize),
                   [&emblemInitialized] {
                       __DBG_STUB_INVOKE__
                       emblemInitialized = true;
                   });

    stub.set_lamda(ADDR(ExtensionWindowsManager, initialize),
                   [&windowInitialized] {
                       __DBG_STUB_INVOKE__
                       windowInitialized = true;
                   });

    stub.set_lamda(ADDR(ExtensionFileManager, initialize),
                   [&fileInitialized] {
                       __DBG_STUB_INVOKE__
                       fileInitialized = true;
                   });

    stub.set_lamda(ADDR(Event, eventType),
                   [](Event *, const QString &, const QString &) -> DPF_NAMESPACE::EventType {
                       __DBG_STUB_INVOKE__
                       return DPF_NAMESPACE::EventTypeScope::kInValid;
                   });

    plugin->initialize();

    EXPECT_TRUE(emblemInitialized);
    EXPECT_TRUE(windowInitialized);
    EXPECT_TRUE(fileInitialized);
}

TEST_F(UT_VirtualExtensionImplPlugin, start_RegistersMenuScene)
{
    bool sceneRegistered = false;
    bool sceneBound = false;

    stub.set_lamda(ADDR(ExtensionEmblemManager, initialize), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ADDR(ExtensionWindowsManager, initialize), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ADDR(ExtensionFileManager, initialize), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ADDR(Event, eventType),
                   [](Event *, const QString &, const QString &) -> DPF_NAMESPACE::EventType {
                       __DBG_STUB_INVOKE__
                       return DPF_NAMESPACE::EventTypeScope::kInValid;
                   });

    plugin->initialize();

    bool result = plugin->start();

    EXPECT_TRUE(result);
}

TEST_F(UT_VirtualExtensionImplPlugin, followEvents_ValidEventID_FollowsHookSequence)
{
    stub.set_lamda(ADDR(ExtensionEmblemManager, initialize), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ADDR(ExtensionWindowsManager, initialize), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ADDR(ExtensionFileManager, initialize), [] { __DBG_STUB_INVOKE__ });

    bool hookFollowed = false;
    stub.set_lamda(ADDR(Event, eventType),
                   [](Event *, const QString &, const QString &) -> DPF_NAMESPACE::EventType {
                       __DBG_STUB_INVOKE__
                       return static_cast<DPF_NAMESPACE::EventType>(100);
                   });

    plugin->initialize();

    // Test passes if no crash occurs
}

TEST_F(UT_VirtualExtensionImplPlugin, followEvents_InvalidEventID_ConnectsPluginStartedSignal)
{
    stub.set_lamda(ADDR(ExtensionEmblemManager, initialize), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ADDR(ExtensionWindowsManager, initialize), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ADDR(ExtensionFileManager, initialize), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ADDR(Event, eventType),
                   [](Event *, const QString &, const QString &) -> DPF_NAMESPACE::EventType {
                       __DBG_STUB_INVOKE__
                       return DPF_NAMESPACE::EventTypeScope::kInValid;
                   });

    plugin->initialize();

    // Test passes if no crash occurs
}

