// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/private/delegatecommon.h"

#include <QRectF>
#include <QPainterPath>
#include <QPointF>

using namespace GlobalPrivate;

class DelegateCommonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(DelegateCommonTest, BoundingRect_EmptyList_ReturnsEmptyRect)
{
    // Test boundingRect with empty list
    QList<QRectF> emptyList;
    QRectF result = boundingRect(emptyList);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(DelegateCommonTest, BoundingRect_SingleRect_ReturnsSameRect)
{
    // Test boundingRect with single rectangle
    QList<QRectF> rects;
    QRectF testRect(10.0, 20.0, 100.0, 50.0);
    rects.append(testRect);
    
    QRectF result = boundingRect(rects);
    
    EXPECT_EQ(result, testRect);
}

TEST_F(DelegateCommonTest, BoundingRect_MultipleRects_ReturnsCorrectBoundingRect)
{
    // Test boundingRect with multiple rectangles
    QList<QRectF> rects;
    rects.append(QRectF(10.0, 20.0, 100.0, 50.0));
    rects.append(QRectF(5.0, 10.0, 80.0, 60.0));
    rects.append(QRectF(15.0, 30.0, 120.0, 40.0));
    
    QRectF result = boundingRect(rects);
    
    // Expected bounding rect should encompass all rectangles
    EXPECT_EQ(result.left(), 5.0);   // Minimum left
    EXPECT_EQ(result.top(), 10.0);   // Minimum top
    EXPECT_EQ(result.right(), 135.0); // Maximum right (15.0 + 120.0)
    EXPECT_EQ(result.bottom(), 70.0); // Maximum bottom (30.0 + 40.0)
}

TEST_F(DelegateCommonTest, BoundingPath_SingleRect_ReturnsRoundedRect)
{
    // Test boundingPath with single rectangle
    QList<QRectF> rects;
    QRectF testRect(10.0, 20.0, 100.0, 50.0);
    rects.append(testRect);
    
    qreal radius = 5.0;
    qreal padding = 2.0;
    QPainterPath result = boundingPath(rects, radius, padding);
    
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains(testRect.marginsAdded(QMarginsF(radius + padding, padding, radius + padding, padding))));
}

TEST_F(DelegateCommonTest, BoundingPath_MultipleRects_ReturnsCorrectPath)
{
    // Test boundingPath with multiple rectangles
    QList<QRectF> rects;
    rects.append(QRectF(10.0, 20.0, 100.0, 50.0));
    rects.append(QRectF(110.0, 20.0, 100.0, 50.0));
    
    qreal radius = 5.0;
    qreal padding = 2.0;
    QPainterPath result = boundingPath(rects, radius, padding);
    
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(DelegateCommonTest, JoinLeftCorner_WithValidPrevRect_DoesNotCrash)
{
    // Test joinLeftCorner with valid previous rectangle
    QRectF rect(10.0, 20.0, 100.0, 50.0);
    QRectF prevRect(5.0, 20.0, 100.0, 50.0);
    QRectF nextRect(15.0, 20.0, 100.0, 50.0);
    
    qreal radius = 5.0;
    qreal padding = 2.0;
    QPainterPath path;
    
    EXPECT_NO_THROW(joinLeftCorner(rect, prevRect, nextRect, radius, padding, &path));
}

TEST_F(DelegateCommonTest, JoinLeftCorner_WithInvalidPrevRect_DoesNotCrash)
{
    // Test joinLeftCorner with invalid previous rectangle
    QRectF rect(10.0, 20.0, 100.0, 50.0);
    QRectF invalidPrevRect;
    QRectF nextRect(15.0, 20.0, 100.0, 50.0);
    
    qreal radius = 5.0;
    qreal padding = 2.0;
    QPainterPath path;
    
    EXPECT_NO_THROW(joinLeftCorner(rect, invalidPrevRect, nextRect, radius, padding, &path));
}

TEST_F(DelegateCommonTest, JoinRightCorner_WithValidPrevRect_DoesNotCrash)
{
    // Test joinRightCorner with valid previous rectangle
    QRectF rect(10.0, 20.0, 100.0, 50.0);
    QRectF prevRect(5.0, 20.0, 100.0, 50.0);
    QRectF nextRect(15.0, 20.0, 100.0, 50.0);
    
    qreal radius = 5.0;
    qreal padding = 2.0;
    QPainterPath path;
    
    EXPECT_NO_THROW(joinRightCorner(rect, prevRect, nextRect, radius, padding, &path));
}

TEST_F(DelegateCommonTest, JoinRightCorner_WithInvalidPrevRect_DoesNotCrash)
{
    // Test joinRightCorner with invalid previous rectangle
    QRectF rect(10.0, 20.0, 100.0, 50.0);
    QRectF invalidPrevRect;
    QRectF nextRect(15.0, 20.0, 100.0, 50.0);
    
    qreal radius = 5.0;
    qreal padding = 2.0;
    QPainterPath path;
    
    EXPECT_NO_THROW(joinRightCorner(rect, invalidPrevRect, nextRect, radius, padding, &path));
}