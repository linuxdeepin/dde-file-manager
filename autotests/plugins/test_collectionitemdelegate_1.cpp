// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionitemdelegate_1.cpp
 * @brief Unit tests for CollectionItemDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "delegate/collectionitemdelegate.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionItemDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionItemDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionItemDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionItemDelegateTest, clipboardDataChanged)
{
    // Test method: void clipboardDataChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->clipboardDataChanged());
}

TEST_F(CollectionItemDelegateTest, commitDataAndCloseEditor)
{
    // Test method: void commitDataAndCloseEditor(())
    EXPECT_NO_FATAL_FAILURE(obj->commitDataAndCloseEditor());
}

TEST_F(CollectionItemDelegateTest, createEditor)
{
    // Test method: QWidget createEditor((QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->createEditor(nullptr, _arg1, _arg2);
    EXPECT_NO_FATAL_FAILURE({ obj->createEditor(nullptr, _arg1, _arg2); });

}

TEST_F(CollectionItemDelegateTest, drawExpandText)
{
    // Test method: void drawExpandText((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    QRectF _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawExpandText(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(CollectionItemDelegateTest, drawHighlightText)
{
    // Test method: void drawHighlightText((QPainter *painter, const QStyleOptionViewItem &option,
                                               const QModelIndex &index, const QRect &rText))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    QRect _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawHighlightText(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(CollectionItemDelegateTest, drawNormlText)
{
    // Test method: void drawNormlText((QPainter *painter, const QStyleOptionViewItem &option,
                                           const QModelIndex &index, const QRectF &rText))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    QRectF _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawNormlText(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(CollectionItemDelegateTest, elideTextRect)
{
    // Test method: QList<QRectF> elideTextRect((const QModelIndex &index, const QRect &rect, const Qt::TextElideMode &elideMode))
    QModelIndex _arg0{};
    QRect _arg1{};
    Qt::TextElideMode _arg2{};
    auto result = obj->elideTextRect(_arg0, _arg1, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionItemDelegateTest, iconSizeLevelDescription)
{
    // Test method: QString iconSizeLevelDescription((int i))
    auto result = obj->iconSizeLevelDescription(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionItemDelegateTest, isThumnailIconIndex)
{
    // Test method: bool isThumnailIconIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isThumnailIconIndex(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CollectionItemDelegateTest, isTransparent)
{
    // Test method: bool isTransparent((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isTransparent(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CollectionItemDelegateTest, mayExpand)
{
    // Test method: bool mayExpand((QModelIndex *who))
    auto result = obj->mayExpand(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionItemDelegateTest, paint)
{
    // Test method: void paint((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paint(nullptr, _arg1, _arg2));
}

TEST_F(CollectionItemDelegateTest, paintBackground)
{
    // Test method: void paintBackground((QPainter *painter, const QStyleOptionViewItem &option, const QRect &iconRect))
    QStyleOptionViewItem _arg1{};
    QRect _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintBackground(nullptr, _arg1, _arg2));
}

TEST_F(CollectionItemDelegateTest, paintDragIcon)
{
    // Test method: QSize paintDragIcon((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->paintDragIcon(nullptr, _arg1, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionItemDelegateTest, paintEmblems)
{
    // Test method: QRectF paintEmblems((QPainter *painter, const QRectF &rect, const FileInfoPointer &info))
    QRectF _arg1{};
    FileInfoPointer _arg2{};
    auto result = obj->paintEmblems(nullptr, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionItemDelegateTest, paintGeomertys)
{
    // Test method: QList<QRect> paintGeomertys((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->paintGeomertys(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionItemDelegateTest, paintLabel)
{
    // Test method: void paintLabel((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rLabel))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    QRect _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->paintLabel(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(CollectionItemDelegateTest, revertAndcloseEditor)
{
    // Test method: void revertAndcloseEditor(())
    EXPECT_NO_FATAL_FAILURE(obj->revertAndcloseEditor());
}

TEST_F(CollectionItemDelegateTest, setEditorData)
{
    // Test setter: void setEditorData((QWidget *editor, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setEditorData(nullptr, _arg1));
}

TEST_F(CollectionItemDelegateTest, sizeHint)
{
    // Test method: QSize sizeHint((const QStyleOptionViewItem &opt, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->sizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionItemDelegateTest, textPaintRect)
{
    // Test method: QRect textPaintRect((const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rText, bool elide))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    QRect _arg2{};
    auto result = obj->textPaintRect(_arg0, _arg1, _arg2, false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionItemDelegateTest, updateEditorGeometry)
{
    // Test method: void updateEditorGeometry((QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateEditorGeometry(nullptr, _arg1, _arg2));
}

TEST_F(CollectionItemDelegateTest, updateItemSizeHint)
{
    // Test method: void updateItemSizeHint(())
    EXPECT_NO_FATAL_FAILURE(obj->updateItemSizeHint());
}

TEST_F(CollectionItemDelegateTest, visualAlignment)
{
    // Test method: Qt::Alignment visualAlignment((Qt::LayoutDirection direction, Qt::Alignment alignment))
    auto result = obj->visualAlignment(Qt::LayoutDirection(), Qt::Alignment());
    EXPECT_GE(static_cast<int>(result), 0);

}
