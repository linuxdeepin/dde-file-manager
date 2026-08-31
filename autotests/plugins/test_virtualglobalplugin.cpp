// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualglobalplugin.cpp
 * @brief Unit tests for VirtualGlobalPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "global/virtualglobalplugin.h"

#include <QTest>

using namespace dfmplugin_utils;

class VirtualGlobalPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualGlobalPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualGlobalPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualGlobalPluginTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(VirtualGlobalPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
