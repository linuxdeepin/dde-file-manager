// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shredmenucreator.cpp
 * @brief Unit tests for ShredMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/shredmenuscene.h"

#include <QTest>

using namespace dfmplugin_utils;

class ShredMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShredMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShredMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShredMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(ShredMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
