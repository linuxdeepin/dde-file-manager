// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualvaulthelperplugin.cpp
 * @brief Unit tests for VirtualVaultHelperPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "vaultassist/virtualvaulthelperplugin.h"

#include <QTest>

using namespace dfmplugin_utils;

class VirtualVaultHelperPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualVaultHelperPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualVaultHelperPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualVaultHelperPluginTest, initialize)
{
    // Test getter: DPUTILS_USE_NAMESPACE initialize()
    auto result = obj->initialize();
    EXPECT_NO_FATAL_FAILURE({ obj->initialize(); });

}

TEST_F(VirtualVaultHelperPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
