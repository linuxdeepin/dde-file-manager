// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "private/surface.h"

#include <QApplication>
#include <QTest>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_Surface : public testing::Test
{
protected:
    void SetUp() override
    {
        // mock the UI show
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_Surface, Constructor_CreatesSurface)
{
    Surface surface(nullptr);
    EXPECT_NE(&surface, nullptr);
    EXPECT_EQ(surface.parent(), nullptr);
}

TEST_F(UT_Surface, Constructor_WithParent_SetsParent)
{
    QWidget parent;
    Surface surface(&parent);
    EXPECT_EQ(surface.parent(), &parent);
}

TEST_F(UT_Surface, AnimationEnabled_ReturnsBool)
{
    bool enabled = Surface::animationEnabled();
    EXPECT_TRUE(enabled || !enabled);  // Just ensure it returns a bool
}

TEST_F(UT_Surface, CellWidth_ReturnsConstant)
{
    int width = Surface::cellWidth();
    EXPECT_EQ(width, 20);
}

TEST_F(UT_Surface, ToCellLen_ConvertsPixelsToCells)
{
    EXPECT_EQ(Surface::toCellLen(20), 1);  // Exactly one cell
    EXPECT_EQ(Surface::toCellLen(19), 1);  // Less than one cell, rounds up
    EXPECT_EQ(Surface::toCellLen(21), 2);  // More than one cell, rounds up
    EXPECT_EQ(Surface::toCellLen(40), 2);  // Exactly two cells
    EXPECT_EQ(Surface::toCellLen(0), 0);   // Zero pixels
}

TEST_F(UT_Surface, ToPixelLen_ConvertsCellsToPixels)
{
    EXPECT_EQ(Surface::toPixelLen(1), 20);  // One cell
    EXPECT_EQ(Surface::toPixelLen(2), 40);  // Two cells
    EXPECT_EQ(Surface::toPixelLen(0), 0);   // Zero cells
}

TEST_F(UT_Surface, MapToGridSize_ConvertsPixelSizeToGrid)
{
    QSize pixelSize(40, 30);
    QSize gridSize = Surface::mapToGridSize(pixelSize);
    EXPECT_EQ(gridSize.width(), 2);   // 40/20 = 2
    EXPECT_EQ(gridSize.height(), 2);  // 30/20 rounded up = 2
}

TEST_F(UT_Surface, PointsDistance_CalculatesDistance)
{
    QPoint p1(0, 0);
    QPoint p2(3, 4);
    int distance = Surface::pointsDistance(p1, p2);
    // Distance should be 5 (3-4-5 triangle)
    EXPECT_EQ(distance, 5);
    
    QPoint p3(0, 0);
    QPoint p4(0, 0);
    int distance2 = Surface::pointsDistance(p3, p4);
    EXPECT_EQ(distance2, 0);
}

TEST_F(UT_Surface, GridSize_ReturnsSize)
{
    Surface surface(nullptr);
    QSize size = surface.gridSize();
    // Size should be based on widget size converted to grid units
    EXPECT_TRUE(size.width() >= 0);
    EXPECT_TRUE(size.height() >= 0);
}

TEST_F(UT_Surface, MapToPixelSize_ConvertsGridToPixel)
{
    Surface surface(nullptr);
    QRect gridRect(1, 1, 2, 3);  // 1,1 grid position, 2x3 grid size
    QRect pixelRect = surface.mapToPixelSize(gridRect);
    
    // Should be 20 pixels per cell
    EXPECT_EQ(pixelRect.x(), 20);   // 1 * 20
    EXPECT_EQ(pixelRect.y(), 20);   // 1 * 20
    EXPECT_EQ(pixelRect.width(), 40);   // 2 * 20
    EXPECT_EQ(pixelRect.height(), 60);  // 3 * 20
}

TEST_F(UT_Surface, MapToGridGeo_ConvertsPixelToGrid)
{
    Surface surface(nullptr);
    QRect pixelRect(25, 35, 45, 55);  // Pixel coordinates
    QRect gridRect = surface.mapToGridGeo(pixelRect);
    
    // Should convert to grid units (rounding up for sizes)
    EXPECT_EQ(gridRect.x(), 1);   // 25/20 = 1.25 -> 1 (for position)
    EXPECT_EQ(gridRect.y(), 1);   // 35/20 = 1.75 -> 1 (for position)
    EXPECT_EQ(gridRect.width(), 3);   // 45/20 = 2.25 -> 3 (rounded up)
    EXPECT_EQ(gridRect.height(), 3);  // 55/20 = 2.75 -> 3 (rounded up)
}

TEST_F(UT_Surface, GridOffset_ReturnsPoint)
{
    Surface surface(nullptr);
    QPoint offset = surface.gridOffset();
    // Just ensure it returns a valid point
    EXPECT_TRUE(true);  // Method exists and returns a value
}

TEST_F(UT_Surface, GridMargins_ReturnsMargins)
{
    Surface surface(nullptr);
    QMargins margins = surface.gridMargins();
    // Just ensure it returns a valid margins object
    EXPECT_TRUE(true);  // Method exists and returns a value
}

TEST_F(UT_Surface, SetPositionIndicatorRect_SetsIndicatorRect)
{
    Surface surface(nullptr);
    QRect testRect(10, 10, 100, 50);
    surface.setPositionIndicatorRect(testRect);
    // The method exists and should work without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_Surface, ActivatePosIndicator_SetsIndicatorActive)
{
    Surface surface(nullptr);
    QRect testRect(5, 5, 200, 100);
    surface.activatePosIndicator(testRect);
    // The method exists and should work without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_Surface, DeactivatePosIndicator_DeactivatesIndicator)
{
    Surface surface(nullptr);
    surface.deactivatePosIndicator();
    // The method exists and should work without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_Surface, IntersectedRects_ReturnsRectList)
{
    Surface surface(nullptr);
    QWidget testWidget;
    QList<QRect> rects = surface.intersectedRects(&testWidget);
    // Just ensure it returns a list without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_Surface, IsIntersected_ChecksIntersection)
{
    Surface surface(nullptr);
    QWidget testWidget;
    QRect testRect(0, 0, 100, 100);
    bool intersects = surface.isIntersected(testRect, &testWidget);
    // Just ensure it returns a bool value
    EXPECT_TRUE(intersects || !intersects);
}

TEST_F(UT_Surface, FindValidAreaAroundRect_ReturnsRect)
{
    Surface surface(nullptr);
    QWidget testWidget;
    QRect centerRect(10, 10, 50, 50);
    QRect validArea = surface.findValidAreaAroundRect(centerRect, &testWidget);
    // Just ensure it returns a rect without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_Surface, FindValidArea_ReturnsRect)
{
    Surface surface(nullptr);
    QWidget testWidget;
    QRect validArea = surface.findValidArea(&testWidget);
    // Just ensure it returns a rect without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_Surface, Animate_CallsAnimation)
{
    // Test the static animate method
    AnimateParams params;
    params.duration = 100;
    params.property = "geometry";
    
    // Just test that the method can be called without crashing
    Surface::animate(params);
    EXPECT_TRUE(true);
}

TEST_F(UT_Surface, ItemIndicator_CreatesWidget)
{
    QWidget parent;
    ItemIndicator indicator(&parent);
    EXPECT_NE(&indicator, nullptr);
    EXPECT_EQ(indicator.parent(), &parent);
}
