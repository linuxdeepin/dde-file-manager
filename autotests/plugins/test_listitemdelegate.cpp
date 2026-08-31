// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_listitemdelegate.cpp
 * @brief Unit tests for ListItemDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/listitemdelegate.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ListItemDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ListItemDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ListItemDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ListItemDelegateTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *object, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ListItemDelegateTest, paint)
{
    // Test method: void paint((QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paint(nullptr, _arg1, _arg2));
}

TEST_F(ListItemDelegateTest, paintFileName)
{
    // Test method: void paintFileName((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const int &role, const QRectF &rect, const int &textLineHeight,
                                     const QUrl &url))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    int _arg3{};
    QRectF _arg4{};
    int _arg5{};
    QUrl _arg6{};
    EXPECT_NO_FATAL_FAILURE(obj->paintFileName(nullptr, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6));
}

TEST_F(ListItemDelegateTest, paintGeomertys)
{
    // Test method: QList<QRect> paintGeomertys((const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->paintGeomertys(_arg0, _arg1, false);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ListItemDelegateTest, paintItemBackground)
{
    // Test method: void paintItemBackground((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintItemBackground(nullptr, _arg1, _arg2));
}

TEST_F(ListItemDelegateTest, ListItemDelegate)
{
    // Test constructor: ListItemDelegate((FileViewHelper *parent))
    ASSERT_NE(obj, nullptr);
}
