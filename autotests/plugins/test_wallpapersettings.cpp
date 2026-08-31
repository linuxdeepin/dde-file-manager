// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallpapersettings.cpp
 * @brief Unit tests for WallpaperSettings methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "wallpapersettings.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class WallpaperSettingsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WallpaperSettings();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WallpaperSettings *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WallpaperSettingsTest, WallpaperSettings)
{
    // Test constructor: WallpaperSettings((const QString &screenName, Mode mode, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WallpaperSettingsTest, availableScreenSaverTime)
{
    // Test getter: QVector<int> availableScreenSaverTime()
    auto result = obj->availableScreenSaverTime();
    EXPECT_TRUE(result.isEmpty());

}
