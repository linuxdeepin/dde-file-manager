// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionitemdelegate.cpp
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

TEST_F(CollectionItemDelegateTest, boundingRect)
{
    // Test method: QRectF boundingRect((const QList<QRectF> &rects))
    QList<QRectF> _arg0{};
    auto result = obj->boundingRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionItemDelegateTest, getIconPixmap)
{
    // Test method: QPixmap getIconPixmap((const QIcon &icon, const QSize &size,
                                              qreal pixelRatio, QIcon::Mode mode, QIcon::State state))
    QIcon _arg0{};
    QSize _arg1{};
    auto result = obj->getIconPixmap(_arg0, _arg1, 0.0, QIcon::Mode(), QIcon::State());
    EXPECT_TRUE(result.isNull());

}

TEST_F(CollectionItemDelegateTest, iconLevel)
{
    // Test getter: int iconLevel()
    auto result = obj->iconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionItemDelegateTest, iconRect)
{
    // Test method: QRect iconRect((const QRect &paintRect))
    QRect _arg0{};
    auto result = obj->iconRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionItemDelegateTest, iconSize)
{
    // Test method: QSize iconSize((int lv))
    auto result = obj->iconSize(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionItemDelegateTest, initStyleOption)
{
    // Test method: void initStyleOption((QStyleOptionViewItem *option, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->initStyleOption(nullptr, _arg1));
}

TEST_F(CollectionItemDelegateTest, labelRect)
{
    // Test method: QRect labelRect((const QRect &paintRect, const QRect &usedRect))
    QRect _arg0{};
    QRect _arg1{};
    auto result = obj->labelRect(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionItemDelegateTest, maximumIconLevel)
{
    // Test getter: int maximumIconLevel()
    auto result = obj->maximumIconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionItemDelegateTest, minimumIconLevel)
{
    // Test getter: int minimumIconLevel()
    auto result = obj->minimumIconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionItemDelegateTest, setIconLevel)
{
    // Test method: int setIconLevel((int lv))
    auto result = obj->setIconLevel(0);
    EXPECT_GE(result, 0);

}

TEST_F(CollectionItemDelegateTest, setModelData)
{
    // Test setter: void setModelData((QWidget *editor, QAbstractItemModel *model, const QModelIndex &index))
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setModelData(nullptr, nullptr, _arg2));
}

TEST_F(CollectionItemDelegateTest, CollectionItemDelegate)
{
    // Test constructor: CollectionItemDelegate((QAbstractItemView *parentPtr))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionItemDelegateTest, parent)
{
    // Test getter: CollectionView parent()
    auto result = obj->parent();
    EXPECT_NO_FATAL_FAILURE({ obj->parent(); });

}
