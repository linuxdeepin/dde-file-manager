// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_gridcore.cpp
 * @brief Unit tests for GridCore Mid-priority methods (ddplugin-canvas)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "grid/gridcore.h"

using namespace ddplugin_canvas;

class GridCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(GridCoreTest, applay)
{
    // Instance method applay
    GridCore obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.applay(nullptr); });
    (void)result;
}

TEST_F(GridCoreTest, insert)
{
    // Instance method insert
    GridCore obj;
    EXPECT_NO_FATAL_FAILURE({ obj.insert(0, QPoint(0, 0), QString("test")); });
}

TEST_F(GridCoreTest, isValid)
{
    // Instance method isValid
    GridCore obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.isValid(0, QPoint(0, 0)); });
    (void)result;
}

TEST_F(GridCoreTest, position)
{
    GridPos _arg_pos;
    // Instance method position
    GridCore obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.position(QString("test"), _arg_pos); });
    (void)result;
}

TEST_F(GridCoreTest, remove)
{
    // Instance method remove
    GridCore obj;
    EXPECT_NO_FATAL_FAILURE({ obj.remove(0, QPoint(0, 0)); });
}

TEST_F(GridCoreTest, surfaceIndex)
{
    // Instance method surfaceIndex
    GridCore obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.surfaceIndex(); (void)r; });
}

TEST_F(GridCoreTest, voidPos)
{
    // Instance method voidPos
    GridCore obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.voidPos(0); (void)r; });
}
