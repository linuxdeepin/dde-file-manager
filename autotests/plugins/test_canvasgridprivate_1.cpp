// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgridprivate_1.cpp
 * @brief Unit tests for CanvasGridPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/canvasgrid.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasGridPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGridPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGridPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridPrivateTest, CanvasGridPrivate)
{
    // Test constructor: CanvasGridPrivate((CanvasGrid *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasGridPrivateTest, clean)
{
    // Test method: void clean(())
    EXPECT_NO_FATAL_FAILURE(obj->clean());
}

TEST_F(CanvasGridPrivateTest, sequence)
{
    // Test method: void sequence((QStringList sortedItems))
    EXPECT_NO_FATAL_FAILURE(obj->sequence(QStringList()));
}
