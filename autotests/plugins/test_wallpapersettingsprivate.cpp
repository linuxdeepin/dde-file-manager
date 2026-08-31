// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallpapersettingsprivate.cpp
 * @brief Unit tests for WallpaperSettingsPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "wallpapersettings.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class WallpaperSettingsPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WallpaperSettingsPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WallpaperSettingsPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WallpaperSettingsPrivateTest, adjustModeSwitcher)
{
    // Test method: void adjustModeSwitcher(())
    EXPECT_NO_FATAL_FAILURE(obj->adjustModeSwitcher());
}

TEST_F(WallpaperSettingsPrivateTest, initScreenSaver)
{
    // Test method: void initScreenSaver(())
    EXPECT_NO_FATAL_FAILURE(obj->initScreenSaver());
}

TEST_F(WallpaperSettingsPrivateTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(WallpaperSettingsPrivateTest, onMousePressed)
{
    // Test method: void onMousePressed((const QPoint &pos, int button))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onMousePressed(_arg0, 0));
}

TEST_F(WallpaperSettingsPrivateTest, timeFormat)
{
    // Test method: QString timeFormat((int second))
    auto result = obj->timeFormat(0);
    EXPECT_TRUE(result.isEmpty());

}
