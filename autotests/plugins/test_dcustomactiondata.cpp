// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactiondata.cpp
 * @brief Unit tests for DCustomActionData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactiondata.h"

#include <QTest>

using namespace dfmplugin_menu;

class DCustomActionDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DCustomActionData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DCustomActionData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DCustomActionDataTest, command)
{
    // Test getter: QString command()
    auto result = obj->command();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, parentPath)
{
    // Test getter: QString parentPath()
    auto result = obj->parentPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionDataTest, position)
{
    // Test getter: int position()
    auto result = obj->position();
    EXPECT_EQ(result, 0);

}

TEST_F(DCustomActionDataTest, DCustomActionData)
{
    // Test constructor: DCustomActionData((const DCustomActionData &other))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DCustomActionDataTest, separator)
{
    // Test getter: DCustomActionDefines::Separator separator()
    auto result = obj->separator();
    EXPECT_GE(static_cast<int>(result), 0);

}
