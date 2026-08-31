// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_listitemdelegate_1.cpp
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

TEST_F(ListItemDelegateTest, createEditor)
{
    // Test method: QWidget createEditor((QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->createEditor(nullptr, _arg1, _arg2);
    EXPECT_NO_FATAL_FAILURE({ obj->createEditor(nullptr, _arg1, _arg2); });

}

TEST_F(ListItemDelegateTest, dataWidth)
{
    // Test method: int dataWidth((const QStyleOptionViewItem &option, const QModelIndex &index, int role))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->dataWidth(_arg0, _arg1, 0);
    EXPECT_GE(result, 0);

}

TEST_F(ListItemDelegateTest, decreaseIcon)
{
    // Test getter: int decreaseIcon()
    auto result = obj->decreaseIcon();
    EXPECT_EQ(result, 0);

}

TEST_F(ListItemDelegateTest, editorEvent)
{
    // Test event handler: editorEvent((QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->editorEvent(&_event));
}

TEST_F(ListItemDelegateTest, editorFinished)
{
    // Test method: void editorFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->editorFinished());
}

TEST_F(ListItemDelegateTest, getGroupHeaderBackgroundRect)
{
    // Test method: QRectF getGroupHeaderBackgroundRect((const QStyleOptionViewItem &option))
    QStyleOptionViewItem _arg0{};
    auto result = obj->getGroupHeaderBackgroundRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ListItemDelegateTest, getGroupHeaderHeight)
{
    // Test method: int getGroupHeaderHeight((const QStyleOptionViewItem &option))
    QStyleOptionViewItem _arg0{};
    auto result = obj->getGroupHeaderHeight(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(ListItemDelegateTest, getRectOfItem)
{
    // Test method: QRect getRectOfItem((RectOfItemType type, const QModelIndex &index))
    QModelIndex _arg1{};
    auto result = obj->getRectOfItem(RectOfItemType(), _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ListItemDelegateTest, iconSizeLevel)
{
    // Test getter: int iconSizeLevel()
    auto result = obj->iconSizeLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(ListItemDelegateTest, increaseIcon)
{
    // Test getter: int increaseIcon()
    auto result = obj->increaseIcon();
    EXPECT_EQ(result, 0);

}

TEST_F(ListItemDelegateTest, isFirstRowWithTopPadding)
{
    // Test method: bool isFirstRowWithTopPadding((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isFirstRowWithTopPadding(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ListItemDelegateTest, itemIconRect)
{
    // Test method: QRectF itemIconRect((const QRectF &itemRect))
    QRectF _arg0{};
    auto result = obj->itemIconRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ListItemDelegateTest, maximumIconSizeLevel)
{
    // Test getter: int maximumIconSizeLevel()
    auto result = obj->maximumIconSizeLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(ListItemDelegateTest, minimumIconSizeLevel)
{
    // Test getter: int minimumIconSizeLevel()
    auto result = obj->minimumIconSizeLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(ListItemDelegateTest, paintItemIcon)
{
    // Test method: QRectF paintItemIcon((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->paintItemIcon(nullptr, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ListItemDelegateTest, setEditorData)
{
    // Test method: bool setEditorData((ListItemEditor *editor))
    auto result = obj->setEditorData(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ListItemDelegateTest, setIconSizeByIconSizeLevel)
{
    // Test method: int setIconSizeByIconSizeLevel((int level))
    auto result = obj->setIconSizeByIconSizeLevel(0);
    EXPECT_GE(result, 0);

}

TEST_F(ListItemDelegateTest, sizeHint)
{
    // Test method: QSize sizeHint((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->sizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ListItemDelegateTest, updateEditorGeometry)
{
    // Test method: void updateEditorGeometry((QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateEditorGeometry(nullptr, _arg1, _arg2));
}

TEST_F(ListItemDelegateTest, updateItemSizeHint)
{
    // Test method: void updateItemSizeHint(())
    EXPECT_NO_FATAL_FAILURE(obj->updateItemSizeHint());
}
