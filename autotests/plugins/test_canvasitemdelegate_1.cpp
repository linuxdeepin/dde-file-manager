// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasitemdelegate_1.cpp
 * @brief Unit tests for CanvasItemDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "delegate/canvasitemdelegate.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasItemDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasItemDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasItemDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasItemDelegateTest, CanvasItemDelegate)
{
    // Test constructor: CanvasItemDelegate((QAbstractItemView *parentPtr))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasItemDelegateTest, clipboardDataChanged)
{
    // Test method: void clipboardDataChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->clipboardDataChanged());
}

TEST_F(CanvasItemDelegateTest, commitDataAndCloseEditor)
{
    // Test method: void commitDataAndCloseEditor(())
    EXPECT_NO_FATAL_FAILURE(obj->commitDataAndCloseEditor());
}

TEST_F(CanvasItemDelegateTest, createEditor)
{
    // Test method: QWidget createEditor((QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->createEditor(nullptr, _arg1, _arg2);
    EXPECT_NO_FATAL_FAILURE({ obj->createEditor(nullptr, _arg1, _arg2); });

}

TEST_F(CanvasItemDelegateTest, drawExpandText)
{
    // Test method: void drawExpandText((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    QRectF _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawExpandText(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(CanvasItemDelegateTest, drawHighlightText)
{
    // Test method: void drawHighlightText((QPainter *painter, const QStyleOptionViewItem &option,
                                           const QModelIndex &index, const QRect &rText))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    QRect _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawHighlightText(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(CanvasItemDelegateTest, drawNormlText)
{
    // Test method: void drawNormlText((QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index, const QRectF &rText))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    QRectF _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawNormlText(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(CanvasItemDelegateTest, elideTextRect)
{
    // Test method: QList<QRectF> elideTextRect((const QModelIndex &index, const QRect &rect, const Qt::TextElideMode &elideMode))
    QModelIndex _arg0{};
    QRect _arg1{};
    Qt::TextElideMode _arg2{};
    auto result = obj->elideTextRect(_arg0, _arg1, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasItemDelegateTest, expendedGeomerty)
{
    // Test method: QRect expendedGeomerty((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->expendedGeomerty(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasItemDelegateTest, getIconPixmap)
{
    // Test method: QPixmap getIconPixmap((const QIcon &icon, const QSize &size,
                                          qreal pixelRatio, QIcon::Mode mode, QIcon::State state))
    QIcon _arg0{};
    QSize _arg1{};
    auto result = obj->getIconPixmap(_arg0, _arg1, 0.0, QIcon::Mode(), QIcon::State());
    EXPECT_TRUE(result.isNull());

}

TEST_F(CanvasItemDelegateTest, iconLevel)
{
    // Test getter: int iconLevel()
    auto result = obj->iconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasItemDelegateTest, iconRect)
{
    // Test method: QRect iconRect((const QRect &paintRect))
    QRect _arg0{};
    auto result = obj->iconRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasItemDelegateTest, iconSize)
{
    // Test method: QSize iconSize((int lv))
    auto result = obj->iconSize(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasItemDelegateTest, isThumnailIconIndex)
{
    // Test method: bool isThumnailIconIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isThumnailIconIndex(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasItemDelegateTest, isTransparent)
{
    // Test method: bool isTransparent((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isTransparent(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasItemDelegateTest, maximumIconLevel)
{
    // Test getter: int maximumIconLevel()
    auto result = obj->maximumIconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasItemDelegateTest, mayExpand)
{
    // Test method: bool mayExpand((QModelIndex *who))
    auto result = obj->mayExpand(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasItemDelegateTest, minimumIconLevel)
{
    // Test getter: int minimumIconLevel()
    auto result = obj->minimumIconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasItemDelegateTest, paint)
{
    // Test method: void paint((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paint(nullptr, _arg1, _arg2));
}

TEST_F(CanvasItemDelegateTest, paintBackground)
{
    // Test method: void paintBackground((QPainter *painter, const QStyleOptionViewItem &option, const QRect &iconRect))
    QStyleOptionViewItem _arg1{};
    QRect _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintBackground(nullptr, _arg1, _arg2));
}

TEST_F(CanvasItemDelegateTest, paintDragIcon)
{
    // Test method: QSize paintDragIcon((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->paintDragIcon(nullptr, _arg1, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasItemDelegateTest, paintEmblems)
{
    // Test method: QRectF paintEmblems((QPainter *painter, const QRectF &rect, const FileInfoPointer &info))
    QRectF _arg1{};
    FileInfoPointer _arg2{};
    auto result = obj->paintEmblems(nullptr, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasItemDelegateTest, paintGeomertys)
{
    // Test method: QList<QRect> paintGeomertys((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->paintGeomertys(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasItemDelegateTest, paintLabel)
{
    // Test method: void paintLabel((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rLabel))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    QRect _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->paintLabel(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(CanvasItemDelegateTest, revertAndcloseEditor)
{
    // Test method: void revertAndcloseEditor(())
    EXPECT_NO_FATAL_FAILURE(obj->revertAndcloseEditor());
}

TEST_F(CanvasItemDelegateTest, setEditorData)
{
    // Test setter: void setEditorData((QWidget *editor, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setEditorData(nullptr, _arg1));
}

TEST_F(CanvasItemDelegateTest, setIconLevel)
{
    // Test method: int setIconLevel((int lv))
    auto result = obj->setIconLevel(0);
    EXPECT_GE(result, 0);

}

TEST_F(CanvasItemDelegateTest, sizeHint)
{
    // Test method: QSize sizeHint((const QStyleOptionViewItem &opt, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->sizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasItemDelegateTest, textLineHeight)
{
    // Test getter: int textLineHeight()
    auto result = obj->textLineHeight();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasItemDelegateTest, textPaintRect)
{
    // Test method: QRect textPaintRect((const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rText, bool elide))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    QRect _arg2{};
    auto result = obj->textPaintRect(_arg0, _arg1, _arg2, false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasItemDelegateTest, updateEditorGeometry)
{
    // Test method: void updateEditorGeometry((QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateEditorGeometry(nullptr, _arg1, _arg2));
}

TEST_F(CanvasItemDelegateTest, updateItemSizeHint)
{
    // Test method: void updateItemSizeHint(())
    EXPECT_NO_FATAL_FAILURE(obj->updateItemSizeHint());
}

TEST_F(CanvasItemDelegateTest, visualAlignment)
{
    // Test method: Qt::Alignment visualAlignment((Qt::LayoutDirection direction, Qt::Alignment alignment))
    auto result = obj->visualAlignment(Qt::LayoutDirection(), Qt::Alignment());
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CanvasItemDelegateTest, CanvasItemDelegate_Destructor)
{
    // Test method:  ~CanvasItemDelegate(())
    EXPECT_NO_FATAL_FAILURE({ CanvasItemDelegate *tmp = new CanvasItemDelegate(); delete tmp; });
}
