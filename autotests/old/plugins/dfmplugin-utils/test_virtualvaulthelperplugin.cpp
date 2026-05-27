// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/vaultassist/virtualvaulthelperplugin.h"
#include "plugins/common/dfmplugin-utils/vaultassist/vaulthelperreceiver.h"

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_VirtualVaultHelperPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(ADDR(VaultHelperReceiver, initEventConnect),
                       [](VaultHelperReceiver *) {
                           __DBG_STUB_INVOKE__
                       });

        plugin = new VirtualVaultHelperPlugin();
    }

    void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    VirtualVaultHelperPlugin *plugin { nullptr };
};

TEST_F(UT_VirtualVaultHelperPlugin, Constructor_CreatesPlugin)
{
    EXPECT_NE(plugin, nullptr);
}

TEST_F(UT_VirtualVaultHelperPlugin, initialize_CallsInitEventConnect)
{
    bool initCalled = false;

    stub.set_lamda(ADDR(VaultHelperReceiver, initEventConnect),
                   [&initCalled](VaultHelperReceiver *) {
                       __DBG_STUB_INVOKE__
                       initCalled = true;
                   });

    plugin->initialize();

    EXPECT_TRUE(initCalled);
}

TEST_F(UT_VirtualVaultHelperPlugin, start_ReturnsTrue)
{
    bool result = plugin->start();

    EXPECT_TRUE(result);
}

