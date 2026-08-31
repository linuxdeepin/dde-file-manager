// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualtestingplugin.cpp
 * @brief Unit tests for VirtualTestingPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "testing/virtualtestingplugin.h"

#include <QTest>

using namespace dfmplugin_utils;

class VirtualTestingPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualTestingPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualTestingPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualTestingPluginTest, initialize)
{
    // Test getter: DPUTILS_USE_NAMESPACE initialize()
    auto result = obj->initialize();
    EXPECT_NO_FATAL_FAILURE({ obj->initialize(); });

}

TEST_F(VirtualTestingPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
