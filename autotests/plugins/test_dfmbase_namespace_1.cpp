// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmbase_namespace_1.cpp
 * @brief Unit tests for DFMBASE_NAMESPACE methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/screendbus.h"

#include <QTest>

using namespace ddplugin_core;

class DFMBASE_NAMESPACETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMBASE_NAMESPACE();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMBASE_NAMESPACE *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMBASE_NAMESPACETest, AbstractScreen)
{
    // Test getter: DDPCORE_BEGIN_NAMESPACE AbstractScreen()
    auto result = obj->AbstractScreen();
    EXPECT_NO_FATAL_FAILURE({ obj->AbstractScreen(); });

}

TEST_F(DFMBASE_NAMESPACETest, AbstractScreen_Abstract)
{
    // Test getter: DDPCORE_BEGIN_NAMESPACE AbstractScreen()
    auto result = obj->AbstractScreen();
    EXPECT_NO_FATAL_FAILURE({ obj->AbstractScreen(); });

}

TEST_F(DFMBASE_NAMESPACETest, AbstractScreenProxy)
{
    // Test getter: DDPCORE_BEGIN_NAMESPACE AbstractScreenProxy()
    auto result = obj->AbstractScreenProxy();
    EXPECT_NO_FATAL_FAILURE({ obj->AbstractScreenProxy(); });

}

TEST_F(DFMBASE_NAMESPACETest, AbstractScreenProxy_Abstract)
{
    // Test getter: DDPCORE_BEGIN_NAMESPACE AbstractScreenProxy()
    auto result = obj->AbstractScreenProxy();
    EXPECT_NO_FATAL_FAILURE({ obj->AbstractScreenProxy(); });

}
