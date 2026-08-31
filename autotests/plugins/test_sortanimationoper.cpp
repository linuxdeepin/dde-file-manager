// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortanimationoper.cpp
 * @brief Unit tests for SortAnimationOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/sortanimationoper.h"

#include <QTest>

using namespace ddplugin_canvas;

class SortAnimationOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortAnimationOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortAnimationOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortAnimationOperTest, SortAnimationOper)
{
    // Test constructor: SortAnimationOper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SortAnimationOperTest, getPrepareMove)
{
    // Test bool getter: getPrepareMove()
    bool result = obj->getPrepareMove();
    EXPECT_FALSE(result);

}

TEST_F(SortAnimationOperTest, startMoveAnimation)
{
    // Test method: void startMoveAnimation(())
    EXPECT_NO_FATAL_FAILURE(obj->startMoveAnimation());
}
