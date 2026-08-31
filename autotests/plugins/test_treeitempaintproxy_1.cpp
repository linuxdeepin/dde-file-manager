// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_treeitempaintproxy_1.cpp
 * @brief Unit tests for TreeItemPaintProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/treeitempaintproxy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class TreeItemPaintProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TreeItemPaintProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TreeItemPaintProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TreeItemPaintProxyTest, TreeItemPaintProxy)
{
    // Test constructor: TreeItemPaintProxy((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TreeItemPaintProxyTest, allPaintRect)
{
    // Test method: QList<QRect> allPaintRect((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->allPaintRect(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TreeItemPaintProxyTest, drawExpandArrow)
{
    // Test method: void drawExpandArrow((QPainter *painter, const QRectF &rect, const QStyleOptionViewItem &option, const QModelIndex &index))
    QRectF _arg1{};
    QStyleOptionViewItem _arg2{};
    QModelIndex _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawExpandArrow(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(TreeItemPaintProxyTest, drawIcon)
{
    // Test method: void drawIcon((QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg2{};
    QModelIndex _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawIcon(nullptr, nullptr, _arg2, _arg3));
}

TEST_F(TreeItemPaintProxyTest, iconRectIndex)
{
    // Test getter: int iconRectIndex()
    auto result = obj->iconRectIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(TreeItemPaintProxyTest, rectByType)
{
    // Test method: QRectF rectByType((RectOfItemType type, const QModelIndex &index))
    QModelIndex _arg1{};
    auto result = obj->rectByType(RectOfItemType(), _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TreeItemPaintProxyTest, view)
{
    // Test getter: FileView view()
    auto result = obj->view();
    EXPECT_NO_FATAL_FAILURE({ obj->view(); });

}
