// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmodelhook.cpp
 * @brief Unit tests for CanvasModelHook methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "hook/canvasmodelhook.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasModelHookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasModelHook();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasModelHook *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasModelHookTest, CanvasModelHook)
{
    // Test constructor: CanvasModelHook((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasModelHookTest, dataRemoved)
{
    // Test method: bool dataRemoved((const QUrl &url, void *extData))
    QUrl _arg0{};
    auto result = obj->dataRemoved(_arg0, nullptr);
    EXPECT_FALSE(result);

}
