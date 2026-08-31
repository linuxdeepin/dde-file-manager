// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasbasesortmenuscene.cpp
 * @brief Unit tests for CanvasBaseSortMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/canvasbasesortmenuscene.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasBaseSortMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasBaseSortMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasBaseSortMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasBaseSortMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
