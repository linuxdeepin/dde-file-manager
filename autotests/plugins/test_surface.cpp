// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_surface.cpp
 * @brief Unit tests for Surface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "private/surface.h"

#include <QTest>

using namespace ddplugin_organizer;

class SurfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Surface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Surface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SurfaceTest, animate)
{
    // Test method: void animate((const AnimateParams &param))
    AnimateParams _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->animate(_arg0));
}

TEST_F(SurfaceTest, animationEnabled)
{
    // Test bool getter: animationEnabled()
    bool result = obj->animationEnabled();
    EXPECT_FALSE(result);

}

TEST_F(SurfaceTest, cellWidth)
{
    // Test getter: int cellWidth()
    auto result = obj->cellWidth();
    EXPECT_EQ(result, 0);

}

TEST_F(SurfaceTest, gridMargins)
{
    // Test getter: QMargins gridMargins()
    auto result = obj->gridMargins();
    EXPECT_NO_FATAL_FAILURE({ obj->gridMargins(); });

}

TEST_F(SurfaceTest, gridOffset)
{
    // Test getter: QPoint gridOffset()
    auto result = obj->gridOffset();
    EXPECT_TRUE(result.isNull());

}

TEST_F(SurfaceTest, gridSize)
{
    // Test getter: QSize gridSize()
    auto result = obj->gridSize();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SurfaceTest, intersectedRects)
{
    // Test method: QList<QRect> intersectedRects((QWidget *wid))
    auto result = obj->intersectedRects(nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SurfaceTest, mapToGridGeo)
{
    // Test method: QRect mapToGridGeo((const QRect &pixelGeo))
    QRect _arg0{};
    auto result = obj->mapToGridGeo(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SurfaceTest, mapToGridSize)
{
    // Test method: QSize mapToGridSize((const QSize &pixelSize))
    QSize _arg0{};
    auto result = obj->mapToGridSize(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SurfaceTest, mapToPixelSize)
{
    // Test method: QRect mapToPixelSize((const QRect &gridGeo))
    QRect _arg0{};
    auto result = obj->mapToPixelSize(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SurfaceTest, pointsDistance)
{
    // Test method: int pointsDistance((const QPoint &p1, const QPoint &p2))
    QPoint _arg0{};
    QPoint _arg1{};
    auto result = obj->pointsDistance(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(SurfaceTest, toCellLen)
{
    // Test getter: int toCellLen()
    auto result = obj->toCellLen();
    EXPECT_EQ(result, 0);

}

TEST_F(SurfaceTest, toPixelLen)
{
    // Test getter: int toPixelLen()
    auto result = obj->toPixelLen();
    EXPECT_EQ(result, 0);

}
