// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/treeitempaintproxy.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QRectF>
#include <QRect>
#include <QStandardItemModel>

using namespace dfmplugin_workspace;

class TreeItemPaintProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create TreeItemPaintProxy with nullptr parent
        paintProxy = new TreeItemPaintProxy(nullptr);
        
        // Create a simple mock model for testing
        mockModel = new QStandardItemModel();
        mockModel->appendRow(new QStandardItem("Test Item"));
    }

    void TearDown() override
    {
        delete paintProxy;
        paintProxy = nullptr;
        delete mockModel;
        mockModel = nullptr;
        stub.clear();
    }

    TreeItemPaintProxy *paintProxy;
    QStandardItemModel *mockModel;
    stub_ext::StubExt stub;
};

TEST_F(TreeItemPaintProxyTest, Constructor_CreatesInstance)
{
    EXPECT_NE(paintProxy, nullptr);
    EXPECT_EQ(paintProxy->iconRectIndex(), 1);
}

TEST_F(TreeItemPaintProxyTest, ArrowRect_WithValidIconRect_ReturnsCorrectArrowRect)
{
    QRectF iconRect(100, 100, 16, 16);
    
    // Test arrowRect with valid input - this method doesn't depend on view()
    EXPECT_NO_THROW({
        QRectF result = paintProxy->arrowRect(iconRect);
        // Should not crash and return valid rect
        EXPECT_TRUE(result.isValid());
        EXPECT_GT(result.width(), 0);
        EXPECT_GT(result.height(), 0);
    });
}

TEST_F(TreeItemPaintProxyTest, View_ReturnsNullForNullParent)
{
    // Test view() method returns nullptr when parent is not a FileView
    EXPECT_NO_THROW({
        FileView *result = paintProxy->view();
        // Should return nullptr since parent is nullptr
        EXPECT_EQ(result, nullptr);
    });
}

TEST_F(TreeItemPaintProxyTest, IconRectIndex_ReturnsCorrectValue)
{
    // Test iconRectIndex returns the expected value
    EXPECT_EQ(paintProxy->iconRectIndex(), 1);
}

TEST_F(TreeItemPaintProxyTest, ArrowRect_CalculatesCorrectPosition)
{
    // Test different icon rect positions
    QRectF iconRect1(50, 50, 24, 24);
    QRectF result1 = paintProxy->arrowRect(iconRect1);
    EXPECT_TRUE(result1.isValid());
    EXPECT_LT(result1.left(), iconRect1.left()); // Arrow should be to the left of icon
    
    QRectF iconRect2(100, 100, 32, 32);
    QRectF result2 = paintProxy->arrowRect(iconRect2);
    EXPECT_TRUE(result2.isValid());
    EXPECT_LT(result2.left(), iconRect2.left()); // Arrow should be to the left of icon
}

TEST_F(TreeItemPaintProxyTest, ArrowRect_HandlesDifferentSizes)
{
    // Test with different icon sizes
    QList<QSize> iconSizes = {QSize(16, 16), QSize(24, 24), QSize(32, 32), QSize(48, 48)};
    
    for (const QSize &size : iconSizes) {
        QRectF iconRect(100, 100, size.width(), size.height());
        QRectF result = paintProxy->arrowRect(iconRect);
        
        EXPECT_TRUE(result.isValid()) << "Failed for size: " << size.width() << "x" << size.height();
        EXPECT_GT(result.width(), 0) << "Failed for size: " << size.width() << "x" << size.height();
        EXPECT_GT(result.height(), 0) << "Failed for size: " << size.width() << "x" << size.height();
    }
}

TEST_F(TreeItemPaintProxyTest, ArrowRect_VerticalCentering)
{
    // Test that arrow is vertically centered with icon
    QRectF iconRect(100, 100, 24, 24);
    QRectF result = paintProxy->arrowRect(iconRect);
    
    // Arrow should be vertically aligned with icon center
    EXPECT_NEAR(result.center().y(), iconRect.center().y(), 1.0);
}

TEST_F(TreeItemPaintProxyTest, ArrowRect_FixedSize)
{
    // Test that arrow rect has expected fixed size
    QRectF iconRect(100, 100, 24, 24);
    QRectF result = paintProxy->arrowRect(iconRect);
    
    // The arrow should have a specific size based on constants in the source
    EXPECT_GT(result.width(), 0);
    EXPECT_GT(result.height(), 0);
    EXPECT_LE(result.width(), 20); // Should be reasonable size
    EXPECT_LE(result.height(), 20); // Should be reasonable size
}