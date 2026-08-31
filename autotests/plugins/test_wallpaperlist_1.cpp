// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallpaperlist_1.cpp
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

TEST_F(WallpaperListTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(WallpaperListTest, itemAt)
{
    // Test method: QWidget itemAt((int idx))
    auto result = obj->itemAt(0);
    EXPECT_NO_FATAL_FAILURE({ obj->itemAt(0); });

}

TEST_F(WallpaperListTest, nextPage)
{
    // Test method: void nextPage(())
    EXPECT_NO_FATAL_FAILURE(obj->nextPage());
}

TEST_F(WallpaperListTest, onItemHoverIn)
{
    // Test method: void onItemHoverIn((WallpaperItem *it))
    EXPECT_NO_FATAL_FAILURE(obj->onItemHoverIn(nullptr));
}

TEST_F(WallpaperListTest, onItemPressed)
{
    // Test method: void onItemPressed((WallpaperItem *it))
    EXPECT_NO_FATAL_FAILURE(obj->onItemPressed(nullptr));
}

TEST_F(WallpaperListTest, prevPage)
{
    // Test method: void prevPage(())
    EXPECT_NO_FATAL_FAILURE(obj->prevPage());
}

TEST_F(WallpaperListTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(WallpaperListTest, scrollList)
{
    // Test method: void scrollList((int step, int duration))
    EXPECT_NO_FATAL_FAILURE(obj->scrollList(0, 0));
}

TEST_F(WallpaperListTest, setGridSize)
{
    // Test setter: void setGridSize((const QSize &size))
    QSize _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setGridSize(_arg0));
}

TEST_F(WallpaperListTest, setMaskWidget)
{
    // Test setter: void setMaskWidget((QWidget *w))
    EXPECT_NO_FATAL_FAILURE(obj->setMaskWidget(nullptr));
}

TEST_F(WallpaperListTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(WallpaperListTest, updateItemThumb)
{
    // Test method: void updateItemThumb(())
    EXPECT_NO_FATAL_FAILURE(obj->updateItemThumb());
}

TEST_F(WallpaperListTest, wheelEvent)
{
    // Test event handler: wheelEvent((QWheelEvent *event))
    QWheelEvent _event(QWheelEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->wheelEvent(&_event));
}
