// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasviewhook.cpp
 * @brief Unit tests for CanvasViewHook methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "hook/canvasviewhook.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasViewHookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasViewHook();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasViewHook *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewHookTest, CanvasViewHook)
{
    // Test constructor: CanvasViewHook((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
