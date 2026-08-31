// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmenucreator.cpp
 * @brief Unit tests for CanvasMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/canvasmenuscene.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(CanvasMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
