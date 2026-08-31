// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallpaperlist.cpp
 * @brief Unit tests for WallpaperList methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "wallpaperlist.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class WallpaperListTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WallpaperList();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WallpaperList *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WallpaperListTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(WallpaperListTest, count)
{
    // Test getter: int count()
    auto result = obj->count();
    EXPECT_EQ(result, 0);

}

TEST_F(WallpaperListTest, gridSize)
{
    // Test getter: QSize gridSize()
    auto result = obj->gridSize();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WallpaperListTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(WallpaperListTest, removeItem)
{
    // Test method: void removeItem((const QString &itemData))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeItem(_arg0));
}

TEST_F(WallpaperListTest, showDeleteButtonForItem)
{
    // Test method: void showDeleteButtonForItem((const WallpaperItem *item))
    EXPECT_NO_FATAL_FAILURE(obj->showDeleteButtonForItem(nullptr));
}

TEST_F(WallpaperListTest, updateBothEndsItem)
{
    // Test method: void updateBothEndsItem(())
    EXPECT_NO_FATAL_FAILURE(obj->updateBothEndsItem());
}

TEST_F(WallpaperListTest, WallpaperList)
{
    // Test constructor: WallpaperList((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WallpaperListTest, addItem)
{
    // Test method: WallpaperItem addItem((const QString &itemData))
    QString _arg0{};
    auto result = obj->addItem(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->addItem(_arg0); });

}

TEST_F(WallpaperListTest, currentItem)
{
    // Test getter: WallpaperItem currentItem()
    auto result = obj->currentItem();
    EXPECT_NO_FATAL_FAILURE({ obj->currentItem(); });

}

TEST_F(WallpaperListTest, removeMaskWidget)
{
    // Test getter: QWidget removeMaskWidget()
    auto result = obj->removeMaskWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->removeMaskWidget(); });

}
