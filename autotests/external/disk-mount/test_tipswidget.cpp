// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_tipswidget.cpp - TipsWidget 单元测试

#include <gtest/gtest.h>

#include <QApplication>
#include <QtTest>

#include "stubext.h"

#include "tipswidget.h"

class TipsWidgetTest : public testing::Test
{
protected:
    void SetUp() override { stub.clear(); }
    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

// setText：文本记录并按字宽调整尺寸
TEST_F(TipsWidgetTest, SetText_ResizesToTextMetrics)
{
    // Arrange
    TipsWidget tips;

    // Act
    tips.setText("Disk");

    // Assert
    const QFontMetrics fm = tips.fontMetrics();
    EXPECT_EQ(tips.width(), fm.horizontalAdvance("Disk") + 20);
    EXPECT_EQ(tips.height(), fm.height());

    // Act：更长的文本 → 更宽
    tips.setText("A much longer disk label");
    EXPECT_GT(tips.width(), fm.horizontalAdvance("Disk") + 20);
}

// refreshFont：按当前文本重算尺寸
TEST_F(TipsWidgetTest, RefreshFont_RecalculatesSizeForCurrentText)
{
    // Arrange
    TipsWidget tips;
    tips.setText("Hello");
    const int widthBefore = tips.width();

    // Act
    tips.refreshFont();

    // Assert：文本未变，尺寸保持
    EXPECT_EQ(tips.width(), widthBefore);
    EXPECT_EQ(tips.height(), tips.fontMetrics().height());
}

// paintEvent：离屏绘制不崩溃且刷新字体
TEST_F(TipsWidgetTest, PaintEvent_OffscreenRender_Succeeds)
{
    // Arrange
    TipsWidget tips;
    tips.setText("Paint");

    // Act / Assert
    EXPECT_FALSE(tips.grab().isNull());
    EXPECT_EQ(tips.height(), tips.fontMetrics().height());   // paintEvent 内 refreshFont 生效
}
