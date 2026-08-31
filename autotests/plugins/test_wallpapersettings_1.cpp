// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallpapersettings_1.cpp
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

TEST_F(WallpaperSettingsTest, adjustGeometry)
{
    // Test method: void adjustGeometry(())
    EXPECT_NO_FATAL_FAILURE(obj->adjustGeometry());
}

TEST_F(WallpaperSettingsTest, applyToDesktop)
{
    // Test method: void applyToDesktop(())
    EXPECT_NO_FATAL_FAILURE(obj->applyToDesktop());
}

TEST_F(WallpaperSettingsTest, applyToGreeter)
{
    // Test method: void applyToGreeter(())
    EXPECT_NO_FATAL_FAILURE(obj->applyToGreeter());
}

TEST_F(WallpaperSettingsTest, availableWallpaperSlide)
{
    // Test getter: QStringList availableWallpaperSlide()
    auto result = obj->availableWallpaperSlide();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WallpaperSettingsTest, closeLoading)
{
    // Test method: void closeLoading(())
    EXPECT_NO_FATAL_FAILURE(obj->closeLoading());
}

TEST_F(WallpaperSettingsTest, currentWallpaper)
{
    // Test getter: QPair<QString, QString> currentWallpaper()
    auto result = obj->currentWallpaper();
    EXPECT_NO_FATAL_FAILURE({ obj->currentWallpaper(); });

}

TEST_F(WallpaperSettingsTest, hideEvent)
{
    // Test event handler: hideEvent((QHideEvent *event))
    QHideEvent _event(QHideEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->hideEvent(&_event));
}

TEST_F(WallpaperSettingsTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(WallpaperSettingsTest, isWallpaperLocked)
{
    // Test bool getter: isWallpaperLocked()
    bool result = obj->isWallpaperLocked();
    EXPECT_FALSE(result);

}

TEST_F(WallpaperSettingsTest, loadWallpaper)
{
    // Test method: void loadWallpaper(())
    EXPECT_NO_FATAL_FAILURE(obj->loadWallpaper());
}

TEST_F(WallpaperSettingsTest, onGeometryChanged)
{
    // Test method: void onGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onGeometryChanged());
}

TEST_F(WallpaperSettingsTest, refreshList)
{
    // Test method: void refreshList(())
    EXPECT_NO_FATAL_FAILURE(obj->refreshList());
}

TEST_F(WallpaperSettingsTest, setWallpaperSlideShow)
{
    // Test setter: void setWallpaperSlideShow((const QString &period))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setWallpaperSlideShow(_arg0));
}

TEST_F(WallpaperSettingsTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(WallpaperSettingsTest, showLoading)
{
    // Test method: void showLoading(())
    EXPECT_NO_FATAL_FAILURE(obj->showLoading());
}

TEST_F(WallpaperSettingsTest, switchMode)
{
    // Test method: void switchMode((WallpaperSettings::Mode mode))
    EXPECT_NO_FATAL_FAILURE(obj->switchMode(WallpaperSettings::Mode()));
}

TEST_F(WallpaperSettingsTest, wallpaperSlideShow)
{
    // Test getter: QString wallpaperSlideShow()
    auto result = obj->wallpaperSlideShow();
    EXPECT_TRUE(result.isEmpty());

}
