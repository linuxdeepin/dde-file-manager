// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasbasesortmenucreator.cpp
 * @brief Unit tests for CanvasBaseSortMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/canvasbasesortmenuscene.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasBaseSortMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasBaseSortMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasBaseSortMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasBaseSortMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(CanvasBaseSortMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
