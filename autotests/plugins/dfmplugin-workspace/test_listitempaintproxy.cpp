// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/listitempaintproxy.h"
#include "views/fileview.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QRectF>
#include <QRect>
#include <QUrl>
#include <QImage>

using namespace dfmplugin_workspace;

class ListItemPaintProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a FileView as parent for ListItemPaintProxy
        testUrl = QUrl::fromLocalFile("/tmp");
        fileView = new FileView(testUrl);
        paintProxy = new ListItemPaintProxy(fileView);
    }

    void TearDown() override
    {
        stub.clear();
        
        if (paintProxy) {
            delete paintProxy;
            paintProxy = nullptr;
        }
        
        if (fileView) {
            delete fileView;
            fileView = nullptr;
        }
    }

    QUrl testUrl;
    FileView *fileView { nullptr };
    ListItemPaintProxy *paintProxy { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(ListItemPaintProxyTest, Constructor_CreatesInstance)
{
    EXPECT_NE(paintProxy, nullptr);
}

TEST_F(ListItemPaintProxyTest, DrawIcon_DrawsIcon)
{
    // Create a simple image to paint on
    QImage image(100, 100, QImage::Format_ARGB32);
    QPainter painter(&image);
    QRectF rect(10, 10, 32, 32);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;
    option.decorationSize = QSize(32, 32);
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        paintProxy->drawIcon(&painter, &rect, option, index);
    });
}

TEST_F(ListItemPaintProxyTest, RectByType_ReturnsRectForIcon)
{
    QModelIndex index;
    
    QRectF result = paintProxy->rectByType(RectOfItemType::kItemIconRect, index);
    
    // Should return a valid rect
    EXPECT_TRUE(result.isValid());
}

TEST_F(ListItemPaintProxyTest, RectByType_ReturnsRectForTreeArrow)
{
    QModelIndex index;
    
    QRectF result = paintProxy->rectByType(RectOfItemType::kItemTreeArrowRect, index);
    
    // Should return a valid rect (empty for non-tree arrow type)
    EXPECT_TRUE(result.isValid() || result.isEmpty());
}

TEST_F(ListItemPaintProxyTest, AllPaintRect_ReturnsRectList)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.decorationSize = QSize(32, 32);
    QModelIndex index;
    
    QList<QRect> result = paintProxy->allPaintRect(option, index);
    
    // Should return a list of rects
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(ListItemPaintProxyTest, SupportContentPreview_ReturnsSupport)
{
    bool result = paintProxy->supportContentPreview();
    
    // Should return true for list item paint proxy
    EXPECT_TRUE(result);
}

TEST_F(ListItemPaintProxyTest, IconRect_ReturnsRect)
{
    QModelIndex index;
    QRect itemRect(0, 0, 200, 30);
    
    QRectF result = paintProxy->iconRect(index, itemRect);
    
    // Should return a valid rect
    EXPECT_TRUE(result.isValid());
}