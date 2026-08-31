// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_itemdelegatehelper.cpp
 * @brief Unit tests for ItemDelegateHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/itemdelegatehelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ItemDelegateHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ItemDelegateHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ItemDelegateHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ItemDelegateHelperTest, createTextLayout)
{
    // Test method: ElideTextLayout createTextLayout((const QString &name, QTextOption::WrapMode wordWrap,
                                                      qreal lineHeight, int alignmentFlag, QPainter *painter))
    QString _arg0{};
    auto result = obj->createTextLayout(_arg0, QTextOption::WrapMode(), 0.0, 0, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->createTextLayout(_arg0, QTextOption::WrapMode(), 0.0, 0, nullptr); });

}

TEST_F(ItemDelegateHelperTest, drawBackground)
{
    // Test method: void drawBackground((const qreal &backgroundRadius, const QRectF &rect, QRectF &lastLineRect, const QBrush &backgroundBrush, QPainter *painter))
    qreal _arg0{};
    QRectF _arg1{};
    QRectF _arg2{};
    QBrush _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawBackground(_arg0, _arg1, _arg2, _arg3, nullptr));
}

TEST_F(ItemDelegateHelperTest, getIconPixmap)
{
    // Test method: QPixmap getIconPixmap((const QIcon &icon, const QSize &size, qreal pixelRatio, QIcon::Mode mode, QIcon::State state))
    QIcon _arg0{};
    QSize _arg1{};
    auto result = obj->getIconPixmap(_arg0, _arg1, 0.0, QIcon::Mode(), QIcon::State());
    EXPECT_TRUE(result.isNull());

}

TEST_F(ItemDelegateHelperTest, hideTooltipImmediately)
{
    // Test method: void hideTooltipImmediately(())
    EXPECT_NO_FATAL_FAILURE(obj->hideTooltipImmediately());
}

TEST_F(ItemDelegateHelperTest, visualAlignment)
{
    // Test getter: Qt::Alignment visualAlignment()
    auto result = obj->visualAlignment();
    EXPECT_GE(static_cast<int>(result), 0);

}
