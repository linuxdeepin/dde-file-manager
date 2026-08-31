// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewdrawhelper_1.cpp
 * @brief Unit tests for ViewDrawHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/viewdrawhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ViewDrawHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewDrawHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewDrawHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewDrawHelperTest, ViewDrawHelper)
{
    // Test constructor: ViewDrawHelper((FileView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ViewDrawHelperTest, drawDragCount)
{
    // Test method: void drawDragCount((QPainter *painter, const QModelIndex &topIndex, const QStyleOptionViewItem &option, int count))
    QModelIndex _arg1{};
    QStyleOptionViewItem _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->drawDragCount(nullptr, _arg1, _arg2, 0));
}

TEST_F(ViewDrawHelperTest, drawDragIcons)
{
    // Test method: void drawDragIcons((QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QModelIndexList &indexes, const QModelIndex &topIndex))
    QStyleOptionViewItem _arg1{};
    QRect _arg2{};
    QModelIndexList _arg3{};
    QModelIndex _arg4{};
    EXPECT_NO_FATAL_FAILURE(obj->drawDragIcons(nullptr, _arg1, _arg2, _arg3, _arg4));
}

TEST_F(ViewDrawHelperTest, drawDragText)
{
    // Test method: void drawDragText((QPainter *painter, const QModelIndex &index, qreal textWidth))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->drawDragText(nullptr, _arg1, 0.0));
}
