// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_expandeditem_1.cpp
 * @brief Unit tests for ExpandedItem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/expandedItem.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ExpandedItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExpandedItem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExpandedItem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExpandedItemTest, event)
{
    // Test method: bool event((QEvent *ee))
    auto result = obj->event(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ExpandedItemTest, getDifferenceOfLastRow)
{
    // Test getter: int getDifferenceOfLastRow()
    auto result = obj->getDifferenceOfLastRow();
    EXPECT_EQ(result, 0);

}

TEST_F(ExpandedItemTest, getIconHeight)
{
    // Test getter: int getIconHeight()
    auto result = obj->getIconHeight();
    EXPECT_EQ(result, 0);

}

TEST_F(ExpandedItemTest, getIndex)
{
    // Test getter: QModelIndex getIndex()
    auto result = obj->getIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(ExpandedItemTest, getOpacity)
{
    // Test getter: qreal getOpacity()
    auto result = obj->getOpacity();
    EXPECT_EQ(result, 0.0);

}

TEST_F(ExpandedItemTest, getOption)
{
    // Test getter: QStyleOptionViewItem getOption()
    auto result = obj->getOption();
    EXPECT_NO_FATAL_FAILURE({ obj->getOption(); });

}

TEST_F(ExpandedItemTest, getTextBounding)
{
    // Test getter: QRectF getTextBounding()
    auto result = obj->getTextBounding();
    EXPECT_FALSE(result.isValid());

}

TEST_F(ExpandedItemTest, heightForWidth)
{
    // Test method: int heightForWidth((int width))
    auto result = obj->heightForWidth(0);
    EXPECT_GE(result, 0);

}

TEST_F(ExpandedItemTest, iconGeometry)
{
    // Test getter: QRectF iconGeometry()
    auto result = obj->iconGeometry();
    EXPECT_FALSE(result.isValid());

}

TEST_F(ExpandedItemTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(ExpandedItemTest, setDifferenceOfLastRow)
{
    // Test setter: void setDifferenceOfLastRow((const int diff))
    EXPECT_NO_FATAL_FAILURE(obj->setDifferenceOfLastRow(0));
}

TEST_F(ExpandedItemTest, setIconHeight)
{
    // Test setter: void setIconHeight((int iconHeight))
    EXPECT_NO_FATAL_FAILURE(obj->setIconHeight(0));
}

TEST_F(ExpandedItemTest, setIconPixmap)
{
    // Test setter: void setIconPixmap((const QPixmap &pixmap, int height))
    QPixmap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setIconPixmap(_arg0, 0));
}

TEST_F(ExpandedItemTest, setIndex)
{
    // Test setter: void setIndex((QModelIndex index))
    EXPECT_NO_FATAL_FAILURE(obj->setIndex(QModelIndex()));
}

TEST_F(ExpandedItemTest, setOption)
{
    // Test setter: void setOption((QStyleOptionViewItem opt))
    EXPECT_NO_FATAL_FAILURE(obj->setOption(QStyleOptionViewItem()));
}

TEST_F(ExpandedItemTest, setTextBounding)
{
    // Test setter: void setTextBounding((QRectF textBounding))
    EXPECT_NO_FATAL_FAILURE(obj->setTextBounding(QRectF()));
}

TEST_F(ExpandedItemTest, sizeHint)
{
    // Test getter: QSize sizeHint()
    auto result = obj->sizeHint();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ExpandedItemTest, textGeometry)
{
    // Test method: QRectF textGeometry((int width))
    auto result = obj->textGeometry(0);
    EXPECT_FALSE(result.isValid());

}
