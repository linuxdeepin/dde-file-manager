// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/testing/virtualtestingplugin.h"
#include "plugins/common/dfmplugin-utils/testing/events/testingeventrecevier.h"

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_VirtualTestingPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(ADDR(TestingEventRecevier, initializeConnections),
                       [](TestingEventRecevier *) {
                           __DBG_STUB_INVOKE__
                       });

        plugin = new VirtualTestingPlugin();
    }

    void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    VirtualTestingPlugin *plugin { nullptr };
};

TEST_F(UT_VirtualTestingPlugin, Constructor_CreatesPlugin)
{
    EXPECT_NE(plugin, nullptr);
}

TEST_F(UT_VirtualTestingPlugin, initialize_CallsInitializeConnections)
{
    bool initCalled = false;

    stub.set_lamda(ADDR(TestingEventRecevier, initializeConnections),
                   [&initCalled](TestingEventRecevier *) {
                       __DBG_STUB_INVOKE__
                       initCalled = true;
                   });

    plugin->initialize();

    EXPECT_TRUE(initCalled);
}

TEST_F(UT_VirtualTestingPlugin, start_ReturnsTrue)
{
    bool result = plugin->start();

    EXPECT_TRUE(result);
}

