// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasinfo.cpp
 * @brief Unit tests for CanvasInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/canvasview_p.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasInfoTest, CanvasInfo)
{
    // Test constructor: CanvasInfo(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasInfoTest, gridCount)
{
    // Test getter: int gridCount()
    auto result = obj->gridCount();
    EXPECT_EQ(result, 0);

}
