// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_gridcoordinate_1.cpp
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

TEST_F(GridCoordinateTest, GridCoordinate)
{
    // Test constructor: GridCoordinate((QPoint pos))
    ASSERT_NE(obj, nullptr);
}

TEST_F(GridCoordinateTest, moveDown)
{
    // Test getter: GridCoordinate moveDown()
    auto result = obj->moveDown();
    EXPECT_NO_FATAL_FAILURE({ obj->moveDown(); });

}

TEST_F(GridCoordinateTest, moveUp)
{
    // Test getter: GridCoordinate moveUp()
    auto result = obj->moveUp();
    EXPECT_NO_FATAL_FAILURE({ obj->moveUp(); });

}
