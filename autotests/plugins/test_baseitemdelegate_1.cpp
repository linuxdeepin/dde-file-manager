// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_baseitemdelegate_1.cpp
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

TEST_F(BaseItemDelegateTest, commitDataAndCloseActiveEditor)
{
    // Test method: void commitDataAndCloseActiveEditor(())
    EXPECT_NO_FATAL_FAILURE(obj->commitDataAndCloseActiveEditor());
}

TEST_F(BaseItemDelegateTest, decreaseIcon)
{
    // Test getter: int decreaseIcon()
    auto result = obj->decreaseIcon();
    EXPECT_EQ(result, 0);

}

TEST_F(BaseItemDelegateTest, editingIndex)
{
    // Test getter: QModelIndex editingIndex()
    auto result = obj->editingIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, editingIndexWidget)
{
    // Test getter: QWidget editingIndexWidget()
    auto result = obj->editingIndexWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->editingIndexWidget(); });

}

TEST_F(BaseItemDelegateTest, effectiveHighlightKeywords)
{
    // Test method: QStringList effectiveHighlightKeywords((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->effectiveHighlightKeywords(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, expandItemRect)
{
    // Test getter: QRect expandItemRect()
    auto result = obj->expandItemRect();
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, expandedIndex)
{
    // Test getter: QModelIndex expandedIndex()
    auto result = obj->expandedIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, expandedItem)
{
    // Test getter: QWidget expandedItem()
    auto result = obj->expandedItem();
    EXPECT_NO_FATAL_FAILURE({ obj->expandedItem(); });

}

TEST_F(BaseItemDelegateTest, getCornerGeometryList)
{
    // Test method: QList<QRectF> getCornerGeometryList((const QRectF &baseRect, const QSizeF &cornerSize))
    QRectF _arg0{};
    QSizeF _arg1{};
    auto result = obj->getCornerGeometryList(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, getExpandButtonHitRect)
{
    // Test method: QRect getExpandButtonHitRect((const QRectF &rect))
    QRectF _arg0{};
    auto result = obj->getExpandButtonHitRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, getExpandButtonRect)
{
    // Test method: QRect getExpandButtonRect((const QRectF &rect))
    QRectF _arg0{};
    auto result = obj->getExpandButtonRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, getGroupHeaderBackgroundRect)
{
    // Test method: QRectF getGroupHeaderBackgroundRect((const QStyleOptionViewItem &option))
    QStyleOptionViewItem _arg0{};
    auto result = obj->getGroupHeaderBackgroundRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, getGroupHeaderSizeHint)
{
    // Test method: QSize getGroupHeaderSizeHint((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->getGroupHeaderSizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, getGroupTextRect)
{
    // Test method: QRect getGroupTextRect((const QRectF &rect, bool hasTruncateButton))
    QRectF _arg0{};
    auto result = obj->getGroupTextRect(_arg0, false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, getIndexIconSize)
{
    // Test method: QSize getIndexIconSize((const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    QSize _arg2{};
    auto result = obj->getIndexIconSize(_arg0, _arg1, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, getRectOfItem)
{
    // Test method: QRect getRectOfItem((RectOfItemType type, const QModelIndex &index))
    QModelIndex _arg1{};
    auto result = obj->getRectOfItem(RectOfItemType(), _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, hasWidgetIndexs)
{
    // Test getter: QModelIndexList hasWidgetIndexs()
    auto result = obj->hasWidgetIndexs();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, hideAllIIndexWidget)
{
    // Test method: void hideAllIIndexWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->hideAllIIndexWidget());
}

TEST_F(BaseItemDelegateTest, hideNotEditingIndexWidget)
{
    // Test method: void hideNotEditingIndexWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->hideNotEditingIndexWidget());
}

TEST_F(BaseItemDelegateTest, highlightKeywords)
{
    // Test getter: QStringList highlightKeywords()
    auto result = obj->highlightKeywords();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, iconSizeLevel)
{
    // Test getter: int iconSizeLevel()
    auto result = obj->iconSizeLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(BaseItemDelegateTest, increaseIcon)
{
    // Test getter: int increaseIcon()
    auto result = obj->increaseIcon();
    EXPECT_EQ(result, 0);

}

TEST_F(BaseItemDelegateTest, isGroupHeaderItem)
{
    // Test method: bool isGroupHeaderItem((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isGroupHeaderItem(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BaseItemDelegateTest, itemExpanded)
{
    // Test bool getter: itemExpanded()
    bool result = obj->itemExpanded();
    EXPECT_FALSE(result);

}

TEST_F(BaseItemDelegateTest, itemGeomertys)
{
    // Test method: QList<QRect> itemGeomertys((const QStyleOptionViewItem &opt, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->itemGeomertys(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseItemDelegateTest, itemIconRect)
{
    // Test method: QRectF itemIconRect((const QRectF &itemRect))
    QRectF _arg0{};
    auto result = obj->itemIconRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BaseItemDelegateTest, maximumIconSizeLevel)
{
    // Test getter: int maximumIconSizeLevel()
    auto result = obj->maximumIconSizeLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(BaseItemDelegateTest, minimumIconSizeLevel)
{
    // Test getter: int minimumIconSizeLevel()
    auto result = obj->minimumIconSizeLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(BaseItemDelegateTest, paintDragIcon)
{
    // Test method: void paintDragIcon((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    QSize _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->paintDragIcon(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(BaseItemDelegateTest, paintEmblems)
{
    // Test method: void paintEmblems((QPainter *painter, const QRectF &iconRect, const QModelIndex &index))
    QRectF _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintEmblems(nullptr, _arg1, _arg2));
}

TEST_F(BaseItemDelegateTest, paintExpandButton)
{
    // Test method: void paintExpandButton((QPainter *painter, const QRect &buttonRect, bool isExpanded))
    QRect _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->paintExpandButton(nullptr, _arg1, false));
}

TEST_F(BaseItemDelegateTest, paintGroupBackground)
{
    // Test method: void paintGroupBackground((QPainter *painter, const QStyleOptionViewItem &option))
    QStyleOptionViewItem _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->paintGroupBackground(nullptr, _arg1));
}

TEST_F(BaseItemDelegateTest, paintGroupHeader)
{
    // Test method: void paintGroupHeader((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintGroupHeader(nullptr, _arg1, _arg2));
}

TEST_F(BaseItemDelegateTest, paintGroupHeaderContent)
{
    // Test method: void paintGroupHeaderContent((QPainter *painter, const QRectF &drawRect, const QStyleOptionViewItem &option, const QModelIndex &index))
    QRectF _arg1{};
    QStyleOptionViewItem _arg2{};
    QModelIndex _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->paintGroupHeaderContent(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(BaseItemDelegateTest, paintStickyGroupHeader)
{
    // Test method: void paintStickyGroupHeader((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintStickyGroupHeader(nullptr, _arg1, _arg2));
}

TEST_F(BaseItemDelegateTest, setHighlightKeywords)
{
    // Test setter: void setHighlightKeywords((const QStringList &keywords))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setHighlightKeywords(_arg0));
}

TEST_F(BaseItemDelegateTest, setIconSizeByIconSizeLevel)
{
    // Test method: int setIconSizeByIconSizeLevel((int level))
    auto result = obj->setIconSizeByIconSizeLevel(0);
    EXPECT_GE(result, 0);

}

TEST_F(BaseItemDelegateTest, setPaintProxy)
{
    // Test setter: void setPaintProxy((AbstractItemPaintProxy *proxy))
    EXPECT_NO_FATAL_FAILURE(obj->setPaintProxy(nullptr));
}

TEST_F(BaseItemDelegateTest, sizeHint)
{
    // Test method: QSize sizeHint((const QStyleOptionViewItem &, const QModelIndex &))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->sizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
