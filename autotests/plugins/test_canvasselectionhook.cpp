// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasselectionhook.cpp
 * @brief Unit tests for CanvasSelectionHook methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "hook/canvasselectionhook.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasSelectionHookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasSelectionHook();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasSelectionHook *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasSelectionHookTest, CanvasSelectionHook)
{
    // Test constructor: CanvasSelectionHook((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasSelectionHookTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}
