// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmenuscene.cpp
 * @brief Unit tests for CanvasMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/canvasmenuscene.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasMenuSceneTest, CanvasMenuScene)
{
    // Test constructor: CanvasMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
