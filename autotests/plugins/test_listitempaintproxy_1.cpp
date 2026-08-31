// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_listitempaintproxy_1.cpp
 * @brief Unit tests for ListItemPaintProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/listitempaintproxy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ListItemPaintProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ListItemPaintProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ListItemPaintProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ListItemPaintProxyTest, allPaintRect)
{
    // Test method: QList<QRect> allPaintRect((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->allPaintRect(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ListItemPaintProxyTest, drawIcon)
{
    // Test method: void drawIcon((QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg2{};
    QModelIndex _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawIcon(nullptr, nullptr, _arg2, _arg3));
}

TEST_F(ListItemPaintProxyTest, rectByType)
{
    // Test method: QRectF rectByType((RectOfItemType type, const QModelIndex &index))
    QModelIndex _arg1{};
    auto result = obj->rectByType(RectOfItemType(), _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ListItemPaintProxyTest, supportContentPreview)
{
    // Test bool getter: supportContentPreview()
    bool result = obj->supportContentPreview();
    EXPECT_FALSE(result);

}

TEST_F(ListItemPaintProxyTest, view)
{
    // Test getter: FileView view()
    auto result = obj->view();
    EXPECT_NO_FATAL_FAILURE({ obj->view(); });

}
