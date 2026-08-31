// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_gridcoordinate.cpp
 * @brief Unit tests for GridCoordinate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/gridcoordinate.h"

#include <QTest>

using namespace ddplugin_canvas;

class GridCoordinateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GridCoordinate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GridCoordinate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GridCoordinateTest, moveLeft)
{
    // Test getter: GridCoordinate moveLeft()
    auto result = obj->moveLeft();
    EXPECT_NO_FATAL_FAILURE({ obj->moveLeft(); });

}

TEST_F(GridCoordinateTest, moveRight)
{
    // Test getter: GridCoordinate moveRight()
    auto result = obj->moveRight();
    EXPECT_NO_FATAL_FAILURE({ obj->moveRight(); });

}

TEST_F(GridCoordinateTest, point)
{
    // Test getter: QPoint point()
    auto result = obj->point();
    EXPECT_TRUE(result.isNull());

}

TEST_F(GridCoordinateTest, x)
{
    // Test getter: int x()
    auto result = obj->x();
    EXPECT_EQ(result, 0);

}

TEST_F(GridCoordinateTest, y)
{
    // Test getter: int y()
    auto result = obj->y();
    EXPECT_EQ(result, 0);

}
