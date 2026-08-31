// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualappendcompressplugin.cpp
 * @brief Unit tests for VirtualAppendCompressPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "appendcompress/virtualappendcompressplugin.h"

#include <QTest>

using namespace dfmplugin_utils;

class VirtualAppendCompressPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualAppendCompressPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualAppendCompressPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualAppendCompressPluginTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(VirtualAppendCompressPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
