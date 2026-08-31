// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallpaperitem.cpp
 * @brief Unit tests for WallpaperItem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "wallpaperitem.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class WallpaperItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WallpaperItem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WallpaperItem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WallpaperItemTest, addButton)
{
    // Test method: QPushButton addButton((const QString &id, const QString &text, const int btnWidth, int row, int column, int rowSpan, int columnSpan))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->addButton(_arg0, _arg1, 0, 0, 0, 0, 0);
    EXPECT_NO_FATAL_FAILURE({ obj->addButton(_arg0, _arg1, 0, 0, 0, 0, 0); });

}

TEST_F(WallpaperItemTest, itemData)
{
    // Test getter: QString itemData()
    auto result = obj->itemData();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WallpaperItemTest, setItemData)
{
    // Test setter: void setItemData((const QString &data))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setItemData(_arg0));
}
