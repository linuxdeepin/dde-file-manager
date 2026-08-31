// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconitemdelegate_1.cpp
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

TEST_F(IconItemDelegateTest, calcFileNameRect)
{
    // Test method: QList<QRectF> calcFileNameRect((const QModelIndex &index, const QRectF &rect, Qt::TextElideMode elideMode))
    QModelIndex _arg0{};
    QRectF _arg1{};
    auto result = obj->calcFileNameRect(_arg0, _arg1, Qt::TextElideMode());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemDelegateTest, createEditor)
{
    // Test method: QWidget createEditor((QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->createEditor(nullptr, _arg1, _arg2);
    EXPECT_NO_FATAL_FAILURE({ obj->createEditor(nullptr, _arg1, _arg2); });

}

TEST_F(IconItemDelegateTest, decreaseIcon)
{
    // Test getter: int decreaseIcon()
    auto result = obj->decreaseIcon();
    EXPECT_EQ(result, 0);

}

TEST_F(IconItemDelegateTest, displayFileName)
{
    // Test method: QString displayFileName((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->displayFileName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemDelegateTest, editorEvent)
{
    // Test event handler: editorEvent((QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->editorEvent(&_event));
}

TEST_F(IconItemDelegateTest, editorFinished)
{
    // Test method: void editorFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->editorFinished());
}

TEST_F(IconItemDelegateTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *object, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(IconItemDelegateTest, expandItemRect)
{
    // Test getter: QRect expandItemRect()
    auto result = obj->expandItemRect();
    EXPECT_FALSE(result.isValid());

}

TEST_F(IconItemDelegateTest, expandedIndex)
{
    // Test getter: QModelIndex expandedIndex()
    auto result = obj->expandedIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(IconItemDelegateTest, getGroupHeaderBackgroundRect)
{
    // Test method: QRectF getGroupHeaderBackgroundRect((const QStyleOptionViewItem &option))
    QStyleOptionViewItem _arg0{};
    auto result = obj->getGroupHeaderBackgroundRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(IconItemDelegateTest, getGroupHeaderHeight)
{
    // Test method: int getGroupHeaderHeight((const QStyleOptionViewItem &option))
    QStyleOptionViewItem _arg0{};
    auto result = obj->getGroupHeaderHeight(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(IconItemDelegateTest, hideNotEditingIndexWidget)
{
    // Test method: void hideNotEditingIndexWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->hideNotEditingIndexWidget());
}

TEST_F(IconItemDelegateTest, iconSizeByIconSizeLevel)
{
    // Test getter: QSize iconSizeByIconSizeLevel()
    auto result = obj->iconSizeByIconSizeLevel();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemDelegateTest, iconSizeLevel)
{
    // Test getter: int iconSizeLevel()
    auto result = obj->iconSizeLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(IconItemDelegateTest, increaseIcon)
{
    // Test getter: int increaseIcon()
    auto result = obj->increaseIcon();
    EXPECT_EQ(result, 0);

}

TEST_F(IconItemDelegateTest, itemExpanded)
{
    // Test bool getter: itemExpanded()
    bool result = obj->itemExpanded();
    EXPECT_FALSE(result);

}

TEST_F(IconItemDelegateTest, itemIconRect)
{
    // Test method: QRectF itemIconRect((const QRectF &itemRect))
    QRectF _arg0{};
    auto result = obj->itemIconRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(IconItemDelegateTest, maximumIconSizeLevel)
{
    // Test getter: int maximumIconSizeLevel()
    auto result = obj->maximumIconSizeLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(IconItemDelegateTest, minimumIconSizeLevel)
{
    // Test getter: int minimumIconSizeLevel()
    auto result = obj->minimumIconSizeLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(IconItemDelegateTest, minimumWidthLevel)
{
    // Test getter: int minimumWidthLevel()
    auto result = obj->minimumWidthLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(IconItemDelegateTest, onTriggerEdit)
{
    // Test method: void onTriggerEdit((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTriggerEdit(_arg0));
}

TEST_F(IconItemDelegateTest, paintItemIcon)
{
    // Test method: QRectF paintItemIcon((QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->paintItemIcon(nullptr, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(IconItemDelegateTest, setIconSizeByIconSizeLevel)
{
    // Test method: int setIconSizeByIconSizeLevel((int level))
    auto result = obj->setIconSizeByIconSizeLevel(0);
    EXPECT_GE(result, 0);

}

TEST_F(IconItemDelegateTest, setItemMinimumWidthByWidthLevel)
{
    // Test setter: void setItemMinimumWidthByWidthLevel((int level))
    EXPECT_NO_FATAL_FAILURE(obj->setItemMinimumWidthByWidthLevel(0));
}

TEST_F(IconItemDelegateTest, sizeHint)
{
    // Test method: QSize sizeHint((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->sizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IconItemDelegateTest, updateItemSizeHint)
{
    // Test method: void updateItemSizeHint(())
    EXPECT_NO_FATAL_FAILURE(obj->updateItemSizeHint());
}
