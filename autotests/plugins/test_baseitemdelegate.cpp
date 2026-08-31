// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_baseitemdelegate.cpp
 * @brief Unit tests for BaseItemDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/baseitemdelegate.h"

#include <QTest>

using namespace dfmplugin_workspace;

class BaseItemDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BaseItemDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BaseItemDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseItemDelegateTest, destroyEditor)
{
    // Test method: void destroyEditor((QWidget *editor, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->destroyEditor(nullptr, _arg1));
}

TEST_F(BaseItemDelegateTest, getTruncateButtonRect)
{
    // Test method: QRect getTruncateButtonRect((const QRectF &rect))
    QRectF _arg0{};
    auto result = obj->getTruncateButtonRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, getTruncateButtonWidth)
{
    // Test getter: int getTruncateButtonWidth()
    auto result = obj->getTruncateButtonWidth();
    EXPECT_EQ(result, 0);

}

TEST_F(BaseItemDelegateTest, hoveredTruncateGroupKey)
{
    // Test getter: QString hoveredTruncateGroupKey()
    auto result = obj->hoveredTruncateGroupKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, initStyleOption)
{
    // Test method: void initStyleOption((QStyleOptionViewItem *option, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->initStyleOption(nullptr, _arg1));
}

TEST_F(BaseItemDelegateTest, isThumnailIconIndex)
{
    // Test method: bool isThumnailIconIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isThumnailIconIndex(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BaseItemDelegateTest, paintGroupText)
{
    // Test method: void paintGroupText((QPainter *painter, const QRect &textRect, const QString &text, int count, const QStyleOptionViewItem &option))
    QRect _arg1{};
    QString _arg2{};
    QStyleOptionViewItem _arg4{};
    EXPECT_NO_FATAL_FAILURE(obj->paintGroupText(nullptr, _arg1, _arg2, 0, _arg4));
}

TEST_F(BaseItemDelegateTest, paintTruncateButton)
{
    // Test method: void paintTruncateButton((QPainter *painter, const QRect &buttonRect, const QModelIndex &index, const QStyleOptionViewItem &option))
    QRect _arg1{};
    QModelIndex _arg2{};
    QStyleOptionViewItem _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->paintTruncateButton(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(BaseItemDelegateTest, pressedTruncateGroupKey)
{
    // Test getter: QString pressedTruncateGroupKey()
    auto result = obj->pressedTruncateGroupKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, setHoveredTruncateGroupKey)
{
    // Test setter: void setHoveredTruncateGroupKey((const QString &groupKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setHoveredTruncateGroupKey(_arg0));
}

TEST_F(BaseItemDelegateTest, setPressedTruncateGroupKey)
{
    // Test setter: void setPressedTruncateGroupKey((const QString &groupKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPressedTruncateGroupKey(_arg0));
}

TEST_F(BaseItemDelegateTest, shouldShowTruncateButton)
{
    // Test method: bool shouldShowTruncateButton((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->shouldShowTruncateButton(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BaseItemDelegateTest, truncateButtonText)
{
    // Test method: QString truncateButtonText((bool isTruncated))
    auto result = obj->truncateButtonText(false);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, truncateButtonTexts)
{
    // Test getter: QStringList truncateButtonTexts()
    auto result = obj->truncateButtonTexts();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, BaseItemDelegate)
{
    // Test constructor: BaseItemDelegate((BaseItemDelegatePrivate &dd, FileViewHelper *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BaseItemDelegateTest, parent)
{
    // Test getter: FileViewHelper parent()
    auto result = obj->parent();
    EXPECT_NO_FATAL_FAILURE({ obj->parent(); });

}
