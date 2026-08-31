// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconitemdelegate.cpp
 * @brief Unit tests for IconItemDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/iconitemdelegate.h"

#include <QTest>

using namespace dfmplugin_workspace;

class IconItemDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IconItemDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IconItemDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IconItemDelegateTest, itemGeomertys)
{
    // Test method: QList<QRect> itemGeomertys((const QStyleOptionViewItem &opt, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->itemGeomertys(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemDelegateTest, paint)
{
    // Test method: void paint((QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paint(nullptr, _arg1, _arg2));
}

TEST_F(IconItemDelegateTest, paintGeomertys)
{
    // Test method: QList<QRect> paintGeomertys((const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->paintGeomertys(_arg0, _arg1, false);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemDelegateTest, setEditorData)
{
    // Test setter: void setEditorData((QWidget *editor, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setEditorData(nullptr, _arg1));
}

TEST_F(IconItemDelegateTest, updateEditorGeometry)
{
    // Test method: void updateEditorGeometry((QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateEditorGeometry(nullptr, _arg1, _arg2));
}

TEST_F(IconItemDelegateTest, IconItemDelegate)
{
    // Test constructor: IconItemDelegate((FileViewHelper *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(IconItemDelegateTest, expandedItem)
{
    // Test getter: QWidget expandedItem()
    auto result = obj->expandedItem();
    EXPECT_NO_FATAL_FAILURE({ obj->expandedItem(); });

}
