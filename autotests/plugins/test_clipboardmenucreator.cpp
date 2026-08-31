// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_clipboardmenucreator.cpp
 * @brief Unit tests for ClipBoardMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/clipboardmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ClipBoardMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ClipBoardMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ClipBoardMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ClipBoardMenuCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(ClipBoardMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
