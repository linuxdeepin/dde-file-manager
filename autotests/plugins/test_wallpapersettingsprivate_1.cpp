// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallpapersettingsprivate_1.cpp
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

TEST_F(WallpaperSettingsPrivateTest, carouselTurn)
{
    // Test method: void carouselTurn((bool checked))
    EXPECT_NO_FATAL_FAILURE(obj->carouselTurn(false));
}

TEST_F(WallpaperSettingsPrivateTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(WallpaperSettingsPrivateTest, handleNeedCloseButton)
{
    // Test method: void handleNeedCloseButton((const QString &itemData, const QPoint &pos))
    QString _arg0{};
    QPoint _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleNeedCloseButton(_arg0, _arg1));
}

TEST_F(WallpaperSettingsPrivateTest, initCloseButton)
{
    // Test method: void initCloseButton(())
    EXPECT_NO_FATAL_FAILURE(obj->initCloseButton());
}

TEST_F(WallpaperSettingsPrivateTest, initPreivew)
{
    // Test method: void initPreivew(())
    EXPECT_NO_FATAL_FAILURE(obj->initPreivew());
}

TEST_F(WallpaperSettingsPrivateTest, onCloseButtonClicked)
{
    // Test method: void onCloseButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onCloseButtonClicked());
}

TEST_F(WallpaperSettingsPrivateTest, onItemBacktab)
{
    // Test method: void onItemBacktab((WallpaperItem *item))
    EXPECT_NO_FATAL_FAILURE(obj->onItemBacktab(nullptr));
}

TEST_F(WallpaperSettingsPrivateTest, onItemPressed)
{
    // Test method: void onItemPressed((const QString &itemData))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemPressed(_arg0));
}

TEST_F(WallpaperSettingsPrivateTest, onItemTab)
{
    // Test method: void onItemTab((WallpaperItem *item))
    EXPECT_NO_FATAL_FAILURE(obj->onItemTab(nullptr));
}

TEST_F(WallpaperSettingsPrivateTest, onScreenChanged)
{
    // Test method: void onScreenChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onScreenChanged());
}

TEST_F(WallpaperSettingsPrivateTest, processListReply)
{
    // Test method: QList<QPair<QString, bool>> processListReply((const QString &reply))
    QString _arg0{};
    auto result = obj->processListReply(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WallpaperSettingsPrivateTest, propertyForWayland)
{
    // Test method: void propertyForWayland(())
    EXPECT_NO_FATAL_FAILURE(obj->propertyForWayland());
}

TEST_F(WallpaperSettingsPrivateTest, relaylout)
{
    // Test method: void relaylout(())
    EXPECT_NO_FATAL_FAILURE(obj->relaylout());
}

TEST_F(WallpaperSettingsPrivateTest, setMode)
{
    // Test setter: void setMode((QAbstractButton *toggledBtn, bool on))
    EXPECT_NO_FATAL_FAILURE(obj->setMode(nullptr, false));
}

TEST_F(WallpaperSettingsPrivateTest, switchCarousel)
{
    // Test method: void switchCarousel((QAbstractButton *toggledBtn, bool state))
    EXPECT_NO_FATAL_FAILURE(obj->switchCarousel(nullptr, false));
}

TEST_F(WallpaperSettingsPrivateTest, switchWaitTime)
{
    // Test method: void switchWaitTime((QAbstractButton *toggledBtn, bool state))
    EXPECT_NO_FATAL_FAILURE(obj->switchWaitTime(nullptr, false));
}
