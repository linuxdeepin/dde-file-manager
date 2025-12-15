// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/itemdelegatehelper.h"

#include <QIcon>
#include <QSize>
#include <QPainter>
#include <QRectF>
#include <QBrush>
#include <QApplication>
#include <QTextOption>
#include <QString>

using namespace dfmplugin_workspace;
using namespace dfmbase;

class ItemDelegateHelperTest : public ::testing::Test
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

TEST_F(ItemDelegateHelperTest, GetIconPixmap_NullIcon_ReturnsEmptyPixmap)
{
    // Test getIconPixmap with null icon
    QIcon nullIcon;
    QSize size(64, 64);
    qreal pixelRatio = 1.0;
    QIcon::Mode mode = QIcon::Normal;
    QIcon::State state = QIcon::Off;
    
    QPixmap result = ItemDelegateHelper::getIconPixmap(nullIcon, size, pixelRatio, mode, state);
    
    EXPECT_TRUE(result.isNull());
}

TEST_F(ItemDelegateHelperTest, GetIconPixmap_InvalidSize_ReturnsEmptyPixmap)
{
    // Test getIconPixmap with invalid size
    QIcon testIcon = QIcon::fromTheme("folder");
    QSize invalidSize(0, 64);
    qreal pixelRatio = 1.0;
    QIcon::Mode mode = QIcon::Normal;
    QIcon::State state = QIcon::Off;
    
    QPixmap result = ItemDelegateHelper::getIconPixmap(testIcon, invalidSize, pixelRatio, mode, state);
    
    EXPECT_TRUE(result.isNull());
}

TEST_F(ItemDelegateHelperTest, GetIconPixmap_ValidParameters_ReturnsPixmap)
{
    // Test getIconPixmap with valid parameters
    QIcon testIcon = QIcon::fromTheme("folder");
    QSize size(64, 64);
    qreal pixelRatio = 1.0;
    QIcon::Mode mode = QIcon::Normal;
    QIcon::State state = QIcon::Off;
    
    QPixmap result = ItemDelegateHelper::getIconPixmap(testIcon, size, pixelRatio, mode, state);
    
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.devicePixelRatio(), pixelRatio);
}

TEST_F(ItemDelegateHelperTest, PaintIcon_ValidOpts_DoesNotCrash)
{
    // Test paintIcon with valid options
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    
    QIcon testIcon = QIcon::fromTheme("folder");
    QRectF rect(10, 10, 64, 64);
    
    ItemDelegateHelper::PaintIconOpts opts;
    opts.rect = rect;
    opts.alignment = Qt::AlignCenter;
    opts.mode = QIcon::Normal;
    opts.state = QIcon::Off;
    opts.viewMode = DFMBASE_NAMESPACE::Global::ViewMode::kIconMode;
    opts.isThumb = false;
    
    EXPECT_NO_THROW(ItemDelegateHelper::paintIcon(&painter, testIcon, opts));
}

TEST_F(ItemDelegateHelperTest, PaintIcon_ThumbMode_DoesNotCrash)
{
    // Test paintIcon with thumbnail mode
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    
    QIcon testIcon = QIcon::fromTheme("folder");
    QRectF rect(10, 10, 64, 64);
    
    ItemDelegateHelper::PaintIconOpts opts;
    opts.rect = rect;
    opts.alignment = Qt::AlignCenter;
    opts.mode = QIcon::Normal;
    opts.state = QIcon::Off;
    opts.viewMode = DFMBASE_NAMESPACE::Global::ViewMode::kIconMode;
    opts.isThumb = true;
    
    EXPECT_NO_THROW(ItemDelegateHelper::paintIcon(&painter, testIcon, opts));
}

TEST_F(ItemDelegateHelperTest, DrawBackground_ValidParams_DoesNotCrash)
{
    // Test drawBackground with valid parameters
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    
    qreal backgroundRadius = 5.0;
    QRectF rect(10, 10, 80, 60);
    QRectF lastLineRect;
    QBrush backgroundBrush(Qt::lightGray);
    
    EXPECT_NO_THROW(ItemDelegateHelper::drawBackground(backgroundRadius, rect, lastLineRect, backgroundBrush, &painter));
}

TEST_F(ItemDelegateHelperTest, HideTooltipImmediately_DoesNotCrash)
{
    // Test hideTooltipImmediately
    EXPECT_NO_THROW(ItemDelegateHelper::hideTooltipImmediately());
}

TEST_F(ItemDelegateHelperTest, CreateTextLayout_ValidParams_ReturnsLayout)
{
    // Test createTextLayout with valid parameters
    QString name("test_file.txt");
    QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere;
    qreal lineHeight = 16.0;
    int alignmentFlag = Qt::AlignCenter;
    
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    
    ElideTextLayout *result = ItemDelegateHelper::createTextLayout(name, wordWrap, lineHeight, alignmentFlag, &painter);
    
    EXPECT_NE(result, nullptr);
    
    delete result;
}

TEST_F(ItemDelegateHelperTest, CreateTextLayout_NullPainter_ReturnsLayout)
{
    // Test createTextLayout with null painter
    QString name("test_file.txt");
    QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere;
    qreal lineHeight = 16.0;
    int alignmentFlag = Qt::AlignCenter;
    
    ElideTextLayout *result = ItemDelegateHelper::createTextLayout(name, wordWrap, lineHeight, alignmentFlag, nullptr);
    
    EXPECT_NE(result, nullptr);
    
    delete result;
}

TEST_F(ItemDelegateHelperTest, CreateTextLayout_EmptyName_ReturnsLayout)
{
    // Test createTextLayout with empty name
    QString name("");
    QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere;
    qreal lineHeight = 16.0;
    int alignmentFlag = Qt::AlignCenter;
    
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    
    ElideTextLayout *result = ItemDelegateHelper::createTextLayout(name, wordWrap, lineHeight, alignmentFlag, &painter);
    
    EXPECT_NE(result, nullptr);
    
    delete result;
}
