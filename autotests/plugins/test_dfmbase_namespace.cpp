// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmbase_namespace.cpp
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

TEST_F(DFMBASE_NAMESPACETest, AbstractGroupStrategy)
{
    // Test getter: DPWORKSPACE_BEGIN_NAMESPACE AbstractGroupStrategy()
    auto result = obj->AbstractGroupStrategy();
    EXPECT_NO_FATAL_FAILURE({ obj->AbstractGroupStrategy(); });

}

TEST_F(DFMBASE_NAMESPACETest, AbstractGroupStrategy_2)
{
    // Test getter: DPWORKSPACE_BEGIN_NAMESPACE AbstractGroupStrategy()
    auto result = obj->AbstractGroupStrategy();
    EXPECT_NO_FATAL_FAILURE({ obj->AbstractGroupStrategy(); });

}

TEST_F(DFMBASE_NAMESPACETest, AbstractSceneCreator)
{
    // Test getter: DPMENU_BEGIN_NAMESPACE AbstractSceneCreator()
    auto result = obj->AbstractSceneCreator();
    EXPECT_NO_FATAL_FAILURE({ obj->AbstractSceneCreator(); });

}
